// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "pva_kmd_dma_cfg.h"
#include "pva_utils.h"
#include "pva_kmd_resource_table.h"
#include "pva_kmd_device.h"
#include "pva_kmd_hwseq_validate.h"

void pva_kmd_unload_dma_config_unsafe(struct pva_kmd_dma_resource_aux *dma_aux)
{
	uint32_t i;

	for (i = 0U; i < dma_aux->dram_res_count; i++) {
		pva_kmd_drop_resource_unsafe(dma_aux->res_table,
					     dma_aux->static_dram_res_ids[i]);
	}

	if (dma_aux->vpu_bin_res_id != PVA_RESOURCE_ID_INVALID) {
		pva_kmd_drop_resource_unsafe(dma_aux->res_table,
					     dma_aux->vpu_bin_res_id);
	}
}

/**
 * @brief Mask of the hardware sequencer frame count (CHHWSEQFCNT)
 *
 * @details The register field is 6 bits wide and holds the frame count minus
 * one, so a sequence has at most 64 frames.
 */
#define PVA_KMD_HWSEQ_FRAME_COUNT_MASK 0x3FU

/**
 * @brief Maximum number of frames in a hardware sequencer program
 */
#define PVA_KMD_HWSEQ_MAX_FRAMES (PVA_KMD_HWSEQ_FRAME_COUNT_MASK + 1U)

/**
 * @brief Clamp a descriptor index under speculative execution
 *
 * @param[in] desc_index  Descriptor index relative to base_descriptor
 * @param[in] num_descs   Number of descriptors in the configuration
 *
 * @return @p desc_index if it is below @p num_descs, otherwise 0
 */
static uint32_t clamp_desc_index(uint32_t desc_index, uint32_t num_descs)
{
	return (uint32_t)array_index_nospec(desc_index, num_descs);
}

/**
 * @brief Convert a 1-based descriptor ID to a configuration descriptor index
 *
 * @details Descriptor ID 0 means no descriptor. Otherwise the descriptor
 * index is desc_id - 1, relative to the configuration's base_descriptor.
 *
 * @param[in] dma_config   DMA configuration
 * @param[in] desc_id      1-based descriptor ID
 * @param[out] desc_index  Descriptor index relative to base_descriptor
 *
 * @retval true   @p desc_id refers to a descriptor of this configuration
 * @retval false  @p desc_id is 0 or outside the configuration
 */
static bool desc_id_to_index(struct pva_dma_config const *dma_config,
			     uint8_t desc_id, uint32_t *desc_index)
{
	uint32_t base = dma_config->header.base_descriptor;
	uint32_t num_descs = dma_config->header.num_descriptors;

	if ((desc_id == 0U) || ((uint32_t)desc_id <= base) ||
	    (((uint32_t)desc_id - base) > num_descs)) {
		return false;
	}
	*desc_index =
		clamp_desc_index((uint32_t)desc_id - 1U - base, num_descs);
	return true;
}

/**
 * @brief Record that a channel uses a descriptor
 *
 * @details A descriptor may be used by only one channel. Assigns the
 * descriptor to @p ch_index if it has no owner yet and fails if another
 * channel already owns it.
 *
 * @param[in] desc_index  Descriptor index relative to base_descriptor
 * @param[in] ch_index    Channel using the descriptor
 * @param[in, out] desc_to_ch  Descriptor index to channel index map
 *
 * @retval PVA_SUCCESS             Descriptor is owned by @p ch_index
 * @retval PVA_INVALID_DMA_CONFIG  Descriptor is owned by another channel
 */
static enum pva_error claim_desc(uint32_t desc_index, uint8_t ch_index,
				 uint8_t *desc_to_ch)
{
	if (desc_to_ch[desc_index] == PVA_KMD_INVALID_CH_IDX) {
		desc_to_ch[desc_index] = ch_index;
	} else if (desc_to_ch[desc_index] != ch_index) {
		pva_kmd_log_err_u64("DMA descriptor used by multiple channels",
				    (uint64_t)desc_index);
		return PVA_INVALID_DMA_CONFIG;
	} else {
		/* Already owned by this channel */
	}
	return PVA_SUCCESS;
}

/**
 * @brief Record the descriptors on a SW-sequenced channel's link chain
 *
 * @details Follows the chain from the channel's start descriptor through
 * link_desc_id until a descriptor ID of 0 (end of chain), a descriptor
 * already visited by this channel (loop), or PVA_MAX_NUM_DMA_DESC steps.
 *
 * @param[in] dma_config  DMA configuration
 * @param[in] ch_index    Channel index
 * @param[in, out] desc_to_ch  Descriptor index to channel index map
 *
 * @retval PVA_SUCCESS             Chain recorded
 * @retval PVA_INVALID_DMA_CONFIG  A chain descriptor is owned by another
 *                                 channel
 */
static enum pva_error trace_chain(struct pva_dma_config const *dma_config,
				  uint8_t ch_index, uint8_t *desc_to_ch)
{
	uint32_t desc_index =
		clamp_desc_index(dma_config->channels[ch_index].desc_index,
				 dma_config->header.num_descriptors);
	bool more = true;
	enum pva_error err = PVA_SUCCESS;

	for (uint32_t i = 0U; (i < PVA_MAX_NUM_DMA_DESC) && more; i++) {
		if (desc_to_ch[desc_index] == ch_index) {
			/* Chain loops back to a descriptor of this channel */
			more = false;
		} else {
			err = claim_desc(desc_index, ch_index, desc_to_ch);
			/* Descriptor ID 0 ends the chain */
			more = (err == PVA_SUCCESS) &&
			       desc_id_to_index(
				       dma_config,
				       dma_config->descriptors[desc_index]
					       .link_desc_id,
				       &desc_index);
		}
	}
	return err;
}

/**
 * @brief Take the next bytes of a hardware sequencer program
 *
 * @param[in, out] buf  Cursor over the remaining program bytes
 * @param[in] num_bytes Number of bytes to take
 *
 * @retval non-null  Pointer to the taken bytes
 * @retval NULL      Fewer than @p num_bytes bytes remain
 */
static const void *take_hwseq_bytes(struct pva_hwseq_buffer *buf,
				    uint32_t num_bytes)
{
	const uint8_t *ret = NULL;

	if (num_bytes <= buf->bytes_left) {
		ret = buf->data;
		buf->data += num_bytes;
		buf->bytes_left -= num_bytes;
	}
	return (const void *)ret;
}

/**
 * @brief Record the descriptor entries of one hardware sequencer column/row
 *
 * @details Reads @p num_entries descriptor entries and the padding entry
 * that follows an odd count, claiming each descriptor for @p ch_index.
 * Descriptor IDs outside the configuration are skipped.
 *
 * @param[in] dma_config   DMA configuration
 * @param[in, out] buf     Cursor over the remaining program bytes
 * @param[in] num_entries  Number of descriptor entries in the column/row
 * @param[in] ch_index     Channel whose program is walked
 * @param[in, out] desc_to_ch  Descriptor index to channel index map
 * @param[out] end         Set when the program ends before all entries
 *
 * @retval PVA_SUCCESS             Entries recorded
 * @retval PVA_INVALID_DMA_CONFIG  A descriptor is owned by another channel
 */
static enum pva_error
trace_hwseq_entries(struct pva_dma_config const *dma_config,
		    struct pva_hwseq_buffer *buf, uint32_t num_entries,
		    uint8_t ch_index, uint8_t *desc_to_ch, bool *end)
{
	const uint32_t entry_size =
		(uint32_t)sizeof(struct pva_dma_hwseq_desc_entry);
	enum pva_error err = PVA_SUCCESS;
	bool stop = false;

	for (uint32_t j = 0U; (j < num_entries) && !stop; j++) {
		const struct pva_dma_hwseq_desc_entry *entry =
			take_hwseq_bytes(buf, entry_size);
		uint32_t desc_index = 0U;

		if (entry == NULL) {
			*end = true;
			stop = true;
		} else if (desc_id_to_index(dma_config, entry->did,
					    &desc_index)) {
			err = claim_desc(desc_index, ch_index, desc_to_ch);
			if (err != PVA_SUCCESS) {
				stop = true;
			}
		} else {
			/* Not a descriptor of this configuration */
		}
	}
	if ((num_entries % 2U) != 0U) {
		(void)take_hwseq_bytes(buf, entry_size);
	}
	return err;
}

/**
 * @brief Take a hardware sequencer frame header with a known frame ID
 *
 * @param[in, out] buf  Cursor over the remaining program bytes
 * @param[out] rra      Set if the frame uses RRA addressing
 *
 * @retval non-null  Frame header for frame, descriptor or RRA addressing
 * @retval NULL      End of the program or an unknown frame ID
 */
static const struct pva_dma_hwseq_hdr *
take_hwseq_frame_header(struct pva_hwseq_buffer *buf, bool *rra)
{
	const struct pva_dma_hwseq_hdr *hdr = take_hwseq_bytes(
		buf, (uint32_t)sizeof(struct pva_dma_hwseq_hdr));

	if (hdr != NULL) {
		*rra = (hdr->fid == (uint16_t)PVA_DMA_HWSEQ_RRA_MODE);
		if (!*rra && (hdr->fid != (uint16_t)PVA_DMA_HWSEQ_FRAME_MODE) &&
		    (hdr->fid != (uint16_t)PVA_DMA_HWSEQ_DESC_MODE)) {
			hdr = NULL;
		}
	}
	return hdr;
}

/**
 * @brief Record the descriptors of one hardware sequencer frame
 *
 * @details A frame is a @ref pva_dma_hwseq_hdr followed by nocr + 1
 * column/row entries. In frame and descriptor addressing modes each entry is
 * a column/row header followed by dec + 1 descriptor entries; in RRA mode
 * each entry is an 8-byte column whose first descriptor entry is used.
 *
 * @param[in] dma_config   DMA configuration
 * @param[in, out] buf     Cursor over the remaining program bytes
 * @param[in] ch_index     Channel whose program is walked
 * @param[in, out] desc_to_ch  Descriptor index to channel index map
 * @param[out] end         Set at the end of the program or an unknown
 *                         frame ID
 *
 * @retval PVA_SUCCESS             Frame recorded
 * @retval PVA_INVALID_DMA_CONFIG  A descriptor is owned by another channel
 */
static enum pva_error trace_hwseq_frame(struct pva_dma_config const *dma_config,
					struct pva_hwseq_buffer *buf,
					uint8_t ch_index, uint8_t *desc_to_ch,
					bool *end)
{
	bool rra = false;
	const struct pva_dma_hwseq_hdr *hdr =
		take_hwseq_frame_header(buf, &rra);
	enum pva_error err = PVA_SUCCESS;
	bool stop = false;

	if (hdr == NULL) {
		*end = true;
		return PVA_SUCCESS;
	}
	for (uint32_t cr = 0U; (cr <= (uint32_t)hdr->nocr) && !stop; cr++) {
		const struct pva_dma_hwseq_colrow_hdr *colrow = take_hwseq_bytes(
			buf, (uint32_t)sizeof(struct pva_dma_hwseq_colrow_hdr));

		if (colrow == NULL) {
			*end = true;
			stop = true;
		} else {
			/* An RRA column holds one descriptor */
			uint32_t num_entries =
				rra ? 1U : ((uint32_t)colrow->dec + 1U);

			err = trace_hwseq_entries(dma_config, buf, num_entries,
						  ch_index, desc_to_ch, end);
			if ((err != PVA_SUCCESS) || *end) {
				stop = true;
			}
		}
	}
	return err;
}

/**
 * @brief Locate a channel's hardware sequencer program
 *
 * @details The program occupies words hwseq_start..hwseq_end of the
 * configuration's hardware sequencer words and runs as many frames as
 * CHHWSEQFCNT gives (the 6-bit hwseq_frame_count plus one) on GEN3, one frame
 * otherwise. Every bound is checked against a compile-time limit as well as
 * the configuration.
 *
 * @param[in] dma_config   DMA configuration
 * @param[in] ch           Channel with hwseq_enable set
 * @param[in] hw_gen       PVA hardware generation
 * @param[out] buf         Cursor over the program bytes
 * @param[out] num_frames  Number of frames in the program
 *
 * @retval true   The program lies within the configuration
 * @retval false  The program range or frame count is out of bounds
 */
static bool get_hwseq_program(struct pva_dma_config const *dma_config,
			      const struct pva_dma_channel *ch,
			      enum pva_hw_gen hw_gen,
			      struct pva_hwseq_buffer *buf,
			      uint32_t *num_frames)
{
	uint32_t start = ch->hwseq_start;
	uint32_t end_word = ch->hwseq_end;

	*num_frames = 1U;
	if (hw_gen == PVA_HW_GEN3) {
		*num_frames = ((uint32_t)ch->hwseq_frame_count &
			       PVA_KMD_HWSEQ_FRAME_COUNT_MASK) +
			      1U;
	}
	if ((end_word < start) || (end_word >= PVA_MAX_NUM_HWSEQ_WORDS) ||
	    (end_word >= (uint32_t)dma_config->header.num_hwseq_words) ||
	    (*num_frames > PVA_KMD_HWSEQ_MAX_FRAMES)) {
		return false;
	}
	buf->data = (const uint8_t *)dma_config->hwseq_words + (start << 2U);
	buf->bytes_left = (end_word + 1U - start) << 2U;
	return true;
}

/**
 * @brief Record descriptors selected by a channel's hardware sequencer
 *
 * @details Walks the channel's hardware sequencer program
 * (@ref get_hwseq_program()) frame by frame via @ref trace_hwseq_frame().
 * Walking stops at the end of the program or at an
 * unknown frame ID. Programs are bounds-checked here because they are not
 * validated for privileged configurations.
 *
 * @param[in] dma_config  DMA configuration
 * @param[in] hw_gen      PVA hardware generation
 * @param[in] ch_index    Index of a channel with hwseq_enable set
 * @param[in, out] desc_to_ch  Descriptor index to channel index map
 *
 * @retval PVA_SUCCESS             Program walked
 * @retval PVA_INVALID_DMA_CONFIG  A selected descriptor is owned by another
 *                                 channel
 */
static enum pva_error
trace_hwseq_channel(struct pva_dma_config const *dma_config,
		    enum pva_hw_gen hw_gen, uint8_t ch_index,
		    uint8_t *desc_to_ch)
{
	struct pva_hwseq_buffer buf = { 0 };
	uint32_t num_frames = 0U;
	enum pva_error err = PVA_SUCCESS;
	bool end = false;
	bool stop = false;

	if (!get_hwseq_program(dma_config, &dma_config->channels[ch_index],
			       hw_gen, &buf, &num_frames)) {
		return PVA_SUCCESS;
	}

	for (uint32_t f = 0U; (f < num_frames) && !stop; f++) {
		err = trace_hwseq_frame(dma_config, &buf, ch_index, desc_to_ch,
					&end);
		if ((err != PVA_SUCCESS) || end) {
			stop = true;
		}
	}
	return err;
}

enum pva_error
pva_kmd_trace_dma_channels(struct pva_dma_config const *dma_config,
			   enum pva_hw_gen hw_gen, uint8_t *desc_to_ch)
{
	uint32_t num_channels = dma_config->header.num_channels;
	enum pva_error err = PVA_SUCCESS;
	bool stop = false;

	/* desc_to_ch holds PVA_MAX_NUM_DMA_DESC entries */
	if (dma_config->header.num_descriptors > PVA_MAX_NUM_DMA_DESC) {
		return PVA_INVALID_DMA_CONFIG;
	}

	for (uint32_t ch_index = 0U; (ch_index < num_channels) && !stop;
	     ch_index++) {
		if (dma_config->channels[ch_index].hwseq_enable == 1U) {
			err = trace_hwseq_channel(dma_config, hw_gen,
						  (uint8_t)ch_index,
						  desc_to_ch);
		} else {
			err = trace_chain(dma_config, (uint8_t)ch_index,
					  desc_to_ch);
		}
		if (err != PVA_SUCCESS) {
			stop = true;
		}
	}
	return err;
}

enum pva_error pva_kmd_load_dma_config(
	struct pva_kmd_resource_table *resource_table,
	const struct pva_ops_dma_config_register *dma_cfg_hdr,
	uint32_t dma_config_size, struct pva_kmd_dma_resource_aux *dma_aux,
	void *fw_dma_cfg, uint32_t *out_fw_fetch_size, bool skip_validation)
{
	enum pva_error err = PVA_SUCCESS;
	uint32_t fw_fetch_size;
	struct pva_dma_config dma_config;
	struct pva_fw_dma_slot *dyn_slots;
	struct pva_fw_dma_reloc *dyn_relocs;
	struct pva_kmd_dma_scratch_buffer *scratch_buf;
	// Mapping descriptor index to channel index
	uint8_t desc_to_ch[PVA_MAX_NUM_DMA_DESC];

	scratch_buf = pva_kmd_zalloc(sizeof(*scratch_buf));
	if (scratch_buf == NULL) {
		err = PVA_NOMEM;
		goto err_out;
	}

	for (uint32_t i = 0U; i < PVA_MAX_NUM_DMA_DESC; i++) {
		desc_to_ch[i] = PVA_KMD_INVALID_CH_IDX;
	}

	err = pva_kmd_parse_dma_config(dma_cfg_hdr, dma_config_size,
				       &dma_config,
				       &resource_table->pva->hw_consts,
				       skip_validation);
	if (err != PVA_SUCCESS) {
		goto free_scratch_buf;
	}

	if (!skip_validation) { // Skip validation for PFSD and test mode
		err = pva_kmd_validate_dma_config(
			&dma_config, &resource_table->pva->hw_consts,
			scratch_buf->access_sizes,
			scratch_buf->hw_dma_descs_mask);
		if (err != PVA_SUCCESS) {
			goto free_scratch_buf;
		}
	}

	err = pva_kmd_trace_dma_channels(
		&dma_config, resource_table->pva->hw_consts.hw_gen, desc_to_ch);
	if (err != PVA_SUCCESS) {
		goto free_scratch_buf;
	}

	err = pva_kmd_compute_dma_access(&dma_config, scratch_buf->access_sizes,
					 scratch_buf->hw_dma_descs_mask);
	if (err != PVA_SUCCESS) {
		goto free_scratch_buf;
	}

	dyn_slots = pva_offset_pointer(fw_dma_cfg,
				       sizeof(struct pva_dma_config_resource));

	dyn_relocs = pva_offset_pointer(dyn_slots,
					dma_config.header.num_dynamic_slots *
						sizeof(*dyn_slots));

	pva_kmd_collect_relocs(&dma_config, scratch_buf->access_sizes,
			       scratch_buf->static_slots,
			       dma_config.header.num_static_slots,
			       scratch_buf->static_relocs, dyn_slots,
			       dma_config.header.num_dynamic_slots, dyn_relocs,
			       desc_to_ch);

	pva_kmd_write_fw_dma_config(
		&dma_config, fw_dma_cfg, &fw_fetch_size,
		resource_table->pva->support_hwseq_frame_linking);

	err = pva_kmd_dma_use_resources(&dma_config, dma_aux);
	if (err != PVA_SUCCESS) {
		goto free_scratch_buf;
	}

	err = pva_kmd_bind_static_buffers(
		(struct pva_dma_config_resource *)fw_dma_cfg, dma_aux,
		scratch_buf->static_slots, dma_config.header.num_static_slots,
		scratch_buf->static_relocs, dma_config.static_bindings,
		dma_config.header.num_static_slots);
	if (err != PVA_SUCCESS) {
		goto drop_res;
	}

	*out_fw_fetch_size = fw_fetch_size;

	pva_kmd_free(scratch_buf);
	return PVA_SUCCESS;
drop_res:
	pva_kmd_unload_dma_config_unsafe(dma_aux);
free_scratch_buf:
	pva_kmd_free(scratch_buf);
err_out:
	return err;
}
