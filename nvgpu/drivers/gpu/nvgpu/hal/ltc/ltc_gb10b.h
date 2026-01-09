/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_GB10B_H
#define NVGPU_LTC_GB10B_H

#include <nvgpu/types.h>

struct nvgpu_tsg;
struct gk20a;

#ifdef CONFIG_NVGPU_GRAPHICS
void gb10b_ltc_set_zbc_stencil_entry(struct gk20a *g, u32 stencil_depth,
			u32 index);
void gb10b_ltc_set_zbc_color_entry(struct gk20a *g, u32 *color_l2, u32 index);
void gb10b_ltc_set_zbc_depth_entry(struct gk20a *g, u32 depth_val, u32 index);
#endif

void gb10b_ltc_init_fs_state(struct gk20a *g);
u64 gb10b_determine_L2_size_bytes(struct gk20a *g);
#ifndef CONFIG_NVGPU_NON_FUSA
void gb10b_set_default_l2_max_ways_evict_last(struct gk20a *g,
			struct nvgpu_gr_ctx *gr_ctx);
#endif
#if defined(CONFIG_NVGPU_NON_FUSA)
u32 gb10b_ltcs_ltss_tstg_cfg1_active_sets(u32 reg_val);
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
bool gb10b_ltc_pri_is_ltc_addr(struct gk20a *g, u32 addr);
bool gb10b_ltc_is_pltcg_ltcs_addr(struct gk20a *g, u32 addr);
bool gb10b_ltc_is_ltcs_ltss_addr(struct gk20a *g, u32 addr);
int gb10b_ltc_pri_shared_addr(struct gk20a *g, u32 addr, u32 *ltc_shared_base);
int gb10b_set_l2_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 num_ways);
int gb10b_get_l2_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 *num_ways);

int gb10b_set_l2_sector_promotion(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 policy);
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) || defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT)
void gb10b_ltc_set_enabled(struct gk20a *g, bool enabled);
#endif

void gb10b_flush_ltc(struct gk20a *g);
int gb10b_ltc_compute_lts_mask(struct gk20a *g, u64 logical_ltc_mask, u64 *logical_lts_mask);

#ifdef CONFIG_NVGPU_MIG
void gb10b_ltc_set_remote_swizzle_id(struct gk20a *g, int gridx, u32 swizzle_id);
#endif

#endif /* NVGPU_LTC_GB10B_H */
