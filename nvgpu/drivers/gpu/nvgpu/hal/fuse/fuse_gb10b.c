// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/fuse.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/gin.h>
#include <nvgpu/nvgpu_init.h>
#include "fuse_gb10b.h"
#include <nvgpu/hw/gb10b/hw_fuse_gb10b.h>

#define GB10B_FUSE_READ_DEVICE_IDENTIFIER_RETRIES 100000U

int gb10b_fuse_read_gcplex_config_fuse(struct gk20a *g, u32 *val)
{
    u32 reg_val = 0U;
    u32 fuse_val = 0U;

    /*
        * SOC FUSE_GCPLEX_CONFIG_FUSE_0 bit(2) mapped to
        * fuse_opt_wpr_enabled igpu fuse register
        */
    reg_val = nvgpu_readl(g, fuse_opt_wpr_enabled_r());
    fuse_val |= (fuse_opt_wpr_enabled_data_v(reg_val) << 2U);

    /*
        * SOC FUSE_GCPLEX_CONFIG_FUSE_0 bit(1) mapped to
        * fuse_opt_vpr_enabled igpu fuse register
        */
    reg_val = nvgpu_readl(g, fuse_opt_vpr_enabled_r());
    fuse_val |= (fuse_opt_vpr_enabled_data_v(reg_val) << 1U);

    /*
        * SOC FUSE_GCPLEX_CONFIG_FUSE_0 bit(0) mapped to
        * fuse_opt_vpr_auto_fetch_disable
        */
    reg_val = nvgpu_readl(g, fuse_opt_vpr_auto_fetch_disable_r());
    fuse_val |= fuse_opt_vpr_auto_fetch_disable_data_v(reg_val);

    *val = fuse_val;

    return 0;
}

int gb10b_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi)
{
	u32 lo = 0U;
	u32 hi = 0U;
	u32 pdi_loaded = 0U;
	u32 retries = GB10B_FUSE_READ_DEVICE_IDENTIFIER_RETRIES;

	if (nvgpu_platform_is_silicon(g)) {
		do {
			pdi_loaded = fuse_p2prx_pdi_loaded_v(
				nvgpu_readl(g, fuse_p2prx_pdi_r()));
			retries = nvgpu_safe_sub_u32(retries, 1U);
		} while ((pdi_loaded != fuse_p2prx_pdi_loaded_true_v()) &&
			retries > 0U);

		if (retries == 0U) {
			nvgpu_err(g, "Device identifier load failed");
			return -EAGAIN;
		}
	}

	lo = nvgpu_readl(g, fuse_opt_pdi_0_r());
	hi = nvgpu_readl(g, fuse_opt_pdi_1_r());

	*pdi = ((u64)lo) | (((u64)hi) << 32);

	return 0;
}

u32 gb10b_fuse_status_opt_cpc_gpc(struct gk20a *g, u32 gpc)
{
	return nvgpu_readl(g, fuse_status_opt_cpc_gpc_r(gpc));
}

static void gb10b_fuse_ue_handler(struct gk20a *g, u64 cookie)
{
	(void)cookie;

	/* Report fatal error to safety services. */
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_FUSE,
						GPU_FUSE_REG_PARITY);

	nvgpu_err(g, "Fuse uncorrected error interrupt");
	nvgpu_err(g, "FUSE Register Parity INTR_STATUS: 0x%x",
			 nvgpu_readl(g, fuse_register_parity_status_r()));

	/* Trigger GPU quiesce for fatal errors */
	nvgpu_sw_quiesce(g);

	/* The INTR_STATUS bits for FUSE are sticky and can only be cleared
	 * with GPU reset.
	 */
}
int gb10b_fuse_init_hw(struct gk20a *g)
{
	u32 ue_vector = 0U;
	u32 intr_ctrl = 0U;
	u32 intr_mask = 0U;

	/* Program vector via INTR_CTRL register for FUSE */
	ue_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_FUSE,
						NVGPU_FUSE_UE_VECTOR_OFFSET);
	/* There is a HW bug w/ this register https://nvbugs/4565022. We can
	 * only set an even number of bits to this register otherwise it will
	 * trigger register parity errors.
	 */
	intr_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
				NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	nvgpu_writel(g, fuse_register_parity_intr_ctrl_r(), intr_ctrl);
	nvgpu_gin_set_stall_handler(g, ue_vector, &gb10b_fuse_ue_handler, 0U);

	/* Initialize top intr leaf bit for FUSE */
	nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_FUSE,
				NVGPU_FUSE_UE_VECTOR_OFFSET,
				NVGPU_GIN_INTR_ENABLE);

	/* Enable the interrupt propagation */
	intr_mask = fuse_register_parity_intr_mask_en_m();
	nvgpu_writel(g, fuse_register_parity_intr_mask_r(), intr_mask);

	nvgpu_log(g, gpu_dbg_intr, "fuse intr mask: 0x%x intr_ctrl:0x%x",
				intr_mask, intr_ctrl);
	return 0;
}

void gb10b_feature_override_ofa_err_contain_en(struct gk20a *g)
{
	u32 override_val;

	override_val = (fuse_feature_override_ofa_poison_err_contain_en_data_f(
		fuse_feature_override_ofa_poison_err_contain_en_data_enabled_v())
		| fuse_feature_override_ofa_poison_err_contain_en_override_f(
		fuse_feature_override_ofa_poison_err_contain_en_override_true_v()));

	nvgpu_writel(g, fuse_feature_override_ofa_poison_err_contain_en_r(),
		override_val);
}

void gb10b_feature_override_nvenc_err_contain_en(struct gk20a *g)
{
	u32 override_val;

	override_val = (fuse_feature_override_nvenc_poison_err_contain_en_data_f(
		fuse_feature_override_nvenc_poison_err_contain_en_data_enabled_v())
		| fuse_feature_override_nvenc_poison_err_contain_en_override_f(
		fuse_feature_override_nvenc_poison_err_contain_en_override_true_v()));

	nvgpu_writel(g, fuse_feature_override_nvenc_poison_err_contain_en_r(),
		override_val);
}
