/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_CHANNEL_H
#define NVGPU_GOPS_CHANNEL_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * Channel HAL interface.
 */
struct gk20a;
struct nvgpu_channel;
struct nvgpu_channel_hw_state;
struct nvgpu_debug_context;

/**
 * Channel HAL operations.
 *
 * @see gpu_ops.
 */
struct gops_channel {
	/**
	 * @brief Enable channel for h/w scheduling.
	 *
	 * @param g [in]	The GPU driver struct.
	 * @param runlist_id [in] ID of the runlist the channel belongs to.
	 * @param chid [in]	The channel to enable.
	 *
	 * The HAL writes CCSR register to enable channel for h/w scheduling.
	 * Once enabled, the channel can be scheduled to run when this
	 * channel is next on the runlist.
	 */
	void (*enable)(struct gk20a *g, u32 runlist_id, u32 chid);

	/**
	 * @brief Disable channel from h/w scheduling.
	 *
	 * @param g [in]	The GPU driver struct.
	 * @param runlist_id [in] ID of the runlist the channel belongs to.
	 * @param chid [in]	The channel to disable.
	 *
	 * The HAL writes CCSR register to disable channel from h/w scheduling.
	 * Once disabled, the channel is not scheduled to run even if it
	 * is next on the runlist.
	 */
	void (*disable)(struct gk20a *g, u32 runlist_id, u32 chid);

	/**
	 * @brief Get number of channels.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL reads max number of channels supported by the GPU h/w.
	 *
	 * @return Number of channels as read from GPU h/w.
	 */
	u32 (*count)(struct gk20a *g);

	/**
	 * @brief Suspend all channels.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL goes through all channels and:
	 * - If channel is not in use, done.
	 * - If channel is not serviceable, done.
	 * - Disable channel.
	 * - Preempt channel.
	 * - Wait for channel to update notifiers.
	 * - Unbind channel context from hardware.
	 *
	 * The HAL is also expected to:
	 * - Update runlists to remove channels.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*suspend_all_serviceable_ch)(struct gk20a *g);

	/**
	 * @brief Resume all channels.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL goes through all channels and:
	 * - If channel is not in use, done.
	 * - If channel is not serviceable, done,
	 * - Bind channel context to hardware.
	 *
	 * The HAL is also expected to:
	 * - Update runlists to add above channels.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*resume_all_serviceable_ch)(struct gk20a *g);

	/**
	 * @brief Set error notifier for a channel.
	 *
	 * @param ch [in]	Channel pointer.
	 * @param error [in]	Error code for notification.
	 *
	 * The HAL does the following:
	 * - Acquire error_notifer mutex.
	 * - If an error notifier buffer was allocated:
	 *   - Get CPU timestamp in ns.
	 *   - Update timestamp in notification buffer.
	 *   - Update error code in notification buffer.
	 *     \a error should be of the form NVGPU_ERR_NOTIFIER_*
	 * - Release error_notifier mutex.
	 *
	 * @see NVGPU_ERR_NOTIFIER_FIFO_ERROR_IDLE_TIMEOUT
	 */
	void (*set_error_notifier)(struct nvgpu_channel *ch, u32 error);

	/**
	 * @see nvgpu_channel_alloc_inst()
	 */
	int (*alloc_inst)(struct gk20a *g, struct nvgpu_channel *ch);

	/**
	 * @see nvgpu_channel_free_inst()
	 */
	void (*free_inst)(struct gk20a *g, struct nvgpu_channel *ch);

	/**
	 * @brief Binds a channel and enables it for use.
	 *
	 * This function performs the following steps to bind a channel:
	 * -# Retrieves the GPU (gk20a) structure from the channel structure.
	 * -# Checks if the function pointer gops_tsg.add_subctx_channel_hw is not NULL.
	 *    If it is not NULL, it means that the hardware supports subcontexts and
	 *    the channel needs to be added to a subcontext.
	 * -# Calls gops_tsg.add_subctx_channel_hw() to add the channel to a subcontext.
	 *    If this operation fails, logs an error message and exits the function.
	 * -# Calls nvgpu_channel_enable() to enable the channel.
	 * -# Sets the 'bound' status of the channel to #CHANNEL_BOUND using
	 *    nvgpu_atomic_set().
	 *
	 * @param [in] ch The channel structure that needs to be bound.
	 */
	void (*bind)(struct nvgpu_channel *ch);

	/**
	 * @brief Unbinds a channel from the GPU runlist.
	 *
	 * The function performs the following steps to unbind a channel:
	 * -# Retrieve the GPU context from the channel structure parameter 'ch'.
	 * -# Retrieve the runlist associated with the channel from the 'ch' parameter.
	 * -# Attempt to atomically compare and exchange the channel's bound state from
	 *    #CHANNEL_BOUND to #CHANNEL_UNBOUND using the nvgpu_atomic_cmpxchg() function.
	 * -# If the compare and exchange operation is successful, indicating the channel
	 *    was previously bound, invoke the clear() operation from the GPU operations
	 *    structure to clear the channel from the runlist using gops_channel.clear().
	 *
	 * @param [in] ch  The channel structure representing the GPU channel to unbind.
	 */
	void (*unbind)(struct nvgpu_channel *ch);

	/**
	 * @brief Reads the hardware state of a channel and constructs a string representing the
	 *        channel's status.
	 *
	 * -# Retrieve the runlist structure from the GPU's FIFO runlists array using the provided
	 *    runlist identifier.
	 * -# Read the channel's hardware state from the channel's register using the
	 *    nvgpu_chram_bar0_readl() function.
	 * -# Determine if the channel is marked as 'next' by comparing the relevant bits in the
	 *    register value.
	 * -# Determine if the channel is enabled by comparing the relevant bits in the register
	 *    value.
	 * -# Determine if the channel has a context reload pending by comparing the relevant bits
	 *    in the register value.
	 * -# Determine if the channel is busy by comparing the relevant bits in the register value.
	 * -# Determine if the channel has a pending acquire and if it has failed to acquire by
	 *    comparing the relevant bits in the register value.
	 * -# Determine if the channel has an engine fault by comparing the relevant bits in the
	 *    register value.
	 * -# Create a mask to identify the status bits in the register value using the
	 *    ga10b_channel_status_mask() function.
	 * -# Iterate over each bit in the status mask and for each set bit, add the corresponding
	 *    status string to the list of status strings. This is done using a loop that checks
	 *    each bit position and uses the BIT32() macro to access the correct status string.
	 *    The nvgpu_safe_add_u32() function is used to safely increment the status string count.
	 *    If any status bit is set, the channel is not idle.
	 * -# If no status bits are set (channel is idle), add the "idle" status string to the list
	 *    of status strings.
	 * -# Combine all status strings into a single comma-separated string using the
	 *    nvgpu_str_join() function. This string represents the channel's overall status.
	 *
	 * @param [in]  g         Pointer to the GPU structure.
	 * @param [in]  runlist_id Identifier of the runlist to which the channel belongs.
	 * @param [in]  chid      Identifier of the channel whose state is to be read.
	 * @param [out] state     Pointer to the structure where the channel's hardware state will
	 *                        be stored.
	 */
	void (*read_state)(struct gk20a *g, u32 runlist_id, u32 chid,
			struct nvgpu_channel_hw_state *state);

	/**
	 * @brief Force a context reload for a specific channel on a given runlist.
	 *
	 * This function forces a context reload for a channel identified by its channel ID
	 * on a runlist specified by the runlist ID. The context reload is a mechanism to
	 * ensure that the channel's context is reloaded from memory the next time it is
	 * scheduled to run. The steps involved in this process are:
	 * -# Retrieve the pointer to the runlist structure from the GPU's FIFO runlists
	 *    array using the provided runlist ID.
	 * -# Write to the channel's context reload register within the runlist using the
	 *    'nvgpu_chram_bar0_writel()' function. This is done by passing the GPU
	 *    structure, the runlist structure pointer, the channel's context reload
	 *    register offset computed by 'runlist_chram_channel_r()', and the value to
	 *    write computed by 'runlist_chram_channel_update_f()' with the context reload
	 *    bit set to force by 'runlist_chram_channel_update_force_ctx_reload_v()'.
	 *
	 * @param [in] g           Pointer to the GPU structure.
	 * @param [in] runlist_id  ID of the runlist to which the channel belongs.
	 * @param [in] chid        ID of the channel for which to force a context reload.
	 */
	void (*force_ctx_reload)(struct gk20a *g, u32 runlist_id, u32 chid);

	/**
	 * @see nvgpu_channel_abort_clean_up()
	 */
	void (*abort_clean_up)(struct nvgpu_channel *ch);

	/**
	 * @brief Resets faulted states for both engine and PBDMA in a channel, and rings the runlist doorbell.
	 *
	 * This function clears the faulted states for both the engine and PBDMA associated with a specific
	 * channel. After clearing the faults, it ensures that the runlist scheduler is reactivated by
	 * ringing the doorbell, which is necessary if the runlist has gone idle.
	 *
	 * The steps performed by the function are as follows:
	 * -# Retrieve the runlist structure from the GPU's FIFO runlists array using the provided runlist ID.
	 * -# Write to the channel's register to reset the engine's faulted state using nvgpu_chram_bar0_writel().
	 * -# Write to the channel's register again to reset the PBDMA's faulted state using nvgpu_chram_bar0_writel().
	 * -# Ring the runlist doorbell to ensure the scheduler is activated if it has gone idle, using
	 *    the ring_doorbell() method from the GPU operations structure.
	 *
	 * @param [in] g Pointer to the GPU structure.
	 * @param [in] runlist_id Identifier for the runlist.
	 * @param [in] chid Channel identifier.
	 */
	void (*reset_faulted)(struct gk20a *g, u32 runlist_id, u32 chid);

	/**
	 * @brief Clear the channel state in channel RAM.
	 *
	 * This function clears the state of a channel in the channel RAM, which is part of
	 * the runlist. It is typically used during channel teardown or when the channel is
	 * being reset. The steps involved in this process are:
	 * -# Write to the channel's state register within the runlist using the
	 *    'nvgpu_chram_bar0_writel()' function. This is done by passing the GPU
	 *    structure, the pointer to the runlist structure retrieved from the GPU's FIFO
	 *    runlists array using the provided runlist ID, the channel's state register
	 *    offset computed by 'runlist_chram_channel_r()', and the value to write
	 *    computed by 'runlist_chram_channel_update_f()' with the clear channel bit set
	 *    to clear by 'runlist_chram_channel_update_clear_channel_v()'.
	 *
	 * @param [in] g           Pointer to the GPU structure.
	 * @param [in] runlist_id  ID of the runlist to which the channel belongs.
	 * @param [in] chid        ID of the channel whose state is to be cleared.
	 */
	void (*clear)(struct gk20a *g, u32 runlist_id, u32 chid);

	/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	int (*get_vmid)(struct nvgpu_channel *ch, u32 *vmid);

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	int (*set_syncpt)(struct nvgpu_channel *ch);
#endif

	/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
};

#endif
