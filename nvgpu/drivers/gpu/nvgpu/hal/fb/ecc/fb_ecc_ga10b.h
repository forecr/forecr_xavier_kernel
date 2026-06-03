/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_ECC_GA10B_H
#define NVGPU_FB_ECC_GA10B_H

struct gk20a;

int ga10b_fb_ecc_init(struct gk20a *g);
void ga10b_fb_ecc_free(struct gk20a *g);
void ga10b_fb_ecc_l2tlb_error_mask(u32 *corrected_error_mask,
		u32 *uncorrected_error_mask);

#endif /* NVGPU_FB_ECC_GA10B_H */
