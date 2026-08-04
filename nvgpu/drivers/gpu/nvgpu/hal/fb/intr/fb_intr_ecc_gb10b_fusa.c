// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "fb_intr_ecc_gb10b.h"

#include <nvgpu/hw/gb10b/hw_fb_gb10b.h>

u32 gb10b_fb_intr_read_l2tlb_ecc_status(struct gk20a *g)
{
	return nvgpu_readl(g, fb_mmu_l2tlb_ecc_status_r());
}

u32 gb10b_fb_intr_read_hubtlb_ecc_status(struct gk20a *g)
{
	return nvgpu_readl(g, fb_mmu_hubtlb_ecc_status_r());
}

u32 gb10b_fb_intr_read_fillunit_ecc_status(struct gk20a *g)
{
	return nvgpu_readl(g, fb_mmu_fillunit_ecc_status_r());
}

void gb10b_fb_intr_get_l2tlb_ecc_info(struct gk20a *g, u32 *ecc_addr,
				u32 *corrected_cnt, u32 *uncorrected_cnt)
{
	*ecc_addr = nvgpu_readl(g, fb_mmu_l2tlb_ecc_address_r());
	*corrected_cnt = nvgpu_readl(g,
		   	fb_mmu_l2tlb_ecc_corrected_err_count_r());
	*uncorrected_cnt = nvgpu_readl(g,
		   	fb_mmu_l2tlb_ecc_uncorrected_err_count_r());
}

void gb10b_fb_intr_get_hubtlb_ecc_info(struct gk20a *g, u32 *ecc_addr,
				u32 *corrected_cnt, u32 *uncorrected_cnt)
{
	*ecc_addr = nvgpu_readl(g, fb_mmu_hubtlb_ecc_address_r());
	*corrected_cnt = nvgpu_readl(g,
		   	fb_mmu_hubtlb_ecc_corrected_err_count_r());
	*uncorrected_cnt = nvgpu_readl(g,
		   	fb_mmu_hubtlb_ecc_uncorrected_err_count_r());
}

void gb10b_fb_intr_get_fillunit_ecc_info(struct gk20a *g, u32 *ecc_addr,
				u32 *corrected_cnt, u32 *uncorrected_cnt)
{
	*ecc_addr = nvgpu_readl(g, fb_mmu_fillunit_ecc_address_r());
	*corrected_cnt = nvgpu_readl(g,
		   	fb_mmu_fillunit_ecc_corrected_err_count_r());
	*uncorrected_cnt = nvgpu_readl(g,
		   	fb_mmu_fillunit_ecc_uncorrected_err_count_r());
}

void gb10b_fb_intr_clear_ecc_l2tlb(struct gk20a *g,
	   	bool clear_corrected, bool clear_uncorrected)
{
	if (clear_corrected) {
		nvgpu_writel(g, fb_mmu_l2tlb_ecc_corrected_err_count_r(), 0);
	}
	if (clear_uncorrected) {
		nvgpu_writel(g, fb_mmu_l2tlb_ecc_uncorrected_err_count_r(), 0);
	}
	nvgpu_writel(g, fb_mmu_l2tlb_ecc_status_r(),
		   	fb_mmu_l2tlb_ecc_status_reset_clear_f());
}

void gb10b_fb_intr_clear_ecc_hubtlb(struct gk20a *g,
	   	bool clear_corrected, bool clear_uncorrected)
{
	if (clear_corrected) {
		nvgpu_writel(g, fb_mmu_hubtlb_ecc_corrected_err_count_r(), 0);
	}
	if (clear_uncorrected) {
		nvgpu_writel(g, fb_mmu_hubtlb_ecc_uncorrected_err_count_r(), 0);
	}
	nvgpu_writel(g, fb_mmu_hubtlb_ecc_status_r(),
		   	fb_mmu_hubtlb_ecc_status_reset_clear_f());
}

void gb10b_fb_intr_clear_ecc_fillunit(struct gk20a *g,
	   	bool clear_corrected, bool clear_uncorrected)
{
	if (clear_corrected) {
		nvgpu_writel(g, fb_mmu_fillunit_ecc_corrected_err_count_r(), 0);
	}
	if (clear_uncorrected) {
		nvgpu_writel(g, fb_mmu_fillunit_ecc_uncorrected_err_count_r(), 0);
	}
	nvgpu_writel(g, fb_mmu_fillunit_ecc_status_r(),
		   	fb_mmu_fillunit_ecc_status_reset_clear_f());
}
