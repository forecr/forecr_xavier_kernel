/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_RAMFC_GK20A_H
#define NVGPU_RAMFC_GK20A_H

#include <nvgpu/types.h>

struct nvgpu_channel;
struct nvgpu_channel_dump_info;

int gk20a_ramfc_setup(struct nvgpu_channel *ch, u64 gpfifo_base,
		u32 gpfifo_entries, u64 pbdma_acquire_timeout, u32 flags);
int gk20a_ramfc_commit_userd(struct nvgpu_channel *ch);
void gk20a_ramfc_capture_ram_dump(struct gk20a *g, struct nvgpu_channel *ch,
		struct nvgpu_channel_dump_info *info);

#endif /* NVGPU_RAMFC_GK20A_H */
