/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_FIFO_GV11B_H
#define NVGPU_RUNLIST_FIFO_GV11B_H

#include <nvgpu/types.h>

struct nvgpu_channel;
struct gk20a;

#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
int gv11b_runlist_reschedule(struct nvgpu_channel *ch, bool preempt_next);
#endif
u32 gv11b_runlist_count_max(struct gk20a *g);

#endif /* NVGPU_RUNLIST_FIFO_GV11B_H */
