/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMU_PSTATE_H
#define NVGPU_PMU_PSTATE_H

#include <nvgpu/types.h>

struct gk20a;

int nvgpu_pmu_pstate_sw_setup(struct gk20a *g);
void nvgpu_pmu_pstate_deinit(struct gk20a *g);
int nvgpu_pmu_pstate_pmu_setup(struct gk20a *g);

#endif /* NVGPU_PMU_PSTATE_H */
