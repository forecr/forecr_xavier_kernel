/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_RAM_GB10B_H
#define NVGPU_RUNLIST_RAM_GB10B_H

#include <nvgpu/types.h>

struct nvgpu_channel;
struct nvgpu_tsg;

u32 gb10b_chan_userd_ptr_hi(u32 addr_hi);
void gb10b_runlist_get_ch_entry(struct nvgpu_channel *ch, u32 *runlist);

#endif /* NVGPU_RUNLIST_RAM_GB10B_H */
