/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZBC_GB20C_H
#define NVGPU_GR_ZBC_GB20C_H

#include <nvgpu/types.h>

#define data_index_0	0U
#define data_index_1	1U
#define data_index_2	2U
#define data_index_3	3U

struct gk20a;
struct nvgpu_gr_zbc_entry;

u32 gb20c_gr_pri_gpcs_rops_crop_zbc_index_address_max(void);
void gb20c_gr_zbc_add_color(struct gk20a *g,
			struct nvgpu_gr_zbc_entry *color_val, u32 index);
#endif /* NVGPU_GR_ZBC_GB20C_H */
