// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/zbc.h>

#include "zbc_gb10b.h"

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>

u32 gb10b_gr_pri_gpcs_rops_crop_zbc_index_address_max(void)
{
	return gr_pri_gpcs_rops_crop_zbc_index_address_max_v();
}

void gb10b_gr_zbc_init_table_indices(struct gk20a *g,
			struct nvgpu_gr_zbc_table_indices *zbc_indices)
{
	(void)g;

	/* Color indices */
	zbc_indices->min_color_index =
		gr_pri_gpcs_rops_crop_zbc_index_address_min_v();
	zbc_indices->max_color_index =
		g->ops.gr.zbc.get_gr_pri_gpcs_rops_crop_zbc_index_address_max();

	/* Depth indices */
	zbc_indices->min_depth_index = NVGPU_GR_ZBC_STARTOF_TABLE;
	zbc_indices->max_depth_index = MAX_DEPTH_INDEX;

	/* Stencil indices */
	zbc_indices->min_stencil_index = NVGPU_GR_ZBC_STARTOF_TABLE;
	zbc_indices->max_stencil_index = gr_gpcs_swdx_dss_zbc_s__size_1_v();
}

u32 gb10b_gr_zbc_get_gpcs_swdx_dss_zbc_c_format_reg(struct gk20a *g)
{
	(void)g;
	return gr_gpcs_swdx_dss_zbc_c_01_to_04_format_r();
}

u32 gb10b_gr_zbc_get_gpcs_swdx_dss_zbc_z_format_reg(struct gk20a *g)
{
	(void)g;
	return gr_gpcs_swdx_dss_zbc_z_01_to_04_format_r();
}

void gb10b_gr_zbc_add_stencil(struct gk20a *g,
			     struct nvgpu_gr_zbc_entry *stencil_val, u32 index)
{
	u32 zbc_s;
	u32 hw_index = nvgpu_safe_sub_u32(index, NVGPU_GR_ZBC_STARTOF_TABLE);

	nvgpu_log(g, gpu_dbg_zbc, "adding stencil at index %u", index);
	nvgpu_log(g, gpu_dbg_zbc, "stencil: 0x%08x",
		nvgpu_gr_zbc_get_entry_stencil(stencil_val));

	nvgpu_writel(g, gr_gpcs_swdx_dss_zbc_s_r(hw_index),
		nvgpu_gr_zbc_get_entry_stencil(stencil_val));

	/* update format register */
	zbc_s = nvgpu_readl(g, gr_gpcs_swdx_dss_zbc_s_01_to_04_format_r() +
				 (hw_index & ~3U));
	zbc_s &= ~(U32(0x7f) << (hw_index % 4U) * 7U);
	zbc_s |= nvgpu_gr_zbc_get_entry_format(stencil_val) <<
		(hw_index % 4U) * 7U;
	nvgpu_writel(g, gr_gpcs_swdx_dss_zbc_s_01_to_04_format_r() +
				 (hw_index & ~3U), zbc_s);
}

void gb10b_gr_zbc_add_depth(struct gk20a *g,
			   struct nvgpu_gr_zbc_entry *depth_val, u32 index)
{
	u32 zbc_z;
	u32 zbc_z_format_reg;
	u32 hw_index = nvgpu_safe_sub_u32(index, NVGPU_GR_ZBC_STARTOF_TABLE);

	nvgpu_log(g, gpu_dbg_zbc, "adding depth at index %u", index);
	nvgpu_log(g, gpu_dbg_zbc, "depth: 0x%08x",
		nvgpu_gr_zbc_get_entry_depth(depth_val));

	nvgpu_writel(g, gr_gpcs_swdx_dss_zbc_z_r(hw_index),
		nvgpu_gr_zbc_get_entry_depth(depth_val));

	/* Update format register */
	zbc_z_format_reg = g->ops.gr.zbc.get_gpcs_swdx_dss_zbc_z_format_reg(g);
	zbc_z = nvgpu_readl(g, zbc_z_format_reg + (hw_index & ~3U));
	zbc_z &= ~(U32(0x7f) << ((hw_index % 4U) * 7U));
	zbc_z |= nvgpu_gr_zbc_get_entry_format(depth_val) <<
			(hw_index % 4U) * 7U;
	nvgpu_writel(g, zbc_z_format_reg + (hw_index & ~3U), zbc_z);
}

void gb10b_gr_zbc_add_color(struct gk20a *g,
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
		gr_pri_gpcs_rops_crop_zbc_color_clear_value_0_bits_f(
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
