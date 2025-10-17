/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TU104_PERF
#define NVGPU_TU104_PERF

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

const u32 *tu104_perf_get_hwpm_sys_perfmon_regs(u32 *count);
const u32 *tu104_perf_get_hwpm_gpc_perfmon_regs(u32 *count);
const u32 *tu104_perf_get_hwpm_fbp_perfmon_regs(u32 *count);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
