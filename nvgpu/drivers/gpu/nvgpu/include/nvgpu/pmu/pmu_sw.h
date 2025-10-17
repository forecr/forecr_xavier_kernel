/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_PMU_SW_H
#define NVGPU_PMU_PMU_SW_H

struct gk20a;
struct nvgpu_pmu;

void nvgpu_pmu_sw_init(struct gk20a *g, struct nvgpu_pmu *pmu);

#endif /* NVGPU_PMU_PMU_SW_H */
