/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_HWPM_MAP_GB20C_H
#define NVGPU_GR_HWPM_MAP_GB20C_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>
#include <nvgpu/netlist.h>
#include <nvgpu/gr/hwpm_map.h>

struct gk20a;

int gb20c_hwpm_map_add_reg_in_fecs_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		u32 *count, u32 *offset, u32 max_reg_count);
int gb20c_hwpm_map_add_padding_for_dtdtpc_support(struct gk20a *g,
		struct netlist_aiv_list *regs, u32 *offset, u32 num_tpc);
int gb20c_hwpm_map_add_hes_reg_to_map(struct gk20a *g,
		struct ctxsw_buf_offset_map_entry *map,
		u32 *count, u32 *offset, u32 max_reg_count);
void gb20c_hwpm_map_get_cpc_info(struct gk20a *g, u32 *max_cpc_per_gpc,
		u32 *num_gnics_per_cpc, const u32 **gnic_stride_array,
		u32 *gnic_stride);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_HWPM_MAP_GB20C_H */
