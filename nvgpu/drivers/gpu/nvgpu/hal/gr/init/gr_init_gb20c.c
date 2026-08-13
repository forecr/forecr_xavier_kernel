// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/hw/gb20c/hw_gr_gb20c.h>

#include "gr_init_gb20c.h"
#include <nvgpu/log.h>

#include <nvgpu/hw/gb20c/hw_gr_gb20c.h>
#include <nvgpu/hw/gb20c/hw_ltc_gb20c.h>

#ifdef CONFIG_NVGPU_GRAPHICS

u32 gb20c_gr_init_get_attrib_cb_gfxp_size(struct gk20a *g)
{
	(void)g;
	return gr_gpc0_ppc0_cbm_beta_cb_size_v_gfxp_v();
}

u32 gb20c_gr_init_get_ctx_spill_size(struct gk20a *g)
{
	(void)g;
	return  nvgpu_safe_mult_u32(
		  gr_gpc0_swdx_rm_spill_buffer_size_256b_default_v(),
		  gr_gpc0_swdx_rm_spill_buffer_size_256b_byte_granularity_v());
}

u32 gb20c_gpcs_rops_crop_debug1_off(void)
{
	return gr_pri_gpcs_rops_crop_debug1_r();
}

#endif

#ifdef CONFIG_NVGPU_SET_FALCON_ACCESS_MAP
void gb20c_gr_init_get_access_map(struct gk20a *g,
				   u32 **gr_access_map,
				   u32 *gr_access_map_num_entries)
{
	/* Update it based on last_index */
	static u32 gr_access_map_gb20c[40U];
	u32 last_index = 0U;
	u32 i;

	/* This list must be sorted (low to high in terms of address values) */
	gr_access_map_gb20c[last_index++] = ltc_ltcs_ltss_tstg_set_mgmt0_r();
	gr_access_map_gb20c[last_index++] = ltc_ltcs_ltss_tstg_set_mgmt_5_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_rasterarb_line_class_r();
	gr_access_map_gb20c[last_index++] =
		gr_pri_gpcs_setup_debug_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_setup_debug_z_gamut_offset_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_mmu_debug_ctrl_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_zcull_ctx_debug_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_swdx_config_r();

	if (gr_gpcs_pri_swdx_tc_bundle_ctrl__size_1_v() != 4U) {
		nvgpu_err(g, "Update the entries for "
				"NV_PGRAPH_PRI_GPCS_SWDX_TC_BUNDLE_CTRL");
	}
	for (i = 0U; i < 4U; ++i) {
		gr_access_map_gb20c[last_index++] =
			gr_gpcs_pri_swdx_tc_bundle_ctrl_r(i);
	}

	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_swdx_tc_bundle_lazyval_ctrl_r(0);

	if (gr_gpcs_pri_swdx_tc_bundle_addr__size_1_v() != 16U) {
		nvgpu_err(g, "Update the entries for "
				"NV_PGRAPH_PRI_GPCS_SWDX_TC_BUNDLE_ADDR");
	}
	for (i = 0U; i < 16U; ++i) {
		gr_access_map_gb20c[last_index++] =
			gr_gpcs_pri_swdx_tc_bundle_addr_r(i);
	}

	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_tpcs_pe_l2_evict_policy_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_tpcs_tex_lod_dbg_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_tpcs_tex_samp_dbg_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_tpcs_sm_context_save_addr_lo_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_tpcs_sm_context_save_addr_hi_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_tpcs_sm_machine_id0_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_tpcs_sm_sch_macro_sched_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_tpcs_sm_disp_ctrl_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_tpcs_sms_dbgr_control0_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_tpcs_sms_hww_warp_esr_report_mask_r();
	gr_access_map_gb20c[last_index++] =
		gr_gpcs_pri_tpcs_sms_hww_global_esr_report_mask_r();

	(void)g;
	*gr_access_map = gr_access_map_gb20c;
	*gr_access_map_num_entries = last_index;
}
#endif
