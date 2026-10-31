/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_FIFO_GB10B_H
#define NVGPU_RUNLIST_FIFO_GB10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_runlist;

u32 gb10b_runlist_count_max(struct gk20a *g);
u32 gb10b_virtual_channel_cfg_off(u32 gfid);
void gb10b_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid);
void gb10b_runlist_wait_for_preempt_before_reset(struct gk20a *g,
		u32 runlist_id);
int gb10b_fifo_check_tsg_preempt_is_complete(struct gk20a *g,
		struct nvgpu_runlist *runlist);

#endif /* NVGPU_RUNLIST_FIFO_GB10B_H */
