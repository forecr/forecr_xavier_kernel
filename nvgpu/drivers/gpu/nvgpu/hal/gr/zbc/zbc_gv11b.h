/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZBC_GV11B_H
#define NVGPU_GR_ZBC_GV11B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_zbc_table_indices;
struct nvgpu_gr_zbc_entry;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gv11b_gr_zbc_init_table_indices(struct gk20a *g,
			struct nvgpu_gr_zbc_table_indices *zbc_indices);
#endif
#ifdef CONFIG_NVGPU_ZBC_USER
u32 gv11b_gr_zbc_get_gpcs_swdx_dss_zbc_c_format_reg(struct gk20a *g);
#endif
u32 gv11b_gr_zbc_get_gpcs_swdx_dss_zbc_z_format_reg(struct gk20a *g);
void gv11b_gr_zbc_add_stencil(struct gk20a *g,
			     struct nvgpu_gr_zbc_entry *stencil_val, u32 index);
#endif /* NVGPU_GR_ZBC_GV11B_H */
