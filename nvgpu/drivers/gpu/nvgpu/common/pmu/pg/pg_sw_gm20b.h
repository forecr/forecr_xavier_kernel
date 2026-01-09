/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_PG_SW_GM20B_H
#define NVGPU_PMU_PG_SW_GM20B_H

#include <nvgpu/types.h>

struct gk20a;
struct pmu_pg_stats_data;

#define ZBC_MASK(i)			U16(~(~(0U) << ((i)+1U)) & 0xfffeU)

u32 gm20b_pmu_pg_engines_list(struct gk20a *g);
u32 gm20b_pmu_pg_feature_list(struct gk20a *g, u32 pg_engine_id);
void gm20b_pmu_save_zbc(struct gk20a *g, u32 entries);
int gm20b_pmu_elpg_statistics(struct gk20a *g, u32 pg_engine_id,
	struct pmu_pg_stats_data *pg_stat_data);
void nvgpu_gm20b_pg_sw_init(struct gk20a *g,
		struct nvgpu_pmu_pg *pg);
int gm20b_pmu_pg_elpg_allow(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int gm20b_pmu_pg_elpg_disallow(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int gm20b_pmu_pg_elpg_init(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int gm20b_pmu_pg_elpg_alloc_dmem(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int gm20b_pmu_pg_elpg_load_buff(struct gk20a *g, struct nvgpu_pmu *pmu);
int gm20b_pmu_pg_elpg_hw_load_zbc(struct gk20a *g, struct nvgpu_pmu *pmu);
int gm20b_pmu_pg_init_send(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id);
int gm20b_pmu_pg_aelpg_init(struct gk20a *g);
int gm20b_pmu_pg_aelpg_init_and_enable(struct gk20a *g, u8 ctrl_id);
int gm20b_pmu_pg_aelpg_enable(struct gk20a *g, u8 ctrl_id);
int gm20b_pmu_pg_aelpg_disable(struct gk20a *g, u8 ctrl_id);
u32 gm20b_pmu_get_pg_buf_gpu_va(struct gk20a *g, struct nvgpu_pmu *pmu,
		u32 buf_id);
#endif /* NVGPU_PMU_PG_SW_GM20B_H */
