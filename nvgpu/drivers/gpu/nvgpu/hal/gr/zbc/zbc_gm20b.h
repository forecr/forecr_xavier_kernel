/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZBC_GM20B_H
#define NVGPU_GR_ZBC_GM20B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_zbc_table_indices;
struct nvgpu_gr_zbc_entry;
struct nvgpu_gr_zbc;

#ifdef CONFIG_NVGPU_ZBC_USER
void gm20b_gr_zbc_init_table_indices(struct gk20a *g,
			struct nvgpu_gr_zbc_table_indices *zbc_indices);
void gm20b_gr_zbc_add_color(struct gk20a *g,
			struct nvgpu_gr_zbc_entry *color_val, u32 index);
void gm20b_gr_zbc_add_depth(struct gk20a *g,
			struct nvgpu_gr_zbc_entry *depth_val, u32 index);
void gm20b_gr_zbc_load_default_sw_table(struct gk20a *g,
					struct nvgpu_gr_zbc *zbc);
#endif
#endif /* NVGPU_GR_ZBC_GM20B_H */
