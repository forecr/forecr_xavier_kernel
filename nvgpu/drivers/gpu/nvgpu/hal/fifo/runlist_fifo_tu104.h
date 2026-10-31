/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_FIFO_TU104_H
#define NVGPU_RUNLIST_FIFO_TU104_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_runlist;

u32 tu104_runlist_count_max(struct gk20a *g);
void tu104_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid);
int tu104_runlist_check_pending(struct gk20a *g, struct nvgpu_runlist *runlist);

#endif /* NVGPU_RUNLIST_FIFO_TU104_H */
