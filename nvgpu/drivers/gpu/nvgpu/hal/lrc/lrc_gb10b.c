// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/regops.h>

#include "lrc_gb10b.h"

#include <nvgpu/hw/gb10b/hw_lrc_gb10b.h>

#ifdef CONFIG_NVGPU_DEBUGGER

/**
 * This api checks if the input addr belongs to LRC pri space
 * Both unicast and broadcast registers belong to a single
 * space.
 * LRCC(N)_LRC(N)
 * N stands for both unicast and broadcast semantics(0,1,2,3,S)
 */
bool gb10b_lrc_pri_is_lrcc_addr(struct gk20a *g, u32 addr)
{
	(void)g;
	if ((addr >= lrc_plrcc_base_v()) && (addr < lrc_plrcc_extent_v())) {
		return true;
	}

	return false;
}

/**
 * API to check if the addr is a LRC broadcast address.
 *
 * Following registers are valid LRC broadcast reg.
 * LRCC0_LRCS
 * LRCC1_LRCS
 * LRCC2_LRCS
 * LRCC3_LRCS
 */
bool gb10b_lrc_pri_is_lrcs_addr(struct gk20a *g, u32 addr)
{
	const u32 lrc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRC_STRIDE);
	const u32 lrcc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRCC_STRIDE);
	u32 num_lrc = g->ltc->ltc_count;
	u32 lrcc0_base, lrcc_mask, lrccn_extent;

	lrcc0_base = lrc_plrcc_base_v() + lrcc_stride;
	lrccn_extent = lrcc0_base + ((num_lrc - 1U) * lrcc_stride) + lrc_stride;
	lrcc_mask = lrcc_stride - 1U;

	if (g->ops.lrc.pri_is_lrcc_addr(g, addr)) {
		if ((addr > lrcc0_base) && (addr <= lrccn_extent)) {
			if ((addr & lrcc_mask) < lrc_stride) {
				return true;
			}
		}
	}

	return false;
}

/**
 * API to check if the addr is a LRCC & LRC broadcast address.
 *
 * Following register is a valid LRCC LRC broadcast reg.
 * LRCCS_LRCS
 */
bool gb10b_lrc_pri_is_lrcs_lrccs_addr(struct gk20a *g, u32 addr)
{
	const u32 lrc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRC_STRIDE);
	u32 lrccs_lrcs_extent = lrc_plrcc_base_v() + lrc_stride;

	if ((addr >= lrc_plrcc_base_v()) && (addr < lrccs_lrcs_extent)) {
		return true;
	}

	return false;
}

/**
 * Supporting API to generate unicast registers for broadcast registers of type
 * LRCC(N)_LRCS where N is fixed by the caller.
 */
static void gb10b_lrc_generate_lrcc_lrc_addr(struct gk20a *g, u32 addr,
					u32 lrcc_num, u32 *priv_addr_table,
					u32 *priv_addr_table_index)
{
	u32 num_lrc = g->ltc->slices_per_ltc;
	u32 index = *priv_addr_table_index;
	const u32 lrc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRC_STRIDE);
	const u32 lrcc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRCC_STRIDE);
	u32 lrcc0_lrc0_base = lrc_lrcc0_lrc0_v();
	u32 lrc_num;

	for (lrc_num = 0; lrc_num < num_lrc; lrc_num++) {
		priv_addr_table[index] = lrcc0_lrc0_base +
			((lrcc_num * lrcc_stride) +
			 (lrc_num * lrc_stride) +
			 (addr & (lrc_stride - 1U)));
		nvgpu_log(g, gpu_dbg_gpu_dbg, "addr[%u]- 0x%x",index, priv_addr_table[index]);
		index++;
	}

	*priv_addr_table_index = index;
}

/**
 * API to generate unicast registers for broadcast registers of type
 * LRCC(N)_LRCS where N is figured out from the the input addr.
 */
void gb10b_lrc_split_lrc_broadcast_addr(struct gk20a *g, u32 addr,
					u32 *priv_addr_table,
					u32 *priv_addr_table_index)
{
	// Convert LRCC(N)_LRCS broadcast addr to its unicast addr
	const u32 lrcc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRCC_STRIDE);
	u32 lrcc_num;
	u32 lrcc0_lrc_base;

	lrcc0_lrc_base = lrc_plrcc_base_v() + lrcc_stride;

	lrcc_num = (addr - lrcc0_lrc_base)/lrcc_stride;

	nvgpu_log(g, gpu_dbg_gpu_dbg, "addr - 0x%x, lrcc_num - %u",
			addr, lrcc_num);

	gb10b_lrc_generate_lrcc_lrc_addr(g, addr, lrcc_num,
			priv_addr_table, priv_addr_table_index);
}

/**
 * API to generate unicast registers for broadcast registers of type
 * LRCCS_LRCS
 */
void gb10b_lrc_split_lrcc_broadcast_addr(struct gk20a *g, u32 addr,
					u32 *priv_addr_table,
					u32 *priv_addr_table_index)
{
	// Convert LRCCS_LRCS broadcast addr to its unicast addr
	u32 num_lrcc = g->ltc->ltc_count;
	u32 lrcc_num;

	for (lrcc_num = 0; lrcc_num < num_lrcc; lrcc_num++) {
		gb10b_lrc_generate_lrcc_lrc_addr(g, addr, lrcc_num,
				priv_addr_table, priv_addr_table_index);
	}
}
#endif
