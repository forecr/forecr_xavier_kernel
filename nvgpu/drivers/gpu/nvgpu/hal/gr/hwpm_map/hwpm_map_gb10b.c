// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/types.h>
#include <nvgpu/bitops.h>

#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gops/debugger.h>

#include <nvgpu/hw/gb10b/hw_xbar_gb10b.h>

#include "hwpm_map_gb10b.h"

int gb10b_hwpm_map_add_gpc_perf_entries_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		struct netlist_aiv_list *regs, u32 *count, u32 *offset,
		u32 max_cnt, u32 base, u32 mask, u32 gpc_num)
{
	u32 cur_gr_instance;
	struct nvgpu_gr_config *config;
	u32 gpc_phys_id;
	u32 tpc_en_mask;
	u32 perfmon_stride;
	u32 tpc_perfmon_start;
	u32 tpc_perfmon_end;
	u32 num_max_tpc_per_gpc;
	u32 perfmon_idx;
	u32 num_gpc_perfmons;
	u32 tpc_phys_id;
	u32 idx;
	u32 cnt = *count;
	u32 off = *offset;
	u32 result = 0U;

	config = nvgpu_gr_get_config_ptr(g);
	cur_gr_instance = nvgpu_gr_get_cur_instance_id(g);

	gpc_phys_id = nvgpu_grmgr_get_gr_gpc_phys_id(g, cur_gr_instance, gpc_num);
	tpc_en_mask = g->ops.gr.config.get_gpc_tpc_mask(g, config, gpc_phys_id);

	g->ops.perf.get_tpc_perfmon_range(&tpc_perfmon_start, &tpc_perfmon_end);

	num_max_tpc_per_gpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_TPC_PER_GPC);
	num_gpc_perfmons = g->ops.perf.get_max_num_gpc_perfmons();
	perfmon_stride = g->ops.perf.get_gpc_perfmon_stride();

	if (nvgpu_safe_add_u32_return(cnt, regs->count, &result) == false) {
		nvgpu_err(g, "Buffer overflow");
		return -EOVERFLOW;
	} else if (result > max_cnt) {
		nvgpu_err(g, "result exceeds max_cnt");
		return -EINVAL;
	}

	for (perfmon_idx = 0; perfmon_idx < num_gpc_perfmons; perfmon_idx++)
	{
		if ((tpc_perfmon_start <= perfmon_idx) &&
				(perfmon_idx <= tpc_perfmon_end))
		{
			tpc_phys_id =
			(perfmon_idx - tpc_perfmon_start) % num_max_tpc_per_gpc;
			// Skip adding floorswept perfmons.
			if (((1UL << tpc_phys_id) & tpc_en_mask) == 0)
			{
				continue;
			}
		}
		for (idx = 0; idx < regs->count - 1; idx++) {
			map[cnt].addr = base +
				((regs->l[idx].addr +
				  (perfmon_idx * perfmon_stride)) & mask);
			map[cnt++].offset = off;

			if (nvgpu_safe_add_u32_return(off, 4U, &off) == false) {
				nvgpu_err(g, "Buffer overflow");
				return -EOVERFLOW;
			}
		}
	}

	map[cnt].addr = regs->l[regs->count - 1].addr;
	map[cnt++].offset = off;

	if (nvgpu_safe_add_u32_return(off, 4U, &off) == false) {
		nvgpu_err(g, "Buffer overflow");
		return -EOVERFLOW;
	}

	*offset = off;
	*count = cnt;

	return 0;
}

int gb10b_hwpm_map_add_pm_ucgpc_entries_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		struct netlist_aiv_list *regs, u32 *count, u32 *offset,
		u32 max_cnt, u32 base, u32 mask, u32 gpc_num)
{
	u32 cur_gr_instance;
	u32 gpc_phys_id;
	u32 val;
	u32 idx;
	u32 cnt;
	u32 off;
	u32 cpc_idx;
	u32 max_cpc_per_gpc;
	u32 num_cpc;
	u32 gnic_id;
	u32 gnic_offset;
	u32 gnic_per_cpc = 5;
	u32 gnic_stride_ary_idx;
	u32 reg_count_limit;
	const u32 gnic_stride_array[5] = {
		xbar_mxbar_pri_gpc0_gnic0_base_v() - xbar_mxbar_pri_gpc0_gnic0_base_v(),
		xbar_mxbar_pri_gpc0_gnic1_base_v() - xbar_mxbar_pri_gpc0_gnic0_base_v(),
		xbar_mxbar_pri_gpc0_gnic2_base_v() - xbar_mxbar_pri_gpc0_gnic0_base_v(),
		xbar_mxbar_pri_gpc0_gnic3_base_v() - xbar_mxbar_pri_gpc0_gnic0_base_v(),
		xbar_mxbar_pri_gpc0_gnic4_base_v() - xbar_mxbar_pri_gpc0_gnic0_base_v(),
	};
	cnt = *count;
	off = *offset;
	(void) mask;

	cur_gr_instance = nvgpu_gr_get_cur_instance_id(g);

	gpc_phys_id = nvgpu_grmgr_get_gr_gpc_phys_id(g, cur_gr_instance, gpc_num);

	max_cpc_per_gpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_CPC_PER_GPC);

	val = g->ops.fuse.fuse_status_opt_cpc_gpc(g, gpc_phys_id);

	num_cpc = ~val & nvgpu_safe_sub_u32(BIT32(max_cpc_per_gpc), 1U);

	/* overflow check */
	if ((UINT_MAX - cnt) < regs->count) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	} else {
		if ((cnt + regs->count) > max_cnt) {
			nvgpu_err(g, "result exceeds max_cnt");
			return -EINVAL;
		}
	}

	if (nvgpu_safe_sub_u32_return(regs->count, 1, &reg_count_limit) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}
	for (idx = 0; idx < reg_count_limit; idx++) {
		for (cpc_idx = 0; cpc_idx < num_cpc; cpc_idx++) {
			if (nvgpu_safe_mult_u32_return(cpc_idx, gnic_per_cpc, &gnic_offset) == false) {
				nvgpu_err(g, "buffer overflow");
				return -EOVERFLOW;
			}
			for (gnic_id = 0; gnic_id < gnic_per_cpc; gnic_id++) {
				if (nvgpu_safe_add_u32_return(base, regs->l[idx].addr, &map[cnt].addr) == false) {
					nvgpu_err(g, "buffer overflow");
					return -EOVERFLOW;
				}
				if (nvgpu_safe_add_u32_return(gnic_id, gnic_offset, &gnic_stride_ary_idx) == false) {
					nvgpu_err(g, "buffer overflow");
					return -EOVERFLOW;
				}
				if (nvgpu_safe_add_u32_return(map[cnt].addr, gnic_stride_array[gnic_stride_ary_idx],
							&map[cnt].addr) == false) {
					nvgpu_err(g, "buffer overflow");
					return -EOVERFLOW;
				}

				map[cnt].offset = off;
				if (nvgpu_safe_add_u32_return(cnt, 1, &cnt) == false) {
					nvgpu_err(g, "buffer overflow");
					return -EOVERFLOW;
				}
				if (nvgpu_safe_add_u32_return(off, 4, &off) == false) {
					nvgpu_err(g, "buffer overflow");
					return -EOVERFLOW;
				}
			}
		}
	}
	map[cnt].addr = regs->l[reg_count_limit].addr;
	map[cnt].offset = off;
	if (nvgpu_safe_add_u32_return(cnt, 1, &cnt) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}
	if (nvgpu_safe_add_u32_return(off, 4, &off) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	*offset = off;
	*count = cnt;

	return 0;
}


int gb10b_hwpm_map_add_lrcc_entries_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		struct netlist_aiv_list *regs, u32 *count, u32 *offset,
		u32 max_cnt, u32 num_lrc)
{
	u32 lrcc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRCC_STRIDE);

	return add_ctxsw_buffer_map_entries_subunits(map,
			regs, count, offset,
			max_cnt, 0, num_lrc, ~U32(0U),
			lrcc_stride, ~U32(0U));
}
