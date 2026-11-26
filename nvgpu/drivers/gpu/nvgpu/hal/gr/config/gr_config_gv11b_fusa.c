// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr.h>
#include "gr_config_gv11b.h"

#include <nvgpu/hw/gv11b/hw_gr_gv11b.h>

u32 gv11b_gr_config_get_gpc_pes_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index)
{
	u32 val;
	u32 pes_cnt = nvgpu_gr_config_get_max_pes_per_gpc_count(config);

	/*
	 * Toggle the bits of NV_FUSE_STATUS_OPT_PES_GPC to get the non-FSed
	 * PES mask.
	 */
	val = g->ops.fuse.fuse_status_opt_pes_gpc(g, gpc_index);

	return (~val) & nvgpu_safe_sub_u32(BIT32(pes_cnt), 1U);
}

void gv11b_gr_config_set_live_pes_mask(struct gk20a *g,
		u32 gpc_count)
{
	u32 pes_mask = 0U;
	u32 temp_mask = 0U;
	u32 offset = 0U;
	u32 gpc_index = 0U;

	for (gpc_index = 0U; gpc_index < gpc_count; gpc_index++) {
		offset = nvgpu_gr_gpc_offset(g, gpc_index);
		temp_mask = nvgpu_readl(g, nvgpu_safe_add_u32(gr_gpc0_gpm_pd_live_physical_pes_r(),
				offset));
		temp_mask =
			gr_gpc0_gpm_pd_live_physical_pes_gpc0_gpm_pd_live_physical_pes_mask_f(temp_mask);
		temp_mask <<=  (NUMBER_OF_BITS_COMPUTE_PES_MASK * gpc_index);
		pes_mask = temp_mask | pes_mask;
	}
	/*
	 * When TPCs are floorswept, Corresponding PES mask needs to be updated
	 * in this register. For an example : If a GPC contains "x" TPCs and "y"
	 * PES. Every PES needs atleast a TPC to be enabled to work. If
	 * contiguous "x/y" TPCs are floorswept , then "y-1" PES will be active.
	 */
	for (gpc_index = 0U; gpc_index < gpc_count; gpc_index++) {
		offset = nvgpu_gr_gpc_offset(g, gpc_index);
		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_swdx_pes_mask_r(), offset), pes_mask);
	}
}
