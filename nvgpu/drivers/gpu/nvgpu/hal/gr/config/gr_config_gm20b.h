/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_CONFIG_GM20B_H
#define NVGPU_GR_CONFIG_GM20B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;

u32 gm20b_gr_config_get_gpc_tpc_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index);
u32 gm20b_gr_config_get_tpc_count_in_gpc(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index);
u32 gm20b_gr_config_get_pes_tpc_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index, u32 pes_index);
u32 gm20b_gr_config_get_pd_dist_skip_table_size(void);
u32 gm20b_gr_config_get_gpc_mask(struct gk20a *g);
#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
int gm20b_gr_config_init_sm_id_table(struct gk20a *g,
	struct nvgpu_gr_config *gr_config);
#endif /* CONFIG_NVGPU_HAL_NON_FUSA */
#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
u32 gm20b_gr_config_get_zcull_count_in_gpc(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index);
#endif /* CONFIG_NVGPU_GRAPHICS and CONFIG_NVGPU_ZCULL */
#endif /* NVGPU_GR_CONFIG_GM20B_H */
