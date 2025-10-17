/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef TOP_GM20B_H
#define TOP_GM20B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_device;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gm20b_device_info_parse_enum(struct gk20a *g, u32 table_entry,
					u32 *engine_id, u32 *runlist_id,
					u32 *intr_id, u32 *reset_id);
int gm20b_device_info_parse_data(struct gk20a *g, u32 table_entry, u32 *inst_id,
					u32 *pri_base, u32 *fault_id);

u32 gm20b_get_ce_inst_id(struct gk20a *g, u32 engine_type);

struct nvgpu_device *gm20b_top_parse_next_dev(struct gk20a *g, u32 *token);
#endif

u32 gm20b_top_get_max_gpc_count(struct gk20a *g);
u32 gm20b_top_get_max_tpc_per_gpc_count(struct gk20a *g);

u32 gm20b_top_get_max_fbps_count(struct gk20a *g);
u32 gm20b_top_get_max_ltc_per_fbp(struct gk20a *g);
u32 gm20b_top_get_max_lts_per_ltc(struct gk20a *g);
u32 gm20b_top_get_num_ltcs(struct gk20a *g);

#endif
