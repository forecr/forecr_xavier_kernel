// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/bug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gin.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/nvgpu_err.h>

#include "xtl_ep_gb10b.h"

#include <nvgpu/hw/gb10b/hw_xtl_ep_gb10b.h>

static void gb10b_xtl_ep_ue_handler(struct gk20a *g, u64 cookie)
{
	u32 xtl_ep_intr_status = 0U;

	(void)cookie;
	xtl_ep_intr_status = nvgpu_readl(g, xtl_ep_xtl_base_v() + xtl_ep_pri_intr_status_r(NVGPU_XTL_UE_TREE_OFFSET));

	if (xtl_ep_intr_status == 0U)
		return;

	/*
	 * Added debug interrupts to UE ISR to avoid creating
	 * a separate vector and intr tree for debug intrs.
	 * We can move debug intrs to a new ce_vector when
	 * HW bug 4254096 is fixed and we need to route
	 * XTL_EP_EC_CORRECTED_ERR to a different vector.
	 */
	#ifdef CONFIG_NVGPU_NON_FUSA
	/* Check, log and clear debug interrupts, if any. */
	if ((xtl_ep_intr_status &
			(xtl_ep_pri_intr_status_vf_bme_gated_m() |
			xtl_ep_pri_intr_status_nv_vdm_type_1_dropped_m() |
			xtl_ep_pri_intr_status_debug_intr_m())) != 0U) {
		nvgpu_log(g, gpu_dbg_intr,
			"XTL err, xtl_ep_intr_status: 0x%08x", xtl_ep_intr_status);

		nvgpu_writel(g,
			xtl_ep_xtl_base_v() + xtl_ep_pri_intr_status_r(NVGPU_XTL_UE_TREE_OFFSET),
			(xtl_ep_pri_intr_status_vf_bme_gated_m() |
			xtl_ep_pri_intr_status_nv_vdm_type_1_dropped_m() |
			xtl_ep_pri_intr_status_debug_intr_m()));
	}
	#endif
	if ((xtl_ep_intr_status &
			(xtl_ep_pri_intr_status_ep_ec_uncorr_err_m() |
			xtl_ep_pri_intr_status_ep_ec_corr_err_m())) != 0U) {

		if ((xtl_ep_intr_status & xtl_ep_pri_intr_status_ep_ec_uncorr_err_m()) != 0U) {
			nvgpu_err(g, "XTL ECC uncorrected errors detected");
		}
		if ((xtl_ep_intr_status & xtl_ep_pri_intr_status_ep_ec_corr_err_m()) != 0U) {
			nvgpu_err(g, "XTL ECC corrected errors detected");
			nvgpu_err(g, "XTL ECC corrected errors are treated as "
				"uncorrected error due to PCIE HW bug");
		}
		/*
		* Corrected errors are also reported as uncorrected
		* errors due to .
		*/
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_XTL,
			GPU_XTL_EP_EC_UNCORRECTED_ERR);

		/* Dump missionerr_status register values to aid debug */
		nvgpu_err(g, "xtl_ep_ec_errslice0_missionerr_status: 0x%08x",
					nvgpu_readl(g, xtl_ep_ec_errslice0_missionerr_status_r()));
		nvgpu_err(g, "xtl_ep_ec_errslice1_missionerr_status: 0x%08x",
					nvgpu_readl(g, xtl_ep_ec_errslice1_missionerr_status_r()));
		nvgpu_err(g, "xtl_ep_ec_errslice2_missionerr_status: 0x%08x",
					nvgpu_readl(g, xtl_ep_ec_errslice2_missionerr_status_r()));

		/* Trigger GPU quiesce for fatal errors */
		nvgpu_sw_quiesce(g);
	}
}

int gb10b_xtl_ep_init_hw(struct gk20a *g)
{
	u32 ue_vector = 0;
	u32 intr_ctrl_msg_ue = 0;
	u32 intr_en_mask = 0U;
	u32 intr_mask_clr = 0U;

	/* Due to HW bug 4254096, the corrected errors need to be routed to
	 * uncorrected line.
	 */
	intr_mask_clr =
	#ifdef CONFIG_NVGPU_NON_FUSA
		xtl_ep_pri_intr_mask_clr_vf_bme_gated_m() |
		xtl_ep_pri_intr_mask_clr_nv_vdm_type_1_dropped_m() |
		xtl_ep_pri_intr_mask_clr_debug_intr_m() |
	#endif
		xtl_ep_pri_intr_mask_clr_ep_ec_uncorr_err_m() |
		xtl_ep_pri_intr_mask_clr_ep_ec_corr_err_m();

	/* Program INTR_MASK(SET/CLR) register to allow INTR_STATUS reg to be set */
	nvgpu_writel(g, xtl_ep_xtl_base_v() + xtl_ep_pri_intr_mask_clr_r(NVGPU_XTL_UE_TREE_OFFSET),
			intr_mask_clr);

	intr_en_mask =
	#ifdef CONFIG_NVGPU_NON_FUSA
		xtl_ep_pri_intr_enable_vf_bme_gated_m() |
		xtl_ep_pri_intr_enable_nv_vdm_type_1_dropped_m() |
		xtl_ep_pri_intr_enable_debug_intr_m() |
	#endif
		xtl_ep_pri_intr_enable_ep_ec_uncorr_err_m() |
		xtl_ep_pri_intr_enable_ep_ec_corr_err_m();

	/* Program INTR_EN register to allow generation of INTR MSG to GIN */
	nvgpu_writel(g, xtl_ep_xtl_base_v() + xtl_ep_pri_intr_enable_r(NVGPU_XTL_UE_TREE_OFFSET),
			intr_en_mask);

	ue_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_XTL_EP,
				NVGPU_XTL_UE_VECTOR_OFFSET);
	intr_ctrl_msg_ue = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	nvgpu_writel(g, xtl_ep_xtl_base_v() + xtl_ep_pri_intr_ctrl_r(NVGPU_XTL_UE_TREE_OFFSET), intr_ctrl_msg_ue);
	nvgpu_gin_set_stall_handler(g, ue_vector, &gb10b_xtl_ep_ue_handler, 0);
	nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_XTL_EP,
			NVGPU_XTL_UE_VECTOR_OFFSET,
			NVGPU_GIN_INTR_ENABLE);

	/* Disable AER unsupported Request error as per PCIE Safety IAS */

	nvgpu_writel(g, xtl_ep_ec_errslice0_missionerr_enable_r(),
			nvgpu_readl(g, xtl_ep_ec_errslice0_missionerr_enable_r()) &
			~xtl_ep_ec_errslice0_missionerr_enable_err7_m());

	return 0;
}
