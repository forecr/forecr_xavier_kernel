// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>

#include "fb_ecc_gv11b.h"

#include <nvgpu/hw/gv11b/hw_fb_gv11b.h>

int gv11b_fb_ecc_init(struct gk20a *g)
{
	int err = 0;

	err = NVGPU_ECC_COUNTER_INIT_FB(mmu_l2tlb_ecc_uncorrected_err_count);
	if (err != 0) {
		goto init_fb_ecc_err;
	}
	err = NVGPU_ECC_COUNTER_INIT_FB(mmu_l2tlb_ecc_corrected_err_count);
	if (err != 0) {
		goto init_fb_ecc_err;
	}
	err = NVGPU_ECC_COUNTER_INIT_FB(mmu_hubtlb_ecc_uncorrected_err_count);
	if (err != 0) {
		goto init_fb_ecc_err;
	}
	err = NVGPU_ECC_COUNTER_INIT_FB(mmu_hubtlb_ecc_corrected_err_count);
	if (err != 0) {
		goto init_fb_ecc_err;
	}
	err = NVGPU_ECC_COUNTER_INIT_FB(
			mmu_fillunit_ecc_uncorrected_err_count);
	if (err != 0) {
		goto init_fb_ecc_err;
	}
	err = NVGPU_ECC_COUNTER_INIT_FB(
			mmu_fillunit_ecc_corrected_err_count);
	if (err != 0) {
		goto init_fb_ecc_err;
	}

init_fb_ecc_err:

	if (err != 0) {
		nvgpu_err(g, "ecc counter allocate failed, err=%d", err);
		gv11b_fb_ecc_free(g);
	}

	return err;
}

void gv11b_fb_ecc_free(struct gk20a *g)
{
	NVGPU_ECC_COUNTER_FREE_FB(mmu_l2tlb_ecc_corrected_err_count);
	NVGPU_ECC_COUNTER_FREE_FB(mmu_l2tlb_ecc_uncorrected_err_count);
	NVGPU_ECC_COUNTER_FREE_FB(mmu_hubtlb_ecc_corrected_err_count);
	NVGPU_ECC_COUNTER_FREE_FB(mmu_hubtlb_ecc_uncorrected_err_count);
	NVGPU_ECC_COUNTER_FREE_FB(mmu_fillunit_ecc_corrected_err_count);
	NVGPU_ECC_COUNTER_FREE_FB(mmu_fillunit_ecc_uncorrected_err_count);
}

void gv11b_fb_ecc_l2tlb_error_mask(u32 *corrected_error_mask,
		u32 *uncorrected_error_mask)
{
	*corrected_error_mask =
		fb_mmu_l2tlb_ecc_status_corrected_err_l2tlb_sa_data_m();
	*uncorrected_error_mask =
		fb_mmu_l2tlb_ecc_status_uncorrected_err_l2tlb_sa_data_m();

	return;
}
