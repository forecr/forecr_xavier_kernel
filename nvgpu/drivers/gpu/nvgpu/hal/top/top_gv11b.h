/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TOP_GV11B_H
#define NVGPU_TOP_GV11B_H

#include <nvgpu/types.h>

struct gk20a;

u32 gv11b_top_get_num_lce(struct gk20a *g);
u32 gv11b_top_get_max_pes_per_gpc(struct gk20a *g);
int gv11b_device_info_parse_data(struct gk20a *g, u32 table_entry, u32 *inst_id,
		u32 *pri_base, u32 *fault_id);

#endif /* NVGPU_TOP_GV11B_H */
