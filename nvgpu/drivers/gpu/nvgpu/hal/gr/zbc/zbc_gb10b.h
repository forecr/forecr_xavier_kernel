/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZBC_GB10B_H
#define NVGPU_GR_ZBC_GB10B_H

#include <nvgpu/types.h>

#define data_index_0	0U
#define data_index_1	1U
#define data_index_2	2U
#define data_index_3	3U

struct gk20a;
struct nvgpu_gr_zbc_table_indices;
struct nvgpu_gr_zbc_entry;

u32 gb10b_gr_pri_gpcs_rops_crop_zbc_index_address_max(void);
void gb10b_gr_zbc_init_table_indices(struct gk20a *g,
			struct nvgpu_gr_zbc_table_indices *zbc_indices);
u32 gb10b_gr_zbc_get_gpcs_swdx_dss_zbc_c_format_reg(struct gk20a *g);
u32 gb10b_gr_zbc_get_gpcs_swdx_dss_zbc_z_format_reg(struct gk20a *g);
void gb10b_gr_zbc_add_stencil(struct gk20a *g,
			     struct nvgpu_gr_zbc_entry *stencil_val, u32 index);
void gb10b_gr_zbc_add_depth(struct gk20a *g,
			   struct nvgpu_gr_zbc_entry *depth_val, u32 index);
void gb10b_gr_zbc_add_color(struct gk20a *g,
			struct nvgpu_gr_zbc_entry *color_val, u32 index);
#endif /* NVGPU_GR_ZBC_GB10B_H */
