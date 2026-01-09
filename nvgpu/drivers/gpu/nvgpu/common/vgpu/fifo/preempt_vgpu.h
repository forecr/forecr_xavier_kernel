/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PREEMPT_VGPU_H
#define NVGPU_PREEMPT_VGPU_H

struct gk20a;
struct nvgpu_channel;
struct nvgpu_tsg;

int vgpu_fifo_preempt_channel(struct gk20a *g, struct nvgpu_channel *ch);
int vgpu_fifo_preempt_tsg(struct gk20a *g, u32 runlist_id, u32 tsgid, u32 gfid);

#endif
