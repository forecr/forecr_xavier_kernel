// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/zbc.h>

#include "zbc_gv11b.h"

#include <nvgpu/hw/gv11b/hw_gr_gv11b.h>

#ifdef CONFIG_NVGPU_NON_FUSA
void gv11b_gr_zbc_init_table_indices(struct gk20a *g,
			struct nvgpu_gr_zbc_table_indices *zbc_indices)
{
	(void)g;

	/* Color indices */
	zbc_indices->min_color_index = NVGPU_GR_ZBC_STARTOF_TABLE;
	zbc_indices->max_color_index = gr_gpcs_swdx_dss_zbc_color_r__size_1_v();

	/* Depth indices */
	zbc_indices->min_depth_index = NVGPU_GR_ZBC_STARTOF_TABLE;
	zbc_indices->max_depth_index = gr_gpcs_swdx_dss_zbc_z__size_1_v();

	/* Stencil indices */
	zbc_indices->min_stencil_index = NVGPU_GR_ZBC_STARTOF_TABLE;
	zbc_indices->max_stencil_index = gr_gpcs_swdx_dss_zbc_s__size_1_v();
}
#endif

#ifdef CONFIG_NVGPU_ZBC_USER
u32 gv11b_gr_zbc_get_gpcs_swdx_dss_zbc_c_format_reg(struct gk20a *g)
{
	(void)g;
	return gr_gpcs_swdx_dss_zbc_c_01_to_04_format_r();
}
#endif

NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 5_1), "SWE-DRC-502-SWSADP-M5.dox")
u32 gv11b_gr_zbc_get_gpcs_swdx_dss_zbc_z_format_reg(struct gk20a *g)
{
	(void)g;
	return gr_gpcs_swdx_dss_zbc_z_01_to_04_format_r();
}

void gv11b_gr_zbc_add_stencil(struct gk20a *g,
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
	zbc_s = nvgpu_readl(g, nvgpu_safe_add_u32(gr_gpcs_swdx_dss_zbc_s_01_to_04_format_r(),
						  (hw_index & ~3U)));
	zbc_s &= ~(U32(0x7f) << (hw_index % 4U) * 7U);
	zbc_s |= nvgpu_gr_zbc_get_entry_format(stencil_val) <<
		(hw_index % 4U) * 7U;
	nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpcs_swdx_dss_zbc_s_01_to_04_format_r(),
				 (hw_index & ~3U)), zbc_s);
}
