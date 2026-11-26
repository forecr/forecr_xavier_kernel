/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_FIFO_GA100_H
#define NVGPU_RUNLIST_FIFO_GA100_H

#include <nvgpu/types.h>

struct gk20a;

u32 ga100_runlist_count_max(struct gk20a *g);
void ga100_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid);
#endif /* NVGPU_RUNLIST_FIFO_GA100_H */
