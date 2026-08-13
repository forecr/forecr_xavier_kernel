// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/utils.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/static_analysis.h>

#include "ctxsw_prog_gb10b.h"

#include <nvgpu/hw/gb10b/hw_ctxsw_prog_gb10b.h>
#include <nvgpu/hw/gb10b/hw_xbar_gb10b.h>

#ifdef CONFIG_NVGPU_FECS_TRACE
u32 gb10b_ctxsw_prog_hw_get_ts_tag_invalid_timestamp(void)
{
	return ctxsw_prog_record_timestamp_timestamp_hi_tag_invalid_timestamp_v();
}

u32 gb10b_ctxsw_prog_hw_get_ts_tag(u64 ts)
{
	return ctxsw_prog_record_timestamp_timestamp_hi_tag_v(
		nvgpu_safe_cast_u64_to_u32(ts >> 32));
}

u64 gb10b_ctxsw_prog_hw_record_ts_timestamp(u64 ts)
{
	return ts &
	       ~(((u64)ctxsw_prog_record_timestamp_timestamp_hi_tag_m()) << 32);
}
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
u32 gb10b_ctxsw_prog_hw_get_pm_gpc_gnic_stride(struct gk20a *g)
{
	(void)g;
	return (xbar_mxbar_pri_gpc1_gnic0_preg_pm_ctrl_r() -
			xbar_mxbar_pri_gpc0_gnic0_preg_pm_ctrl_r());
}
#endif
