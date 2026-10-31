/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TSG_VGPU_H
#define NVGPU_TSG_VGPU_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_tsg;

int vgpu_tsg_open(struct nvgpu_tsg *tsg);
void vgpu_tsg_release(struct nvgpu_tsg *tsg);
void vgpu_tsg_enable(struct nvgpu_tsg *tsg);
void vgpu_tsg_disable(struct nvgpu_tsg *tsg);
int vgpu_tsg_bind_channel(struct nvgpu_tsg *tsg, struct nvgpu_channel *ch);
int vgpu_tsg_unbind_channel(struct nvgpu_tsg *tsg, struct nvgpu_channel *ch);
int vgpu_tsg_set_timeslice(struct nvgpu_tsg *tsg, u32 timeslice);
int vgpu_set_sm_exception_type_mask(struct nvgpu_channel *ch,
		u32 exception_mask);
int vgpu_tsg_set_interleave(struct nvgpu_tsg *tsg, u32 new_level);
int vgpu_tsg_force_reset_ch(struct nvgpu_channel *ch,
					u32 err_code, bool verbose);
u32 vgpu_tsg_default_timeslice_us(struct gk20a *g);
void vgpu_tsg_set_ctx_mmu_error(struct gk20a *g, u32 chid);
void vgpu_tsg_handle_event(struct gk20a *g,
			struct tegra_vgpu_channel_event_info *info);
void vgpu_tsg_set_error_notifier(struct gk20a *g,
				struct tegra_vgpu_tsg_set_error_notifier *p);

#endif
