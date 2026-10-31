/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RAMFC_GB10B_H
#define NVGPU_RAMFC_GB10B_H

#include <nvgpu/types.h>

struct nvgpu_channel;

void gb10b_set_channel_info(struct nvgpu_channel *ch);
int gb10b_ramfc_setup(struct nvgpu_channel *ch, u64 gpfifo_base,
		u32 gpfifo_entries, u64 pbdma_acquire_timeout, u32 flags);
void gb10b_ramfc_capture_ram_dump_2(struct gk20a *g,
		struct nvgpu_channel *ch, struct nvgpu_channel_dump_info *info);
#endif /* NVGPU_RAMFC_GB10B_H */
