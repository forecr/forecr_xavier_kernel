/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef FIFO_CHANNEL_GV11B_H
#define FIFO_CHANNEL_GV11B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_channel_hw_state;
struct nvgpu_debug_context;

void gv11b_channel_bind(struct nvgpu_channel *ch);
void gv11b_channel_unbind(struct nvgpu_channel *ch);
u32 gv11b_channel_count(struct gk20a *g);
void gv11b_channel_read_state(struct gk20a *g, u32 runlist_id, u32 chid,
		struct nvgpu_channel_hw_state *state);
void gv11b_channel_reset_faulted(struct gk20a *g, u32 runlist_id, u32 chid);
#endif /* FIFO_CHANNEL_GV11B_H */
