// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/utils.h>
#include <nvgpu/nvgpu_mem.h>

#include "ctxsw_prog_gm20b.h"
#include "ctxsw_prog_gp10b.h"

#include <nvgpu/hw/gp10b/hw_ctxsw_prog_gp10b.h>

#ifdef CONFIG_NVGPU_CILP
void gp10b_ctxsw_prog_set_compute_preemption_mode_cilp(struct gk20a *g,
	struct nvgpu_mem *ctx_mem)
{
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_compute_preemption_options_o(),
		ctxsw_prog_main_image_compute_preemption_options_control_cilp_f());
}
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
void gp10b_ctxsw_prog_set_pmu_options_boost_clock_frequencies(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u32 boosted_ctx)
{
	u32 data = ctxsw_prog_main_image_pmu_options_boost_clock_frequencies_f(boosted_ctx);

	nvgpu_mem_wr(g, ctx_mem, ctxsw_prog_main_image_pmu_options_o(), data);
}
#endif /* CONFIG_NVGPU_DEBUGGER */

#ifdef CONFIG_DEBUG_FS
void gp10b_ctxsw_prog_dump_ctxsw_stats(struct gk20a *g,
	struct nvgpu_mem *ctx_mem)
{
	nvgpu_err(g, "ctxsw_prog_main_image_magic_value_o : %x (expect %x)",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_magic_value_o()),
		ctxsw_prog_main_image_magic_value_v_value_v());

	nvgpu_err(g, "ctxsw_prog_main_image_context_timestamp_buffer_ptr_hi : %x",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_context_timestamp_buffer_ptr_hi_o()));

	nvgpu_err(g, "ctxsw_prog_main_image_context_timestamp_buffer_ptr : %x",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_context_timestamp_buffer_ptr_o()));

	nvgpu_err(g, "ctxsw_prog_main_image_context_timestamp_buffer_control : %x",
		nvgpu_mem_rd(g, ctx_mem,
				ctxsw_prog_main_image_context_timestamp_buffer_control_o()));

	nvgpu_err(g, "NUM_SAVE_OPERATIONS : %d",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_num_save_ops_o()));
	nvgpu_err(g, "WFI_SAVE_OPERATIONS : %d",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_num_wfi_save_ops_o()));
	nvgpu_err(g, "CTA_SAVE_OPERATIONS : %d",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_num_cta_save_ops_o()));
	nvgpu_err(g, "GFXP_SAVE_OPERATIONS : %d",
		nvgpu_mem_rd(g, ctx_mem,
			ctxsw_prog_main_image_num_gfxp_save_ops_o()));
	nvgpu_err(g, "CILP_SAVE_OPERATIONS : %d",
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

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gp10b_ctxsw_prog_init_ctxsw_hdr_data(struct gk20a *g,
	struct nvgpu_mem *ctx_mem)
{
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_num_wfi_save_ops_o(), 0);
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_num_cta_save_ops_o(), 0);
#ifdef CONFIG_NVGPU_GRAPHICS
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_num_gfxp_save_ops_o(), 0);
#endif
#ifdef CONFIG_NVGPU_CILP
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_num_cilp_save_ops_o(), 0);
#endif

	gm20b_ctxsw_prog_init_ctxsw_hdr_data(g, ctx_mem);
}
#endif
