/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RAMFC_GV11B_H
#define NVGPU_RAMFC_GV11B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_channel_dump_info;

int gv11b_ramfc_setup(struct nvgpu_channel *ch, u64 gpfifo_base,
		u32 gpfifo_entries, u64 pbdma_acquire_timeout, u32 flags);
void gv11b_ramfc_capture_ram_dump(struct gk20a *g, struct nvgpu_channel *ch,
		struct nvgpu_channel_dump_info *info);

#endif /* NVGPU_RAMFC_GV11B_H */
