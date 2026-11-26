// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/ecc.h>
#include <nvgpu/gk20a.h>

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>

#include "ecc_gb10b.h"

#ifdef CONFIG_NVGPU_INJECT_HWERR
void gb10b_gr_intr_inject_sm_ecc_error(struct gk20a *g,
		struct nvgpu_hw_err_inject_info *err,
		u32 error_info)
{
	unsigned int gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	unsigned int tpc_stride =
		nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	unsigned int gpc = (error_info & 0xFF00U) >> 8U;
	unsigned int tpc = (error_info & 0xFFU);
	unsigned int reg_addr = nvgpu_safe_add_u32(err->get_reg_addr(),
					nvgpu_safe_add_u32(
					nvgpu_safe_mult_u32(gpc, gpc_stride),
					nvgpu_safe_mult_u32(tpc, tpc_stride)));

	nvgpu_info(g, "Injecting SM fault %s for gpc: %d, tpc: %d",
			err->name, gpc, tpc);
	nvgpu_writel(g, reg_addr, err->get_reg_val(1U));
}

static inline u32 pri_gpc0_tpc0_sm_l1_tag_ecc_control_r(void)
{
	return gr_pri_gpc0_tpc0_sm_l1_tag_ecc_control_r();
}

static inline u32 pri_gpc0_tpc0_sm_l1_tag_ecc_control_inject_corrected_err_f(u32 v)
{
	return gr_pri_gpc0_tpc0_sm_l1_tag_ecc_control_inject_corrected_err_f(v);
}

static inline u32 pri_gpc0_tpc0_sm_l1_tag_ecc_control_inject_uncorrected_err_f(u32 v)
{
	return gr_pri_gpc0_tpc0_sm_l1_tag_ecc_control_inject_uncorrected_err_f(v);
}

static inline u32 pri_gpc0_tpc0_sm_cbu_ecc_control_r(void)
{
	return gr_pri_gpc0_tpc0_sm_cbu_ecc_control_r();
}

static inline u32 pri_gpc0_tpc0_sm_cbu_ecc_control_inject_uncorrected_err_f(u32 v)
{
	return gr_pri_gpc0_tpc0_sm_cbu_ecc_control_inject_uncorrected_err_f(v);
}

static inline u32 pri_gpc0_tpc0_sm_lrf_ecc_control_r(void)
{
	return gr_pri_gpc0_tpc0_sm_lrf_ecc_control_r();
}

static inline u32 pri_gpc0_tpc0_sm_lrf_ecc_control_inject_uncorrected_err_f(u32 v)
{
	return gr_pri_gpc0_tpc0_sm_lrf_ecc_control_inject_uncorrected_err_f(v);
}

static inline u32 pri_gpc0_tpc0_sm_l1_data_ecc_control_r(void)
{
	return gr_pri_gpc0_tpc0_sm_l1_data_ecc_control_r();
}

static inline u32 pri_gpc0_tpc0_sm_l1_data_ecc_control_inject_uncorrected_err_f(u32 v)
{
	return gr_pri_gpc0_tpc0_sm_l1_data_ecc_control_inject_uncorrected_err_f(v);
}

static inline u32 pri_gpc0_tpc0_sm_icache_ecc_control_r(void)
{
	return gr_pri_gpc0_tpc0_sm_icache_ecc_control_r();
}

static inline u32 pri_gpc0_tpc0_sm_icache_ecc_control_inject_uncorrected_err_f(u32 v)
{
	return gr_pri_gpc0_tpc0_sm_icache_ecc_control_inject_uncorrected_err_f(v);
}

static struct nvgpu_hw_err_inject_info sm_ecc_err_desc[] = {
	NVGPU_ECC_ERR("l1_tag_ecc_corrected",
			gb10b_gr_intr_inject_sm_ecc_error,
			pri_gpc0_tpc0_sm_l1_tag_ecc_control_r,
			pri_gpc0_tpc0_sm_l1_tag_ecc_control_inject_corrected_err_f),
	NVGPU_ECC_ERR("l1_tag_ecc_uncorrected",
			gb10b_gr_intr_inject_sm_ecc_error,
			pri_gpc0_tpc0_sm_l1_tag_ecc_control_r,
			pri_gpc0_tpc0_sm_l1_tag_ecc_control_inject_uncorrected_err_f),
	NVGPU_ECC_ERR("cbu_ecc_uncorrected",
			gb10b_gr_intr_inject_sm_ecc_error,
			pri_gpc0_tpc0_sm_cbu_ecc_control_r,
			pri_gpc0_tpc0_sm_cbu_ecc_control_inject_uncorrected_err_f),
	NVGPU_ECC_ERR("lrf_ecc_uncorrected",
			gb10b_gr_intr_inject_sm_ecc_error,
			pri_gpc0_tpc0_sm_lrf_ecc_control_r,
			pri_gpc0_tpc0_sm_lrf_ecc_control_inject_uncorrected_err_f),
	NVGPU_ECC_ERR("l1_data_ecc_uncorrected",
			gb10b_gr_intr_inject_sm_ecc_error,
			pri_gpc0_tpc0_sm_l1_data_ecc_control_r,
			pri_gpc0_tpc0_sm_l1_data_ecc_control_inject_uncorrected_err_f),
	NVGPU_ECC_ERR("icache_l0_data_ecc_uncorrected",
			gb10b_gr_intr_inject_sm_ecc_error,
			pri_gpc0_tpc0_sm_icache_ecc_control_r,
			pri_gpc0_tpc0_sm_icache_ecc_control_inject_uncorrected_err_f),
};

static struct nvgpu_hw_err_inject_info_desc sm_err_desc;

struct nvgpu_hw_err_inject_info_desc *
gb10b_gr_intr_get_sm_err_desc(struct gk20a *g)
{
	(void)g;
	sm_err_desc.info_ptr = sm_ecc_err_desc;
	sm_err_desc.info_size = nvgpu_safe_cast_u64_to_u32(
			sizeof(sm_ecc_err_desc) /
			sizeof(struct nvgpu_hw_err_inject_info));

	return &sm_err_desc;
}
#endif /* CONFIG_NVGPU_INJECT_HWERR */
