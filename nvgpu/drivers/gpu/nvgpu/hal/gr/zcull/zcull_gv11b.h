/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZCULL_GV11B_H
#define NVGPU_GR_ZCULL_GV11B_H

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_ZCULL
void gv11b_gr_program_zcull_mapping(struct gk20a *g, u32 zcull_num_entries,
				u32 *zcull_map_tiles);
#endif

#endif /* NVGPU_GR_ZCULL_GV11B_H */
