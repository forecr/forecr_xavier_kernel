/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_RAMFC_H
#define NVGPU_GOPS_RAMFC_H

#include <nvgpu/types.h>


struct gk20a;
struct nvgpu_channel;
struct nvgpu_channel_dump_info;

struct gops_ramfc {
	/**
	 * @brief Sets up the RAMFC (Runlist Active Memory FIFO Control) for a given channel.
	 *
	 * The function initializes the channel's RAMFC area with the provided parameters and
	 * configures various settings such as the GP FIFO base, size, PBDMA acquire timeout, and
	 * interrupt handling.
	 * The steps are as follows:
	 * -# Retrieve the GPU structure pointer from the channel structure.
	 * -# Retrieve the pointer to the channel's instance block memory.
	 * -# Initialize a variable to store intermediate data values.
	 * -# Initialize variables for engine identification and interrupt handling.
	 * -# Cast the flags parameter to void to indicate it is unused.
	 * -# Log the function entry with nvgpu_log_fn().
	 * -# Determine the bitmask of active engines on the runlist and calculate the first active
	 *    engine ID using nvgpu_ffs() and nvgpu_safe_sub_u32().
	 * -# Clear the RAMFC area in the instance block memory using nvgpu_memset().
	 * -# Log the PBDMA acquire timeout value with nvgpu_log_info().
	 * -# Write the lower 32 bits of the GP FIFO base address to the RAMFC area using
	 *    nvgpu_mem_wr32().
	 * -# Write the upper 32 bits of the GP FIFO base address and the number of GP FIFO entries
	 *    to the RAMFC area using nvgpu_mem_wr32().
	 * -# Write the PBDMA signature to the RAMFC area using nvgpu_mem_wr32().
	 * -# Write the PB header value to the RAMFC area using nvgpu_mem_wr32().
	 * -# Write the subdevice value to the RAMFC area using nvgpu_mem_wr32().
	 * -# Write the target engine information to the RAMFC area using nvgpu_mem_wr32() and
	 *    nvgpu_engine_get_active_eng_info().
	 * -# Write the PBDMA acquire timeout value to the RAMFC area using nvgpu_mem_wr32().
	 * -# Read the current channel information from the RAMFC area using nvgpu_mem_rd32().
	 * -# Update the channel information with the channel's VEID and CHID using bitwise OR and
	 *    write it back to the RAMFC area using nvgpu_mem_wr32().
	 * -# Write the VEID for WFI (Wait For Idle) to the RAMFC area using nvgpu_mem_wr32().
	 * -# Calculate the engine interrupt vector based on the active engine interrupt mask using
	 *    nvgpu_engine_act_interrupt_mask() and nvgpu_ffs().
	 * -# Write the interrupt notification configuration to the RAMFC area using
	 *    nvgpu_mem_wr32().
	 * -# If the channel is privileged, set the privilege level and enable HCE priv mode for
	 *    physical mode transfer using nvgpu_mem_wr32().
	 * -# Enable the userd writeback by reading the current configuration, enabling the
	 *    writeback, and writing it back to the RAMFC area using nvgpu_mem_wr32().
	 * -# Return 0 to indicate success.
	 *
	 * @param [in]  ch                   Pointer to the channel structure.
	 * @param [in]  gpfifo_base          Base address of the GP FIFO.
	 * @param [in]  gpfifo_entries       Number of entries in the GP FIFO.
	 * @param [in]  pbdma_acquire_timeout Timeout value for PBDMA acquire.
	 * @param [in]  flags                Flags for additional configuration (currently unused).
	 *
	 * @return 0 on success, negative error code on failure.
	 */
	int (*setup)(struct nvgpu_channel *ch, u64 gpfifo_base,
			u32 gpfifo_entries, u64 pbdma_acquire_timeout,
			u32 flags);

	/**
	 * @brief Captures the RAM dump of a channel.
	 *
	 * This function is responsible for capturing the RAM dump of a channel and
	 * populating the relevant information into a provided structure. The steps
	 * are as follows:
	 * -# Call ga10b_ramfc_capture_ram_dump_1() to capture the first part of the RAM
	 *    dump specific to the channel.
	 * -# Call ga10b_ramfc_capture_ram_dump_2() to capture the second part of the RAM
	 *    dump specific to the channel.
	 *
	 * @param [in]  g               Pointer to the GPU driver structure.
	 * @param [in]  ch              Pointer to the channel structure.
	 * @param [out] info            Pointer to the channel dump information structure
	 *                              where the RAM dump will be stored.
	 */
	void (*capture_ram_dump)(struct gk20a *g,
			struct nvgpu_channel *ch,
			struct nvgpu_channel_dump_info *info);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	int (*commit_userd)(struct nvgpu_channel *ch);
	u32 (*get_syncpt)(struct nvgpu_channel *ch);
	void (*set_syncpt)(struct nvgpu_channel *ch, u32 syncpt);
	void (*set_channel_info)(struct nvgpu_channel *ch);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
};


#endif
