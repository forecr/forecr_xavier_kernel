/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_USERMODE_H
#define NVGPU_GOPS_USERMODE_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * Usermode HAL interface.
 */
struct gk20a;
struct nvgpu_channel;

/**
 * @file
 *
 * Usermode HAL operations.
 */
struct gops_usermode {

	/**
	 * @brief Base address for usermode drivers.
	 *
	 * @param g [in]	Pointer to GPU driver struct.
	 *
	 * Usermode is a mappable range of registers for use by usermode
	 * drivers.
	 *
	 * @return 64-bit base address for usermode accessible registers.
	 */
	u64 (*base)(struct gk20a *g);

	/**
	 * @brief Doorbell token.
	 *
	 * @param ch [in]	Channel Pointer.
	 *
	 * The function builds a doorbell token for channel \a ch.
	 *
	 * This token is used to notify H/W that new work is available for
	 * a given channel. This allows "usermode submit", where application
	 * handles GP and PB entries by itself, then writes token to submit
	 * work, without intervention of nvgpu rm.
	 *
	 * @return 32-bit token to ring doorbell for channel \a ch.
	 */
	u32 (*doorbell_token)(struct gk20a *g, u32 runlist_id, u32 chid);

	/**
	 * @brief Configures usermode hardware settings for runlists.
	 *
	 * This function iterates over all runlists and configures the virtual channel
	 * settings for each runlist. It only supports GFID 0 at the moment. The
	 * configuration includes setting the hardware mask and enabling pending
	 * interrupts.
	 *
	 * The steps performed by the function are as follows:
	 * -# Retrieve the maximum number of runlists supported by the hardware using
	 *    the gops_runlist.count_max() function.
	 * -# Iterate over each runlist index from 0 up to the maximum number of
	 *    runlists.
	 *    - For each runlist index:
	 *      -# Retrieve the pointer to the runlist structure from the 'runlists'
	 *         array. If the pointer is NULL, skip the current iteration.
	 *      -# Read the current value of the virtual channel configuration register
	 *         for GFID 0 using the nvgpu_runlist_readl() function.
	 *      -# Set the hardware mask bit in the register value using the
	 *         'runlist_virtual_channel_cfg_mask_hw_mask_hw_init_f()' macro.
	 *      -# Set the pending interrupt enable bit in the register value using the
	 *         'runlist_virtual_channel_cfg_pending_enable_true_f()' macro.
	 *      -# Write the updated register value back to the virtual channel
	 *         configuration register for GFID 0 using the nvgpu_runlist_writel()
	 *         function.
	 *
	 * @param [in]  g  Pointer to the GPU superstructure.
	 *
	 * @return None.
	 */
	void (*setup_hw)(struct gk20a *g);

	/**
	 * @brief Rings the usermode doorbell for a specific channel and runlist.
	 *
	 * This function triggers the doorbell for a given channel and runlist on the TU104 GPU architecture.
	 * It logs the action and writes the appropriate doorbell token to the usermode doorbell register.
	 *
	 * The steps performed by the function are as follows:
	 * -# Log the action of ringing the doorbell for the specified channel and runlist.
	 * -# Retrieve the doorbell token for the specified channel and runlist using the doorbell_token() function
	 *    from the GPU operations structure.
	 * -# Write the retrieved doorbell token to the usermode doorbell register using nvgpu_usermode_writel().
	 *
	 * @param [in] g           Pointer to the GPU structure.
	 * @param [in] runlist_id  ID of the runlist associated with the channel.
	 * @param [in] chid        ID of the channel for which the doorbell is to be rung.
	 */
	void (*ring_doorbell)(struct gk20a *g, u32 runlist_id, u32 chid);

	/**
	 * @brief Returns the base address for usermode bus.
	 *
	 * This function calculates the base address for the usermode bus by adding the
	 * full physical offset to the configuration space offset. The full physical
	 * offset and configuration space offset are obtained from predefined macros.
	 *
	 * The steps performed by the function are as follows:
	 * -# Cast the result of the macro #func_full_phys_offset_v() to type u64.
	 * -# Add the result of the macro #func_cfg0_r() to the casted value.
	 * -# Return the calculated base address.
	 *
	 * @param [in]  g  Pointer to the GPU structure (unused in the function).
	 *
	 * @return The calculated base address for the usermode bus.
	 */
	u64 (*bus_base)(struct gk20a *g);
};


#endif
