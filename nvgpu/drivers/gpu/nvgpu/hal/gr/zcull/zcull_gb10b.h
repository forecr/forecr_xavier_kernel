/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZCULL_GB10B_H
#define NVGPU_GR_ZCULL_GB10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;
struct nvgpu_gr_zcull;

int gb10b_gr_init_zcull_hw(struct gk20a *g,
			struct nvgpu_gr_zcull *gr_zcull,
			struct nvgpu_gr_config *gr_config);
void gb10b_gr_program_zcull_mapping(struct gk20a *g, u32 zcull_num_entries,
				u32 *zcull_map_tiles);
#endif /* NVGPU_GR_ZCULL_GB10B_H */
