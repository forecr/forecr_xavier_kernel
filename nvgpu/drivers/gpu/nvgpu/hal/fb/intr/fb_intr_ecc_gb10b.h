/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_INTR_ECC_GB10B_H
#define NVGPU_FB_INTR_ECC_GB10B_H

#include <nvgpu/types.h>

struct gk20a;

u32 gb10b_fb_intr_read_l2tlb_ecc_status(struct gk20a *g);
u32 gb10b_fb_intr_read_hubtlb_ecc_status(struct gk20a *g);
u32 gb10b_fb_intr_read_fillunit_ecc_status(struct gk20a *g);

void gb10b_fb_intr_get_l2tlb_ecc_info(struct gk20a *g, u32 *ecc_addr,
				u32 *corrected_cnt, u32 *uncorrected_cnt);
void gb10b_fb_intr_get_hubtlb_ecc_info(struct gk20a *g, u32 *ecc_addr,
				u32 *corrected_cnt, u32 *uncorrected_cnt);
void gb10b_fb_intr_get_fillunit_ecc_info(struct gk20a *g, u32 *ecc_addr,
				u32 *corrected_cnt, u32 *uncorrected_cnt);

void gb10b_fb_intr_clear_ecc_l2tlb(struct gk20a *g,
				bool clear_corrected, bool clear_uncorrected);
void gb10b_fb_intr_clear_ecc_hubtlb(struct gk20a *g,
				bool clear_corrected, bool clear_uncorrected);
void gb10b_fb_intr_clear_ecc_fillunit(struct gk20a *g,
				bool clear_corrected, bool clear_uncorrected);

#endif /* NVGPU_FB_INTR_ECC_GB10B_H */
