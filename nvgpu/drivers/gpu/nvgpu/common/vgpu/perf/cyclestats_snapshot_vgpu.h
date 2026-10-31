/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CSS_VGPU_H
#define NVGPU_CSS_VGPU_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct gk20a_cs_snapshot_client;

int vgpu_css_init(struct gk20a *g);
void vgpu_css_release_snapshot_buffer(struct gk20a *g);
int vgpu_css_flush_snapshots(struct nvgpu_channel *ch,
			u32 *pending, bool *hw_overflow);
int vgpu_css_detach(struct gk20a *g,
		struct gk20a_cs_snapshot_client *cs_client);
int vgpu_css_enable_snapshot_buffer(struct gk20a *g,
				struct gk20a_cs_snapshot_client *cs_client);
u32 vgpu_css_get_buffer_size(struct gk20a *g);
#endif /* NVGPU_CSS_VGPU_H */
