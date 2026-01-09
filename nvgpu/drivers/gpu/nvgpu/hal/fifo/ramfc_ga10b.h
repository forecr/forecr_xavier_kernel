/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RAMFC_GA10B_H
#define NVGPU_RAMFC_GA10B_H

#include <nvgpu/types.h>

struct nvgpu_channel;
struct nvgpu_channel_dump_info;

int ga10b_ramfc_setup(struct nvgpu_channel *ch, u64 gpfifo_base,
		u32 gpfifo_entries, u64 pbdma_acquire_timeout, u32 flags);

/**
 * @brief Captures the top-level GET pointer from the RAMFC area of a channel's instance block.
 *
 * This function reads the 64-bit top-level GET pointer from the RAMFC area and stores it in the
 * provided channel dump information structure. The steps are as follows:
 * -# Obtain a pointer to the channel's instance block memory.
 * -# Read the 32-bit lower and upper parts of the top-level GET pointer using the
 *    nvgpu_mem_rd32_pair() function, providing the GPU, memory pointer, and the offsets for the
 *    lower and upper parts of the pointer as defined by the macros #ram_fc_pb_top_level_get_w and
 *    #ram_fc_pb_top_level_get_hi_w.
 * -# Store the combined 64-bit value into the channel dump information structure.
 *
 * @param [in]  g        Pointer to the GPU structure.
 * @param [in]  ch       Pointer to the channel structure.
 * @param [out] info     Pointer to the channel dump information structure where the top-level GET
 *                       pointer will be stored.
 */
void ga10b_ramfc_capture_ram_dump_1(struct gk20a *g,
		struct nvgpu_channel *ch, struct nvgpu_channel_dump_info *info);

/**
 * @brief Captures various RAMFC register values from a channel's instance block and stores them in
 *        the channel dump information structure.
 *
 * The function performs the following steps for capturing the RAMFC dump:
 * -# Obtain a pointer to the channel's instance block memory.
 * -# Read the 64-bit PUT pointer from the RAMFC area using the nvgpu_mem_rd32_pair() function,
 *    providing the GPU, memory pointer, and the offsets for the lower and upper parts of the
 *    pointer as defined by the macros #ram_fc_pb_put_w and #ram_fc_pb_put_hi_w. Store the combined
 *    64-bit value into the channel dump information structure.
 * -# Read the 64-bit GET pointer from the RAMFC area using the nvgpu_mem_rd32_pair() function,
 *    providing the GPU, memory pointer, and the offsets for the lower and upper parts of the
 *    pointer as defined by the macros #ram_fc_pb_get_w and #ram_fc_pb_get_hi_w. Store the combined
 *    64-bit value into the channel dump information structure.
 * -# Read the 32-bit PB header value from the RAMFC area using the nvgpu_mem_rd32() function,
 *    providing the GPU, memory pointer, and the offset as defined by the macro #ram_fc_pb_header_w.
 *    Store the value into the channel dump information structure.
 * -# Read the 32-bit PB count value from the RAMFC area using the nvgpu_mem_rd32() function,
 *    providing the GPU, memory pointer, and the offset as defined by the macro #ram_fc_pb_count_w.
 *    Store the value into the channel dump information structure.
 * -# Read the 64-bit semaphore address from the RAMFC area using the nvgpu_mem_rd32_pair() function,
 *    providing the GPU, memory pointer, and the offsets for the lower and upper parts of the
 *    address as defined by the macros #ram_fc_sem_addr_lo_w and #ram_fc_sem_addr_hi_w. Store the
 *    combined 64-bit value into the channel dump information structure.
 * -# Read the 64-bit semaphore payload from the RAMFC area using the nvgpu_mem_rd32_pair() function,
 *    providing the GPU, memory pointer, and the offsets for the lower and upper parts of the
 *    payload as defined by the macros #ram_fc_sem_payload_lo_w and #ram_fc_sem_payload_hi_w. Store
 *    the combined 64-bit value into the channel dump information structure.
 * -# Read the 32-bit semaphore execute value from the RAMFC area using the nvgpu_mem_rd32() function,
 *    providing the GPU, memory pointer, and the offset as defined by the macro #ram_fc_sem_execute_w.
 *    Store the value into the channel dump information structure.
 *
 * @param [in]  g        Pointer to the GPU structure.
 * @param [in]  ch       Pointer to the channel structure.
 * @param [out] info     Pointer to the channel dump information structure where the RAMFC register
 *                       values will be stored.
 */
void ga10b_ramfc_capture_ram_dump_2(struct gk20a *g,
		struct nvgpu_channel *ch, struct nvgpu_channel_dump_info *info);
void ga10b_ramfc_capture_ram_dump(struct gk20a *g, struct nvgpu_channel *ch,
				  struct nvgpu_channel_dump_info *info);
#endif /* NVGPU_RAMFC_GA10B_H */
