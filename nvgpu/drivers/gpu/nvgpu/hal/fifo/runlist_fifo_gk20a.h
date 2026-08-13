/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_FIFO_GK20A_H
#define NVGPU_RUNLIST_FIFO_GK20A_H

#include <nvgpu/types.h>

struct nvgpu_channel;
struct nvgpu_tsg;
struct gk20a;
struct nvgpu_runlist;
enum nvgpu_aperture;

#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
int gk20a_runlist_reschedule(struct nvgpu_channel *ch, bool preempt_next);
int gk20a_fifo_reschedule_preempt_next(struct nvgpu_channel *ch,
		bool wait_preempt);
#endif
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
u32 gk20a_runlist_count_max(struct gk20a *g);
#endif

u32 gk20a_runlist_length_max(struct gk20a *g);
void gk20a_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid);
int gk20a_runlist_check_pending(struct gk20a *g, struct nvgpu_runlist *runlist);
void gk20a_runlist_write_state(struct gk20a *g, u32 runlists_mask,
		u32 runlist_state);

#endif /* NVGPU_RUNLIST_FIFO_GK20A_H */
