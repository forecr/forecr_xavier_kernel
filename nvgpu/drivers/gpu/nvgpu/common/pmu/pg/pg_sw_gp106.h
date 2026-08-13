/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PG_SW_GP106_H
#define NVGPU_PG_SW_GP106_H

#include <nvgpu/types.h>

struct gk20a;
struct pmu_pg_stats_data;

int gp106_pg_param_init(struct gk20a *g, u32 pg_engine_id);
int gp106_pmu_elpg_statistics(struct gk20a *g, u32 pg_engine_id,
		struct pmu_pg_stats_data *pg_stat_data);
u32 gp106_pmu_pg_engines_list(struct gk20a *g);
u32 gp106_pmu_pg_feature_list(struct gk20a *g, u32 pg_engine_id);
bool gp106_pmu_is_lpwr_feature_supported(struct gk20a *g, u32 feature_id);

#endif /* NVGPU_PG_SW_GP106_H */
