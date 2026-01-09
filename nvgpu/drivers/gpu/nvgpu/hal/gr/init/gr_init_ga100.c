// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/io.h>

#include "gr_init_ga100.h"

#include <nvgpu/hw/ga100/hw_gr_ga100.h>

#ifdef CONFIG_NVGPU_GRAPHICS
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
u32 ga100_gr_init_get_attrib_cb_gfxp_default_size(struct gk20a *g)
{
	(void)g;
	return gr_gpc0_ppc0_cbm_beta_cb_size_v_gfxp_v();
}

NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 5_1), "SWE-DRC-502-SWSADP-M5.dox")
u32 ga100_gr_init_get_attrib_cb_gfxp_size(struct gk20a *g)
{
	(void)g;
	return gr_gpc0_ppc0_cbm_beta_cb_size_v_gfxp_v();
}

u32 ga100_gr_init_get_ctx_spill_size(struct gk20a *g)
{
	(void)g;
	return  nvgpu_safe_mult_u32(
		  gr_gpc0_swdx_rm_spill_buffer_size_256b_default_v(),
		  gr_gpc0_swdx_rm_spill_buffer_size_256b_byte_granularity_v());
}

u32 ga100_gr_init_get_ctx_betacb_size(struct gk20a *g)
{
	return nvgpu_safe_add_u32(
		g->ops.gr.init.get_attrib_cb_default_size(g),
		nvgpu_safe_sub_u32(
			gr_gpc0_ppc0_cbm_beta_cb_size_v_gfxp_v(),
			gr_gpc0_ppc0_cbm_beta_cb_size_v_default_v()));
}
#endif
#endif

void ga100_gr_init_set_sm_l1tag_surface_collector(struct gk20a *g)
{
	u32 reg_val = 0U;
	reg_val = nvgpu_readl(g, gr_gpc0_tpc0_sm_l1tag_ctrl_r());
	reg_val |= gr_gpcs_tpcs_sm_l1tag_ctrl_surface_cut_collector_enable_f();
	nvgpu_writel(g, gr_gpcs_tpcs_sm_l1tag_ctrl_r(), reg_val);
}
