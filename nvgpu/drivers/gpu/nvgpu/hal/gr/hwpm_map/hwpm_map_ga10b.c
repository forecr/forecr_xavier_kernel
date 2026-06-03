// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/types.h>
#include <nvgpu/bitops.h>

#include <nvgpu/utils.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gops/debugger.h>

#include "hwpm_map_ga10b.h"

int ga10b_hwpm_map_add_reg_in_fecs_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		u32 *count, u32 *offset, u32 max_reg_count)
{
	int ret = 0;
	u32 active_fbpa_mask;
	u32 current_gr_instance_id = nvgpu_gr_get_cur_instance_id(g);
	u32 num_fbps = nvgpu_grmgr_get_gr_num_fbps(g, current_gr_instance_id);
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 num_fbpas = nvgpu_get_litter_value(g, GPU_LIT_NUM_FBPAS);
	u32 fbpa_stride = nvgpu_get_litter_value(g, GPU_LIT_FBPA_STRIDE);
	u32 num_ltc = g->ops.top.get_max_ltc_per_fbp(g) *
		      g->ops.priv_ring.get_fbp_count(g);

	/* Add entries from _LIST_pm_ctx_reg_SYS */
	if (nvgpu_netlist_get_pm_sys_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries_pmsys(map,
			nvgpu_netlist_get_pm_sys_ctxsw_regs(g),
			count, offset, max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	/* Add entries from _LIST_nv_perf_ctx_reg_SYS */
	if (nvgpu_netlist_get_perf_sys_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_perf_sys_ctxsw_regs(g),
			count, offset, max_reg_count, 0, ~U32(0U)) ;
		if (ret != 0)
			goto cleanup;
	}

	/* Add entries from _LIST_nv_perf_sysrouter_ctx_reg*/
	if (nvgpu_netlist_get_perf_sys_router_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_perf_sys_router_ctxsw_regs(g),
			count, offset, max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	/* Add entries from _LIST_nv_perf_sys_control_ctx_reg*/
	if (nvgpu_netlist_get_perf_sys_control_ctxsw_regs(g)->count > 0U) {
		*offset = NVGPU_ALIGN(*offset, 256U);

		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_perf_sys_control_ctxsw_regs(g),
			count, offset,
			max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	if (g->ops.gr.hwpm_map.align_regs_perf_pma)
		g->ops.gr.hwpm_map.align_regs_perf_pma(offset);

	/* Add entries from _LIST_nv_perf_pma_ctx_reg*/
	if (nvgpu_netlist_get_perf_pma_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_perf_pma_ctxsw_regs(g), count, offset,
				max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	*offset = NVGPU_ALIGN(*offset, 256U);

	/* Add entries from _LIST_nv_perf_pma_control_ctx_reg*/
	if (nvgpu_netlist_get_perf_pma_control_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_perf_pma_control_ctxsw_regs(g), count, offset,
				max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	*offset = NVGPU_ALIGN(*offset, 256U);

	/* Add entries from _LIST_nv_perf_fbp_ctx_regs */
	if (nvgpu_netlist_get_fbp_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries_subunits(map,
			nvgpu_netlist_get_fbp_ctxsw_regs(g), count, offset,
				max_reg_count, 0, num_fbps, ~U32(0U),
				g->ops.perf.get_pmmfbp_per_chiplet_offset(),
				~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	/* Add entries from _LIST_nv_perf_fbprouter_ctx_regs */
	if (nvgpu_netlist_get_fbp_router_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries_subunits(map,
				nvgpu_netlist_get_fbp_router_ctxsw_regs(g),
				count, offset, max_reg_count, 0,
				num_fbps, ~U32(0U), g->ops.perf.get_pmmfbprouter_per_chiplet_offset(),
				~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	if (g->ops.gr.hwpm_map.get_active_fbpa_mask)
		active_fbpa_mask = g->ops.gr.hwpm_map.get_active_fbpa_mask(g);
	else
		active_fbpa_mask = ~U32(0U);

	/* Add entries from _LIST_nv_pm_fbpa_ctx_regs */
	if (nvgpu_netlist_get_pm_fbpa_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries_subunits(map,
				nvgpu_netlist_get_pm_fbpa_ctxsw_regs(g),
				count, offset, max_reg_count, 0,
				num_fbpas, active_fbpa_mask, fbpa_stride, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	/* Add entries from _LIST_nv_pm_rop_ctx_regs */
	if (nvgpu_netlist_get_pm_rop_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_pm_rop_ctxsw_regs(g), count, offset,
				max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	/* Add entries from _LIST_compressed_nv_pm_ltc_ctx_regs */
	if (nvgpu_netlist_get_pm_ltc_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries_subunits(map,
				nvgpu_netlist_get_pm_ltc_ctxsw_regs(g), count, offset,
				max_reg_count, 0, num_ltc, ~U32(0U),
				ltc_stride, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	if (g->ops.gr.hwpm_map.add_ctxsw_buffer_lrcc_entries_to_map != NULL) {
		if (nvgpu_netlist_get_pm_lrcc_ctxsw_regs(g)->count > 0U) {
			ret = g->ops.gr.hwpm_map.add_ctxsw_buffer_lrcc_entries_to_map(g,
					map, nvgpu_netlist_get_pm_lrcc_ctxsw_regs(g),
					count, offset,
					max_reg_count, num_ltc);
			if (ret != 0)
				goto cleanup;
		}
	}

	*offset = NVGPU_ALIGN(*offset, 256U);

	/* Add entries from _LIST_nv_perf_fbp_control_ctx_regs */
	if (nvgpu_netlist_get_perf_fbp_control_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries_subunits(map,
				nvgpu_netlist_get_perf_fbp_control_ctxsw_regs(g),
				count, offset, max_reg_count, 0,
				num_fbps, ~U32(0U),
				g->ops.perf.get_pmmfbp_per_chiplet_offset(),
				~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

cleanup:
	return ret;
}

int ga10b_hwpm_map_add_reg_in_gpccs_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		u32 *count, u32 *offset, u32 max_reg_count,
		struct nvgpu_gr_config *config)
{
	return add_ctxsw_buffer_map_entries_gpcs(g, map, count, offset,
			max_reg_count, config);
}
