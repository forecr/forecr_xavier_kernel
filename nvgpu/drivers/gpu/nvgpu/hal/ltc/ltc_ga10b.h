/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef LTC_GA10B_H
#define LTC_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_ctx;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
u32 ga10b_ltc_zbc_table_size(struct gk20a *g);
#endif
#ifdef CONFIG_NVGPU_GRAPHICS
void ga10b_ltc_set_zbc_stencil_entry(struct gk20a *g, u32 stencil_depth,
			u32 index);
void ga10b_ltc_set_zbc_color_entry(struct gk20a *g, u32 *color_l2, u32 index);
void ga10b_ltc_set_zbc_depth_entry(struct gk20a *g, u32 depth_val, u32 index);
#endif

void ga10b_ltc_init_fs_state(struct gk20a *g);
int ga10b_ltc_lts_set_mgmt_setup(struct gk20a *g);
u64 ga10b_determine_L2_size_bytes(struct gk20a *g);
int ga10b_lts_ecc_init(struct gk20a *g);
#ifndef CONFIG_NVGPU_NON_FUSA
void ga10b_set_default_l2_max_ways_evict_last(struct gk20a *g,
			struct nvgpu_gr_ctx *gr_ctx);
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
int ga10b_ltc_pri_shared_addr(struct gk20a *g, u32 addr, u32 *ltc_shared_addr) ;
int ga10b_set_l2_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 num_ways);
int ga10b_get_l2_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 *num_ways);
#endif

#endif /* LTC_GA10B_H */
