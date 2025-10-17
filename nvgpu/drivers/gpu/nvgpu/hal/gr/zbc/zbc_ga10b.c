// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/zbc.h>
#include <nvgpu/gr/gr_instances.h>

#include "zbc_ga10b.h"

#include <nvgpu/hw/ga10b/hw_gr_ga10b.h>

#define data_index_0	0
#define data_index_1	1
#define data_index_2	2
#define data_index_3	3

void ga10b_gr_zbc_init_table_indices(struct gk20a *g,
			struct nvgpu_gr_zbc_table_indices *zbc_indices)
{
	(void)g;

	/* Color indices */
	zbc_indices->min_color_index =
		gr_pri_gpcs_rops_crop_zbc_index_address_min_v();
	zbc_indices->max_color_index =
		gr_pri_gpcs_rops_crop_zbc_index_address_max_v();

	/* Depth indices */
	zbc_indices->min_depth_index = NVGPU_GR_ZBC_STARTOF_TABLE;
	zbc_indices->max_depth_index = MAX_DEPTH_INDEX;

	/* Stencil indices */
	zbc_indices->min_stencil_index = NVGPU_GR_ZBC_STARTOF_TABLE;
	zbc_indices->max_stencil_index = gr_gpcs_swdx_dss_zbc_s__size_1_v();
}

void ga10b_gr_zbc_add_color(struct gk20a *g,
			   struct nvgpu_gr_zbc_entry *color_val, u32 index)
{
	nvgpu_log(g, gpu_dbg_zbc, "adding color at index %u", index);
	nvgpu_log(g, gpu_dbg_zbc,
		"color_clear_val[%u-%u]: 0x%08x 0x%08x 0x%08x 0x%08x",
		data_index_0, data_index_3,
		nvgpu_gr_zbc_get_entry_color_l2(color_val, data_index_0),
		nvgpu_gr_zbc_get_entry_color_l2(color_val, data_index_1),
		nvgpu_gr_zbc_get_entry_color_l2(color_val, data_index_2),
		nvgpu_gr_zbc_get_entry_color_l2(color_val, data_index_3));

	nvgpu_writel(g, gr_pri_gpcs_rops_crop_zbc_index_r(),
		gr_pri_gpcs_rops_crop_zbc_index_address_f(index));

	nvgpu_writel(g, gr_pri_gpcs_rops_crop_zbc_color_clear_value_0_r(),
		(u32)gr_pri_gpcs_rops_crop_zbc_color_clear_value_0_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_0)));
	nvgpu_writel(g, gr_pri_gpcs_rops_crop_zbc_color_clear_value_1_r(),
		gr_pri_gpcs_rops_crop_zbc_color_clear_value_1_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_1)));
	nvgpu_writel(g, gr_pri_gpcs_rops_crop_zbc_color_clear_value_2_r(),
		gr_pri_gpcs_rops_crop_zbc_color_clear_value_2_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_2)));
	nvgpu_writel(g, gr_pri_gpcs_rops_crop_zbc_color_clear_value_3_r(),
		gr_pri_gpcs_rops_crop_zbc_color_clear_value_3_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_3)));
}

static void ga10b_gr_zbc_load_default_sw_color_table(struct gk20a *g,
					struct nvgpu_gr_zbc *zbc)
{
	u32 index = zbc->min_color_index;

	(void)g;

	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 1);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 2);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 3);
#ifndef CONFIG_NVGPU_NON_FUSA
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 4);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 5);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 6);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 7);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 8);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 9);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 10);

	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 11);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 12);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 13);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 14);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 15);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 16);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 17);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 18);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 19);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 20);

	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 21);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 22);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 23);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 24);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 25);
	NVGPU_ZBC_SET_COLOR_ATTR(zbc->zbc_col_tbl, index, 26);
#endif

	/*
	 * Reached to last entry, reduce index by 1 as it has increased
	 * as part of macro.
	 */
	index = nvgpu_wrapping_sub_u32(index, 1U);
	zbc->max_used_color_index = index;
}

static void ga10b_gr_zbc_load_default_sw_depth_table(struct gk20a *g,
					struct nvgpu_gr_zbc *zbc)
{
	u32 index = zbc->min_depth_index;

	(void)g;

	NVGPU_ZBC_SET_DEPTH_ATTR(zbc->zbc_dep_tbl[index], 1);
	index = nvgpu_safe_add_u32(index, 1U);

	NVGPU_ZBC_SET_DEPTH_ATTR(zbc->zbc_dep_tbl[index], 2);

	zbc->max_used_depth_index = index;
}

static void ga10b_gr_zbc_load_default_sw_stencil_table(struct gk20a *g,
					  struct nvgpu_gr_zbc *zbc)
{
	u32 index = zbc->min_stencil_index;

	(void)g;

	NVGPU_ZBC_SET_STENCIL_ATTR(zbc->zbc_s_tbl[index], 1);
	index = nvgpu_safe_add_u32(index, 1U);

	NVGPU_ZBC_SET_STENCIL_ATTR(zbc->zbc_s_tbl[index], 2);
	index = nvgpu_safe_add_u32(index, 1U);

	NVGPU_ZBC_SET_STENCIL_ATTR(zbc->zbc_s_tbl[index], 3);

	zbc->max_used_stencil_index = index;
}

void ga10b_gr_zbc_load_static_table(struct gk20a *g,
					struct nvgpu_gr_zbc *zbc)
{
	u32 cur_instance_id = 0;

	cur_instance_id = nvgpu_gr_get_cur_instance_id(g);
	nvgpu_mutex_init(&zbc->zbc_lock);

	ga10b_gr_zbc_load_default_sw_color_table(g, zbc);

	ga10b_gr_zbc_load_default_sw_depth_table(g, zbc);

	if (nvgpu_is_gr_class_enabled(g, NVGPU_SUPPORT_ZBC_STENCIL, cur_instance_id)) {
		ga10b_gr_zbc_load_default_sw_stencil_table(g, zbc);
	}
}
