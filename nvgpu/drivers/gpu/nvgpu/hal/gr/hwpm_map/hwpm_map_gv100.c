// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/types.h>
#include <nvgpu/bitops.h>

#include "hwpm_map_gv100.h"

void gv100_gr_hwpm_map_align_regs_perf_pma(u32 *offset)
{
	*offset = NVGPU_ALIGN(*offset, 256U);
}

u32 gv100_gr_hwpm_map_get_active_fbpa_mask(struct gk20a *g)
{
	u32 active_fbpa_mask;
	u32 num_fbpas;

	num_fbpas = g->ops.top.get_max_fbpas_count(g);

	/*
	 * Read active fbpa mask from fuse
	 * Note that 0:enable and 1:disable in value read from fuse so we've to
	 * flip the bits.
	 * Also set unused bits to zero
	 */
	active_fbpa_mask = g->ops.fuse.fuse_status_opt_fbio(g);
	active_fbpa_mask = ~active_fbpa_mask;
	active_fbpa_mask = active_fbpa_mask & (BIT32(num_fbpas) - 1U);

	return active_fbpa_mask;
}

int gv100_hwpm_map_add_gpc_perf_entries_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		struct netlist_aiv_list *regs, u32 *count, u32 *offset,
		u32 max_cnt, u32 base, u32 mask, u32 gpc_num)
{
	u32 idx;
	u32 cnt = *count;
	u32 off = *offset;

	(void) g;
	(void) gpc_num;

	if (nvgpu_safe_add_u32(cnt, regs->count) > max_cnt) {
		return -EINVAL;
	}

	for (idx = 0; idx < regs->count; idx++) {
		map[cnt].addr = base + (regs->l[idx].addr & mask);
		map[cnt].offset = off;
		cnt = nvgpu_safe_add_u32(cnt, 1U);
		if (nvgpu_safe_add_u32_return(off, 4U, &off) == false) {
			nvgpu_err(g, "buffer overflow");
			return -EOVERFLOW;
		}
	}
	*count = cnt;
	*offset = off;
	return 0;
}
