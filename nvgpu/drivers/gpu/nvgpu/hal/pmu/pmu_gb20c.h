/* SPDX-License-Identifier: GPL-2.0-only OR MIT */
/* SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved. */

#ifndef NVGPU_PMU_GB20C_H
#define NVGPU_PMU_GB20C_H

struct gk20a;
struct nvgpu_pmu;

int gb20c_pmu_elcg_sub_feature_id_to_engine_id(struct gk20a *g,
								u32 sub_feature_id);
u32 gb20c_pmu_elcg_all_engine_mask(void);
u32 gb20c_pmu_elcg_mm_engine_mask(void);
u32 gb20c_pmu_fgpg_get_all_engine_mask(void);

#endif /* NVGPU_PMU_GB20C_H */
