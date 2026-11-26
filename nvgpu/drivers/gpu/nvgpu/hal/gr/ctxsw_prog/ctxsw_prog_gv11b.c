// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/utils.h>
#include <nvgpu/nvgpu_mem.h>

#include "ctxsw_prog_gm20b.h"
#include "ctxsw_prog_gv11b.h"

#include <nvgpu/hw/gv11b/hw_ctxsw_prog_gv11b.h>

#ifdef CONFIG_NVGPU_DEBUGGER
void gv11b_ctxsw_prog_set_pm_ptr(struct gk20a *g, struct nvgpu_mem *ctx_mem,
	u64 addr)
{
	addr = addr >> 8;
	nvgpu_mem_wr(g, ctx_mem, ctxsw_prog_main_image_pm_ptr_o(),
		u64_lo32(addr));
	nvgpu_mem_wr(g, ctx_mem, ctxsw_prog_main_image_pm_ptr_hi_o(),
		u64_hi32(addr));
}

u32 gv11b_ctxsw_prog_hw_get_pm_mode_stream_out_ctxsw(void)
{
	return ctxsw_prog_main_image_pm_mode_stream_out_ctxsw_f();
}

u32 gv11b_ctxsw_prog_hw_get_perf_counter_register_stride(void)
{
	return ctxsw_prog_extended_sm_dsm_perf_counter_register_stride_v();
}
#endif /* CONFIG_NVGPU_DEBUGGER */
