/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_HWPM_MAP_GA10B_H
#define NVGPU_GR_HWPM_MAP_GA10B_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>
#include <nvgpu/netlist.h>
#include <nvgpu/gr/hwpm_map.h>

struct gk20a;

int ga10b_hwpm_map_add_reg_in_fecs_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		u32 *count, u32 *offset, u32 max_reg_count);

int ga10b_hwpm_map_add_reg_in_gpccs_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		u32 *count, u32 *offset, u32 max_reg_count,
		struct nvgpu_gr_config *config);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_HWPM_MAP_GA10B_H */
