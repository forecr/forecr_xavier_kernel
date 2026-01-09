/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_INTR_ECC_GA10B_H
#define NVGPU_FB_INTR_ECC_GA10B_H

struct gk20a;

void ga10b_fb_intr_handle_ecc_l2tlb(struct gk20a *g, u32 ecc_status);
void ga10b_fb_intr_handle_ecc_hubtlb(struct gk20a *g, u32 ecc_status);
void ga10b_fb_intr_handle_ecc_fillunit(struct gk20a *g, u32 ecc_status);

#endif /* NVGPU_FB_INTR_ECC_GA10B_H */