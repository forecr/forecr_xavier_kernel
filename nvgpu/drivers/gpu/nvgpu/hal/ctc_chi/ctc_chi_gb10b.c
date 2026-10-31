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

#include "ctc_chi_gb10b.h"

#include <nvgpu/hw/gb10b/hw_ctc_chi_bridge_gb10b.h>

static void gb10b_ctc_chi_ue_handler(struct gk20a *g, u64 cookie)
{
	u32 ctc_chi_bridge = 0;
	u32 ctc_chi_bridge_count = 0;
	u32 ctc_chi_bridge_stride = 0;

	(void)cookie;

	ctc_chi_bridge_count = nvgpu_get_litter_value(g,
				GPU_LIT_NUM_CTC_CHI_BRIDGE_NUM_LINKS);
	ctc_chi_bridge_stride = nvgpu_get_litter_value(g,
				GPU_LIT_CTC_CHI_BRIDGE_PRIV_STRIDE);

	/* Go through all CTC_CHI_BRIDGES explicitly. */
	for (ctc_chi_bridge = 0; ctc_chi_bridge < ctc_chi_bridge_count;
			ctc_chi_bridge++) {
		u32 offset;
		u32 ctc_chi_intr_status = 0U;
		u32 handled_intr = 0U;
		u32 ctrl_channel_err_mask = 0U;
		u32 ctc_chi_bridge_parity_status = 0U;

		offset = nvgpu_safe_mult_u32(ctc_chi_bridge, ctc_chi_bridge_stride);
		ctc_chi_intr_status = nvgpu_readl(g,
				nvgpu_safe_add_u32(ctc_chi_bridge_bridge0_intr0_r(), offset));

		if (ctc_chi_intr_status == 0U)
			continue;

		ctc_chi_bridge_parity_status = nvgpu_readl(g,
			nvgpu_safe_add_u32(ctc_chi_bridge_bridge0_parity_status_r(), offset));
		nvgpu_err(g, "CTC_CHI%d uncorrected err: ctc_chi_intr_status 0x%08x, "
			"ctc_chi_bridge_parity_status 0x%08x", ctc_chi_bridge,
			ctc_chi_intr_status, ctc_chi_bridge_parity_status);

		/*
		 * Register parity for CTC_CHI includes parity for control channels data
		 * channels. Control channel  errors in CTC_CHI are represented
		 * by the following bits.
		 */
		ctrl_channel_err_mask = ctc_chi_bridge_bridge0_intr0_req_parity_err_m() |
								ctc_chi_bridge_bridge0_intr0_rsp_parity_err_m();

		if ((ctc_chi_intr_status & (ctrl_channel_err_mask)) != 0U) {
			nvgpu_err(g, "ctc_chi bridge parity uncorr err on control channel ");
			handled_intr |= ctrl_channel_err_mask;
		}
		if ((ctc_chi_intr_status & ctc_chi_bridge_bridge0_intr0_dat_parity_err_m()) != 0U) {
			nvgpu_err(g, "ctc_chi bridge parity uncorr err on data channel");
			handled_intr |= ctc_chi_bridge_bridge0_intr0_dat_parity_err_m();
		}

		/* Check for unsupported interrupts */
		if ((ctc_chi_intr_status & ~handled_intr) != 0U) {
			nvgpu_err(g, "Unsupported CTC_CHI error detected. "
					"Treating this as fatal error.");
		}

		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_CTCCHI,
				GPU_CTC_CHI_UNCORRECTED_ERR);

		/* Trigger GPU quiesce for fatal errors */
		nvgpu_sw_quiesce(g);
	}
}

int gb10b_ctc_chi_init_hw(struct gk20a *g)
{
	u32 ue_vector = 0;
	u32 intr_ctrl_msg_ue = 0;
	u32 ctc_chi_bridge = 0;
	u32 ctc_chi_bridge_count = 0;
	u32 ctc_chi_bridge_stride = 0;

	ue_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_CTC_CHI,
				NVGPU_CTC_CHI_UE_VECTOR_OFFSET);
	intr_ctrl_msg_ue = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);

	ctc_chi_bridge_count = nvgpu_get_litter_value(g,
				GPU_LIT_NUM_CTC_CHI_BRIDGE_NUM_LINKS);
	ctc_chi_bridge_stride = nvgpu_get_litter_value(g,
				GPU_LIT_CTC_CHI_BRIDGE_PRIV_STRIDE);

	/* Loop over all instances of CTC_CHI_BRIDGE links to initialise them */
	for (ctc_chi_bridge = 0; ctc_chi_bridge < ctc_chi_bridge_count;
			ctc_chi_bridge++) {
		u32 offset;

		offset = nvgpu_safe_mult_u32(ctc_chi_bridge, ctc_chi_bridge_stride);
		/* Program INTR_MASK register to allow INTR_STATUS reg to be set */

		nvgpu_writel(g, nvgpu_safe_add_u32(ctc_chi_bridge_bridge0_intr2_r(),
					offset), ctc_chi_bridge_bridge0_intr2_mask_req_parity_err_m()|
					ctc_chi_bridge_bridge0_intr2_mask_rsp_parity_err_m()|
					ctc_chi_bridge_bridge0_intr2_mask_dat_parity_err_m());

		/* Program INTR_EN register to allow generation of INTR MSG to GIN */

		nvgpu_writel(g, nvgpu_safe_add_u32(ctc_chi_bridge_bridge0_intr1_r(),
					offset), ctc_chi_bridge_bridge0_intr1_enable_req_parity_err_m()|
					ctc_chi_bridge_bridge0_intr1_enable_rsp_parity_err_m()|
					ctc_chi_bridge_bridge0_intr1_enable_dat_parity_err_m());

		/* Program INTR_CTRL registers with the same defined vector */
		nvgpu_writel(g, nvgpu_safe_add_u32(ctc_chi_bridge_bridge0_intr_ctrl_r(),
					offset), intr_ctrl_msg_ue);
	}

	nvgpu_gin_set_stall_handler(g, ue_vector, &gb10b_ctc_chi_ue_handler, 0);
	nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_CTC_CHI,
			NVGPU_CTC_CHI_UE_VECTOR_OFFSET,
			NVGPU_GIN_INTR_ENABLE);

	return 0;
}
