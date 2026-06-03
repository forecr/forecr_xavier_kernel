/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef FIFO_CHANNEL_GA10B_H
#define FIFO_CHANNEL_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_channel_hw_state;

u32 ga10b_channel_count(struct gk20a *g);
void ga10b_channel_enable(struct gk20a *g, u32 runlist_id, u32 chid);
void ga10b_channel_disable(struct gk20a *g, u32 runlist_id, u32 chid);
void ga10b_channel_bind(struct nvgpu_channel *ch);
void ga10b_channel_unbind(struct nvgpu_channel *ch);
void ga10b_channel_clear(struct gk20a *g, u32 runlist_id, u32 chid);
void ga10b_channel_read_state(struct gk20a *g, u32 runlist_id, u32 chid,
					struct nvgpu_channel_hw_state *state);
void ga10b_channel_reset_faulted(struct gk20a *g, u32 runlist_id, u32 chid);
void ga10b_channel_force_ctx_reload(struct gk20a *g, u32 runlist_id, u32 chid);

#endif /* FIFO_CHANNEL_GA10B_H */
