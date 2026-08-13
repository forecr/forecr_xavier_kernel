/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_GA10B_H
#define NVGPU_FB_GA10B_H

#define VPR_INFO_FETCH_POLL_MS	5U
#define ALIGN_HI32(x)	nvgpu_safe_sub_u32(32U, (x))

struct gk20a;

void ga10b_fb_init_fs_state(struct gk20a *g);
void ga10b_fb_init_hw(struct gk20a *g);
u32 ga10b_fb_get_num_active_ltcs(struct gk20a *g);
#ifdef CONFIG_NVGPU_VPR
void ga10b_fb_dump_vpr_info(struct gk20a *g);
int ga10b_fb_vpr_info_fetch(struct gk20a *g);
#endif
void ga10b_fb_dump_wpr_info(struct gk20a *g);
void ga10b_fb_read_wpr_info(struct gk20a *g, u64 *wpr_base, u64 *wpr_size);

#ifdef CONFIG_NVGPU_COMPRESSION

/**
 * @brief This function is used to initialize the GPU hardware for compression
 *  use case.
 *
 * Get and compute required information like compression alignment,
 *  physical address of the compbit memory.
 * Program the values in the required GPU registers.
 *
 * @param g     [in]    Pointer to the gk20a structure.
 * @param cbc   [in]    Pointer to the #nvgpu_cbc structure.
 *
 * @return None.
 */
void ga10b_fb_cbc_configure(struct gk20a *g, struct nvgpu_cbc *cbc);
#endif

#ifdef CONFIG_NVGPU_MIG
int ga10b_fb_config_veid_smc_map(struct gk20a *g, bool enable);
int ga10b_fb_set_smc_eng_config(struct gk20a *g, bool enable);
int ga10b_fb_set_remote_swizid(struct gk20a *g, bool enable);
void ga10b_fb_set_smc_veid_table_mask(struct gk20a *g, u32 veid_enable_mask);
#endif

int ga10b_fb_set_atomic_mode(struct gk20a *g);

#endif /* NVGPU_FB_GA10B_H */
