/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_PG_SW_GV11B_H
#define NVGPU_PMU_PG_SW_GV11B_H

#include <nvgpu/types.h>

struct gk20a;

int gv11b_pg_gr_init(struct gk20a *g, u32 pg_engine_id);
int gv11b_pg_set_subfeature_mask(struct gk20a *g, u32 pg_engine_id);
void nvgpu_gv11b_pg_sw_init(struct gk20a *g, struct nvgpu_pmu_pg *pg);

#endif /* NVGPU_PMU_PG_SW_GV11B_H */
