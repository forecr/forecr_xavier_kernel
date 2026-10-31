// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/zbc.h>

#include "ltc_ga10b.h"

#include <nvgpu/hw/ga10b/hw_ltc_ga10b.h>

#ifdef CONFIG_NVGPU_GRAPHICS
void ga10b_ltc_set_zbc_stencil_entry(struct gk20a *g, u32 stencil_depth,
					u32 index)
{
	nvgpu_writel(g, ltc_ltcs_ltss_dstg_zbc_index_r(),
		ltc_ltcs_ltss_dstg_zbc_index_address_f(index));

	nvgpu_writel(g,
 		ltc_ltcs_ltss_dstg_zbc_stencil_clear_value_r(), stencil_depth);
}

void ga10b_ltc_set_zbc_color_entry(struct gk20a *g, u32 *color_l2, u32 index)
{
	u32 i;

	nvgpu_writel(g, ltc_ltcs_ltss_dstg_zbc_index_r(),
		ltc_ltcs_ltss_dstg_zbc_index_address_f(index));

	for (i = 0; i < ltc_ltcs_ltss_dstg_zbc_color_clear_value__size_1_v();
									i++) {
		nvgpu_writel(g,
			ltc_ltcs_ltss_dstg_zbc_color_clear_value_r(i),
			color_l2[i]);
	}
}

/*
 * Sets the ZBC depth for the passed index.
 */
void ga10b_ltc_set_zbc_depth_entry(struct gk20a *g, u32 depth_val, u32 index)
{
	nvgpu_writel(g, ltc_ltcs_ltss_dstg_zbc_index_r(),
		ltc_ltcs_ltss_dstg_zbc_index_address_f(index));

	nvgpu_writel(g,
		ltc_ltcs_ltss_dstg_zbc_depth_clear_value_r(), depth_val);
}
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
int ga10b_ltc_pri_shared_addr(struct gk20a *g, u32 addr, u32 *ltc_shared_addr)
{
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);
	u32 ltc_shared_base = ltc_ltcs_ltss_v();
	u32 ltc_addr_mask = nvgpu_safe_sub_u32(ltc_stride, 1);
	u32 lts_addr_mask = nvgpu_safe_sub_u32(lts_stride, 1);
	u32 ltc_addr = addr & ltc_addr_mask;
	u32 lts_addr = ltc_addr & lts_addr_mask;

	if (nvgpu_safe_add_u32_return(ltc_shared_base, lts_addr, ltc_shared_addr) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	return 0;
}
#endif
