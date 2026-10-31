/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_INTR_ECC_GV11B_H
#define NVGPU_FB_INTR_ECC_GV11B_H

#include <nvgpu/types.h>
#include <nvgpu/nvgpu_err.h>

struct gk20a;
struct nvgpu_hw_err_inject_info;
struct nvgpu_hw_err_inject_info_desc;

u32 gv11b_fb_intr_read_l2tlb_ecc_status(struct gk20a *g);
u32 gv11b_fb_intr_read_hubtlb_ecc_status(struct gk20a *g);
u32 gv11b_fb_intr_read_fillunit_ecc_status(struct gk20a *g);

void gv11b_fb_intr_get_l2tlb_ecc_info(struct gk20a *g, u32 *ecc_addr,
				u32 *corrected_cnt, u32 *uncorrected_cnt);
void gv11b_fb_intr_get_hubtlb_ecc_info(struct gk20a *g, u32 *ecc_addr,
				u32 *corrected_cnt, u32 *uncorrected_cnt);
void gv11b_fb_intr_get_fillunit_ecc_info(struct gk20a *g, u32 *ecc_addr,
				u32 *corrected_cnt, u32 *uncorrected_cnt);

void gv11b_fb_intr_clear_ecc_l2tlb(struct gk20a *g,
				bool clear_corrected, bool clear_uncorrected);
void gv11b_fb_intr_clear_ecc_hubtlb(struct gk20a *g,
				bool clear_corrected, bool clear_uncorrected);
void gv11b_fb_intr_clear_ecc_fillunit(struct gk20a *g,
				bool clear_corrected, bool clear_uncorrected);

void gv11b_fb_intr_handle_ecc(struct gk20a *g);
void gv11b_fb_intr_handle_ecc_l2tlb(struct gk20a *g, u32 ecc_status);
void gv11b_fb_intr_handle_ecc_fillunit(struct gk20a *g, u32 ecc_status);
void gv11b_fb_intr_handle_ecc_hubtlb(struct gk20a *g, u32 ecc_status);

#ifdef CONFIG_NVGPU_INJECT_HWERR
struct nvgpu_hw_err_inject_info_desc *
		gv11b_fb_intr_get_hubmmu_err_desc(struct gk20a *g);
void gv11b_fb_intr_inject_hubmmu_ecc_error(struct gk20a *g,
		struct nvgpu_hw_err_inject_info *err, u32 error_info);
#endif

#endif /* NVGPU_FB_INTR_ECC_GV11B_H */
