/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GA100_PERF
#define NVGPU_GA100_PERF

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;

u32 ga100_perf_get_pmmsys_per_chiplet_offset(void);
u32 ga100_perf_get_pmmgpc_per_chiplet_offset(void);
u32 ga100_perf_get_pmmfbp_per_chiplet_offset(void);

const u32 *ga100_perf_get_hwpm_sys_perfmon_regs(u32 *count);
const u32 *ga100_perf_get_hwpm_gpc_perfmon_regs(u32 *count);
const u32 *ga100_perf_get_hwpm_fbp_perfmon_regs(u32 *count);

void ga100_perf_get_num_hwpm_perfmon(struct gk20a *g, u32 *num_sys_perfmon,
		u32 *num_fbp_perfmon, u32 *num_gpc_perfmon);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
