/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CTXSW_PROG_GB10B_H
#define NVGPU_CTXSW_PROG_GB10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

#ifdef CONFIG_NVGPU_FECS_TRACE
u32 gb10b_ctxsw_prog_hw_get_ts_tag_invalid_timestamp(void);
u32 gb10b_ctxsw_prog_hw_get_ts_tag(u64 ts);
u64 gb10b_ctxsw_prog_hw_record_ts_timestamp(u64 ts);
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
u32 gb10b_ctxsw_prog_hw_get_pm_gpc_gnic_stride(struct gk20a *g);
u32 gb10b_ctxsw_prog_hw_get_gpccs_header_size(void);
u32 gb10b_ctxsw_prog_hw_get_gpccs_header_stride(void);
#endif /* CONFIG_NVGPU_DEBUGGER */

#endif /* NVGPU_CTXSW_PROG_GB10B_H */
