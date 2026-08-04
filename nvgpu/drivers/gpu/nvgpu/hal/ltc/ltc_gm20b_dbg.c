// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/trace.h>
#include <nvgpu/timers.h>
#include <nvgpu/enabled.h>
#include <nvgpu/bug.h>
#include <nvgpu/ltc.h>
#include <nvgpu/fbp.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include <nvgpu/hw/gm20b/hw_ltc_gm20b.h>

#include "ltc_gm20b.h"

#ifdef CONFIG_NVGPU_DEBUGGER
/*
 * LTC pri addressing
 */
bool gm20b_ltc_pri_is_ltc_addr(struct gk20a *g, u32 addr)
{
	(void)g;
	return ((addr >= ltc_pltcg_base_v()) && (addr < ltc_pltcg_extent_v()));
}

bool gm20b_ltc_is_pltcg_ltcs_addr(struct gk20a *g, u32 addr)
{
	(void)g;
	return ((addr >= ltc_pltcg_ltcs_base_v()) && (addr < ltc_pltcg_extent_v()));
}

bool gm20b_ltc_is_ltcs_ltss_addr(struct gk20a *g, u32 addr)
{
	u32 ltc_shared_base = ltc_ltcs_ltss_v();
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);

	if (addr >= ltc_shared_base) {
		return (addr < nvgpu_safe_add_u32(ltc_shared_base, lts_stride));
	}
	return false;
}

bool gm20b_ltc_is_ltcn_ltss_addr(struct gk20a *g, u32 addr)
{
	u32 lts_shared_base = ltc_ltc0_ltss_v();
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);
	u32 addr_mask;
	u32 base_offset;
	u32 end_offset;

	if (nvgpu_safe_sub_u32_return(nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE), 1U, &addr_mask) == false) {
		nvgpu_err(g, "buffer underflow");
		return false;
	}

	base_offset = lts_shared_base & addr_mask;

	if (nvgpu_safe_add_u32_return(base_offset, lts_stride, &end_offset) == false) {
		nvgpu_err(g, "buffer overflow");
		return false;
	}

	return (!gm20b_ltc_is_ltcs_ltss_addr(g, addr)) &&
		((addr & addr_mask) >= base_offset) &&
		((addr & addr_mask) < end_offset);
}

static void gm20b_ltc_update_ltc_lts_addr(struct gk20a *g, u32 addr,
		u32 ltc_num, u32 *priv_addr_table, u32 *priv_addr_table_index)
{
	u32 num_ltc_slices = g->ops.top.get_max_lts_per_ltc(g);
	u32 index = *priv_addr_table_index;
	u32 lts_num;
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);

	for (lts_num = 0; lts_num < num_ltc_slices;
				lts_num = nvgpu_safe_add_u32(lts_num, 1U)) {
		priv_addr_table[index] = nvgpu_safe_add_u32(
			ltc_ltc0_lts0_v(),
			nvgpu_safe_add_u32(
				nvgpu_safe_add_u32(
				nvgpu_safe_mult_u32(ltc_num, ltc_stride),
				nvgpu_safe_mult_u32(lts_num, lts_stride)),
						(addr & nvgpu_safe_sub_u32(
							lts_stride, 1U))));
		index = nvgpu_safe_add_u32(index, 1U);
	}

	*priv_addr_table_index = index;
}

void gm20b_ltc_split_lts_broadcast_addr(struct gk20a *g, u32 addr,
					u32 *priv_addr_table,
					u32 *priv_addr_table_index)
{
	u32 num_ltc = g->ltc->ltc_count;
	u32 i, start, ltc_num = 0;
	u32 pltcg_base = ltc_pltcg_base_v();
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);

	for (i = 0; i < num_ltc; i++) {
		start = nvgpu_safe_add_u32(pltcg_base,
				nvgpu_safe_mult_u32(i, ltc_stride));
		if (addr >= start) {
			if (addr < nvgpu_safe_add_u32(start, ltc_stride)) {
				ltc_num = i;
				break;
			}
		}
	}
	gm20b_ltc_update_ltc_lts_addr(g, addr, ltc_num, priv_addr_table,
				priv_addr_table_index);
}

void gm20b_ltc_split_ltc_broadcast_addr(struct gk20a *g, u32 addr,
					u32 *priv_addr_table,
					u32 *priv_addr_table_index)
{
	u32 num_ltc = g->ltc->ltc_count;
	u32 ltc_num;

	for (ltc_num = 0; ltc_num < num_ltc; ltc_num =
					nvgpu_safe_add_u32(ltc_num, 1U)) {
		gm20b_ltc_update_ltc_lts_addr(g, addr, ltc_num,
					priv_addr_table, priv_addr_table_index);
	}
}
#endif /* CONFIG_NVGPU_DEBUGGER */
