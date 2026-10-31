/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#include <nvgpu/channel.h>
#include <nvgpu/engines.h>
#include <nvgpu/runlist.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/tsg.h>
#include <nvgpu/tsg_subctx.h>
#include <nvgpu/dma.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include "hal/fifo/tsg_gv11b.h"


/* can be removed after runque support is added */
#define GR_RUNQUE			0U	/* pbdma 0 */
#define ASYNC_CE_RUNQUE			2U	/* pbdma 2 */
#define NVENC_RUNQUE			3U	/* pbdma 3 */

/*
 * TSG enable sequence applicable for Volta and onwards
 * Must always be called with tsg->ch_list_lock held.
 */
void gv11b_tsg_enable(struct nvgpu_tsg *tsg)
{
	struct gk20a *g = tsg->g;
	struct nvgpu_channel *ch;
	struct nvgpu_channel *last_ch = NULL;

	nvgpu_log_fn(g, " ");

	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_atomic_read(&ch->bound) == 1) {
			nvgpu_channel_enable(ch);
			last_ch = ch;
		} else {
			nvgpu_log_info(g, "tsg: %u, skipped enabling unbound channel: %u",
				       tsg->tsgid, ch->chid);
		}
	}
	if (last_ch != NULL) {
		g->ops.usermode.ring_doorbell(g, last_ch->runlist->id, last_ch->chid);
	} else {
		nvgpu_log_info(g, "tsg: %u, doorbell was not rung for any channel",
			       tsg->tsgid);
	}

	nvgpu_log_fn(g, "done");
}

/*
 * TSG disable sequence.
 * Must always be called with tsg->ch_list_lock held.
 */
void gv11b_tsg_disable(struct nvgpu_tsg *tsg)
{
	struct nvgpu_channel *ch;

	nvgpu_log_fn(tsg->g, " ");
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		nvgpu_channel_disable(ch);
	}
	nvgpu_log_fn(tsg->g, "done");
}

void gv11b_tsg_unbind_channel_check_eng_faulted(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch,
		struct nvgpu_channel_hw_state *hw_state)
{
	struct gk20a *g = tsg->g;
	struct nvgpu_mem *mem;

	/*
	 * If channel has FAULTED set, clear the CE method buffer
	 * if saved out channel is same as faulted channel
	 */
	if (!hw_state->eng_faulted || (tsg->eng_method_buffers == NULL)) {
		return;
	}

	/*
	 * CE method buffer format :
	 * DWord0 = method count
	 * DWord1 = channel id
	 *
	 * It is sufficient to write 0 to method count to invalidate
	 */
	mem = &tsg->eng_method_buffers[ASYNC_CE_RUNQUE];
	if (ch->chid == nvgpu_mem_rd32(g, mem, 1)) {
		nvgpu_mem_wr32(g, mem, 0, 0);
	}
}

void gv11b_tsg_bind_channel_eng_method_buffers(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch)
{
	int err;
	struct gk20a *g = tsg->g;
	u64 gpu_va;
	u32 ce_runlist_id;

	nvgpu_assert(tsg->runlist != NULL);

	if (tsg->eng_method_buffers == NULL) {
		nvgpu_log_info(g, "eng method buffer NULL");
		return;
	}

	err = nvgpu_engine_get_fast_ce_runlist_id(g,
			tsg->gpu_instance_id,
			&ce_runlist_id);
	if (err != 0) {
		nvgpu_err(g, "failed to get fast ce runlist id");
		return;
	}

	if (tsg->runlist->id == ce_runlist_id) {
		gpu_va = tsg->eng_method_buffers[ASYNC_CE_RUNQUE].gpu_va;
	} else if (tsg->runlist->id == nvgpu_engine_get_nvenc_runlist_id(g, 0)) {
		gpu_va = tsg->eng_method_buffers[NVENC_RUNQUE].gpu_va;
	} else {
		gpu_va = tsg->eng_method_buffers[GR_RUNQUE].gpu_va;
	}

	g->ops.ramin.set_eng_method_buffer(g, &ch->inst_block, gpu_va);
}

int gv11b_tsg_init_eng_method_buffers(struct gk20a *g, struct nvgpu_tsg *tsg)
{
	struct vm_gk20a *vm = g->mm.bar2.vm;
	int err = 0;
	int i;
	unsigned int runque, buffer_size;
	unsigned int num_pbdma = nvgpu_get_litter_value(g, GPU_LIT_HOST_NUM_PBDMA);

	if (tsg->eng_method_buffers != NULL) {
		nvgpu_warn(g, "eng method buffers already allocated");
		return 0;
	}

	buffer_size =  nvgpu_safe_add_u32(nvgpu_safe_mult_u32((9U + 1U + 3U),
				g->ops.ce.get_num_pce(g)), 2U);
	buffer_size = nvgpu_safe_mult_u32((27U * 5U), buffer_size);
	buffer_size = PAGE_ALIGN(buffer_size);
	nvgpu_log_info(g, "method buffer size in bytes %d", buffer_size);

	tsg->eng_method_buffers = nvgpu_kzalloc(g,
			num_pbdma * sizeof(struct nvgpu_mem));
	if (tsg->eng_method_buffers == NULL) {
		nvgpu_err(g, "could not alloc eng method buffers");
		return -ENOMEM;
	}

	for (runque = 0; runque < num_pbdma; runque++) {
		err = nvgpu_dma_alloc_map_sys(vm, buffer_size,
					&tsg->eng_method_buffers[runque]);
		if (err != 0) {
			nvgpu_err(g, "alloc eng method buffers, runque=%d",
					runque);
			goto clean_up;
		}
	}

	nvgpu_log_info(g, "eng method buffers allocated");
	return 0;

clean_up:
	for (i = ((int)runque - 1); i >= 0; i--) {
		nvgpu_dma_unmap_free(vm, &tsg->eng_method_buffers[i]);
	}
	nvgpu_kfree(g, tsg->eng_method_buffers);
	tsg->eng_method_buffers = NULL;
	return -ENOMEM;
}

void gv11b_tsg_deinit_eng_method_buffers(struct gk20a *g,
		struct nvgpu_tsg *tsg)
{
	struct vm_gk20a *vm = g->mm.bar2.vm;
	unsigned int runque;

	if (tsg->eng_method_buffers == NULL) {
		return;
	}

	for (runque = 0;
	     runque < nvgpu_get_litter_value(g, GPU_LIT_HOST_NUM_PBDMA);
	     runque++) {
		nvgpu_dma_unmap_free(vm, &tsg->eng_method_buffers[runque]);
	}

	nvgpu_kfree(g, tsg->eng_method_buffers);
	tsg->eng_method_buffers = NULL;

	nvgpu_log_info(g, "eng method buffers de-allocated");
}

int gv11b_tsg_init_subctx_state(struct gk20a *g, struct nvgpu_tsg *tsg)
{
	u32 max_subctx_count;

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	max_subctx_count = g->ops.gr.init.get_max_subctx_count();

	/*
	 * Allocate an array of subctx PDB configuration values for all supported
	 * subcontexts. For each subctx, there will be two registers to be
	 * configured, ram_in_sc_page_dir_base_lo_w(i) and
	 * ram_in_sc_page_dir_base_hi_w(i) in the instance block for the channels
	 * belonging to this TSG. Two more unused registers follow these for each
	 * subcontext. Same PDB table/array is programmed in the instance block
	 * of all the channels.
	 *
	 * As the subcontexts are bound to the TSG, their configurations register
	 * values are added to the array and corresponding bit is set in the
	 * valid_subctxs bitmask. And as the subcontexts are unbound from
	 * the TSG, their configurations register values are added to the
	 * array and corresponding bit is set in the valid_subctxs bitmask.
	 */
	tsg->subctx_pdb_map = nvgpu_kzalloc(g, max_subctx_count * sizeof(u32) * 4U);
	if (tsg->subctx_pdb_map == NULL) {
		nvgpu_err(g, "subctx_pdb_map alloc failed");
		return -ENOMEM;
	}

	g->ops.ramin.init_subctx_pdb_map(g, tsg->subctx_pdb_map);

	tsg->valid_subctxs = nvgpu_kzalloc(g,
				BITS_TO_LONGS(max_subctx_count) *
				sizeof(unsigned long));
	if (tsg->valid_subctxs == NULL) {
		nvgpu_err(g, "valid_subctxs bitmap alloc failed");
		nvgpu_kfree(g, tsg->subctx_pdb_map);
		tsg->subctx_pdb_map = NULL;
		return -ENOMEM;
	}

	return 0;
}

void gv11b_tsg_deinit_subctx_state(struct gk20a *g, struct nvgpu_tsg *tsg)
{
	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return;
	}

	nvgpu_kfree(g, tsg->subctx_pdb_map);
	tsg->subctx_pdb_map = NULL;

	nvgpu_kfree(g, tsg->valid_subctxs);
	tsg->valid_subctxs = NULL;
}

/**
 * @brief Update instance blocks for all channels in a TSG with subcontext PDB and mask.
 *
 * This function iterates over all channels in a TSG and updates each channel's
 * instance block with the subcontext page directory base (PDB) and the valid
 * subcontexts mask. The PDB and mask are taken from the TSG's subcontext PDB map
 * and valid subcontexts bitmask, respectively.
 *
 * The steps performed by the function are as follows:
 * -# Iterate over each channel in the TSG's channel list.
 * -# For each channel, call the gops_ramin.init_subctx_pdb() function to initialize the
 *    subcontext PDB in the channel's instance block with the TSG's subcontext PDB map.
 * -# For each channel, call the gops_ramin.init_subctx_mask() function to initialize the
 *    subcontext mask in the channel's instance block with the TSG's valid subcontexts bitmask.
 *
 * @param [in] tsg  Pointer to the TSG structure whose channels' instance blocks are to be updated.
 */
static void gv11b_tsg_update_inst_blocks_subctxs(struct nvgpu_tsg *tsg)
{
	struct gk20a *g = tsg->g;
	struct nvgpu_channel *ch;

	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		g->ops.ramin.init_subctx_pdb(g, &ch->inst_block,
					     tsg->subctx_pdb_map);
		g->ops.ramin.init_subctx_mask(g, &ch->inst_block,
					      tsg->valid_subctxs);
	}
}

/**
 * @brief Update subcontext PDB info and valid subcontexts bitmask, then update instance blocks.
 *
 * This function updates the subcontext page directory base (PDB) information and the valid
 * subcontexts bitmask for a given subcontext ID within a TSG. It then calls another function
 * to update the instance blocks of all channels within the TSG with the new subcontext PDB
 * information and bitmask. If the 'add' parameter is true, it adds the subcontext; otherwise,
 * it removes the subcontext.
 *
 * The steps performed by the function are as follows:
 * -# Check if the subcontext is to be added or removed based on the 'add' parameter.
 * -# If adding the subcontext:
 *    -# Call gops_ramin.set_subctx_pdb_info() to set the subcontext PDB information in the TSG's
 *       subcontext PDB map with the provided VM's PDB memory and replayable state.
 *    -# Call nvgpu_set_bit() to set the bit corresponding to the subcontext ID in the
 *       TSG's valid subcontexts bitmask.
 * -# If removing the subcontext:
 *    -# Call gops_ramin.set_subctx_pdb_info() to clear the subcontext PDB information in the TSG's
 *       subcontext PDB map for the given subcontext ID.
 *    -# Call nvgpu_clear_bit() to clear the bit corresponding to the subcontext ID in the
 *       TSG's valid subcontexts bitmask.
 * -# Call 'gv11b_tsg_update_inst_blocks_subctxs()' to update the instance blocks of all
 *    channels in the TSG with the new subcontext PDB map and valid subcontexts bitmask.
 *
 * @param [in] tsg         Pointer to the TSG structure whose subcontext PDB info and bitmask are to be updated.
 * @param [in] subctx_id   Subcontext ID that is being updated.
 * @param [in] vm          Pointer to the VM structure associated with the subcontext.
 * @param [in] replayable  Indicates whether the subcontext is replayable.
 * @param [in] add         Flag indicating whether to add (true) or remove (false) the subcontext.
 */
static void gv11b_tsg_update_subctxs(struct nvgpu_tsg *tsg, u32 subctx_id,
				struct vm_gk20a *vm, bool replayable, bool add)
{
	struct gk20a *g = tsg->g;

	if (add) {
		g->ops.ramin.set_subctx_pdb_info(g, subctx_id, vm->pdb.mem,
				replayable, true, tsg->subctx_pdb_map);
		nvgpu_set_bit(subctx_id, tsg->valid_subctxs);
	} else {
		g->ops.ramin.set_subctx_pdb_info(g, subctx_id, NULL,
				false, false, tsg->subctx_pdb_map);
		nvgpu_clear_bit(subctx_id, tsg->valid_subctxs);
	}

	gv11b_tsg_update_inst_blocks_subctxs(tsg);
}

/**
 * @brief Add a new subcontext for a channel and update the TSG's subcontext structures.
 *
 * This function sets up a new subcontext for a given channel. It marks the subcontext as
 * replayable or non-replayable based on the input parameter and updates the TSG's subcontext
 * page directory base (PDB) information and valid subcontexts bitmask to include this new
 * subcontext. It then updates the instance blocks of all channels in the TSG with the new
 * subcontext information.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the TSG associated with the channel by calling 'nvgpu_tsg_from_ch()'.
 * -# Retrieve the subcontext associated with the channel.
 * -# Retrieve the VM associated with the subcontext by calling 'nvgpu_tsg_subctx_get_vm()'.
 * -# Retrieve the subcontext ID by calling 'nvgpu_tsg_subctx_get_id()'.
 * -# Set the replayable state of the subcontext by calling 'nvgpu_tsg_subctx_set_replayable()'.
 * -# Assert that the TSG is not NULL.
 * -# Update the TSG's subcontext structures to include the new subcontext by calling
 *    'gv11b_tsg_update_subctxs()' with the add flag set to true.
 *
 * @param [in] ch          Pointer to the channel structure associated with the new subcontext.
 * @param [in] replayable  Indicates whether the new subcontext is replayable.
 */
static void gv11b_tsg_add_new_subctx_channel_hw(struct nvgpu_channel *ch,
						bool replayable)
{
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	struct nvgpu_tsg_subctx *subctx = ch->subctx;
	struct vm_gk20a *vm = nvgpu_tsg_subctx_get_vm(subctx);
	u32 subctx_id = nvgpu_tsg_subctx_get_id(subctx);

	nvgpu_tsg_subctx_set_replayable(subctx, replayable);
	nvgpu_assert(tsg != NULL);
	gv11b_tsg_update_subctxs(tsg, subctx_id, vm, replayable, true);
}

/**
 * @brief Update hardware context for a channel with an existing subcontext.
 *
 * This function updates the instance block of a channel with the subcontext PDB and mask
 * if the channel is part of an existing subcontext. It checks if the replayable state of the
 * subcontext matches the requested state and logs an error if there is a mismatch.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the TSG associated with the channel by calling 'nvgpu_tsg_from_ch()'.
 * -# Retrieve the subcontext associated with the channel.
 * -# Retrieve the GPU driver structure associated with the channel.
 * -# Check if the replayable state of the subcontext matches the requested state by calling
 *    'nvgpu_tsg_subctx_get_replayable()'. If there is a mismatch, log an error message.
 * -# Assert that the TSG is not NULL.
 * -# Initialize the subcontext PDB in the channel's instance block by calling
 *    gops_ramin.init_subctx_pdb() with the TSG's subcontext PDB map.
 * -# Initialize the subcontext mask in the channel's instance block by calling
 *    gops_ramin.init_subctx_mask() with the TSG's valid subcontexts bitmask.
 *
 * @param [in] ch          Pointer to the channel structure associated with the existing subcontext.
 * @param [in] replayable  Indicates the requested replayable state for the subcontext.
 */
static void gv11b_tsg_add_existing_subctx_channel_hw(struct nvgpu_channel *ch,
						 bool replayable)
{
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	struct nvgpu_tsg_subctx *subctx = ch->subctx;
	struct gk20a *g = ch->g;

	if (nvgpu_tsg_subctx_get_replayable(subctx) != replayable) {
		nvgpu_err(g, "subctx replayable mismatch. ignoring.");
	}

	nvgpu_assert(tsg != NULL);
	g->ops.ramin.init_subctx_pdb(g, &ch->inst_block, tsg->subctx_pdb_map);
	g->ops.ramin.init_subctx_mask(g, &ch->inst_block, tsg->valid_subctxs);
}

int gv11b_tsg_add_subctx_channel_hw(struct nvgpu_channel *ch, bool replayable)
{
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	struct gk20a *g;
	int err;

	nvgpu_assert(tsg != NULL);
	g = tsg->g;
	nvgpu_log(g, gpu_dbg_fn, " ");

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	/*
	 * Add new subcontext to the TSG. Sequence for this is below:
	 *   1. Disable TSG.
	 *   2. Preempt TSG.
	 *   3. Program subctx PDBs in instance blocks of all channels in
	 *      the TSG.
	 *   4. Enable TSG.
	 * This sequence is executed acquiring TSG level lock ctx_init_lock.
	 * to synchronize with channels from other subcontexts.
	 * ctx_init_lock is reused here. It is originally there for
	 * synchronizing the GR context initialization by various
	 * channels in the TSG.
	 */

	nvgpu_mutex_acquire(&tsg->ctx_init_lock);

	nvgpu_tsg_disable(tsg);
	err = nvgpu_tsg_preempt(g, tsg);
	if (err != 0) {
		nvgpu_tsg_enable(tsg);
		nvgpu_mutex_release(&tsg->ctx_init_lock);
		nvgpu_err(g, "preempt failed %d", err);
		return err;
	}

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);

	if (!nvgpu_test_bit(ch->subctx_id, tsg->valid_subctxs)) {
		gv11b_tsg_add_new_subctx_channel_hw(ch, replayable);
	} else {
		gv11b_tsg_add_existing_subctx_channel_hw(ch, replayable);
	}

	nvgpu_rwsem_up_read(&tsg->ch_list_lock);


	nvgpu_tsg_enable(tsg);
	nvgpu_mutex_release(&tsg->ctx_init_lock);

	nvgpu_log(g, gpu_dbg_fn, "done");

	return 0;
}

void gv11b_tsg_remove_subctx_channel_hw(struct nvgpu_channel *ch)
{
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	struct gk20a *g;
	u32 subctx_id;

	nvgpu_assert(tsg != NULL);
	g = tsg->g;
	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return;
	}

	subctx_id = nvgpu_tsg_subctx_get_id(ch->subctx);

	gv11b_tsg_update_subctxs(tsg, subctx_id, NULL, false, false);
}
