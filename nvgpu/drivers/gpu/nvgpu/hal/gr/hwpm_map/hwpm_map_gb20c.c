// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/types.h>
#include <nvgpu/bitops.h>

#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gops/debugger.h>
#include <nvgpu/ltc.h>
#include <nvgpu/device.h>
#include <common/gr/gr_priv.h>
#include <common/gr/gr_config_priv.h>

#include <nvgpu/hw/gb20c/hw_xbar_gb20c.h>

#include "hwpm_map_gb20c.h"

#define GNIC_CNT_PER_CPC	(2U)

static int add_ctxsw_buffer_map_sys_perfmon_entries(struct gk20a *g,
			struct ctxsw_buf_offset_map_entry *map,
			struct netlist_aiv_list *regs, u32 *count, u32 *offset,
			u32 max_reg_count, u32 base)
{
	u32 idx, perfmon_stride, perfmon_idx, start_perfmon_idx, end_perfmon_idx;
	u32 cnt, off;

	cnt = *count;
	off = *offset;

	g->ops.perf.get_sys_perfmon_idx_range(&start_perfmon_idx, &end_perfmon_idx);
	perfmon_stride = g->ops.perf.get_sys_perfmon_stride();

	if ((cnt + (regs->count * end_perfmon_idx)) > max_reg_count)
		return -EINVAL;

	for (perfmon_idx = start_perfmon_idx; perfmon_idx <= end_perfmon_idx; perfmon_idx++) {
		for (idx = 0 ; idx < regs->count - 1U; idx++) {
			map[cnt].addr = regs->l[idx].addr + base + (perfmon_idx * perfmon_stride);
			map[cnt++].offset = off;
			off += 4U;
		}
	}
	map[cnt].addr = regs->l[regs->count - 1].addr;
	map[cnt++].offset = off;
	off += 4;

	*count = cnt;
	*offset = off;

	return 0;
}

static int add_ctxsw_buffer_map_entries_pm_sysltc_lists(struct gk20a *g,
				struct ctxsw_buf_offset_map_entry *map,
				struct netlist_aiv_list *regs,
				u32 *count, u32 *offset, u32 max_reg_count)
{
	u32 idx, i, base;
	const struct nvgpu_device *dev;
	u32 cnt = *count;
	u32 off = *offset;

	if ((cnt + regs->count) > max_reg_count)
		return -EINVAL;

	for (idx = 0 ; idx < regs->count - 1U; idx++) {
		for (i = 0; i < nvgpu_device_count(g, NVGPU_DEVTYPE_SYSLTC); i++) {
			dev = nvgpu_device_get(g, NVGPU_DEVTYPE_SYSLTC, i);
			nvgpu_assert(dev != NULL);
			base = dev->pri_base;
			map[cnt].addr = regs->l[idx].addr + base;
			map[cnt++].offset = off;
			off += 4U;
		}
	}

	map[cnt].addr = regs->l[regs->count - 1].addr;
	map[cnt++].offset = off;
	off += 4;

	*count = cnt;
	*offset = off;

	return 0;
}

static int add_ctxsw_buffer_map_entries_per_ltc_pmlists(struct gk20a *g,
				struct ctxsw_buf_offset_map_entry *map,
				struct netlist_aiv_list *regs,
				u32 *count, u32 *offset, u32 max_reg_count,
				u32 unit_stride, u32 sub_unit_stride)
{
	u32 cnt = *count;
	u32 off = *offset;
	u32 idx, ltc_idx, lts_mask, i;
	u32 available_ltc = nvgpu_ltc_get_ltc_count(g);
	u32 available_lts_per_ltc = nvgpu_ltc_get_slices_per_ltc(g);

	if ((cnt + (regs->count * available_ltc * available_lts_per_ltc)) > max_reg_count)
		return -EINVAL;

	lts_mask = BIT32(available_lts_per_ltc) - 1U;

	for (idx = 0; idx < regs->count - 1U; idx++) {
		for (ltc_idx = 0; ltc_idx < available_ltc; ltc_idx++) {
			for_each_set_bit(i, (unsigned long *)&lts_mask,
					(BITS_PER_BYTE * sizeof(lts_mask))) {
				map[cnt].addr = regs->l[idx].addr +
					(ltc_idx * unit_stride) + (i * sub_unit_stride);
				map[cnt++].offset = off;
				off += 4U;
			}
		}
	}

	map[cnt].addr = regs->l[regs->count - 1].addr;
	map[cnt++].offset = off;
	off += 4;

	*count = cnt;
	*offset = off;

	return 0;

}


int gb20c_hwpm_map_add_reg_in_fecs_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		u32 *count, u32 *offset, u32 max_reg_count)
{
	int ret = 0;
	u32 active_fbpa_mask;
	u32 current_gr_instance_id = nvgpu_gr_get_cur_instance_id(g);
	u32 num_fbps = nvgpu_grmgr_get_gr_num_fbps(g, current_gr_instance_id);
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);
	u32 lrc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRC_STRIDE);
	u32 num_fbpas = nvgpu_get_litter_value(g, GPU_LIT_NUM_FBPAS);
	u32 fbpa_stride = nvgpu_get_litter_value(g, GPU_LIT_FBPA_STRIDE);
	u32 num_ltc = g->ops.top.get_max_ltc_per_fbp(g) *
		      g->ops.priv_ring.get_fbp_count(g);
	u32 ctx_switched_pma_channels = g->num_ctx_switched_pma_channels;
	u32 router_channel_stride = g->ops.perf.get_sysrouter_channel_stride();
	u32 channel_stride = g->ops.perf.get_pmasys_channel_stride();
	u32 cmd_slice_stride = g->ops.perf.get_pmasys_cmdslice_stride();
	u32 fwd_channel_stride = g->ops.perf.get_pmasys_fwd_channel_stride();
	u32 i;

	/* Add entries from _LIST_pm_ctx_reg_SYS */
	if (nvgpu_netlist_get_pm_sys_ctxsw_regs(g)->count > 0) {
		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_pm_sys_ctxsw_regs(g),
			count, offset, max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	if (nvgpu_netlist_get_pm_sysltc_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries_pm_sysltc_lists(g, map,
		nvgpu_netlist_get_pm_sysltc_ctxsw_regs(g), count, offset,
		max_reg_count);
		if (ret != 0)
			goto cleanup;
	}

	/* Add entries from _LIST_nv_perf_ctx_reg_SYS */
	if (nvgpu_netlist_get_perf_sys_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_sys_perfmon_entries(g, map,
			nvgpu_netlist_get_perf_sys_ctxsw_regs(g), count, offset,
			max_reg_count, 0U);
		if (ret != 0)
			goto cleanup;
	}

	*offset = NVGPU_ALIGN(*offset, 256U);

	/* Add entries from _LIST_nv_perf_sysrouter_ctx_reg*/
	if (nvgpu_netlist_get_perf_sys_router_ctxsw_regs(g)->count > 0) {
		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_perf_sys_router_ctxsw_regs(g),
			count, offset, max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	*offset = NVGPU_ALIGN(*offset, 256U);

	for (i = 0; i < ctx_switched_pma_channels; i++) {
		if (nvgpu_netlist_get_perf_sys_router_user_channel_ctxsw_regs(g)->count > 0) {
			ret = add_ctxsw_buffer_map_entries(map,
				nvgpu_netlist_get_perf_sys_router_user_channel_ctxsw_regs(g),
				count, offset, max_reg_count, (i * router_channel_stride), ~U32(0U));
			if (ret != 0)
				goto cleanup;

			*offset = NVGPU_ALIGN(*offset, 256U);
		}
	}
	/* Add entries from _LIST_nv_perf_sys_control_ctx_reg*/
	if (nvgpu_netlist_get_perf_sys_control_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_sys_perfmon_entries(g, map,
			nvgpu_netlist_get_perf_sys_control_ctxsw_regs(g), count, offset,
			max_reg_count, 0U);
		if (ret != 0)
			goto cleanup;

		*offset = NVGPU_ALIGN(*offset, 256U);
	}

	if (g->ops.gr.hwpm_map.align_regs_perf_pma)
		g->ops.gr.hwpm_map.align_regs_perf_pma(offset);

	/* Add entries from _LIST_nv_perf_pma_ctx_reg*/
	if (nvgpu_netlist_get_perf_pma_ctxsw_regs(g)->count > 0) {
		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_perf_pma_ctxsw_regs(g), count, offset,
				max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	*offset = NVGPU_ALIGN(*offset, 256U);

	for (i = 0; i < ctx_switched_pma_channels; i++) {
		if (nvgpu_netlist_get_perf_pma_user_channel_ctxsw_regs(g)->count > 0) {
			ret = add_ctxsw_buffer_map_entries(map,
				nvgpu_netlist_get_perf_pma_user_channel_ctxsw_regs(g),
				count, offset, max_reg_count, (i * channel_stride), ~U32(0U));
			if (ret != 0)
				goto cleanup;
		}
		if (nvgpu_netlist_get_perf_pma_cmdslice_ctxsw_regs(g)->count > 0) {
			ret = add_ctxsw_buffer_map_entries(map,
				nvgpu_netlist_get_perf_pma_cmdslice_ctxsw_regs(g),
				count, offset, max_reg_count, (i * cmd_slice_stride), ~U32(0U));
			if (ret != 0)
				goto cleanup;
		}
		if (nvgpu_netlist_get_perf_pma_fwd_channel_ctxsw_regs(g)->count > 0) {
			ret = add_ctxsw_buffer_map_entries(map,
				nvgpu_netlist_get_perf_pma_fwd_channel_ctxsw_regs(g),
				count, offset, max_reg_count, (i * fwd_channel_stride), ~U32(0U));
			if (ret != 0)
				goto cleanup;
		}
		if ((nvgpu_netlist_get_perf_pma_user_channel_ctxsw_regs(g)->count > 0) ||
				(nvgpu_netlist_get_perf_pma_cmdslice_ctxsw_regs(g)->count > 0) ||
				(nvgpu_netlist_get_perf_pma_fwd_channel_ctxsw_regs(g)->count > 0)) {
			*offset = NVGPU_ALIGN(*offset, 256U);
		}
	}


	/* Add entries from _LIST_nv_perf_pma_control_ctx_reg*/
	if (nvgpu_netlist_get_perf_pma_control_ctxsw_regs(g)->count > 0) {
		ret = add_ctxsw_buffer_map_entries(map,
			nvgpu_netlist_get_perf_pma_control_ctxsw_regs(g), count, offset,
				max_reg_count, 0, ~U32(0U));
		if (ret != 0)
			goto cleanup;
	}

	*offset = NVGPU_ALIGN(*offset, 256U);

	for (i = 0; i < ctx_switched_pma_channels; i++) {
		if (nvgpu_netlist_get_perf_pma_user_channel_control_ctxsw_regs(g)->count > 0) {
			ret = add_ctxsw_buffer_map_entries(map,
				nvgpu_netlist_get_perf_pma_user_channel_control_ctxsw_regs(g),
				count, offset, max_reg_count, (i * channel_stride), ~U32(0U));
			if (ret != 0)
				goto cleanup;
		}
		if (nvgpu_netlist_get_perf_pma_cmdslice_control_ctxsw_regs(g)->count > 0) {
			ret = add_ctxsw_buffer_map_entries(map,
				nvgpu_netlist_get_perf_pma_cmdslice_control_ctxsw_regs(g),
				count, offset, max_reg_count, (i * cmd_slice_stride), ~U32(0U));
			if (ret != 0)
				goto cleanup;
		}
		if ((nvgpu_netlist_get_perf_pma_user_channel_control_ctxsw_regs(g)->count > 0) ||
				(nvgpu_netlist_get_perf_pma_cmdslice_control_ctxsw_regs(g)->count > 0)) {
			*offset = NVGPU_ALIGN(*offset, 256U);
		}
	}

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

	/* Add entries from _LIST_pm_lrcc_ctx_regs */
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

	if (nvgpu_netlist_get_pm_lts_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries_per_ltc_pmlists(g, map,
				nvgpu_netlist_get_pm_lts_ctxsw_regs(g),
				count, offset, max_reg_count, ltc_stride,
				lts_stride);
		if (ret != 0)
			goto cleanup;
	}

	if (nvgpu_netlist_get_pm_lrc_ctxsw_regs(g)->count > 0U) {
		ret = add_ctxsw_buffer_map_entries_per_ltc_pmlists(g, map,
				nvgpu_netlist_get_pm_lrc_ctxsw_regs(g),
				count, offset, max_reg_count, lrc_stride,
				lts_stride);
		if (ret != 0)
			goto cleanup;
	}

	*offset = NVGPU_ALIGN(*offset, 256U);

	for (i = 0; i < ctx_switched_pma_channels; i++) {
		if (nvgpu_netlist_get_perf_fbp_router_user_channel_ctxsw_regs(g)->count > 0) {
			ret = add_ctxsw_buffer_map_entries_subunits(map,
				nvgpu_netlist_get_perf_fbp_router_user_channel_ctxsw_regs(g),
				count, offset, max_reg_count, (i * router_channel_stride),
				num_fbps, ~U32(0U),
				g->ops.perf.get_pmmfbprouter_per_chiplet_offset(), ~U32(0U));
			if (ret != 0)
				goto cleanup;

			*offset = NVGPU_ALIGN(*offset, 256U);
		}
	}

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
	/* 256 B alignment in the end is taken care in the caller */
cleanup:
	return ret;
}

int gb20c_hwpm_map_add_padding_for_dtdtpc_support(struct gk20a *g,
		struct netlist_aiv_list *regs, u32 *offset, u32 num_tpc)
{
	u32 num_regs, padding_element_count, num_tpc_per_gpc;

	num_regs = regs->count;
	num_tpc_per_gpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_TPC_PER_GPC);
	padding_element_count = (num_regs - 1U) * (num_tpc_per_gpc - num_tpc);

	*offset += padding_element_count * 4U;

	return 0;
}

int gb20c_hwpm_map_add_hes_reg_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		u32 *count, u32 *offset, u32 max_reg_count)
{
	if (nvgpu_netlist_get_pm_sys_hem_ctxsw_regs(g)->count > 0) {
		if (add_ctxsw_buffer_map_entries(map,
				nvgpu_netlist_get_pm_sys_hem_ctxsw_regs(g),
				count, offset, max_reg_count, 0U, ~U32(0U)) != 0) {
			return -EINVAL;
		}
	}

	if (nvgpu_netlist_get_pm_sys_hep_ctxsw_regs(g)->count > 0) {
		if (add_ctxsw_buffer_map_entries(map,
				nvgpu_netlist_get_pm_sys_hep_ctxsw_regs(g),
				count, offset, max_reg_count, 0U, ~U32(0U)) != 0) {
			return -EINVAL;
		}
	}

	*offset = NVGPU_ALIGN(*offset, 256U);

	return 0;
}

void gb20c_hwpm_map_get_cpc_info(struct gk20a *g, u32 *max_cpc_per_gpc,
		u32 *num_gnics_per_cpc, const u32 **gnic_stride_array,
		u32 *gnic_stride)
{

	static const u32 arr[] = {
		xbar_mxbar_pri_gpc0_ig_gnic0_base_v() - xbar_mxbar_pri_gpc0_ig_gnic0_base_v(),
		xbar_mxbar_pri_gpc0_eg_gnic1_base_v() - xbar_mxbar_pri_gpc0_ig_gnic0_base_v(),
	};

	(void) g;
	*max_cpc_per_gpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_CPC_PER_GPC);
	*num_gnics_per_cpc = GNIC_CNT_PER_CPC;

	*gnic_stride_array = arr;
	*gnic_stride = ARRAY_SIZE(arr);
}
