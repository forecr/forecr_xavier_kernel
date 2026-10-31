// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/utils.h>
#include <nvgpu/nvgpu_mem.h>

#include <nvgpu/hw/ga100/hw_ctxsw_prog_ga100.h>
#include <nvgpu/hw/ga100/hw_xbar_ga100.h>

#include "ctxsw_prog_ga100.h"

#ifdef CONFIG_DEBUG_FS
void ga100_ctxsw_prog_dump_ctxsw_stats(struct gk20a *g,
	struct nvgpu_mem *ctx_mem)
{
	nvgpu_err(g, "ctxsw_prog_main_image_magic_value_1_o: %x (expect %x)",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_magic_value_1_o()),
		ctxsw_prog_main_image_magic_value_1_v_value_v());

	nvgpu_err(g, "ctxsw_prog_main_image_magic_value_2_o: %x (expect %x)",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_magic_value_2_o()),
		ctxsw_prog_main_image_magic_value_2_v_value_v());

	nvgpu_err(g, "ctxsw_prog_main_image_magic_value_3_o: %x (expect %x)",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_magic_value_3_o()),
		ctxsw_prog_main_image_magic_value_3_v_value_v());

	nvgpu_err(g, "ctxsw_prog_main_image_context_timestamp_buffer_ptr_hi: %x",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_context_timestamp_buffer_ptr_hi_o()));

	nvgpu_err(g, "ctxsw_prog_main_image_context_timestamp_buffer_ptr: %x",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_context_timestamp_buffer_ptr_o()));

	nvgpu_err(g, "ctxsw_prog_main_image_context_timestamp_buffer_control: %x",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_context_timestamp_buffer_control_o()));

	nvgpu_err(g, "NUM_SAVE_OPERATIONS: %d",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_num_save_ops_o()));
	nvgpu_err(g, "WFI_SAVE_OPERATIONS: %d",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_num_wfi_save_ops_o()));
	nvgpu_err(g, "CTA_SAVE_OPERATIONS: %d",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_num_cta_save_ops_o()));
	nvgpu_err(g, "GFXP_SAVE_OPERATIONS: %d",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_num_gfxp_save_ops_o()));
	nvgpu_err(g, "CILP_SAVE_OPERATIONS: %d",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_num_cilp_save_ops_o()));
	nvgpu_err(g,
		"image gfx preemption option (GFXP is 1) %x",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_graphics_preemption_options_o()));
	nvgpu_err(g,
		"image compute preemption option (CTA is 1) %x",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_compute_preemption_options_o()));
}
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
u32 ga100_ctxsw_prog_hw_get_pm_gpc_gnic_stride(struct gk20a *g)
{
	(void)g;
	return (xbar_mxbar_pri_gpc1_gnic0_preg_pm_ctrl_r() -
			xbar_mxbar_pri_gpc0_gnic0_preg_pm_ctrl_r());
}
#endif
