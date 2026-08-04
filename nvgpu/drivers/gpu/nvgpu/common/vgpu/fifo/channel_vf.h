/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CHANNEL_VF_H
#define NVGPU_CHANNEL_VF_H

void vgpu_vf_channel_read_state(struct gk20a *g, u32 runlist_id, u32 chid,
			struct nvgpu_channel_hw_state *state);
void vgpu_vf_channel_enable(struct gk20a *g, u32 runlist_id, u32 chid);
void vgpu_vf_channel_disable(struct gk20a *g, u32 runlist_id, u32 chid);
void vgpu_vf_channel_clear(struct gk20a *g, u32 runlist_id, u32 chid);
void vgpu_vf_channel_force_ctx_reload(struct gk20a *g, u32 runlist_id, u32 chid);
void vgpu_vf_channel_reset_faulted(struct gk20a *g, u32 runlist_id, u32 chid);

#endif
