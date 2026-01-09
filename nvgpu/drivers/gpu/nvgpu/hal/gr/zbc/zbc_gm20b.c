// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/zbc.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/gr_instances.h>
#include "zbc_gm20b.h"

#include <nvgpu/hw/gm20b/hw_gr_gm20b.h>

#ifdef CONFIG_NVGPU_ZBC_USER
void gm20b_gr_zbc_init_table_indices(struct gk20a *g,
			struct nvgpu_gr_zbc_table_indices *zbc_indices)
{
	(void)g;

	/* Color indices */
	zbc_indices->min_color_index = NVGPU_GR_ZBC_STARTOF_TABLE;
	zbc_indices->max_color_index = 15U;

	/* Depth indices */
	zbc_indices->min_depth_index = NVGPU_GR_ZBC_STARTOF_TABLE;
	zbc_indices->max_depth_index = 15U;

	/* Stencil indices */
	zbc_indices->min_stencil_index = 0U;
	zbc_indices->max_stencil_index = 0U;
}

void gm20b_gr_zbc_add_color(struct gk20a *g,
			struct nvgpu_gr_zbc_entry *color_val, u32 index)
{
	/* update ds table */
	nvgpu_writel(g, gr_ds_zbc_color_r_r(),
		gr_ds_zbc_color_r_val_f(
			nvgpu_gr_zbc_get_entry_color_ds(color_val, 0)));
	nvgpu_writel(g, gr_ds_zbc_color_g_r(),
		gr_ds_zbc_color_g_val_f(
			nvgpu_gr_zbc_get_entry_color_ds(color_val, 1)));
	nvgpu_writel(g, gr_ds_zbc_color_b_r(),
		gr_ds_zbc_color_b_val_f(
			nvgpu_gr_zbc_get_entry_color_ds(color_val, 2)));
	nvgpu_writel(g, gr_ds_zbc_color_a_r(),
		gr_ds_zbc_color_a_val_f(
			nvgpu_gr_zbc_get_entry_color_ds(color_val, 3)));

	nvgpu_writel(g, gr_ds_zbc_color_fmt_r(),
		gr_ds_zbc_color_fmt_val_f(
			nvgpu_gr_zbc_get_entry_format(color_val)));

	nvgpu_writel(g, gr_ds_zbc_tbl_index_r(),
		gr_ds_zbc_tbl_index_val_f(index));

	/* trigger the write */
	nvgpu_writel(g, gr_ds_zbc_tbl_ld_r(),
		gr_ds_zbc_tbl_ld_select_c_f() |
		gr_ds_zbc_tbl_ld_action_write_f() |
		gr_ds_zbc_tbl_ld_trigger_active_f());

}

void gm20b_gr_zbc_add_depth(struct gk20a *g,
			   struct nvgpu_gr_zbc_entry *depth_val, u32 index)
{
	/* update ds table */
	nvgpu_writel(g, gr_ds_zbc_z_r(),
		gr_ds_zbc_z_val_f(
			nvgpu_gr_zbc_get_entry_depth(depth_val)));

	nvgpu_writel(g, gr_ds_zbc_z_fmt_r(),
		gr_ds_zbc_z_fmt_val_f(
			nvgpu_gr_zbc_get_entry_format(depth_val)));

	nvgpu_writel(g, gr_ds_zbc_tbl_index_r(),
		gr_ds_zbc_tbl_index_val_f(index));

	/* trigger the write */
	nvgpu_writel(g, gr_ds_zbc_tbl_ld_r(),
		gr_ds_zbc_tbl_ld_select_z_f() |
		gr_ds_zbc_tbl_ld_action_write_f() |
		gr_ds_zbc_tbl_ld_trigger_active_f());

}

static void gm20b_gr_zbc_load_default_sw_stencil_table(struct gk20a *g,
					  struct nvgpu_gr_zbc *zbc)
{
	u32 index = zbc->min_stencil_index;

	(void)g;

	zbc->zbc_s_tbl[index].stencil = 0x0;
	zbc->zbc_s_tbl[index].format = GR_ZBC_STENCIL_CLEAR_FMT_U8;
	zbc->zbc_s_tbl[index].ref_cnt =
		nvgpu_safe_add_u32(zbc->zbc_s_tbl[index].ref_cnt, 1U);
	index = nvgpu_safe_add_u32(index, 1U);

	zbc->zbc_s_tbl[index].stencil = 0x1;
	zbc->zbc_s_tbl[index].format = GR_ZBC_STENCIL_CLEAR_FMT_U8;
	zbc->zbc_s_tbl[index].ref_cnt =
		nvgpu_safe_add_u32(zbc->zbc_s_tbl[index].ref_cnt, 1U);
	index = nvgpu_safe_add_u32(index, 1U);

	zbc->zbc_s_tbl[index].stencil = 0xff;
	zbc->zbc_s_tbl[index].format = GR_ZBC_STENCIL_CLEAR_FMT_U8;
	zbc->zbc_s_tbl[index].ref_cnt =
		nvgpu_safe_add_u32(zbc->zbc_s_tbl[index].ref_cnt, 1U);

	zbc->max_used_stencil_index = index;
}

static void gm20b_gr_zbc_load_default_sw_depth_table(struct gk20a *g,
					struct nvgpu_gr_zbc *zbc)
{
	u32 index = zbc->min_depth_index;

	(void)g;

	zbc->zbc_dep_tbl[index].format = GR_ZBC_Z_FMT_VAL_FP32;
	zbc->zbc_dep_tbl[index].depth = 0x3f800000;
	zbc->zbc_dep_tbl[index].ref_cnt =
		nvgpu_safe_add_u32(zbc->zbc_dep_tbl[index].ref_cnt, 1U);
	index = nvgpu_safe_add_u32(index, 1U);

	zbc->zbc_dep_tbl[index].format = GR_ZBC_Z_FMT_VAL_FP32;
	zbc->zbc_dep_tbl[index].depth = 0;
	zbc->zbc_dep_tbl[index].ref_cnt =
		nvgpu_safe_add_u32(zbc->zbc_dep_tbl[index].ref_cnt, 1U);

	zbc->max_used_depth_index = index;
}

static void gm20b_gr_zbc_load_default_sw_color_table(struct gk20a *g,
					struct nvgpu_gr_zbc *zbc)
{
	u32 i;
	u32 index = zbc->min_color_index;

	(void)g;

	/* Opaque black (i.e. solid black, fmt 0x28 = A8B8G8R8) */
	zbc->zbc_col_tbl[index].format = GR_ZBC_SOLID_BLACK_COLOR_FMT;
	for (i = 0U; i < NVGPU_GR_ZBC_COLOR_VALUE_SIZE; i++) {
		zbc->zbc_col_tbl[index].color_ds[i] = 0U;
		zbc->zbc_col_tbl[index].color_l2[i] = 0xff000000U;
	}
	zbc->zbc_col_tbl[index].color_ds[3] = 0x3f800000U;
	zbc->zbc_col_tbl[index].ref_cnt =
		nvgpu_safe_add_u32(zbc->zbc_col_tbl[index].ref_cnt, 1U);
	index = nvgpu_safe_add_u32(index, 1U);

	/* Transparent black = (fmt 1 = zero) */
	zbc->zbc_col_tbl[index].format = GR_ZBC_TRANSPARENT_BLACK_COLOR_FMT;
	for (i = 0; i < NVGPU_GR_ZBC_COLOR_VALUE_SIZE; i++) {
		zbc->zbc_col_tbl[index].color_ds[i] = 0U;
		zbc->zbc_col_tbl[index].color_l2[i] = 0U;
	}
	zbc->zbc_col_tbl[index].ref_cnt =
		nvgpu_safe_add_u32(zbc->zbc_col_tbl[index].ref_cnt, 1U);
	index = nvgpu_safe_add_u32(index, 1U);

	/* Opaque white (i.e. solid white) = (fmt 2 = uniform 1) */
	zbc->zbc_col_tbl[index].format = GR_ZBC_SOLID_WHITE_COLOR_FMT;
	for (i = 0; i < NVGPU_GR_ZBC_COLOR_VALUE_SIZE; i++) {
		zbc->zbc_col_tbl[index].color_ds[i] = 0x3f800000U;
		zbc->zbc_col_tbl[index].color_l2[i] = 0xffffffffU;
	}
	zbc->zbc_col_tbl[index].ref_cnt =
		nvgpu_safe_add_u32(zbc->zbc_col_tbl[index].ref_cnt, 1U);

	zbc->max_used_color_index = index;
}

void gm20b_gr_zbc_load_default_sw_table(struct gk20a *g,
					struct nvgpu_gr_zbc *zbc)
{
	u32 cur_instance_id = 0U;

	cur_instance_id = nvgpu_gr_get_cur_instance_id(g);
	nvgpu_mutex_init(&zbc->zbc_lock);

	gm20b_gr_zbc_load_default_sw_color_table(g, zbc);

	gm20b_gr_zbc_load_default_sw_depth_table(g, zbc);

	if (nvgpu_is_gr_class_enabled(g,
				NVGPU_SUPPORT_ZBC_STENCIL, cur_instance_id)) {
		gm20b_gr_zbc_load_default_sw_stencil_table(g, zbc);
	}
}
#endif
