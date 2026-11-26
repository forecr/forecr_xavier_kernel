// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/zcull.h>

#include "zcull_gv11b.h"

#include <nvgpu/hw/gv11b/hw_gr_gv11b.h>

void gv11b_gr_program_zcull_mapping(struct gk20a *g, u32 zcull_num_entries,
					u32 *zcull_map_tiles)
{
	u32 val, i, j = 0U;

	nvgpu_log_fn(g, " ");

	for (i = 0U; i < (zcull_num_entries / 8U); i++) {
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

		gk20a_writel(g, gr_gpcs_zcull_sm_in_gpc_number_map_r(i), val);

		j += 8U;
	}
}

