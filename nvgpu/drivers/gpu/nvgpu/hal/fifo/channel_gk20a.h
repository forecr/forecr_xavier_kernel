/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef FIFO_CHANNEL_GK20A_H
#define FIFO_CHANNEL_GK20A_H

struct gk20a;
struct nvgpu_channel;
struct nvgpu_channel_hw_state;
struct nvgpu_debug_context;
struct nvgpu_channel_dump_info;

void gk20a_channel_enable(struct gk20a *g, u32 runlist_id, u32 chid);
void gk20a_channel_disable(struct gk20a *g, u32 runlist_id, u32 chid);
void gk20a_channel_read_state(struct gk20a *g, u32 runlist_id, u32 chid,
		struct nvgpu_channel_hw_state *state);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gk20a_channel_debug_dump(struct gk20a *g,
	     struct nvgpu_debug_context *o,
	     struct nvgpu_channel_dump_info *info);
void gk20a_channel_unbind(struct nvgpu_channel *ch);
#endif

#endif /* FIFO_CHANNEL_GK20A_H */
