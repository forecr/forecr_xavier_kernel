/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMU_THERM_H
#define NVGPU_PMU_THERM_H

struct gk20a;
struct nvgpu_pmu;

int nvgpu_pmu_therm_sw_setup(struct gk20a *g, struct nvgpu_pmu *pmu);
int nvgpu_pmu_therm_pmu_setup(struct gk20a *g, struct nvgpu_pmu *pmu);
int nvgpu_pmu_therm_init(struct gk20a *g, struct nvgpu_pmu *pmu);
void nvgpu_pmu_therm_deinit(struct gk20a *g, struct nvgpu_pmu *pmu);
int nvgpu_pmu_therm_channel_get_curr_temp(struct gk20a *g, u32 *temp);

#endif /* NVGPU_PMU_THREM_H */
