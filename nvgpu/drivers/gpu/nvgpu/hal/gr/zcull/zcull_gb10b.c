// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gr/subctx.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/zcull.h>

#include "common/gr/zcull_priv.h"

#include "zcull_gb10b.h"

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>

int gb10b_gr_init_zcull_hw(struct gk20a *g,
			struct nvgpu_gr_zcull *gr_zcull,
			struct nvgpu_gr_config *gr_config)
{
	u32 gpc_index, gpc_tpc_count, gpc_zcull_count;
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	bool floorsweep = false;
	u32 rcp_conserv;
	u32 offset;

	gr_zcull->total_aliquots =
		gr_gpc0_zcull_total_ram_size_num_aliquots_f(
			nvgpu_readl(g, gr_gpc0_zcull_total_ram_size_r()));

	for (gpc_index = 0;
	     gpc_index < nvgpu_gr_config_get_gpc_count(gr_config);
	     gpc_index++) {
		gpc_tpc_count =
			nvgpu_gr_config_get_gpc_tpc_count(gr_config, gpc_index);
		gpc_zcull_count =
			nvgpu_gr_config_get_gpc_zcb_count(gr_config, gpc_index);

		if (gpc_zcull_count !=
			nvgpu_gr_config_get_max_zcull_per_gpc_count(gr_config) &&
			gpc_zcull_count < gpc_tpc_count) {
			nvgpu_err(g,
				"zcull_banks (%d) less than tpcs (%d) for gpc (%d)",
				gpc_zcull_count, gpc_tpc_count, gpc_index);
			return -EINVAL;
		}
		if (gpc_zcull_count !=
			nvgpu_gr_config_get_max_zcull_per_gpc_count(gr_config) &&
		    gpc_zcull_count != 0U) {
			floorsweep = true;
		}
	}

	/* ceil(1.0f / SM_NUM * gr_gpc0_zcull_sm_num_rcp_conservative__max_v())
	 * sm_num in the register manual actually means TPC count
	 */
	rcp_conserv = DIV_ROUND_UP(gr_gpc0_zcull_sm_num_rcp_conservative__max_v(),
		nvgpu_gr_config_get_tpc_count(gr_config));

	for (gpc_index = 0;
	     gpc_index < nvgpu_gr_config_get_gpc_count(gr_config);
	     gpc_index++) {
		offset = nvgpu_safe_mult_u32(gpc_index, gpc_stride);

		if (floorsweep) {
			nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_zcull_ram_addr_r(), offset),
				gr_gpc0_zcull_ram_addr_row_offset_f(
					nvgpu_gr_config_get_map_row_offset(gr_config)) |
				gr_gpc0_zcull_ram_addr_tiles_per_hypertile_row_per_gpc_f(
					nvgpu_gr_config_get_max_zcull_per_gpc_count(gr_config)));
		} else {
			nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_zcull_ram_addr_r(), offset),
				gr_gpc0_zcull_ram_addr_row_offset_f(
					nvgpu_gr_config_get_map_row_offset(gr_config)) |
				gr_gpc0_zcull_ram_addr_tiles_per_hypertile_row_per_gpc_f(
					nvgpu_gr_config_get_gpc_tpc_count(gr_config, gpc_index)));
		}

		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_zcull_fs_r(), offset),
			gr_gpc0_zcull_fs_num_active_banks_f(
				nvgpu_gr_config_get_gpc_zcb_count(gr_config, gpc_index)) |
			gr_gpc0_zcull_fs_num_sms_f(
				nvgpu_gr_config_get_tpc_count(gr_config)));

		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_zcull_sm_num_rcp_r(), offset),
			gr_gpc0_zcull_sm_num_rcp_conservative_f(rcp_conserv));
	}

	nvgpu_writel(g, gr_gpcs_ppcs_wwdx_sm_num_rcp_r(),
		gr_gpcs_ppcs_wwdx_sm_num_rcp_conservative_f(rcp_conserv));

	return 0;
}

void gb10b_gr_program_zcull_mapping(struct gk20a *g, u32 zcull_num_entries,
					u32 *zcull_map_tiles)
{
	u32 val, i, j = 0U;

	nvgpu_log_fn(g, " ");

	/* Total 8 fields per map, i.e., tile_0 to tile_7 */
	for (i = 0U; i < (zcull_num_entries / 8U); i++) {
		j = i * 8U;
		val =
		gr_gpcs_zcull_sm_in_gpc_number_map_tile_0_f(
						zcull_map_tiles[j+0U]) |
		gr_gpcs_zcull_sm_in_gpc_number_map_tile_1_f(
						zcull_map_tiles[j+1U]) |
		gr_gpcs_zcull_sm_in_gpc_number_map_tile_2_f(
						zcull_map_tiles[j+2U]) |
		gr_gpcs_zcull_sm_in_gpc_number_map_tile_3_f(
						zcull_map_tiles[j+3U]) |
		gr_gpcs_zcull_sm_in_gpc_number_map_tile_4_f(
						zcull_map_tiles[j+4U]) |
		gr_gpcs_zcull_sm_in_gpc_number_map_tile_5_f(
						zcull_map_tiles[j+5U]) |
		gr_gpcs_zcull_sm_in_gpc_number_map_tile_6_f(
						zcull_map_tiles[j+6U]) |
		gr_gpcs_zcull_sm_in_gpc_number_map_tile_7_f(
						zcull_map_tiles[j+7U]);

		nvgpu_writel(g, gr_gpcs_zcull_sm_in_gpc_number_map_r(i), val);
	}
}
