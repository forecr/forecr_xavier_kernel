/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CHANNEL_VGPU_H
#define NVGPU_CHANNEL_VGPU_H

struct gk20a;
struct nvgpu_channel;
u32 vgpu_channel_count(struct gk20a *g);
void vgpu_channel_set_ctx_mmu_error(struct gk20a *g, struct nvgpu_channel *ch);
void vgpu_channel_set_error_notifier(struct gk20a *g,
			struct tegra_vgpu_channel_set_error_notifier *p);
void vgpu_channel_abort_cleanup(struct gk20a *g, u32 chid);
int vgpu_channel_get_vmid(struct nvgpu_channel *ch, u32 *vmid);

#endif
