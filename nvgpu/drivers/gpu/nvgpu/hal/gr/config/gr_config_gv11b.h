/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_CONFIG_GV11B_H
#define NVGPU_GR_CONFIG_GV11B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;

u32 gv11b_gr_config_get_gpc_pes_mask(struct gk20a *g,
		struct nvgpu_gr_config *config, u32 gpc_index);
void gv11b_gr_config_set_live_pes_mask(struct gk20a *g,
		u32 gpc_count);
#endif /* NVGPU_GR_CONFIG_GV11B_H */
