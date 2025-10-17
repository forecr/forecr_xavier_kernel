// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/log.h>
#include <nvgpu/bug.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/enabled.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/ctx_mappings.h>

#include "gr_init_gb10b.h"

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>
#include <nvgpu/hw/gb10b/hw_ltc_gb10b.h>

#ifdef CONFIG_NVGPU_GRAPHICS
u32 gb10b_gr_init_get_attrib_cb_gfxp_size(struct gk20a *g)
{
	(void)g;
	return gr_gpc0_ppc0_cbm_beta_cb_size_v_gfxp_v();
}

u32 gb10b_gr_init_get_ctx_spill_size(struct gk20a *g)
{
	(void)g;
	return  nvgpu_safe_mult_u32(
		  gr_gpc0_swdx_rm_spill_buffer_size_256b_default_v(),
		  gr_gpc0_swdx_rm_spill_buffer_size_256b_byte_granularity_v());
}

u32 gb10b_gr_init_get_ctx_betacb_size(struct gk20a *g)
{
	u32 cb_default_size, cb_gfxp_default_size;

	cb_default_size = g->ops.gr.init.get_attrib_cb_default_size(g);
	cb_gfxp_default_size = g->ops.gr.init.get_attrib_cb_gfxp_default_size(g);

	return nvgpu_safe_add_u32(cb_default_size,
		nvgpu_safe_sub_u32(cb_gfxp_default_size, cb_default_size));
}

static void gb10b_gr_init_patch_rtv_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	u32 addr, u32 size, u32 gfxpAddSize, bool patch)
{
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_scc_rm_rtv_cb_base_r(),
		gr_scc_rm_rtv_cb_base_addr_39_8_f(addr), patch);
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_scc_rm_rtv_cb_size_r(),
		gr_scc_rm_rtv_cb_size_div_256b_f(size), patch);
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_gpcs_gcc_rm_rtv_cb_base_r(),
		gr_gpcs_gcc_rm_rtv_cb_base_addr_39_8_f(addr), patch);
	nvgpu_gr_ctx_patch_write(g, gr_ctx, gr_scc_rm_gfxp_reserve_r(),
		gr_scc_rm_gfxp_reserve_rtv_cb_size_div_256b_f(gfxpAddSize),
		patch);
}

void gb10b_gr_init_commit_rtv_cb(struct gk20a *g, u64 addr,
	struct nvgpu_gr_ctx *gr_ctx, bool patch)
{
	u32 size = nvgpu_safe_add_u32(
			gr_scc_rm_rtv_cb_size_div_256b_default_f(),
			gr_scc_rm_rtv_cb_size_div_256b_db_adder_f());

	addr = addr >> gr_scc_rm_rtv_cb_base_addr_39_8_align_bits_f();

	nvgpu_assert(u64_hi32(addr) == 0U);
	gb10b_gr_init_patch_rtv_cb(g, gr_ctx, (u32)addr, size, 0, patch);
}

u32 gb10b_gpcs_rops_crop_debug1_off(void)
{
	return gr_pri_gpcs_rops_crop_debug1_r();
}

void gb10b_gr_init_commit_rops_crop_override(struct gk20a *g,
				struct nvgpu_gr_ctx *gr_ctx, bool patch)
{
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_EMULATE_MODE) &&
			(g->emulate_mode > 0U)) {
		u32 data = 0U;
		u32 reg_off = g->ops.gr.init.get_gpcs_rops_crop_debug1_off();

		data = nvgpu_readl(g, reg_off);
		data = set_field(data,
			gr_pri_gpcs_rops_crop_debug1_crd_cond_read_m(),
			gr_pri_gpcs_rops_crop_debug1_crd_cond_read_disable_f());
		nvgpu_gr_ctx_patch_write(g, gr_ctx, reg_off, data, patch);
	}
}
#endif

void gb10b_gr_init_set_sm_l1tag_surface_collector(struct gk20a *g)
{
	u32 reg_val = 0U;
	reg_val = nvgpu_readl(g, gr_gpc0_tpc0_sm_l1tag_ctrl_r());
	reg_val |= gr_gpcs_tpcs_sm_l1tag_ctrl_surface_cut_collector_enable_f();
	nvgpu_writel(g, gr_gpcs_tpcs_sm_l1tag_ctrl_r(), reg_val);
}

#ifdef CONFIG_NVGPU_GFXP
void gb10b_gr_init_commit_gfxp_rtv_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, struct nvgpu_gr_ctx_mappings *mappings,
	bool patch)
{
	u64 addr;
	u64 gpu_va;
	u32 rtv_cb_size;
	u32 gfxp_addr_size;

	nvgpu_log_fn(g, " ");

	rtv_cb_size = nvgpu_safe_add_u32(
		nvgpu_safe_add_u32(
			gr_scc_rm_rtv_cb_size_div_256b_default_f(),
			gr_scc_rm_rtv_cb_size_div_256b_db_adder_f()),
		gr_scc_rm_rtv_cb_size_div_256b_gfxp_adder_f());
	gfxp_addr_size = gr_scc_rm_rtv_cb_size_div_256b_gfxp_adder_f();

	/* GFXP RTV circular buffer */
	gpu_va = nvgpu_gr_ctx_mappings_get_ctx_va(mappings, NVGPU_GR_CTX_GFXP_RTVCB_CTXSW);
	addr = gpu_va >> gr_scc_rm_rtv_cb_base_addr_39_8_align_bits_f();

	nvgpu_assert(u64_hi32(addr) == 0U);
	gb10b_gr_init_patch_rtv_cb(g, gr_ctx, (u32)addr,
		rtv_cb_size, gfxp_addr_size, patch);
}
#endif

#ifdef CONFIG_NVGPU_SET_FALCON_ACCESS_MAP
void gb10b_gr_init_get_access_map(struct gk20a *g,
				   u32 **gr_access_map,
				   u32 *gr_access_map_num_entries)
{
	/* Update it based on last_index */
	static u32 gr_access_map_gb10b[37];
	u32 last_index = 0U;

	/* This list must be sorted (low to high in terms of address values) */
	gr_access_map_gb10b[last_index++] = ltc_ltcs_ltss_tstg_set_mgmt0_r();
	gr_access_map_gb10b[last_index++] = ltc_ltcs_ltss_tstg_set_mgmt_5_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_rasterarb_line_class_r();
	gr_access_map_gb10b[last_index++] =
		gr_pri_gpcs_setup_debug_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_setup_debug_z_gamut_offset_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_mmu_debug_ctrl_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_zcull_ctx_debug_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_config_r();

	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_ctrl_r(0);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_ctrl_r(1);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_ctrl_r(2);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_ctrl_r(3);

	if (gr_gpcs_pri_swdx_tc_bundle_ctrl__size_1_v() != 4) {
		nvgpu_err(g, "Update the entries for "
				"NV_PGRAPH_PRI_GPCS_SWDX_TC_BUNDLE_CTRL");
	}

	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_lazyval_ctrl_r(0);

	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(0);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(1);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(2);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(3);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(4);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(5);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(6);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(7);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(8);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(9);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(10);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(11);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(12);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(13);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(14);
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_addr_r(15);

	if (gr_gpcs_pri_swdx_tc_bundle_addr__size_1_v() != 16) {
		nvgpu_err(g, "Update the entries for "
				"NV_PGRAPH_PRI_GPCS_SWDX_TC_BUNDLE_ADDR");
	}

	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_tpcs_pe_l2_evict_policy_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_tpcs_tex_lod_dbg_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_tpcs_tex_samp_dbg_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_tpcs_sm_sch_macro_sched_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_tpcs_sm_disp_ctrl_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_tpcs_sms_dbgr_control0_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_tpcs_sms_hww_warp_esr_report_mask_r();
	gr_access_map_gb10b[last_index++] =
		gr_gpcs_pri_tpcs_sms_hww_global_esr_report_mask_r();

	(void)g;
	*gr_access_map = gr_access_map_gb10b;
	*gr_access_map_num_entries = last_index;
}
#endif
