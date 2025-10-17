/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_CONFIG_GB10B_H
#define NVGPU_GR_CONFIG_GB10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;

int gb10b_gr_config_init_sm_id_table(struct gk20a *g,
		struct nvgpu_gr_config *gr_config);
u32 gb10b_gr_config_get_pes_tpc_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index, u32 pes_index);
void gb10b_gr_config_set_live_pes_mask(struct gk20a *g,  u32 gpc_count);
#endif /* NVGPU_GR_CONFIG_GB10B_H */
