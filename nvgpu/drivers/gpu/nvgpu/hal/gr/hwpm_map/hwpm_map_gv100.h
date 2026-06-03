/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_HWPM_MAP_GV100_H
#define NVGPU_GR_HWPM_MAP_GV100_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>
#include <nvgpu/netlist.h>
#include <nvgpu/gr/hwpm_map.h>

struct gk20a;

void gv100_gr_hwpm_map_align_regs_perf_pma(u32 *offset);
u32 gv100_gr_hwpm_map_get_active_fbpa_mask(struct gk20a *g);
int gv100_hwpm_map_add_gpc_perf_entries_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		struct netlist_aiv_list *regs, u32 *count, u32 *offset,
		u32 max_cnt, u32 base, u32 mask, u32 gpc_num);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_HWPM_MAP_GV100_H */
