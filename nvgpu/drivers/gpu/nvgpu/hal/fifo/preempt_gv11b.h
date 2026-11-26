/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef FIFO_PREEMPT_GV11B_H
#define FIFO_PREEMPT_GV11B_H

#include <nvgpu/types.h>

#define PREEMPT_PENDING_POLL_PRE_SI_RETRIES	200000U	/* 1G/500KHz * 100 */

struct gk20a;
struct nvgpu_channel;
struct nvgpu_tsg;

void gv11b_fifo_preempt_trigger(struct gk20a *g,
		u32 runlist_id, u32 id, unsigned int id_type);
int  gv11b_fifo_preempt_channel(struct gk20a *g, struct nvgpu_channel *ch);
int  gv11b_fifo_is_preempt_pending(struct gk20a *g, u32 runlist_id, u32 id,
			unsigned int id_type, bool preempt_retries_left);
int gv11b_fifo_preempt_poll_pbdma(struct gk20a *g, u32 tsgid, u32 pbdma_id);

#endif /* FIFO_PREEMPT_GV11B_H */
