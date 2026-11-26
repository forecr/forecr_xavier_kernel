/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZBC_GP10B_H
#define NVGPU_GR_ZBC_GP10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_zbc_table_indices;
struct nvgpu_gr_zbc_entry;

#ifdef CONFIG_NVGPU_ZBC_USER
void gp10b_gr_zbc_init_table_indices(struct gk20a *g,
			struct nvgpu_gr_zbc_table_indices *zbc_indices);
u32 gp10b_gr_zbc_get_gpcs_swdx_dss_zbc_c_format_reg(struct gk20a *g);
u32 gp10b_gr_zbc_get_gpcs_swdx_dss_zbc_z_format_reg(struct gk20a *g);
void gp10b_gr_zbc_add_color(struct gk20a *g,
			   struct nvgpu_gr_zbc_entry *color_val, u32 index);
#endif
void gp10b_gr_zbc_add_depth(struct gk20a *g,
			   struct nvgpu_gr_zbc_entry *depth_val, u32 index);

#endif /* NVGPU_GR_ZBC_GP10B_H */
