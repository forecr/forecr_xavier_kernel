/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RAMFC_GP10B_H
#define NVGPU_RAMFC_GP10B_H

#include <nvgpu/types.h>

struct nvgpu_channel;

int gp10b_ramfc_commit_userd(struct nvgpu_channel *ch);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
int gp10b_ramfc_setup(struct nvgpu_channel *ch, u64 gpfifo_base,
		u32 gpfifo_entries, u64 pbdma_acquire_timeout, u32 flags);
u32 gp10b_ramfc_get_syncpt(struct nvgpu_channel *ch);
void gp10b_ramfc_set_syncpt(struct nvgpu_channel *ch, u32 syncpt);
#endif

#endif /* NVGPU_RAMFC_GP10B_H */
