/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_DEBUG_H
#define NVGPU_PMU_DEBUG_H

#include <nvgpu/types.h>

struct nvgpu_pmu;
struct gk20a;

/* PMU debug */
void nvgpu_pmu_dump_falcon_stats(struct nvgpu_pmu *pmu);
bool nvgpu_find_hex_in_string(char *strings, struct gk20a *g, u32 *hex_pos);
int nvgpu_pmu_debug_init(struct gk20a *g, struct nvgpu_pmu *pmu);
void nvgpu_pmu_debug_deinit(struct gk20a *g, struct nvgpu_pmu *pmu);

#endif /* NVGPU_PMU_DEBUG_H */
