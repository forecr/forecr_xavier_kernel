// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/ctx.h>

#include "gr_init_gv100.h"

#include <nvgpu/hw/gv100/hw_gr_gv100.h>

u32 gv100_gr_init_get_bundle_cb_default_size(struct gk20a *g)
{
	return gr_scc_bundle_cb_size_div_256b__prod_v();
}

u32 gv100_gr_init_get_min_gpm_fifo_depth(struct gk20a *g)
{
	return gr_pd_ab_dist_cfg2_state_limit_min_gpm_fifo_depths_v();
}

u32 gv100_gr_init_get_bundle_cb_token_limit(struct gk20a *g)
{
	return gr_pd_ab_dist_cfg2_token_limit_init_v();
}

u32 gv100_gr_init_get_attrib_cb_default_size(struct gk20a *g)
{
	return gr_gpc0_ppc0_cbm_beta_cb_size_v_default_v();
}

u32 gv100_gr_init_get_alpha_cb_default_size(struct gk20a *g)
{
	return gr_gpc0_ppc0_cbm_alpha_cb_size_v_default_v();
}

u32 gv100_gr_init_get_attrib_cb_gfxp_default_size(struct gk20a *g)
{
	return gr_gpc0_ppc0_cbm_beta_cb_size_v_gfxp_v();
}

u32 gv100_gr_init_get_attrib_cb_gfxp_size(struct gk20a *g)
{
	return gr_gpc0_ppc0_cbm_beta_cb_size_v_gfxp_v();
}

u32 gv100_gr_init_get_ctx_spill_size(struct gk20a *g)
{
	return  nvgpu_safe_mult_u32(
		gr_gpc0_swdx_rm_spill_buffer_size_256b_default_v(),
		gr_gpc0_swdx_rm_spill_buffer_size_256b_byte_granularity_v());
}

u32 gv100_gr_init_get_ctx_betacb_size(struct gk20a *g)
{
	return nvgpu_safe_add_u32(
		g->ops.gr.init.get_attrib_cb_default_size(g),
		nvgpu_safe_sub_u32(
			gr_gpc0_ppc0_cbm_beta_cb_size_v_gfxp_v(),
			gr_gpc0_ppc0_cbm_beta_cb_size_v_default_v()));
}
