/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_HWPM_MAP_GB10B_H
#define NVGPU_GR_HWPM_MAP_GB10B_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>
#include <nvgpu/netlist.h>
#include <nvgpu/gr/hwpm_map.h>

struct gk20a;

int gb10b_hwpm_map_add_gpc_perf_entries_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		struct netlist_aiv_list *regs, u32 *count, u32 *offset,
		u32 max_cnt, u32 base, u32 mask, u32 gpc_num);

int gb10b_hwpm_map_add_pm_ucgpc_entries_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		struct netlist_aiv_list *regs, u32 *count, u32 *offset,
		u32 max_cnt, u32 base, u32 mask, u32 gpc_num);

int gb10b_hwpm_map_add_lrcc_entries_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		struct netlist_aiv_list *regs, u32 *count, u32 *offset,
		u32 max_cnt, u32 num_lrc);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_HWPM_MAP_GB10B_H */
