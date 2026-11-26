/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GOPS_TSG_H
#define NVGPU_GOPS_TSG_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_tsg;

#ifdef CONFIG_NVGPU_CHANNEL_TSG_CONTROL
enum nvgpu_event_id_type;
#endif

struct gops_tsg {

	/**
	* @brief Enable TSG
	*
	* @param tsg [in]	Pointer to the TSG struct.
	*
	* Configure H/W so that this TSG can be scheduled.
	*/
	void (*enable)(struct nvgpu_tsg *tsg);

	/**
	* @brief Disable TSG
	*
	* @param tsg [in]	Pointer to the TSG struct.
	*
	* Configure H/W so that it skips this TSG for scheduling.
	*/
	void (*disable)(struct nvgpu_tsg *tsg);

	/**
	 * @see gv11b_tsg_init_subctx_state()
	 */
	int (*init_subctx_state)(struct gk20a *g, struct nvgpu_tsg *tsg);

	/**
	 * @see gv11b_tsg_deinit_subctx_state()
	 */
	void (*deinit_subctx_state)(struct gk20a *g, struct nvgpu_tsg *tsg);

	/**
	 * @see gv11b_tsg_add_subctx_channel_hw()
	 */
	int (*add_subctx_channel_hw)(struct nvgpu_channel *ch,
				      bool replayable);

	/**
	 * @see gv11b_tsg_remove_subctx_channel_hw()
	 */
	void (*remove_subctx_channel_hw)(struct nvgpu_channel *ch);

	/**
	 * @brief Initialize engine method buffers for a TSG.
	 *
	 * -# Check if the engine method buffers are already allocated for the TSG.
	 *    If they are, log a warning and return success (0).
	 * -# Calculate the buffer size required for the engine method buffers. This
	 *    size is based on the number of PCEs (Processing Cluster Engines) and
	 *    some predefined constants.
	 * -# Allocate memory for the engine method buffers array, with one entry per
	 *    PBDMA (Push Buffer DMA).
	 * -# If the allocation fails, log an error and return -ENOMEM.
	 * -# For each PBDMA, allocate and map system memory for the engine method
	 *    buffers using nvgpu_dma_alloc_map_sys().
	 * -# If allocation or mapping fails for any PBDMA, log an error, go to cleanup
	 *    to free any previously allocated buffers, and return -ENOMEM.
	 * -# Log that engine method buffers have been successfully allocated.
	 * -# Return success (0).
	 *
	 * @param [in]  g         Pointer to the GPU driver struct.
	 * @param [in]  tsg       Pointer to the TSG struct.
	 *
	 * @return 0 if the initialization is successful.
	 * @return -ENOMEM if memory allocation fails.
	 */
	int (*init_eng_method_buffers)(struct gk20a *g,
			struct nvgpu_tsg *tsg);

	/**
	 * @brief Deinitialize and free the engine method buffers associated with a TSG.
	 *
	 * This function performs the following steps to deinitialize and free the engine
	 * method buffers:
	 * -# Check if the engine method buffers are already deinitialized. If they are,
	 *    exit the function.
	 * -# Iterate over each engine method buffer based on the number of PBDMAs in the
	 *    system and unmap and free each buffer.
	 * -# Free the memory allocated for the engine method buffers array.
	 * -# Log that the engine method buffers have been de-allocated.
	 *
	 * @param [in] g         Pointer to the GPU driver struct.
	 * @param [in] tsg       Pointer to the TSG struct.
	 */
	void (*deinit_eng_method_buffers)(struct gk20a *g,
			struct nvgpu_tsg *tsg);

	/**
	 * @brief Bind the engine method buffers to a channel within a TSG.
	 *
	 * This function performs the following steps to bind the engine method buffers:
	 * -# Assert that the TSG's runlist is not NULL.
	 * -# If the TSG's engine method buffers are NULL, log the information and exit
	 *    the function.
	 * -# Determine the GPU virtual address of the engine method buffer to use based
	 *    on the runlist ID. If the runlist ID corresponds to the fast CE runlist,
	 *    use the #ASYNC_CE_RUNQUE buffer; otherwise, use the #GR_RUNQUE buffer.
	 * -# Call gops_ramin.set_eng_method_buffer() to bind the engine method buffer's
	 *    GPU virtual address to the channel's instance block.
	 *
	 * @param [in] tsg       Pointer to the TSG struct.
	 * @param [in] ch        Pointer to the channel struct.
	 */
	void (*bind_channel_eng_method_buffers)(struct nvgpu_tsg *tsg,
			struct nvgpu_channel *ch);
	/**
	 * @see nvgpu_tsg_unbind_channel_hw_state_check()
	 */
	int (*unbind_channel_check_hw_state)(struct nvgpu_tsg *tsg,
			struct nvgpu_channel *ch);

	/**
	 * @brief Check if the channel can be unbound from the TSG based on hardware state.
	 *
	 * This function performs the following steps:
	 * -# Check if the 'next' bit is set in the hardware state of the channel.
	 *    The 'next' bit indicates whether the channel is scheduled to run next.
	 * -# If the 'next' bit is set, log an informational message indicating that the
	 *    channel cannot be removed from the TSG because it has the 'NEXT' bit set.
	 *    The log message includes the channel ID and the TSG ID.
	 * -# Return the error code #-EAGAIN if the 'next' bit is set, indicating that
	 *    the operation should be retried later.
	 * -# Return 0 if the 'next' bit is not set, indicating that the channel can be
	 *    safely unbound from the TSG.
	 *
	 * @param [in] ch        Pointer to the channel structure.
	 * @param [in] hw_state  Pointer to the structure holding the hardware state of the channel.
	 *
	 * @return 0 if the channel does not have the 'next' bit set and can be unbound.
	 * @return #-EAGAIN if the channel has the 'next' bit set and cannot be unbound yet.
	 */
	int (*unbind_channel_check_hw_next)(struct nvgpu_channel *ch,
			struct nvgpu_channel_hw_state *state);
	/**
	 * @see nvgpu_tsg_unbind_channel_ctx_reload_check()
	 */
	void (*unbind_channel_check_ctx_reload)(struct nvgpu_tsg *tsg,
			struct nvgpu_channel *ch,
			struct nvgpu_channel_hw_state *state);

	/**
	 * @brief Check if the channel has an engine fault and clear the CE method buffer if
	 *        necessary.
	 *
	 * This function performs the following steps:
	 * -# Check if the channel has an engine fault by examining the 'eng_faulted' flag
	 *    in the hardware state structure.
	 * -# If the channel does not have an engine fault or if the TSG's engine method buffers
	 *    are NULL, exit the function.
	 * -# If the channel has an engine fault and the TSG's engine method buffers are not NULL,
	 *    proceed to clear the CE method buffer.
	 * -# Determine the index of the CE method buffer to clear by checking if the channel ID
	 *    matches the channel ID stored at the second DWORD of the method buffer.
	 * -# If the channel IDs match, write 0 to the first DWORD of the method buffer to
	 *    invalidate it, effectively clearing the CE method buffer.
	 *
	 * @param [in] tsg       Pointer to the TSG struct.
	 * @param [in] ch        Pointer to the channel struct.
	 * @param [in] hw_state  Pointer to the structure holding the hardware state of the channel.
	 */
	void (*unbind_channel_check_eng_faulted)(struct nvgpu_tsg *tsg,
			struct nvgpu_channel *ch,
			struct nvgpu_channel_hw_state *state);

	/**
	 * @see nvgpu_tsg_default_timeslice_us()
	 */
	u32 (*default_timeslice_us)(struct gk20a *g);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	int (*open)(struct nvgpu_tsg *tsg);
	void (*release)(struct nvgpu_tsg *tsg);
	int (*bind_channel)(struct nvgpu_tsg *tsg,
			struct nvgpu_channel *ch);
	int (*unbind_channel)(struct nvgpu_tsg *tsg,
			struct nvgpu_channel *ch);
	int (*set_timeslice)(struct nvgpu_tsg *tsg, u32 timeslice_us);
	int (*set_long_timeslice)(struct nvgpu_tsg *tsg, u32 timeslice_us);
	int (*set_interleave)(struct nvgpu_tsg *tsg, u32 new_level);
	void (*set_error_notifier)(struct gk20a *g, u32 gfid, u32 tsgid, u32 error_notifier);
	void (*set_unserviceable)(struct gk20a *g, u32 gfid, u32 tsgid);
	int (*store_sm_error_state)(struct gk20a *g, struct nvgpu_gr_isr_data *isr_data,
			u32 sm_id, u32 hww_global_esr, u32 hww_warp_esr, u64 hww_warp_esr_pc,
			u32 hww_global_esr_report_mask, u32 hww_warp_esr_report_mask);
#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	bool (*check_ctxsw_timeout)(struct gk20a *g, u32 tsgid,
			bool *verbose, u32 *ms);
#endif

#ifdef CONFIG_NVGPU_CHANNEL_TSG_CONTROL
	int (*force_reset)(struct nvgpu_channel *ch,
				u32 err_code, bool verbose);
	void (*post_event_id)(struct gk20a *g, u32 gfid, u32 tsgid,
			      enum nvgpu_event_id_type event_id);
#endif
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

};


#endif
