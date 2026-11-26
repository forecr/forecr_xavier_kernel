/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef FIFO_PREEMPT_GK20A_H
#define FIFO_PREEMPT_GK20A_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_tsg;

void gk20a_fifo_preempt_trigger(struct gk20a *g,
		u32 runlist_id, u32 tsgid, unsigned int id_type);
int  gk20a_fifo_preempt_channel(struct gk20a *g, struct nvgpu_channel *ch);
int  gk20a_fifo_preempt_tsg(struct gk20a *g, u32 runlist_id, u32 tsgid);
int  gk20a_fifo_is_preempt_pending(struct gk20a *g, u32 runlist_id, u32 id,
			unsigned int id_type, bool preempt_retries_left);

#endif /* FIFO_PREEMPT_GK20A_H */
