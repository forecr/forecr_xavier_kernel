/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_FIFO_GA10B_H
#define NVGPU_RUNLIST_FIFO_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_runlist;

u32 ga10b_runlist_count_max(struct gk20a *g);
u32 ga10b_runlist_length_max(struct gk20a *g);
void ga10b_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid);
int ga10b_runlist_check_pending(struct gk20a *g, struct nvgpu_runlist *runlist);
u32 ga10b_sched_disable_reg_off(void);
void ga10b_runlist_write_state(struct gk20a *g, u32 runlists_mask,
				u32 runlist_state);
u32 ga10b_get_runlist_aperture(struct gk20a *g, struct nvgpu_mem *mem);
#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
int ga10b_fifo_reschedule_preempt_next(struct nvgpu_channel *ch,
		bool wait_preempt);
#endif

#endif /* NVGPU_RUNLIST_FIFO_GA10B_H */
