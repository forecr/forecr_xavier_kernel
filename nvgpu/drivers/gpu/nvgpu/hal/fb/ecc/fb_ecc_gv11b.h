/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_ECC_GV11B_H
#define NVGPU_FB_ECC_GV11B_H

struct gk20a;

/*
 * @brief Allocate and initialize counters for memories within FB.
 *
 * @param stat [in] Address of pointer to struct nvgpu_ecc_stat.
 *
 */
#define NVGPU_ECC_COUNTER_INIT_FB(stat) \
	nvgpu_ecc_counter_init(g, &g->ecc.fb.stat, #stat)

#define NVGPU_ECC_COUNTER_FREE_FB(stat)	\
	nvgpu_ecc_counter_deinit(g, &g->ecc.fb.stat)

int gv11b_fb_ecc_init(struct gk20a *g);
void gv11b_fb_ecc_free(struct gk20a *g);
void gv11b_fb_ecc_l2tlb_error_mask(u32 *corrected_error_mask,
		u32 *uncorrected_error_mask);

#endif /* NVGPU_FB_ECC_GV11B_H */
