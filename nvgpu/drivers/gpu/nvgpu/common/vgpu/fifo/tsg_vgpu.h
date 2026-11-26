/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TSG_VGPU_H
#define NVGPU_TSG_VGPU_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_tsg;

int vgpu_tsg_force_reset_ch(struct nvgpu_channel *ch,
					u32 err_code, bool verbose);
void vgpu_tsg_handle_event(struct gk20a *g,
			struct tegra_vgpu_channel_event_info *info);
void vgpu_tsg_set_error_notifier(struct gk20a *g,
				struct tegra_vgpu_tsg_set_error_notifier *p);

#endif
