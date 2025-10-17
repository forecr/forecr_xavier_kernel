/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_HWPM_MAP_H
#define NVGPU_GR_HWPM_MAP_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;
struct ctxsw_buf_offset_map_entry;
struct nvgpu_gr_config;
struct nvgpu_gr_hwpm_map;
struct netlist_aiv_list;

struct ctxsw_buf_offset_map_entry {
	u32 addr;	/* Register address */
	u32 offset;	/* Offset in ctxt switch buffer */
};

struct nvgpu_gr_hwpm_map {
	u32 pm_ctxsw_image_size;

	u32 count;
	struct ctxsw_buf_offset_map_entry *map;

	bool init;
};

int nvgpu_gr_hwpm_map_init(struct gk20a *g, struct nvgpu_gr_hwpm_map **hwpm_map,
	u32 size);
void nvgpu_gr_hwpm_map_deinit(struct gk20a *g,
	struct nvgpu_gr_hwpm_map *hwpm_map);

u32 nvgpu_gr_hwpm_map_get_size(struct nvgpu_gr_hwpm_map *hwpm_map);

int nvgpu_gr_hwpm_map_create(struct gk20a *g,
	struct nvgpu_gr_hwpm_map *hwpm_map, struct nvgpu_gr_config *config);
int nvgpu_gr_hwmp_map_find_priv_offset(struct gk20a *g,
	struct nvgpu_gr_hwpm_map *hwpm_map,
	u32 addr, u32 *priv_offset, struct nvgpu_gr_config *config);
int add_ctxsw_buffer_map_entries_subunits(struct ctxsw_buf_offset_map_entry *map,
				struct netlist_aiv_list *regs,
				u32 *count, u32 *offset,
				u32 max_cnt, u32 base, u32 num_units,
				u32 active_unit_mask, u32 stride, u32 mask);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_HWPM_MAP_H */
