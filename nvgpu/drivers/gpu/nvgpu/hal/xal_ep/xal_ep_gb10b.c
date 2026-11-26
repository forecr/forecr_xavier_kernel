// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.


#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/bug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gin.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/nvgpu_init.h>

#include "xal_ep_gb10b.h"

#include <nvgpu/hw/gb10b/hw_func_gb10b.h>
#include <nvgpu/hw/gb10b/hw_xal_ep_gb10b.h>

static void gb10b_xal_ep_ue_handler(struct gk20a *g, u64 cookie)
{
	(void)cookie;
	g->ops.bus.isr(g);
}

static void xal_ep_handle_ecc_corrected_errors(struct gk20a *g)
{
	u32 ecc_correctable_status = nvgpu_readl(g, xal_ep_ecc_correctable_status_r());
	u32 clear_intr = 0U;

	if ((ecc_correctable_status &
			xal_ep_ecc_correctable_status_dpfifo_m()) != 0U) {
		nvgpu_err(g, "dpfifo ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_dpfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_dpfifo_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_dpfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_dpfifo_m();
	}
	if ((ecc_correctable_status &
		xal_ep_ecc_correctable_status_dnpfifo_m()) != 0U) {
		nvgpu_err(g, "dnpfifo ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_dnpfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_dnpfifo_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_dnpfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_dnpfifo_m();
	}
	if ((ecc_correctable_status &
		xal_ep_ecc_correctable_status_upcplfifo_m()) != 0U) {
		nvgpu_err(g, "upcplfifo ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_upcplfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_upcplfifo_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_upcplfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_upcplfifo_m();
	}
	if ((ecc_correctable_status &
		xal_ep_ecc_correctable_status_prireqpfifo_m()) != 0U) {
		nvgpu_err(g, "prireqpfifo ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_prireqpfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_prireqpfifo_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_prireqpfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_prireqpfifo_m();
	}
	if ((ecc_correctable_status &
		xal_ep_ecc_correctable_status_prireqnpfifo_m()) != 0U) {
		nvgpu_err(g, "prireqnpfifo ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_prireqnpfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_prireqnpfifo_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_prireqnpfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_prireqnpfifo_m();
	}
	if ((ecc_correctable_status &
		xal_ep_ecc_correctable_status_pritrackerfifo_m()) != 0U) {
		nvgpu_err(g, "pritrackerfifo ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_pritrackerfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_pritrackerfifo_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_pritrackerfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_pritrackerfifo_m();
	}
	if ((ecc_correctable_status &
		xal_ep_ecc_correctable_status_prireorder_m()) != 0U) {
		nvgpu_err(g, "prireorder ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_prireorder_ecc_status_r()),
			nvgpu_readl(g, xal_ep_prireorder_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_prireorder_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_prireorder_m();
	}
	if ((ecc_correctable_status &
		xal_ep_ecc_correctable_status_prirspcplfifo_m()) != 0U) {
		nvgpu_err(g, "prirspcplfifo ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_prirspcplfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_prirspcplfifo_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_prirspcplfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_prirspcplfifo_m();
	}
	if ((ecc_correctable_status &
		xal_ep_ecc_correctable_status_xdcegressfifo_m()) != 0U) {
		nvgpu_err(g, "xdcegressfifo ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_xdcegressfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_xdcegressfifo_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_xdcegressfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_xdcegressfifo_m();
	}
	if ((ecc_correctable_status &
		xal_ep_ecc_correctable_status_xdcingressfifo_m()) != 0U) {
		nvgpu_err(g, "xdcingressfifo ecc corrected error! "
			"ecc_status: 0x%08x, corrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_xdcingressfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_xdcingressfifo_ecc_corrected_err_count_r()),
			nvgpu_readl(g, xal_ep_xdcingressfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_correctable_status_xdcingressfifo_m();
	}
	/* Clear interrupts */
	nvgpu_writel(g, xal_ep_ecc_correctable_status_r(), clear_intr);
}

static void gb10b_xal_ep_ce_handler(struct gk20a *g, u64 cookie)
{
	u32 xal_ep_intr_0 = 0U;

	(void)cookie;
	xal_ep_intr_0 = nvgpu_readl(g, xal_ep_intr_0_r());

	nvgpu_log(g, gpu_dbg_intr, "xal corrected err, intr0: 0x%08x",
		xal_ep_intr_0);

	/*
	 * Don't check for unused, disabled timeout_notification
	 * type interrupts. They are kept disabled since
	 * they are benign and create noise.
	 */
	if ((xal_ep_intr_0 & xal_ep_intr_0_ecc_correctable_m()) != 0U) {
		nvgpu_err(g, "XAL ECC corrected errors detected");
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_XAL,
				GPU_XAL_EP_ECC_CORRECTED);
		xal_ep_handle_ecc_corrected_errors(g);
		nvgpu_writel(g, xal_ep_intr_0_r(), xal_ep_intr_0_ecc_correctable_m());
	}
	nvgpu_writel(g, xal_ep_intr_retrigger_r(),
			xal_ep_intr_retrigger_trigger_true_f());
}

static u32 xal_ep_intr_0_pri_mask(void)
{
	u32 mask = xal_ep_intr_0_pri_fecserr_m() |
		xal_ep_intr_0_pri_req_timeout_m() |
		xal_ep_intr_0_pri_rsp_timeout_m();

	return mask;
}

int gb10b_xal_ep_init_hw(struct gk20a *g)
{
	u32 ue_vector = 0;
	u32 ce_vector = 0;
	u32 intr_0_en_mask = 0;
	u32 intr_1_en_mask = 0;
	u32 intr_ctrl_msg = 0;
	u32 intr_ctrl_1_msg = 0;
	u32 old_vector = 0U;
	u32 old_intr_ctrl_msg = 0U;

	/* xal_ep_intr_en_0_r and xal_ep_intr_ctrl_r control the uncorrected
	 * error tree.
	 */
	intr_0_en_mask =
			xal_ep_intr_en_0_jtag_timeout_m() |
			xal_ep_intr_en_0_pri_fecserr_m() |
			xal_ep_intr_en_0_pri_req_timeout_m() |
			xal_ep_intr_en_0_pri_rsp_timeout_m() |
			xal_ep_intr_en_0_ecc_uncorrectable_m() |
			xal_ep_intr_en_0_parity_error_m() |
			xal_ep_intr_en_0_ingress_poison_m() |
			xal_ep_intr_en_0_egress_poison_m() |
			xal_ep_intr_en_0_decoupler_error_m();

	/* xal_ep_intr_en_1_r and xal_ep_intr_ctrl_1_r control the corrected
	 * error tree.
	 */

	/*
	 * XAL timeout_notification type interrupts
	 * represent shorter timeouts that indicate that things
	 * are taking longer but may still complete okay.
	 * Keeping them disabled since they are benign notifications
	 * that create noise.
	 */
	intr_1_en_mask =
			xal_ep_intr_en_1_ecc_correctable_m();

	/* JTAG timeout, RSP timeout, Ingress/ Egress poison report
	 * functionality is enabled by default in HW.
	 */

	nvgpu_log_info(g, "xal_ep pcie_rsp_fb_timeout=0x%x",
		xal_ep_pcie_rsp_fb_timeout_period_v(nvgpu_readl(g,
				xal_ep_pcie_rsp_fb_timeout_r())));

	ue_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_XAL_EP, NVGPU_XAL_UE_VECTOR_OFFSET);
	ce_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_XAL_EP, NVGPU_XAL_CE_VECTOR_OFFSET);

	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	intr_ctrl_1_msg = nvgpu_gin_get_intr_ctrl_msg(g, ce_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);

	old_intr_ctrl_msg = nvgpu_readl(g, xal_ep_intr_ctrl_r());
	old_vector = nvgpu_gin_get_vector_from_intr_ctrl_msg(g, old_intr_ctrl_msg);
	nvgpu_writel(g, xal_ep_intr_ctrl_r(), intr_ctrl_msg);
	nvgpu_writel(g, xal_ep_intr_retrigger_r(),
			xal_ep_intr_retrigger_trigger_true_f());
	if (old_vector != ue_vector) {
		nvgpu_gin_clear_pending_intr(g, old_vector);
	}

	old_intr_ctrl_msg = nvgpu_readl(g, xal_ep_intr_ctrl_1_r());
	old_vector = nvgpu_gin_get_vector_from_intr_ctrl_msg(g, old_intr_ctrl_msg);
	nvgpu_writel(g, xal_ep_intr_ctrl_1_r(), intr_ctrl_1_msg);
	nvgpu_writel(g, xal_ep_intr_retrigger_1_r(),
			xal_ep_intr_retrigger_1_trigger_true_f());
	if (old_vector != ce_vector) {
		nvgpu_gin_clear_pending_intr(g, old_vector);
	}

	nvgpu_gin_set_stall_handler(g, ue_vector, &gb10b_xal_ep_ue_handler, 0);
	nvgpu_gin_set_stall_handler(g, ce_vector, &gb10b_xal_ep_ce_handler, 0);
	nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_XAL_EP,
			NVGPU_XAL_UE_VECTOR_OFFSET,
			NVGPU_GIN_INTR_ENABLE);
	nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_XAL_EP,
			NVGPU_XAL_CE_VECTOR_OFFSET,
			NVGPU_GIN_INTR_ENABLE);

	nvgpu_writel(g, xal_ep_intr_en_0_r(), intr_0_en_mask);
	nvgpu_writel(g, xal_ep_intr_en_1_r(), intr_1_en_mask);

	return 0;
}

static void xal_ep_handle_pri_errors(struct gk20a *g, u32 intr_0)
{
	u32 adr, write;
	u32 err;

	if ((intr_0 & xal_ep_intr_0_pri_fecserr_m()) != 0U) {
		u32 fecs_err_code;

		fecs_err_code = nvgpu_readl(g, xal_ep_intr_triggered_pri_error_data_r());

		/* Skip error report call since it is covered in below HAL. */
		if (g->ops.priv_ring.decode_error_code)
			g->ops.priv_ring.decode_error_code(g, fecs_err_code);

		err = nvgpu_readl(g, xal_ep_intr_triggered_pri_error_r());
		adr = xal_ep_intr_triggered_pri_error_adr_v(err);
		write = xal_ep_intr_triggered_pri_error_write_v(err);

		nvgpu_err(g, "PRI FECS ERROR: ADR 0x%08x %s  ERROR CODE 0x%08x",
			  adr, write ? "WRITE" : "READ", fecs_err_code);
	}

	if ((intr_0 & xal_ep_intr_0_pri_req_timeout_m()) != 0U) {
		err = nvgpu_readl(g, xal_ep_intr_triggered_pri_req_timeout_r());
		adr = xal_ep_intr_triggered_pri_req_timeout_adr_v(err);
		write = xal_ep_intr_triggered_pri_req_timeout_write_v(err);

		nvgpu_err(g, "PRI Request timeout ERROR: ADR 0x%08x %s",
			  adr, write ? "WRITE" : "READ");

		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_PRI, GPU_PRI_TIMEOUT_ERROR);
	}

	if ((intr_0 & xal_ep_intr_0_pri_rsp_timeout_m()) != 0U) {
		err = nvgpu_readl(g, xal_ep_intr_triggered_pri_rsp_timeout_r());
		adr = xal_ep_intr_triggered_pri_rsp_timeout_adr_v(err);
		write = xal_ep_intr_triggered_pri_rsp_timeout_write_v(err);

		nvgpu_err(g, "PRI Response timeout ERROR: ADR 0x%08x %s",
			  adr, write ? "WRITE" : "READ");

		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_PRI, GPU_PRI_TIMEOUT_ERROR);
	}
}

static void xal_ep_handle_poison_errors(struct gk20a *g, u32 intr_0)
{
	if ((intr_0 & xal_ep_intr_0_ingress_poison_m()) != 0U) {
		u32 log0, log1, log2 = 0U;

		log0 = nvgpu_readl(g, xal_ep_poison_ingress_log0_r());
		log1 = nvgpu_readl(g, xal_ep_poison_ingress_log1_r());
		log2 = nvgpu_readl(g, xal_ep_poison_ingress_log2_r());

		nvgpu_err(g, "XAL_EP detected an ingress packet with poison bit set! "
			"xal_ep_poison_ingress_log0: 0x%08x "
			"xal_ep_poison_ingress_log1: 0x%08x "
			"xal_ep_poison_ingress_log2: 0x%08x ",
			log0, log1, log2);

		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_XAL,
			GPU_XAL_EP_INGRESS_POISON_ERROR);
	}
	if ((intr_0 & xal_ep_intr_0_egress_poison_m()) != 0U) {
		u32 log0, log1, log2 = 0U;

		log0 = nvgpu_readl(g, xal_ep_poison_egress_log0_r());
		log1 = nvgpu_readl(g, xal_ep_poison_egress_log1_r());
		log2 = nvgpu_readl(g, xal_ep_poison_egress_log2_r());

		nvgpu_err(g, "XAL_EP detected an egress write request or "
			"a completion packet with poison bit set! "
			"xal_ep_poison_egress_log0: 0x%08x "
			"xal_ep_poison_egress_log1: 0x%08x "
			"xal_ep_poison_egress_log2: 0x%08x ",
			log0, log1, log2);

		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_XAL,
			GPU_XAL_EP_EGRESS_POISON_ERROR);
	}
}

static void xal_handle_decoupler_error(struct gk20a *g)
{
	u32 decoupler_err_status, log1, log2 = 0U;

	decoupler_err_status = nvgpu_readl(g, xal_ep_decoupler_err_status_r());
	log1 = nvgpu_readl(g, xal_ep_decoupler_err_log1_r());
	log2 = nvgpu_readl(g, xal_ep_decoupler_err_log2_r());

	nvgpu_err(g, "XAL_EP Decoupler Programming Error detected! "
		"decoupler_err_status: 0x%08x, log1: 0x%08x, "
		"log2: 0x%08x", decoupler_err_status, log1, log2);

	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_XAL,
			GPU_XAL_EP_DECOUPLER_ERROR);
}

static void xal_ep_handle_ecc_uncorrected_errors(struct gk20a *g)
{
	u32 ecc_uncorrectable_status = nvgpu_readl(g, xal_ep_ecc_uncorrectable_status_r());
	u32 clear_intr = 0U;

	if ((ecc_uncorrectable_status &
			xal_ep_ecc_uncorrectable_status_dpfifo_m()) != 0U) {
		nvgpu_err(g, "dpfifo ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_dpfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_dpfifo_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_dpfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_dpfifo_m();
	}
	if ((ecc_uncorrectable_status &
		xal_ep_ecc_uncorrectable_status_dnpfifo_m()) != 0U) {
		nvgpu_err(g, "dnpfifo ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_dnpfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_dnpfifo_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_dnpfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_dnpfifo_m();
	}
	if ((ecc_uncorrectable_status &
		xal_ep_ecc_uncorrectable_status_upcplfifo_m()) != 0U) {
		nvgpu_err(g, "upcplfifo ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_upcplfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_upcplfifo_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_upcplfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_upcplfifo_m();
	}
	if ((ecc_uncorrectable_status &
		xal_ep_ecc_uncorrectable_status_prireqpfifo_m()) != 0U) {
		nvgpu_err(g, "prireqpfifo ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_prireqpfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_prireqpfifo_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_prireqpfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_prireqpfifo_m();
	}
	if ((ecc_uncorrectable_status &
		xal_ep_ecc_uncorrectable_status_prireqnpfifo_m()) != 0U) {
		nvgpu_err(g, "prireqnpfifo ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_prireqnpfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_prireqnpfifo_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_prireqnpfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_prireqnpfifo_m();
	}
	if ((ecc_uncorrectable_status &
		xal_ep_ecc_uncorrectable_status_pritrackerfifo_m()) != 0U) {
		nvgpu_err(g, "pritrackerfifo ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_pritrackerfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_pritrackerfifo_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_pritrackerfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_pritrackerfifo_m();
	}
	if ((ecc_uncorrectable_status &
		xal_ep_ecc_uncorrectable_status_prireorder_m()) != 0U) {
		nvgpu_err(g, "prireorder ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_prireorder_ecc_status_r()),
			nvgpu_readl(g, xal_ep_prireorder_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_prireorder_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_prireorder_m();
	}
	if ((ecc_uncorrectable_status &
		xal_ep_ecc_uncorrectable_status_prirspcplfifo_m()) != 0U) {
		nvgpu_err(g, "prirspcplfifo ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_prirspcplfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_prirspcplfifo_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_prirspcplfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_prirspcplfifo_m();
	}
	if ((ecc_uncorrectable_status &
		xal_ep_ecc_uncorrectable_status_xdcegressfifo_m()) != 0U) {
		nvgpu_err(g, "xdcegressfifo ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_xdcegressfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_xdcegressfifo_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_xdcegressfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_xdcegressfifo_m();
	}
	if ((ecc_uncorrectable_status &
		xal_ep_ecc_uncorrectable_status_xdcingressfifo_m()) != 0U) {
		nvgpu_err(g, "xdcingressfifo ecc uncorrected error! "
			"ecc_status: 0x%08x, uncorrected_count: 0x%08x, "
			"ecc_address: 0x%08x",
			nvgpu_readl(g, xal_ep_xdcingressfifo_ecc_status_r()),
			nvgpu_readl(g, xal_ep_xdcingressfifo_ecc_uncorrected_err_count_r()),
			nvgpu_readl(g, xal_ep_xdcingressfifo_ecc_address_r()));
		clear_intr |= xal_ep_ecc_uncorrectable_status_xdcingressfifo_m();
	}
	/* Clear interrupts */
	nvgpu_writel(g, xal_ep_ecc_uncorrectable_status_r(), clear_intr);
}

void gb10b_xal_ep_isr(struct gk20a *g)
{
	u32 xal_ep_intr_0 = 0U;
	u32 xal_ep_intr_0_handled = 0U;
	bool needs_quiesce = false;

	xal_ep_intr_0 = nvgpu_readl(g, xal_ep_intr_0_r());

	if (xal_ep_intr_0 == 0U)
		return;

	/* Handle ECC uncorrected errors. */
	if ((xal_ep_intr_0 & xal_ep_intr_0_ecc_uncorrectable_m()) != 0U) {
		nvgpu_err(g, "XAL ECC uncorrected errors detected");
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_XAL,
			GPU_XAL_EP_ECC_UNCORRECTED);
		xal_ep_handle_ecc_uncorrected_errors(g);
		xal_ep_intr_0_handled |= (xal_ep_intr_0 & xal_ep_intr_0_ecc_uncorrectable_m());
		needs_quiesce |= true;
	}
	/*
	 * Handle JTAG timeout error: This error represents that
	 * an outstanding JTAG request did not get a response by the limit
	 * set in NV_XAL_EP_JTAG_TIMEOUT.
	 */
	if ((xal_ep_intr_0 & xal_ep_intr_0_jtag_timeout_m()) != 0U) {
		u32 jtag_timeout;
		u32 jtag_timeout_upper, jtag_timeout_lower;

		jtag_timeout = nvgpu_readl(g, xal_ep_jtag_timeout_r());
		jtag_timeout_upper = nvgpu_readl(g, xal_ep_jtag_timeout_upper_r());
		jtag_timeout_lower = nvgpu_readl(g, xal_ep_jtag_timeout_lower_r());

		nvgpu_err(g, "JTAG timeout error in XAL_EP! jtag_timeout: 0x%08x"
			"jtag_timeout_upper: 0x%08x, jtag_timeout_lower: 0x%08x",
			jtag_timeout, jtag_timeout_upper, jtag_timeout_lower);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_XAL,
			GPU_XAL_EP_UNCORRECTED_ERROR);
		xal_ep_intr_0_handled |= (xal_ep_intr_0 & xal_ep_intr_0_jtag_timeout_m());
		needs_quiesce |= true;
	}
	/*
	 * These bits signal that a PRI transaction has failed or timed out
	 * pri_fecserr_m(): fecs initiated PRI transaction failed.
	 * pri_req_timeout_m(): PRI transaction timed out.
	 * pri_rsp_timeout_m(): PRI transaction timed out.
	 */
	if ((xal_ep_intr_0 & xal_ep_intr_0_pri_mask()) != 0U) {
		xal_ep_handle_pri_errors(g, xal_ep_intr_0);

		xal_ep_intr_0_handled |= (xal_ep_intr_0 & xal_ep_intr_0_pri_mask());
		/* TODO: Add quiesce calls for XAL PRI errors: JIRA NVGPU-12353	*/
	}
	/* Handle parity error. */
	if ((xal_ep_intr_0 & xal_ep_intr_0_parity_error_m()) != 0U) {
		nvgpu_err(g, "XAL Parity error detected");
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_XAL,
				GPU_XAL_EP_PARITY_ERROR);
		xal_ep_intr_0_handled |= (xal_ep_intr_0 & xal_ep_intr_0_parity_error_m());
		needs_quiesce |= true;
	}
	/*
	 * Handle ingress and egress poison errors.
	 */
	if ((xal_ep_intr_0 & (xal_ep_intr_0_ingress_poison_m() |
		xal_ep_intr_0_egress_poison_m())) != 0U) {
		xal_ep_handle_poison_errors(g, xal_ep_intr_0);

		xal_ep_intr_0_handled |= (xal_ep_intr_0 & (xal_ep_intr_0_egress_poison_m() |
			xal_ep_intr_0_ingress_poison_m()));
		needs_quiesce |= true;
	}
	/*
	 * Handle Decoupler Programming Error.
	 * This error fires due to a multi-match in the decoupler.
	 */
	if ((xal_ep_intr_0 & xal_ep_intr_0_decoupler_error_m()) != 0U) {
		xal_handle_decoupler_error(g);

		xal_ep_intr_0_handled |= (xal_ep_intr_0 & xal_ep_intr_0_decoupler_error_m());
		needs_quiesce |= true;
	}
	/* Check for unhandled interrupts */
	if (xal_ep_intr_0 != xal_ep_intr_0_handled) {
		nvgpu_err(g, "unhandled interrupts, status: 0x%x",
				xal_ep_intr_0 & ~xal_ep_intr_0_handled);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_XAL,
			GPU_XAL_EP_UNCORRECTED_ERROR);
		needs_quiesce |= true;
	}

	/* Clear interrupts */
	nvgpu_writel(g, xal_ep_intr_0_r(),
				xal_ep_intr_0);

	/* Trigger GPU quiesce for fatal errors */
	if (needs_quiesce)
		nvgpu_sw_quiesce(g);

	nvgpu_writel(g, xal_ep_intr_retrigger_r(),
			xal_ep_intr_retrigger_trigger_true_f());
}
