/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZCULL_GM20B_H
#define NVGPU_GR_ZCULL_GM20B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;
#ifdef CONFIG_NVGPU_ZCULL
struct nvgpu_gr_zcull;
struct nvgpu_gr_zcull_info;
#endif
struct nvgpu_channel;

#ifdef CONFIG_NVGPU_ZCULL
int gm20b_gr_init_zcull_hw(struct gk20a *g,
			struct nvgpu_gr_zcull *gr_zcull,
			struct nvgpu_gr_config *gr_config);
int gm20b_gr_get_zcull_info(struct gk20a *g,
			struct nvgpu_gr_config *gr_config,
			struct nvgpu_gr_zcull *zcull,
			struct nvgpu_gr_zcull_info *zcull_params);
void gm20b_gr_program_zcull_mapping(struct gk20a *g, u32 zcull_num_entries,
					u32 *zcull_map_tiles);
#endif

#endif /* NVGPU_GR_ZCULL_GM20B_H */
