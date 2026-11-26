// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/zbc.h>

#include "zbc_gb20c.h"

#include <nvgpu/hw/gb20c/hw_gr_gb20c.h>

u32 gb20c_gr_pri_gpcs_rops_crop_zbc_index_address_max(void)
{
	return gr_pri_gpcs_rops_crop_zbc_index_address_max_v();
}

void gb20c_gr_zbc_add_color(struct gk20a *g,
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

	// Set ZBC data to GXC
	nvgpu_writel(g, gr_xbar_pri_gxcs_zbc_index_r(),
		gr_xbar_pri_gxcs_zbc_index_address_f(index));

	nvgpu_writel(g, gr_xbar_pri_gxcs_zbc_color_clear_value_0_r(),
		gr_xbar_pri_gxcs_zbc_color_clear_value_0_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_0)));

	nvgpu_writel(g, gr_xbar_pri_gxcs_zbc_color_clear_value_1_r(),
		gr_xbar_pri_gxcs_zbc_color_clear_value_1_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_1)));

	nvgpu_writel(g, gr_xbar_pri_gxcs_zbc_color_clear_value_2_r(),
		gr_xbar_pri_gxcs_zbc_color_clear_value_2_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_2)));

	nvgpu_writel(g, gr_xbar_pri_gxcs_zbc_color_clear_value_3_r(),
		gr_xbar_pri_gxcs_zbc_color_clear_value_3_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_3)));

	// Set ZBC data to HXC
	nvgpu_writel(g, gr_xbar_pri_sys_collar0_zbc_index_r(),
		gr_xbar_pri_sys_collar0_zbc_index_address_f(index));

	nvgpu_writel(g, gr_xbar_pri_sys_collar0_zbc_color_clear_value_0_r(),
		gr_xbar_pri_sys_collar0_zbc_color_clear_value_0_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_0)));

	nvgpu_writel(g, gr_xbar_pri_sys_collar0_zbc_color_clear_value_1_r(),
		gr_xbar_pri_sys_collar0_zbc_color_clear_value_1_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_1)));

	nvgpu_writel(g, gr_xbar_pri_sys_collar0_zbc_color_clear_value_2_r(),
		gr_xbar_pri_sys_collar0_zbc_color_clear_value_2_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_2)));

	nvgpu_writel(g, gr_xbar_pri_sys_collar0_zbc_color_clear_value_3_r(),
		gr_xbar_pri_sys_collar0_zbc_color_clear_value_3_bits_f(
			nvgpu_gr_zbc_get_entry_color_l2(
				color_val, data_index_3)));
}
