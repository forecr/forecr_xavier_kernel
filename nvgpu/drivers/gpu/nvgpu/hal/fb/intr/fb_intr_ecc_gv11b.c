// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>

#include "fb_intr_ecc_gv11b.h"

#include <nvgpu/hw/gv11b/hw_fb_gv11b.h>

#ifdef CONFIG_NVGPU_INJECT_HWERR
void gv11b_fb_intr_inject_hubmmu_ecc_error(struct gk20a *g,
		struct nvgpu_hw_err_inject_info *err,
		u32 error_info)
{
	unsigned int reg_addr = err->get_reg_addr();

	(void)error_info;
	nvgpu_info(g, "Injecting HUBMMU fault %s", err->name);
	nvgpu_writel(g, reg_addr, err->get_reg_val(1U));
}

static inline u32 l2tlb_ecc_control_r(void)
{
	return fb_mmu_l2tlb_ecc_control_r();
}

static inline u32 l2tlb_ecc_control_inject_uncorrected_err_f(u32 v)
{
	return fb_mmu_l2tlb_ecc_control_inject_uncorrected_err_f(v);
}

static inline u32 hubtlb_ecc_control_r(void)
{
	return fb_mmu_hubtlb_ecc_control_r();
}

static inline u32 hubtlb_ecc_control_inject_uncorrected_err_f(u32 v)
{
	return fb_mmu_hubtlb_ecc_control_inject_uncorrected_err_f(v);
}

static inline u32 fillunit_ecc_control_r(void)
{
	return fb_mmu_fillunit_ecc_control_r();
}

static inline u32 fillunit_ecc_control_inject_uncorrected_err_f(u32 v)
{
	return fb_mmu_fillunit_ecc_control_inject_uncorrected_err_f(v);
}

static struct nvgpu_hw_err_inject_info hubmmu_ecc_err_desc[] = {
	NVGPU_ECC_ERR("hubmmu_l2tlb_sa_data_ecc_uncorrected",
			gv11b_fb_intr_inject_hubmmu_ecc_error,
			l2tlb_ecc_control_r,
			l2tlb_ecc_control_inject_uncorrected_err_f),
	NVGPU_ECC_ERR("hubmmu_tlb_sa_data_ecc_uncorrected",
			gv11b_fb_intr_inject_hubmmu_ecc_error,
			hubtlb_ecc_control_r,
			hubtlb_ecc_control_inject_uncorrected_err_f),
	NVGPU_ECC_ERR("hubmmu_pte_data_ecc_uncorrected",
			gv11b_fb_intr_inject_hubmmu_ecc_error,
			fillunit_ecc_control_r,
			fillunit_ecc_control_inject_uncorrected_err_f),
};

static struct nvgpu_hw_err_inject_info_desc hubmmu_err_desc;

struct nvgpu_hw_err_inject_info_desc *
gv11b_fb_intr_get_hubmmu_err_desc(struct gk20a *g)
{
	(void)g;

	hubmmu_err_desc.info_ptr = hubmmu_ecc_err_desc;
	hubmmu_err_desc.info_size = nvgpu_safe_cast_u64_to_u32(
			sizeof(hubmmu_ecc_err_desc) /
			sizeof(struct nvgpu_hw_err_inject_info));

	return &hubmmu_err_desc;
}
#endif /* CONFIG_NVGPU_INJECT_HWERR */
