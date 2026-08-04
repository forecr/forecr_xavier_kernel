// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/class.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/string.h>
#include <nvgpu/errata.h>
#include <nvgpu/device.h>
#include <nvgpu/gin.h>
#include <nvgpu/power_features/pg.h>
#include <nvgpu/debug.h>

#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/gr/gr_intr.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/gr/gr_utils.h>

#include "common/gr/gr_priv.h"
#include "common/gr/gr_intr_priv.h"
#include "hal/gr/intr/gr_intr_gv11b.h"
#include "hal/gr/intr/gr_intr_ga10b.h"

#include "gr_intr_gb10b.h"

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>

void gb10b_gr_intr_handle_gpc_gpccs_exception(struct gk20a *g, u32 gpc,
		u32 gpc_exception, u32 *corrected_err, u32 *uncorrected_err)
{
	u32 hww_esr;
	u32 offset = nvgpu_gr_gpc_offset(g, gpc);

	if ((gpc_exception & gr_gpc0_gpccs_gpc_exception_gpccs_m()) == 0U) {
		return;
	}

	hww_esr = nvgpu_readl(g,
			nvgpu_safe_add_u32(gr_gpc0_gpccs_hww_esr_r(),
						offset));
	/*
	 * Check for poison bit sent from XBAR in case
	 * parity is detected in data packet sent from L2.
	 */
	if ((hww_esr & gr_gpc0_gpccs_hww_esr_poisoned_read_m()) != 0U) {
		nvgpu_err(g, "GPCCS: Poisoned read error, GPC: %d, "
						"hww_esr: 0x%08x", gpc, hww_esr);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GPCCS,
						GPU_GPCCS_POISON_ERROR);
		if (g->ops.xbar.handle_xbar_read_ecc_err != NULL) {
			g->ops.xbar.handle_xbar_read_ecc_err(g);
		}
		/* Toggle the bit to clear the intr */

		g->ops.gr.falcon.dump_stats(g);
		gk20a_fecs_gr_debug_dump(g);

		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_gpccs_hww_esr_r(),
				offset), gr_gpc0_gpccs_hww_esr_poisoned_read_toggle_f());
	}

	/* Check for NACK received on a memory transaction */
	if ((hww_esr & gr_gpc0_gpccs_hww_esr_fault_during_ctxsw_m()) != 0U) {
		nvgpu_err(g, "GPCCS: Fault during CTXSW. GPCCS arbiter received a NACK "
				"on a memory transaction! GPC %d, hww_esr: 0x%08x", gpc,
				hww_esr);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GPCCS,
					GPU_GPCCS_FAULT_DURING_CTXSW);

		g->ops.gr.falcon.dump_stats(g);
		gk20a_fecs_gr_debug_dump(g);

		/* Toggle the bit to clear the intr */
		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_gpccs_hww_esr_r(),
				offset), gr_gpc0_gpccs_hww_esr_fault_during_ctxsw_toggle_f());
	}

	/* Check if the watchdog timer expired in GPCCS. */
	if ((hww_esr & gr_gpc0_gpccs_hww_esr_watchdog_m()) != 0U) {
		nvgpu_err(g, "GPCCS: Watchdog timer expired! GPC: %d, hww_esr: 0x%08x",
				gpc, hww_esr);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GPCCS,
					GPU_GPCCS_WATCHDOG_EXPIRED);

		g->ops.gr.falcon.dump_stats(g);
		gk20a_fecs_gr_debug_dump(g);

		/* Toggle the bit to clear the intr */
		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_gpccs_hww_esr_r(),
				offset), gr_gpc0_gpccs_hww_esr_watchdog_toggle_f());
	}

	/* Check for parity error reported on packets sent
	 * from BSI to GPCC during GPCCS bootstraping
	 */
	if ((hww_esr & gr_gpc0_gpccs_hww_esr_bsi_cmd_parity_error_m()) != 0U) {
		nvgpu_err(g, "GPCCS: bsi_cmd_parity_error! GPC: %d, hww_esr: 0x%08x",
				gpc, hww_esr);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GPCCS,
					GPU_GPCCS_BSI_CMD_PARITY_ERROR);

		g->ops.gr.falcon.dump_stats(g);
		gk20a_fecs_gr_debug_dump(g);

		/* Toggle the bit to clear the intr */
		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_gpccs_hww_esr_r(),
			offset), gr_gpc0_gpccs_hww_esr_bsi_cmd_parity_error_toggle_f());
	}

	/* If BSI sends packets when GPCCS FIFO is full, the
	 * packet will be dropped and this bit will be set.
	 */
	if ((hww_esr & gr_gpc0_gpccs_hww_esr_bsi_cmd_pkt_dropped_error_m()) != 0U) {
		nvgpu_err(g, "GPCCS: bsi_cmd_pkt_dropped_error! GPC: %d, hww_esr: 0x%08x",
				gpc, hww_esr);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GPCCS,
					GPU_GPCCS_BSI_CMD_PKT_DROPPED_ERROR);

		g->ops.gr.falcon.dump_stats(g);
		gk20a_fecs_gr_debug_dump(g);

		/* Toggle the bit to clear the intr */
		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_gpccs_hww_esr_r(),
			offset), gr_gpc0_gpccs_hww_esr_bsi_cmd_pkt_dropped_error_toggle_f());
	}

	/* This bit will be set if the IMEM/DMEM address
	 * in the BSI packet is out of bound.
	 */
	if ((hww_esr & gr_gpc0_gpccs_hww_esr_bsi_cmd_addr_out_of_bound_m()) != 0U) {
		nvgpu_err(g, "GPCCS: bsi_cmd_addr_out_of_bound! GPC: %d, hww_esr: 0x%08x",
				gpc, hww_esr);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GPCCS,
					GPU_GPCCS_BSI_CMD_ADDR_OOB);

		g->ops.gr.falcon.dump_stats(g);
		gk20a_fecs_gr_debug_dump(g);

		/* Toggle the bit to clear the intr */
		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_gpccs_hww_esr_r(),
				offset), gr_gpc0_gpccs_hww_esr_bsi_cmd_addr_out_of_bound_toggle_f());
	}

	gv11b_gr_intr_handle_gpc_gpccs_exception(g, gpc, gpc_exception,
			corrected_err, uncorrected_err);
}

bool gb10b_gr_intr_is_valid_sw_method(struct gk20a *g, u32 class_num, u32 offset)
{
	bool ret = false;

	(void)class_num;
	(void)offset;

	nvgpu_log_fn(g, " ");

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	if (class_num == gr_compute_class_v()) {
		ret = g->ops.gr.intr.is_valid_compute_sw_method(g, offset);
	}
#endif

#if defined(CONFIG_NVGPU_DEBUGGER) && defined(CONFIG_NVGPU_GRAPHICS)
	if (class_num == gr_graphics_class_v()) {
		ret = g->ops.gr.intr.is_valid_gfx_sw_method(g, offset);
	}
#endif

	return ret;
}

int gb10b_gr_intr_handle_sw_method(struct gk20a *g, u32 addr,
			u32 class_num, u32 offset, u32 data)
{
	int ret = -EINVAL;

	(void)addr;
	(void)class_num;
	(void)offset;
	(void)data;

	nvgpu_log_fn(g, " ");

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	if (class_num == gr_compute_class_v()) {
		ret = g->ops.gr.intr.handle_compute_sw_method(g, addr, class_num,
				offset, data);
	}
#endif

#if defined(CONFIG_NVGPU_DEBUGGER) && defined(CONFIG_NVGPU_GRAPHICS)
	if (class_num == gr_graphics_class_v()) {
		ret = g->ops.gr.intr.handle_gfx_sw_method(g, addr, class_num,
				offset, data);
	}
#endif

	return ret;
}

u32 gb10b_gr_intr_read_gpc_tpc_exception(u32 gpc_exception)
{
	return gr_gpc0_gpccs_gpc_exception_tpc_v(gpc_exception);
}

void gb10b_gr_intr_handle_tpc_tpccs_exception(struct gk20a *g, u32 gpc, u32 tpc)
{
	u32 esr;
	u32 gpc_offset, tpc_offset, offset;

	gpc_offset = nvgpu_gr_gpc_offset(g, gpc);
	tpc_offset = nvgpu_gr_tpc_offset(g, tpc);
	offset = nvgpu_safe_add_u32(gpc_offset, tpc_offset);

	esr = nvgpu_readl(g,
			nvgpu_safe_add_u32(gr_gpc0_tpc0_tpccs_hww_esr_r(),
						offset));

	/*
	 * Check for poison bit sent from XBAR in case
	 * parity is detected in data packet sent from L2.
	 */
	if ((esr & gr_gpc0_tpc0_tpccs_hww_esr_poisoned_read_m()) != 0U) {
		nvgpu_err(g, "TPCCS: poisoned read error in GPC%d, TPC%d! "
				"hww_esr: 0x%08x", gpc, tpc, esr);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_PGRAPH,
					GPU_PGRAPH_TPCCS_POISON_ERROR);
		if (g->ops.xbar.handle_xbar_read_ecc_err != NULL) {
			g->ops.xbar.handle_xbar_read_ecc_err(g);
		}
		/* Toggle the bit to clear the intr*/
		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_tpc0_tpccs_hww_esr_r(),
			offset), gr_gpc0_tpc0_tpccs_hww_esr_poisoned_read_toggle_f());
	}
	/* Check for NACK received by the TPCCS arbiter on a memory transaction */
	if ((esr & gr_gpc0_tpc0_tpccs_hww_esr_fault_during_ctxsw_m()) != 0U) {
		nvgpu_err(g, "TPCCS: Fault during CTXSW. TPCCS arbiter received a NACK "
				"on a memory transaction! GPC %d, TPC %d, hww_esr: 0x%08x", gpc,
				tpc, esr);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_PGRAPH,
					GPU_PGRAPH_TPCCS_FAULT_DURING_CTXSW);
		/* Toggle the bit to clear the intr*/
		nvgpu_writel(g, nvgpu_safe_add_u32(gr_gpc0_tpc0_tpccs_hww_esr_r(),
				offset), gr_gpc0_tpc0_tpccs_hww_esr_fault_during_ctxsw_toggle_f());
	}
}

void gb10b_gr_intr_handle_tpc_mpc_exception(struct gk20a *g, u32 gpc, u32 tpc)
{
	u32 esr;
	u32 gpc_offset, tpc_offset, offset;

	gpc_offset = nvgpu_gr_gpc_offset(g, gpc);
	tpc_offset = nvgpu_gr_tpc_offset(g, tpc);
	offset = nvgpu_safe_add_u32(gpc_offset, tpc_offset);

	esr = nvgpu_readl(g,
			nvgpu_safe_add_u32(gr_gpc0_tpc0_mpc_hww_esr_r(),
						offset));
	nvgpu_err(g, "mpc hww esr 0x%08x", esr);

	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_PGRAPH,
			GPU_PGRAPH_MPC_EXCEPTION);

	esr = nvgpu_readl(g,
			nvgpu_safe_add_u32(gr_gpc0_tpc0_mpc_hww_esr_info_r(),
						offset));
	nvgpu_log(g, gpu_dbg_intr | gpu_dbg_gpu_dbg,
			"mpc hww esr info: veid 0x%08x",
			gr_gpc0_tpc0_mpc_hww_esr_info_veid_v(esr));

	nvgpu_writel(g,
		     nvgpu_safe_add_u32(gr_gpc0_tpc0_mpc_hww_esr_r(),
						offset),
		     gr_gpc0_tpc0_mpc_hww_esr_reset_trigger_f());
}

void gb10b_gr_intr_clear_sm_hww(struct gk20a *g, u32 gpc, u32 tpc, u32 sm,
				u32 global_esr)
{
	u32 offset;
	u32 gpc_offset, tpc_offset, sm_offset;

	gpc_offset = nvgpu_gr_gpc_offset(g, gpc);
	tpc_offset = nvgpu_gr_tpc_offset(g, tpc);
	sm_offset = nvgpu_gr_sm_offset(g, sm);

	offset = nvgpu_safe_add_u32(gpc_offset,
			nvgpu_safe_add_u32(tpc_offset, sm_offset));

	nvgpu_writel(g, nvgpu_safe_add_u32(
				gr_gpc0_tpc0_sm0_hww_global_esr_r(), offset),
			global_esr);
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg,
			"Cleared HWW global esr, current reg val: 0x%x",
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_tpc0_sm0_hww_global_esr_r(), offset)));

	nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_gpc0_tpc0_sm0_hww_warp_esr_r(), offset), 0);
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg,
			"Cleared HWW warp esr, current reg val: 0x%x",
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_tpc0_sm0_hww_warp_esr_r(), offset)));
}

u32 gb10b_gr_intr_get_warp_esr_sm_hww(struct gk20a *g,
			u32 gpc, u32 tpc, u32 sm)
{
	u32 gpc_offset, tpc_offset, sm_offset, offset;
	u32 hww_warp_esr;

	gpc_offset = nvgpu_gr_gpc_offset(g, gpc);
	tpc_offset = nvgpu_gr_tpc_offset(g, tpc);
	sm_offset = nvgpu_gr_sm_offset(g, sm);

	offset = nvgpu_safe_add_u32(gpc_offset,
			nvgpu_safe_add_u32(tpc_offset, sm_offset));

	hww_warp_esr = nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_tpc0_sm0_hww_warp_esr_r(), offset));
	return hww_warp_esr;
}

u32 gb10b_gr_intr_get_sm_hww_global_esr(struct gk20a *g,
			u32 gpc, u32 tpc, u32 sm)
{
	u32 gpc_offset, tpc_offset, sm_offset, offset;
	u32 hww_global_esr;

	gpc_offset = nvgpu_gr_gpc_offset(g, gpc);
	tpc_offset = nvgpu_gr_tpc_offset(g, tpc);
	sm_offset = nvgpu_gr_sm_offset(g, sm);

	offset = nvgpu_safe_add_u32(gpc_offset,
			nvgpu_safe_add_u32(tpc_offset, sm_offset));

	hww_global_esr = nvgpu_readl(g, nvgpu_safe_add_u32(
				 gr_gpc0_tpc0_sm0_hww_global_esr_r(), offset));

	return hww_global_esr;
}

u32 gb10b_gr_intr_get_sm_hww_cga_esr(struct gk20a *g,
			u32 gpc, u32 tpc, u32 sm)
{
	u32 gpc_offset, tpc_offset, sm_offset, offset;
	u32 hww_cga_esr;

	gpc_offset = nvgpu_gr_gpc_offset(g, gpc);
	tpc_offset = nvgpu_gr_tpc_offset(g, tpc);
	sm_offset = nvgpu_gr_sm_offset(g, sm);

	offset = nvgpu_safe_add_u32(gpc_offset,
			nvgpu_safe_add_u32(tpc_offset, sm_offset));

	hww_cga_esr = nvgpu_readl(g, nvgpu_safe_add_u32(
				 gr_gpc0_tpc0_sm0_hww_cga_esr_r(), offset));

	return hww_cga_esr;
}

void gb10b_gr_intr_handle_sm_hww_cga_esr(struct gk20a *g, u32 cga_esr,
			u32 gpc, u32 tpc, u32 sm)
{
	u32 gpc_offset, tpc_offset, sm_offset, offset;

	if (gr_gpc0_tpc0_sm0_hww_cga_esr_error_v(cga_esr) ==
			gr_gpc0_tpc0_sm0_hww_cga_esr_error_none_v()) {
		return;
	}

	gpc_offset = nvgpu_gr_gpc_offset(g, gpc);
	tpc_offset = nvgpu_gr_tpc_offset(g, tpc);
	sm_offset = nvgpu_gr_sm_offset(g, sm);
	offset = nvgpu_safe_add_u32(gpc_offset,
			nvgpu_safe_add_u32(tpc_offset, sm_offset));
	nvgpu_err (g, "HWW_CGA_ESR error on gpc(%d) tpc(%d) sm(%d): cga_esr(0x%x)",
				gpc, tpc, sm, cga_esr);
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SM,
			GPU_SM_CGA_ERROR);
	if (cga_esr & gr_gpc0_tpc0_sm0_hww_cga_esr_error_cta_not_present_f()) {
		nvgpu_err (g, "CTA NOT PRESENT");
	} else if (cga_esr & gr_gpc0_tpc0_sm0_hww_cga_esr_error_oor_addr_f()) {
		nvgpu_err (g, "OOR ADDR");
	} else if (cga_esr & gr_gpc0_tpc0_sm0_hww_cga_esr_error_poison_f()) {
		nvgpu_err (g, "ERROR POISON");
	}

	nvgpu_err (g, "HWW_CGA_ESR: CTA_ID_IN_CGA (0x%08x), GPC_LOCAL_CGA_ID (0x%08x)",
			gr_gpc0_tpc0_sm0_hww_cga_esr_cta_id_in_cga_v(cga_esr),
			gr_gpc0_tpc0_sm0_hww_cga_esr_gpc_local_cga_id_v(cga_esr));

	nvgpu_writel(g, nvgpu_safe_add_u32(
				 gr_gpc0_tpc0_sm0_hww_cga_esr_r(), offset), 0U);

	return;
}

static u32 get_sm_hww_warp_esr_report_mask(void)
{
	u32 mask = gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_api_stack_error_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_misaligned_pc_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_pc_overflow_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_misaligned_reg_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_illegal_instr_encoding_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_illegal_instr_param_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_oor_reg_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_oor_addr_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_misaligned_addr_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_invalid_addr_space_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_invalid_const_addr_ldc_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_mmu_fault_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_tex_format_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_tex_layout_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_mmu_nack_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_arrive_report_f() |
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_cta_not_present_report_f();

	return mask;
}

void gb10b_handle_sm_poison_error(struct gk20a *g, u32 global_esr)
{

	if ((global_esr &
		gr_fecs_host_int_status_poisoned_read_active_f()) == 0U)
		return;

	nvgpu_log(g, gpu_dbg_intr, "sm poison error");

	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SM,
		GPU_SM_POISON_ERROR);

	if (g->ops.xbar.handle_xbar_read_ecc_err != NULL) {
		g->ops.xbar.handle_xbar_read_ecc_err(g);
	}
}

static u32 get_sm_hww_global_esr_report_mask(void)
{
	u32 mask = gr_gpc0_tpc0_sm0_hww_global_esr_report_mask_multiple_warp_errors_report_f() |
		gr_gpc0_tpc0_sm0_hww_global_esr_report_mask_bpt_int_report_f() |
		gr_gpc0_tpc0_sm0_hww_global_esr_report_mask_bpt_pause_report_f() |
		gr_gpc0_tpc0_sm0_hww_global_esr_report_mask_single_step_complete_report_f() |
		gr_gpc0_tpc0_sm0_hww_global_esr_report_mask_error_in_trap_report_f();

		/*
		 * REPORT_MASK_POISON_DATA bit is defined but not used by HW.
		 * A GPU fuse option enables 'poison' detection. Bug 2090067.
		 */

	return mask;
}

void gb10b_gr_intr_set_hww_esr_report_mask(struct gk20a *g)
{
	u32 sm_hww_warp_esr_report_mask;
	u32 sm_hww_global_esr_report_mask;

	/*
	 * Perform a RMW to the warp, global ESR report mask registers.
	 * This is done in-order to retain the default values loaded from
	 * sw_ctx_load.
	 */
	sm_hww_warp_esr_report_mask = nvgpu_readl(g,
		gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_r());
	sm_hww_global_esr_report_mask = nvgpu_readl(g,
		gr_gpc0_tpc0_sm0_hww_global_esr_report_mask_r());

	/* clear hww */
	nvgpu_writel(g, gr_gpcs_tpcs_sms_hww_global_esr_r(), U32_MAX);

	/*
	 * setup sm warp esr report masks
	 */
	nvgpu_writel(g, gr_gpcs_tpcs_sms_hww_warp_esr_report_mask_r(),
		sm_hww_warp_esr_report_mask | get_sm_hww_warp_esr_report_mask());

	nvgpu_writel(g, gr_gpcs_tpcs_sms_hww_global_esr_report_mask_r(),
		sm_hww_global_esr_report_mask | get_sm_hww_global_esr_report_mask());

	nvgpu_log_info(g,
		"configured (global, warp)_esr_report_mask(0x%x, 0x%x)",
		sm_hww_global_esr_report_mask | get_sm_hww_global_esr_report_mask(),
		sm_hww_warp_esr_report_mask | get_sm_hww_warp_esr_report_mask());
}

u64 gb10b_gr_intr_get_warp_esr_pc_sm_hww(struct gk20a *g, u32 offset)
{
	u64 hww_warp_esr_pc;
	u32 addr_hi = nvgpu_readl(g, nvgpu_safe_add_u32(
			gr_gpc0_tpc0_sm0_hww_warp_esr_pc_hi_r(), offset));
	u32 addr_lo = nvgpu_readl(g, nvgpu_safe_add_u32(
			gr_gpc0_tpc0_sm0_hww_warp_esr_pc_r(), offset));
	hww_warp_esr_pc = hi32_lo32_to_u64(addr_hi, addr_lo);

	return hww_warp_esr_pc;
}

void gb10b_gr_intr_get_esr_sm_sel(struct gk20a *g, u32 gpc, u32 tpc,
				u32 *esr_sm_sel)
{
	u32 reg_val;
	u32 gpc_offset, tpc_offset, offset;

	gpc_offset = nvgpu_gr_gpc_offset(g, gpc);
	tpc_offset = nvgpu_gr_tpc_offset(g, tpc);
	offset = nvgpu_safe_add_u32(gpc_offset, tpc_offset);

	reg_val = nvgpu_readl(g, nvgpu_safe_add_u32(
			gr_gpc0_tpc0_sm_tpc_esr_sm_sel_r(), offset));
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg,
			"sm tpc esr sm sel reg val: 0x%x", reg_val);
	*esr_sm_sel = 0;
	if (gr_gpc0_tpc0_sm_tpc_esr_sm_sel_sm0_error_v(reg_val) != 0U) {
		*esr_sm_sel = 1;
	}
	if (gr_gpc0_tpc0_sm_tpc_esr_sm_sel_sm1_error_v(reg_val) != 0U) {
		*esr_sm_sel |= BIT32(1);
	}
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg,
			"esr_sm_sel bitmask: 0x%x", *esr_sm_sel);
}

u32 gb10b_gr_intr_get_tpc_exception(struct gk20a *g, u32 offset,
				    struct nvgpu_gr_tpc_exception *pending_tpc)
{
	u32 tpc_exception = nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_tpc0_tpccs_tpc_exception_r(), offset));

	(void) memset(pending_tpc, 0, sizeof(struct nvgpu_gr_tpc_exception));

	if ((tpc_exception & gr_gpc0_tpc0_tpccs_tpc_exception_tpccs_m()) != 0U) {
		pending_tpc->tpccs_exception = true;
	}

	if (gr_gpc0_tpc0_tpccs_tpc_exception_sm_v(tpc_exception) ==
		gr_gpc0_tpc0_tpccs_tpc_exception_sm_pending_v()) {
			pending_tpc->sm_exception = true;
	}

	if ((tpc_exception & gr_gpc0_tpc0_tpccs_tpc_exception_mpc_m()) != 0U) {
		pending_tpc->mpc_exception = true;
	}

	return tpc_exception;
}

void gb10b_gr_intr_handle_gpc_wdxps_hww(struct gk20a *g, u32 gpc,
		u32 gpc_exception)
{
	u32 offset = nvgpu_gr_gpc_offset(g, gpc);
	u32 hww_esr, hww_esr_r = 0U;

	if ((gpc_exception &
		gr_gpc0_gpccs_gpc_exception_wdxps_pending_f()) == 0U) {
		return;
	}

	if (!nvgpu_safe_add_u32_return(gr_gpc0_wdxps_hww_esr_r(), offset,
			&hww_esr_r)) {
		nvgpu_err(g, "overflow while calculating WDXPS HWW ESR offset!");
		return;
	}

	hww_esr = nvgpu_readl(g, hww_esr_r);

	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_PGRAPH,
			GPU_PGRAPH_GPC_WDXPS_EXCEPTION);

	/* clear the interrupt */
	nvgpu_writel(g, hww_esr_r,
				gr_gpc0_wdxps_hww_esr_reset_active_f());

	nvgpu_err(g, "gpc:%d wdxps_exception interrupt intr: 0x%x",
		gpc, hww_esr);

}

void gb10b_gr_intr_enable_gpc_exceptions(struct gk20a *g,
					 struct nvgpu_gr_config *gr_config)
{
	u32 tpc_mask, tpc_mask_calc;

	nvgpu_writel(g, gr_gpcs_tpcs_tpccs_tpc_exception_en_r(),
			gr_gpcs_tpcs_tpccs_tpc_exception_en_sm_enabled_f() |
			gr_gpcs_tpcs_tpccs_tpc_exception_en_mpc_enabled_f() |
			gr_gpcs_tpcs_tpccs_tpc_exception_en_tpccs_enabled_f());

	tpc_mask_calc = BIT32(
			 nvgpu_gr_config_get_max_tpc_per_gpc_count(gr_config));
	tpc_mask =
		gr_gpcs_gpccs_gpc_exception_en_tpc_f(
			nvgpu_safe_sub_u32(tpc_mask_calc, 1U));

	/*
	 * Enable exceptions from ROP subunits: zrop and crop. The rrh subunit
	 * does not have a subunit level enable.
	 */
	g->ops.gr.intr.enable_gpc_zrop_hww(g);
	g->ops.gr.intr.enable_gpc_crop_hww(g);

	nvgpu_writel(g, gr_gpcs_gpccs_gpc_exception_en_r(),
		(tpc_mask | gr_gpcs_gpccs_gpc_exception_en_gcc_enabled_f() |
			    gr_gpcs_gpccs_gpc_exception_en_gpccs_enabled_f() |
			    gr_gpcs_gpccs_gpc_exception_en_gpcmmu0_enabled_f() |
			    gr_gpcs_gpccs_gpc_exception_en_crop0_enabled_f() |
			    gr_gpcs_gpccs_gpc_exception_en_zrop0_enabled_f() |
			    gr_gpcs_gpccs_gpc_exception_en_rrh0_enabled_f() |
			    gr_gpcs_gpccs_gpc_exception_en_crop1_enabled_f() |
			    gr_gpcs_gpccs_gpc_exception_en_zrop1_enabled_f() |
			    gr_gpcs_gpccs_gpc_exception_en_rrh1_enabled_f()) |
			    gr_gpcs_gpccs_gpc_exception_en_wdxps_enabled_f());
}

static void gb10b_gr_intr_handle_cbu_exception(struct gk20a *g, u32 gpc, u32 tpc)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_in_gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	u32 offset;
	u32 cbu_ecc_status;
	u32 cbu_corrected_err_count_delta = 0U;
	u32 cbu_uncorrected_err_count_delta = 0U;
	bool is_cbu_ecc_corrected_total_err_overflow = false;
	bool is_cbu_ecc_uncorrected_total_err_overflow = false;
	struct nvgpu_gr_sm_ecc_status ecc_status;

	offset = nvgpu_safe_add_u32(
			nvgpu_safe_mult_u32(gpc_stride, gpc),
			nvgpu_safe_mult_u32(tpc_in_gpc_stride, tpc));

	/* Check for CBU ECC errors. */
	cbu_ecc_status = nvgpu_readl(g, nvgpu_safe_add_u32(
		gr_pri_gpc0_tpc0_sm_cbu_ecc_status_r(), offset));

	(void) memset(&ecc_status, 0, sizeof(struct nvgpu_gr_sm_ecc_status));

	if (g->ops.gr.intr.sm_ecc_status_errors(g, cbu_ecc_status,
				SM_CBU_ECC_ERROR, &ecc_status) == false) {
		return;
	}

	cbu_corrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_cbu_ecc_corrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_cbu_ecc_corrected_err_count_r(),
				offset)));
	cbu_uncorrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_cbu_ecc_uncorrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_cbu_ecc_uncorrected_err_count_r(),
				offset)));
	is_cbu_ecc_corrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_cbu_ecc_status_corrected_err_total_counter_overflow_v(cbu_ecc_status) != 0U;
	is_cbu_ecc_uncorrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_cbu_ecc_status_uncorrected_err_total_counter_overflow_v(cbu_ecc_status) != 0U;

	/* This check has been added to ensure that the TPC id is less than
	 * 8-bits and hence, it can be packed as part of LSB 8-bits along with
	 * the GPC id while reporting SM related ECC errors.
	 */
	tpc = tpc & U8_MAX;

	if ((cbu_corrected_err_count_delta > 0U) ||
		is_cbu_ecc_corrected_total_err_overflow) {
		nvgpu_err(g, "unexpected corrected error (SBE) detected in SM CBU!"
			" err_mask [%08x] is_overf [%d]",
			ecc_status.corrected_err_status,
			is_cbu_ecc_corrected_total_err_overflow);

		/* This error is not expected to occur in gv11b and hence,
		 * this scenario is considered as a fatal error.
		 */
		BUG();
	}
	if ((cbu_uncorrected_err_count_delta > 0U) || is_cbu_ecc_uncorrected_total_err_overflow) {
		nvgpu_err(g,
			"Uncorrected error (DBE) detected in SM CBU! err_mask [%08x] is_overf [%d]",
			ecc_status.uncorrected_err_status, is_cbu_ecc_uncorrected_total_err_overflow);

		/* HW uses 16-bits counter */
		if (is_cbu_ecc_uncorrected_total_err_overflow) {
			cbu_uncorrected_err_count_delta =
			   nvgpu_wrapping_add_u32(cbu_uncorrected_err_count_delta,
				BIT32(gr_pri_gpc0_tpc0_sm_cbu_ecc_uncorrected_err_count_total_s()));
		}
		g->ecc.gr.sm_cbu_ecc_uncorrected_err_count[gpc][tpc].counter =
		   nvgpu_wrapping_add_u32(
			g->ecc.gr.sm_cbu_ecc_uncorrected_err_count[gpc][tpc].counter,
			cbu_uncorrected_err_count_delta);
		nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_cbu_ecc_uncorrected_err_count_r(), offset),
			0U);
	}

	nvgpu_writel(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_cbu_ecc_status_r(), offset),
			gr_pri_gpc0_tpc0_sm_cbu_ecc_status_reset_task_f());
}

static void gb10b_gr_intr_handle_icache_exception(struct gk20a *g, u32 gpc, u32 tpc)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_in_gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	u32 offset;
	u32 icache_ecc_status;
	u32 icache_corrected_err_count_delta = 0U;
	u32 icache_uncorrected_err_count_delta = 0U;
	bool is_icache_ecc_corrected_total_err_overflow = false;
	bool is_icache_ecc_uncorrected_total_err_overflow = false;
	struct nvgpu_gr_sm_ecc_status ecc_status;

	offset = nvgpu_safe_add_u32(
			nvgpu_safe_mult_u32(gpc_stride, gpc),
			nvgpu_safe_mult_u32(tpc_in_gpc_stride, tpc));

	/* Check for L0 && L1 icache ECC errors. */
	icache_ecc_status = nvgpu_readl(g, nvgpu_safe_add_u32(
		gr_pri_gpc0_tpc0_sm_icache_ecc_status_r(), offset));

	(void) memset(&ecc_status, 0, sizeof(struct nvgpu_gr_sm_ecc_status));

	if (g->ops.gr.intr.sm_ecc_status_errors(g, icache_ecc_status,
				SM_ICACHE_ECC_ERROR, &ecc_status) == false) {
		return;
	}

	icache_corrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_icache_ecc_corrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_icache_ecc_corrected_err_count_r(),
				offset)));
	icache_uncorrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_icache_ecc_uncorrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_icache_ecc_uncorrected_err_count_r(),
				offset)));
	is_icache_ecc_corrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_icache_ecc_status_corrected_err_total_counter_overflow_v(icache_ecc_status) != 0U;
	is_icache_ecc_uncorrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_icache_ecc_status_uncorrected_err_total_counter_overflow_v(icache_ecc_status) != 0U;

	if ((icache_corrected_err_count_delta > 0U) || is_icache_ecc_corrected_total_err_overflow) {
		nvgpu_err(g,
			"corrected error (SBE) detected in SM L0 && L1 icache! err_mask [%08x] is_overf [%d]",
			ecc_status.corrected_err_status, is_icache_ecc_corrected_total_err_overflow);

		/* HW uses 16-bits counter */
		if (is_icache_ecc_corrected_total_err_overflow) {
			icache_corrected_err_count_delta =
			   nvgpu_wrapping_add_u32(icache_corrected_err_count_delta,
				BIT32(gr_pri_gpc0_tpc0_sm_icache_ecc_corrected_err_count_total_s()));
		}
		g->ecc.gr.sm_icache_ecc_corrected_err_count[gpc][tpc].counter =
		   nvgpu_wrapping_add_u32(
			g->ecc.gr.sm_icache_ecc_corrected_err_count[gpc][tpc].counter,
			icache_corrected_err_count_delta);
		nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_icache_ecc_corrected_err_count_r(), offset),
			0U);
	}

	if ((icache_uncorrected_err_count_delta > 0U) || is_icache_ecc_uncorrected_total_err_overflow) {
		nvgpu_err(g,
			"Uncorrected error (DBE) detected in SM L0 && L1 icache! err_mask [%08x] is_overf [%d]",
			ecc_status.uncorrected_err_status, is_icache_ecc_uncorrected_total_err_overflow);

		/* HW uses 16-bits counter */
		if (is_icache_ecc_uncorrected_total_err_overflow) {
			icache_uncorrected_err_count_delta =
			   nvgpu_wrapping_add_u32(
				icache_uncorrected_err_count_delta,
				BIT32(gr_pri_gpc0_tpc0_sm_icache_ecc_uncorrected_err_count_total_s()));
		}
		g->ecc.gr.sm_icache_ecc_uncorrected_err_count[gpc][tpc].counter =
		  nvgpu_wrapping_add_u32(
			g->ecc.gr.sm_icache_ecc_uncorrected_err_count[gpc][tpc].counter,
			icache_uncorrected_err_count_delta);
		nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_icache_ecc_uncorrected_err_count_r(), offset),
			0U);
		gv11b_gr_intr_report_icache_uncorrected_err(g, &ecc_status, gpc, tpc);
	}

	nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_icache_ecc_status_r(), offset),
			gr_pri_gpc0_tpc0_sm_icache_ecc_status_reset_task_f());
}

static void gb10b_gr_intr_handle_l1_data_exception(struct gk20a *g, u32 gpc, u32 tpc)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_in_gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	u32 offset;
	u32 l1_data_ecc_status;
	u32 l1_data_corrected_err_count_delta = 0U;
	u32 l1_data_uncorrected_err_count_delta = 0U;
	bool is_l1_data_ecc_corrected_total_err_overflow = false;
	bool is_l1_data_ecc_uncorrected_total_err_overflow = false;
	struct nvgpu_gr_sm_ecc_status ecc_status;

	offset = nvgpu_safe_add_u32(
			nvgpu_safe_mult_u32(gpc_stride, gpc),
			nvgpu_safe_mult_u32(tpc_in_gpc_stride, tpc));

	/* Check for L1 data ECC errors. */
	l1_data_ecc_status = nvgpu_readl(g, nvgpu_safe_add_u32(
		gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_r(), offset));

	(void) memset(&ecc_status, 0, sizeof(struct nvgpu_gr_sm_ecc_status));

	if (g->ops.gr.intr.sm_ecc_status_errors(g, l1_data_ecc_status,
				SM_L1_DATA_ECC_ERROR, &ecc_status) == false) {
		return;
	}

	l1_data_corrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_l1_data_ecc_corrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_l1_data_ecc_corrected_err_count_r(),
				offset)));
	l1_data_uncorrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_l1_data_ecc_uncorrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_l1_data_ecc_uncorrected_err_count_r(),
				offset)));
	is_l1_data_ecc_corrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_corrected_err_total_counter_overflow_v(l1_data_ecc_status) != 0U;
	is_l1_data_ecc_uncorrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_uncorrected_err_total_counter_overflow_v(l1_data_ecc_status) != 0U;

	/* This check has been added to ensure that the TPC id is less than
	 * 8-bits and hence, it can be packed as part of LSB 8-bits along with
	 * the GPC id while reporting SM related ECC errors.
	 */
	tpc = tpc & U8_MAX;

	if ((l1_data_corrected_err_count_delta > 0U) ||
		is_l1_data_ecc_corrected_total_err_overflow) {
		nvgpu_err(g, "unexpected corrected error (SBE) detected in SM L1 data!"
			" err_mask [%08x] is_overf [%d]",
			ecc_status.corrected_err_status,
			is_l1_data_ecc_corrected_total_err_overflow);

		/* This error is not expected to occur in gv11b and hence,
		 * this scenario is considered as a fatal error.
		 */
		BUG();
	}

	if ((l1_data_uncorrected_err_count_delta > 0U) || is_l1_data_ecc_uncorrected_total_err_overflow) {
		nvgpu_err(g,
			"Uncorrected error (DBE) detected in SM L1 data! err_mask [%08x] is_overf [%d]",
			ecc_status.uncorrected_err_status, is_l1_data_ecc_uncorrected_total_err_overflow);

		/* HW uses 16-bits counter */
		if (is_l1_data_ecc_uncorrected_total_err_overflow) {
			l1_data_uncorrected_err_count_delta =
			   nvgpu_wrapping_add_u32(l1_data_uncorrected_err_count_delta,
				BIT32(gr_pri_gpc0_tpc0_sm_l1_data_ecc_uncorrected_err_count_total_s()));
		}
		g->ecc.gr.sm_l1_data_ecc_uncorrected_err_count[gpc][tpc].counter =
		   nvgpu_wrapping_add_u32(
			g->ecc.gr.sm_l1_data_ecc_uncorrected_err_count[gpc][tpc].counter,
			l1_data_uncorrected_err_count_delta);
		nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_l1_data_ecc_uncorrected_err_count_r(), offset),
			0U);
	}
	nvgpu_writel(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_r(), offset),
			gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_reset_task_f());
}

static void gb10b_gr_intr_report_l1_tag_uncorrected_err(struct gk20a *g,
		struct nvgpu_gr_sm_ecc_status *ecc_status, u32 gpc, u32 tpc)
{
	u32 i;

	/* This check has been added to ensure that the TPC id is less than
	 * 8-bits and hence, it can be packed as part of LSB 8-bits along with
	 * the GPC id while reporting SM related ECC errors.
	 */
	tpc = tpc & U8_MAX;

	for (i = 0U; i < ecc_status->err_count; i++) {
		if (ecc_status->err_id[i] == GPU_SM_L1_TAG_PQ_ECC_UNCORRECTED) {
			nvgpu_err(g, "sm_l1_tag_pq_ecc_uncorrected "
					"gpc_id(%d), tpc_id(%d)", gpc, tpc);
		}
	}
}

static void gb10b_gr_intr_handle_l1_tag_exception(struct gk20a *g, u32 gpc, u32 tpc)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_in_gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	u32 offset;
	u32 l1_tag_ecc_status;
	u32 l1_tag_corrected_err_count_delta = 0U;
	u32 l1_tag_uncorrected_err_count_delta = 0U;
	bool is_l1_tag_ecc_corrected_total_err_overflow = false;
	bool is_l1_tag_ecc_uncorrected_total_err_overflow = false;
	struct nvgpu_gr_sm_ecc_status ecc_status;

	offset = nvgpu_safe_add_u32(
			nvgpu_safe_mult_u32(gpc_stride, gpc),
			nvgpu_safe_mult_u32(tpc_in_gpc_stride, tpc));

	/* Check for L1 tag ECC errors. */
	l1_tag_ecc_status = nvgpu_readl(g, nvgpu_safe_add_u32(
		gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_r(), offset));

	(void) memset(&ecc_status, 0, sizeof(struct nvgpu_gr_sm_ecc_status));

	if (g->ops.gr.intr.sm_ecc_status_errors(g, l1_tag_ecc_status,
				SM_L1_TAG_ERROR, &ecc_status) == false) {
		return;
	}

	l1_tag_corrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_l1_tag_ecc_corrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_l1_tag_ecc_corrected_err_count_r(),
				offset)));
	l1_tag_uncorrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_l1_tag_ecc_uncorrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_l1_tag_ecc_uncorrected_err_count_r(),
				offset)));
	is_l1_tag_ecc_corrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_corrected_err_total_counter_overflow_v(l1_tag_ecc_status) != 0U;
	is_l1_tag_ecc_uncorrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_uncorrected_err_total_counter_overflow_v(l1_tag_ecc_status) != 0U;

	if ((l1_tag_corrected_err_count_delta > 0U) || is_l1_tag_ecc_corrected_total_err_overflow) {
		nvgpu_err(g,
			"corrected error (SBE) detected in SM L1 tag! err_mask [%08x] is_overf [%d]",
			ecc_status.corrected_err_status, is_l1_tag_ecc_corrected_total_err_overflow);

		/* HW uses 16-bits counter */
		if (is_l1_tag_ecc_corrected_total_err_overflow) {
			l1_tag_corrected_err_count_delta =
			   nvgpu_wrapping_add_u32(
				l1_tag_corrected_err_count_delta,
				BIT32(gr_pri_gpc0_tpc0_sm_l1_tag_ecc_corrected_err_count_total_s()));
		}
		g->ecc.gr.sm_l1_tag_ecc_corrected_err_count[gpc][tpc].counter =
		   nvgpu_wrapping_add_u32(
			g->ecc.gr.sm_l1_tag_ecc_corrected_err_count[gpc][tpc].counter,
			l1_tag_corrected_err_count_delta);
		gv11b_gr_intr_report_l1_tag_corrected_err(g, &ecc_status, gpc, tpc);
		nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_l1_tag_ecc_corrected_err_count_r(), offset),
			0U);
	}
	if ((l1_tag_uncorrected_err_count_delta > 0U) || is_l1_tag_ecc_uncorrected_total_err_overflow) {
		nvgpu_err(g,
			"Uncorrected error (DBE) detected in SM L1 tag! err_mask [%08x] is_overf [%d]",
			ecc_status.uncorrected_err_status, is_l1_tag_ecc_uncorrected_total_err_overflow);

		/* HW uses 16-bits counter */
		if (is_l1_tag_ecc_uncorrected_total_err_overflow) {
			l1_tag_uncorrected_err_count_delta =
			    nvgpu_wrapping_add_u32(
				l1_tag_uncorrected_err_count_delta,
				BIT32(gr_pri_gpc0_tpc0_sm_l1_tag_ecc_uncorrected_err_count_total_s()));
		}
		g->ecc.gr.sm_l1_tag_ecc_uncorrected_err_count[gpc][tpc].counter =
		   nvgpu_wrapping_add_u32(
			g->ecc.gr.sm_l1_tag_ecc_uncorrected_err_count[gpc][tpc].counter,
			l1_tag_uncorrected_err_count_delta);
		gv11b_gr_intr_report_l1_tag_uncorrected_err(g, &ecc_status, gpc, tpc);
		gb10b_gr_intr_report_l1_tag_uncorrected_err(g, &ecc_status, gpc, tpc);
		nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_l1_tag_ecc_uncorrected_err_count_r(), offset),
			0U);
	}

	nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_r(), offset),
			gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_reset_task_f());
}

static void gb10b_gr_intr_handle_lrf_exception(struct gk20a *g, u32 gpc, u32 tpc)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_in_gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	u32 offset;
	u32 lrf_ecc_status;
	u32 lrf_corrected_err_count_delta = 0U;
	u32 lrf_uncorrected_err_count_delta = 0U;
	bool is_lrf_ecc_corrected_total_err_overflow = false;
	bool is_lrf_ecc_uncorrected_total_err_overflow = false;
	struct nvgpu_gr_sm_ecc_status ecc_status;

	offset = nvgpu_safe_add_u32(
			nvgpu_safe_mult_u32(gpc_stride, gpc),
			nvgpu_safe_mult_u32(tpc_in_gpc_stride, tpc));

	/* Check for LRF ECC errors. */
	lrf_ecc_status = nvgpu_readl(g,
		nvgpu_safe_add_u32(gr_pri_gpc0_tpc0_sm_lrf_ecc_status_r(),
				   offset));

	(void) memset(&ecc_status, 0, sizeof(struct nvgpu_gr_sm_ecc_status));

	if (g->ops.gr.intr.sm_ecc_status_errors(g, lrf_ecc_status,
			SM_LRF_ECC_ERROR, &ecc_status) == false) {
		return;
	}

	lrf_corrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_lrf_ecc_corrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_lrf_ecc_corrected_err_count_r(),
				offset)));
	lrf_uncorrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_lrf_ecc_uncorrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_lrf_ecc_uncorrected_err_count_r(),
				offset)));
	is_lrf_ecc_corrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_lrf_ecc_status_corrected_err_total_counter_overflow_v(lrf_ecc_status) != 0U;
	is_lrf_ecc_uncorrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_lrf_ecc_status_uncorrected_err_total_counter_overflow_v(lrf_ecc_status) != 0U;

	/* This check has been added to ensure that the TPC id is less than
	 * 8-bits and hence, it can be packed as part of LSB 8-bits along with
	 * the GPC id while reporting SM related ECC errors.
	 */
	tpc = tpc & U8_MAX;

	if ((lrf_corrected_err_count_delta > 0U) ||
			is_lrf_ecc_corrected_total_err_overflow) {
		nvgpu_err(g, "unexpected corrected error (SBE) detected in SM LRF!"
			" err_mask [%08x] is_overf [%d]",
			ecc_status.corrected_err_status,
			is_lrf_ecc_corrected_total_err_overflow);

		/* This error is not expected to occur in gv11b and hence,
		 * this scenario is considered as a fatal error.
		 */
		BUG();
	}
	if ((lrf_uncorrected_err_count_delta > 0U) || is_lrf_ecc_uncorrected_total_err_overflow) {
		nvgpu_err(g,
			"Uncorrected error (DBE) detected in SM LRF! err_mask [%08x] is_overf [%d]",
			ecc_status.uncorrected_err_status, is_lrf_ecc_uncorrected_total_err_overflow);

		/* HW uses 16-bits counter */
		if (is_lrf_ecc_uncorrected_total_err_overflow) {
			lrf_uncorrected_err_count_delta =
			   nvgpu_wrapping_add_u32(
				lrf_uncorrected_err_count_delta,
				BIT32(gr_pri_gpc0_tpc0_sm_lrf_ecc_uncorrected_err_count_total_s()));
		}
		g->ecc.gr.sm_lrf_ecc_double_err_count[gpc][tpc].counter =
		   nvgpu_wrapping_add_u32(
			g->ecc.gr.sm_lrf_ecc_double_err_count[gpc][tpc].counter,
			lrf_uncorrected_err_count_delta);
		nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_lrf_ecc_uncorrected_err_count_r(), offset),
			0U);
	}

	nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_lrf_ecc_status_r(), offset),
			gr_pri_gpc0_tpc0_sm_lrf_ecc_status_reset_task_f());
}

static void gb10b_gr_intr_report_rams_ecc_err(struct gk20a *g,
		struct nvgpu_gr_sm_ecc_status *ecc_status, u32 gpc, u32 tpc)
{
	u32 i;

	/* This check has been added to ensure that the TPC id is less than
	 * 8-bits and hence, it can be packed as part of LSB 8-bits along with
	 * the GPC id while reporting SM related ECC errors.
	 */
	tpc = tpc & U8_MAX;

	for (i = 0U; i < ecc_status->err_count; i++) {
		if (ecc_status->err_id[i] == GPU_SM_RAMS_DECC_ECC_UNCORRECTED) {
			nvgpu_err(g, "sm_rams_decc_ecc_uncorrected "
					"gpc_id(%d), tpc_id(%d)", gpc, tpc);
		}
	}
}

static void gb10b_gr_intr_handle_tpc_sm_rams_ecc_exception(struct gk20a *g,
		u32 gpc, u32 tpc)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_in_gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	u32 offset;
	u32 rams_ecc_status;
	u32 rams_uncorrected_err_count_delta = 0U;
	bool is_rams_ecc_uncorrected_total_err_overflow = false;
	struct nvgpu_gr_sm_ecc_status ecc_status;

	offset = nvgpu_safe_add_u32(
			nvgpu_safe_mult_u32(gpc_stride, gpc),
			nvgpu_safe_mult_u32(tpc_in_gpc_stride, tpc));


	/* Check for SM RAMS ECC errors. */
	rams_ecc_status = nvgpu_readl(g, nvgpu_safe_add_u32(
		gr_pri_gpc0_tpc0_sm_rams_ecc_status_r(), offset));

	(void) memset(&ecc_status, 0, sizeof(struct nvgpu_gr_sm_ecc_status));

	if (g->ops.gr.intr.sm_ecc_status_errors(g, rams_ecc_status,
				SM_RAMS_ECC_ERROR, &ecc_status) == false) {
		return;
	}

	rams_uncorrected_err_count_delta =
		gr_pri_gpc0_tpc0_sm_rams_ecc_uncorrected_err_count_total_v(
			nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_tpc0_sm_rams_ecc_uncorrected_err_count_r(),
				offset)));
	is_rams_ecc_uncorrected_total_err_overflow =
		gr_pri_gpc0_tpc0_sm_rams_ecc_status_uncorrected_err_total_counter_overflow_v(rams_ecc_status) != 0U;

	if ((rams_uncorrected_err_count_delta > 0U) || is_rams_ecc_uncorrected_total_err_overflow) {
		nvgpu_log(g, gpu_dbg_fn | gpu_dbg_intr,
			"Uncorrected error (DBE) detected in SM RAMS! err_mask [%08x] is_overf [%d]",
			ecc_status.uncorrected_err_status, is_rams_ecc_uncorrected_total_err_overflow);

		/* HW uses 16-bits counter */
		if (is_rams_ecc_uncorrected_total_err_overflow) {
			rams_uncorrected_err_count_delta =
			   nvgpu_wrapping_add_u32(
				rams_uncorrected_err_count_delta,
				BIT32(gr_pri_gpc0_tpc0_sm_rams_ecc_uncorrected_err_count_total_s()));
		}
		g->ecc.gr.sm_rams_ecc_uncorrected_err_count[gpc][tpc].counter =
		  nvgpu_wrapping_add_u32(
			g->ecc.gr.sm_rams_ecc_uncorrected_err_count[gpc][tpc].counter,
			rams_uncorrected_err_count_delta);
		nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_rams_ecc_uncorrected_err_count_r(), offset),
			0U);
	}
	gb10b_gr_intr_report_rams_ecc_err(g, &ecc_status, gpc, tpc);
	ga10b_gr_intr_report_tpc_sm_rams_ecc_err(g, &ecc_status, gpc, tpc);
	nvgpu_writel(g, nvgpu_safe_add_u32(
			gr_pri_gpc0_tpc0_sm_rams_ecc_status_r(), offset),
			gr_pri_gpc0_tpc0_sm_rams_ecc_status_reset_task_f());
}

void gb10b_gr_intr_handle_tpc_sm_ecc_exception(struct gk20a *g,
					u32 gpc, u32 tpc)
{
	/* Check for L1 tag ECC errors. */
	gb10b_gr_intr_handle_l1_tag_exception(g, gpc, tpc);

	/* Check for LRF ECC errors. */
	gb10b_gr_intr_handle_lrf_exception(g, gpc, tpc);

	/* Check for CBU ECC errors. */
	gb10b_gr_intr_handle_cbu_exception(g, gpc, tpc);

	/* Check for L1 data ECC errors. */
	gb10b_gr_intr_handle_l1_data_exception(g, gpc, tpc);

	/* Check for L0 && L1 icache ECC errors. */
	gb10b_gr_intr_handle_icache_exception(g, gpc, tpc);

	/* Check for RAMS ECC errors. */
	gb10b_gr_intr_handle_tpc_sm_rams_ecc_exception(g, gpc, tpc);
}

static int gb10b_gr_intr_read_sm_error_state(struct gk20a *g, u32 gpc, u32 tpc,
		struct nvgpu_gr_isr_data *isr_data, u32 offset, u32 sm_id)
{
	u32 instance_id;
	int err = 0;
	u32 current_gr_instance_id = nvgpu_gr_get_cur_instance_id(g);

	u32 hww_global_esr = nvgpu_readl(g, nvgpu_safe_add_u32(
		gr_gpc0_tpc0_sm0_hww_global_esr_r(), offset));

	u32 hww_warp_esr = nvgpu_readl(g, nvgpu_safe_add_u32(
		gr_gpc0_tpc0_sm0_hww_warp_esr_r(), offset));

	u32 addr_hi = nvgpu_readl(g, nvgpu_safe_add_u32(
			gr_gpc0_tpc0_sm0_hww_warp_esr_pc_hi_r(), offset));
	u32 addr_lo = nvgpu_readl(g, nvgpu_safe_add_u32(
			gr_gpc0_tpc0_sm0_hww_warp_esr_pc_r(), offset));

	u64 hww_warp_esr_pc = hi32_lo32_to_u64(addr_hi, addr_lo);

	u32 hww_global_esr_report_mask = nvgpu_readl(g,
		nvgpu_safe_add_u32(
			gr_gpc0_tpc0_sm0_hww_global_esr_report_mask_r(),
			offset));

	u32 hww_warp_esr_report_mask = nvgpu_readl(g,
		nvgpu_safe_add_u32(
			gr_gpc0_tpc0_sm0_hww_warp_esr_report_mask_r(),
			offset));

	u32 hww_cga_esr = nvgpu_readl(g,
		nvgpu_safe_add_u32(
			gr_gpc0_tpc0_sm0_hww_cga_esr_r(),
			offset));

	instance_id = nvgpu_grmgr_get_gpu_instance_id(g, current_gr_instance_id);
	err =  nvgpu_tsg_store_sm_error_state(g, isr_data, sm_id,
		hww_global_esr, hww_warp_esr,
		hww_warp_esr_pc, hww_global_esr_report_mask,
		hww_warp_esr_report_mask, hww_cga_esr, instance_id);
	if (err == 0) {
		nvgpu_err(g, "sm err state gpc_id(%d), tpc_id(%d), "
				"offset(%d), sm_id(%d), hww_global_esr (0x%x), "
				"hww_warp_esr (0x%x), hww_warp_esr_pc (0x%llx), "
				"hww_cga_esr (0x%x)",
				gpc, tpc, offset, sm_id,
				hww_global_esr,
				hww_warp_esr,
				hww_warp_esr_pc, hww_cga_esr);
	}

	return err;
}

u32 gb10b_gr_intr_record_sm_error_state(struct gk20a *g, u32 gpc, u32 tpc, u32 sm,
				struct nvgpu_gr_isr_data *isr_data)
{
	u32 sm_id;
	u32 offset, sm_per_tpc, tpc_id;
	u32 gpc_offset, gpc_tpc_offset;
	int err = 0;

#ifdef CONFIG_NVGPU_DEBUGGER
	nvgpu_mutex_acquire(&g->dbg_sessions_lock);
#endif

	sm_per_tpc = nvgpu_get_litter_value(g, GPU_LIT_NUM_SM_PER_TPC);
	gpc_offset = nvgpu_gr_gpc_offset(g, gpc);
	gpc_tpc_offset = nvgpu_safe_add_u32(gpc_offset,
				nvgpu_gr_tpc_offset(g, tpc));

	tpc_id = nvgpu_readl(g, nvgpu_safe_add_u32(
			gr_gpc0_gpm_pd_sm_id_r(tpc), gpc_offset));
	sm_id = nvgpu_safe_add_u32(
			nvgpu_safe_mult_u32(tpc_id, sm_per_tpc),
			sm);

	offset = nvgpu_safe_add_u32(gpc_tpc_offset,
			nvgpu_gr_sm_offset(g, sm));

	err = gb10b_gr_intr_read_sm_error_state(g, gpc, tpc, isr_data, offset, sm_id);
	if (err != 0) {
		nvgpu_err(g, "error populating tsg sm_error_state");
	}

#ifdef CONFIG_NVGPU_DEBUGGER
	nvgpu_mutex_release(&g->dbg_sessions_lock);
#endif

	return sm_id;
}

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gb10b_gr_intr_set_shader_exceptions(struct gk20a *g, u32 data)
{
	nvgpu_log_fn(g, " ");

	if (data == gr_graphics_method_set_shader_exceptions_enable_false_v()) {
		nvgpu_writel(g, gr_gpcs_tpcs_sms_hww_warp_esr_report_mask_r(),
				 0);
		nvgpu_writel(g, gr_gpcs_tpcs_sms_hww_global_esr_report_mask_r(),
				 0);
	} else {
		g->ops.gr.intr.set_hww_esr_report_mask(g);
	}
}
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
void gb10b_gr_intr_tpc_exception_sm_disable(struct gk20a *g, u32 offset)
{
	u32 tpc_exception_en = nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_tpc0_tpccs_tpc_exception_en_r(),
				offset));

	tpc_exception_en &=
			~gr_gpc0_tpc0_tpccs_tpc_exception_en_sm_enabled_f();
	nvgpu_writel(g, nvgpu_safe_add_u32(
		     gr_gpc0_tpc0_tpccs_tpc_exception_en_r(), offset),
		     tpc_exception_en);
}
#endif

static void gb10b_gr_intr_set_l1_tag_uncorrected_err(struct gk20a *g,
	u32 l1_tag_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status)
{
	(void)g;

	if ((l1_tag_ecc_status &
	    gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_uncorrected_err_el1_0_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SM,
				GPU_SM_L1_TAG_ECC_UNCORRECTED);
		nvgpu_err(g, "sm_l1_tag_ecc_uncorrected");
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_L1_TAG_ECC_UNCORRECTED;
		ecc_status->err_count =
				nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}

	if ((l1_tag_ecc_status &
	     gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_uncorrected_err_miss_fifo_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SM,
				GPU_SM_L1_TAG_MISS_FIFO_ECC_UNCORRECTED);
		nvgpu_err(g, "sm_l1_tag_miss_fifo_ecc_uncorrected");
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_L1_TAG_MISS_FIFO_ECC_UNCORRECTED;
		ecc_status->err_count =
				nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}

	if ((l1_tag_ecc_status &
	     gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_uncorrected_err_pixrpf_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SM,
				GPU_SM_L1_TAG_S2R_PIXPRF_ECC_UNCORRECTED);
		nvgpu_err(g, "sm_l1_tag_s2r_pixprf_ecc_uncorrected");
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_L1_TAG_S2R_PIXPRF_ECC_UNCORRECTED;
		ecc_status->err_count =
				nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}

	if ((l1_tag_ecc_status &
		gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_uncorrected_err_pq_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SM,
				GPU_SM_L1_TAG_PQ_ECC_UNCORRECTED);
		nvgpu_err(g, "sm_l1_tag_pq_ecc_uncorrected error");
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_L1_TAG_PQ_ECC_UNCORRECTED;
		ecc_status->err_count =
				nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}
}

static void gb10b_gr_intr_set_l1_tag_corrected_err(struct gk20a *g,
	u32 l1_tag_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status)
{
	(void)g;

	if ((l1_tag_ecc_status &
	    gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_corrected_err_el1_0_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SM,
				GPU_SM_L1_TAG_ECC_CORRECTED);
		nvgpu_err(g, "sm_l1_tag_ecc_corrected");
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_L1_TAG_ECC_CORRECTED;
		ecc_status->err_count =
				nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}
}

bool gb10b_gr_intr_sm_l1_tag_ecc_status_errors(struct gk20a *g,
	u32 l1_tag_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status)
{
	u32 corr_err, uncorr_err;

	corr_err =  l1_tag_ecc_status &
		gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_corrected_err_el1_0_m();

	uncorr_err = l1_tag_ecc_status &
		(gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_uncorrected_err_el1_0_m() |
		 gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_uncorrected_err_pixrpf_m() |
		 gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_uncorrected_err_miss_fifo_m() |
		 gr_pri_gpc0_tpc0_sm_l1_tag_ecc_status_uncorrected_err_pq_m());

	if ((corr_err == 0U) && (uncorr_err == 0U)) {
		return false;
	}

	ecc_status->err_count = 0U;
	ecc_status->corrected_err_status = corr_err;
	ecc_status->uncorrected_err_status = uncorr_err;

	gb10b_gr_intr_set_l1_tag_corrected_err(g, l1_tag_ecc_status, ecc_status);
	gb10b_gr_intr_set_l1_tag_uncorrected_err(g, l1_tag_ecc_status, ecc_status);

	return true;
}

static void gb10b_gr_intr_set_rams_uncorrected_err(struct gk20a *g,
	u32 rams_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status)
{
	(void)g;

	if ((rams_ecc_status &
	    gr_pri_gpc0_tpc0_sm_rams_ecc_status_uncorrected_err_l0ic_data_m()) != 0U) {
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_ICACHE_L0_DATA_ECC_UNCORRECTED;
		ecc_status->err_count =
				nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}

	if ((rams_ecc_status &
	    gr_pri_gpc0_tpc0_sm_rams_ecc_status_uncorrected_err_l0ic_predecode_m()) != 0U) {
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_ICACHE_L0_PREDECODE_ECC_UNCORRECTED;
		ecc_status->err_count =
				nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}

	if ((rams_ecc_status &
	     gr_pri_gpc0_tpc0_sm_rams_ecc_status_uncorrected_err_urf_data_m()) != 0U) {
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_RAMS_URF_ECC_UNCORRECTED;
		ecc_status->err_count =
				nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}

	if ((rams_ecc_status &
	     gr_pri_gpc0_tpc0_sm_rams_ecc_status_uncorrected_err_decc_m()) != 0U) {
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_RAMS_DECC_ECC_UNCORRECTED;
		ecc_status->err_count =
				nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}
}

bool gb10b_gr_intr_sm_rams_ecc_status_errors(struct gk20a *g,
	u32 rams_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status)
{
	u32 uncorr_err;

	(void)g;

	uncorr_err = rams_ecc_status &\
		(gr_pri_gpc0_tpc0_sm_rams_ecc_status_uncorrected_err_l0ic_data_m() |\
		 gr_pri_gpc0_tpc0_sm_rams_ecc_status_uncorrected_err_l0ic_predecode_m() |\
		 gr_pri_gpc0_tpc0_sm_rams_ecc_status_uncorrected_err_urf_data_m() |\
		 gr_pri_gpc0_tpc0_sm_rams_ecc_status_uncorrected_err_decc_m());

	if (uncorr_err == 0U) {
		return false;
	}

	ecc_status->err_count = 0U;

	ecc_status->corrected_err_status = 0U;
	ecc_status->uncorrected_err_status = uncorr_err;

	gb10b_gr_intr_set_rams_uncorrected_err(g, rams_ecc_status, ecc_status);

	return true;
}

bool gb10b_gr_intr_sm_l1_data_ecc_status_errors(struct gk20a *g,
	u32 l1_data_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status)
{
	u32 corr_err, uncorr_err;

	(void)g;

	corr_err = l1_data_ecc_status &
		gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_corrected_err_el1_0_m();
	uncorr_err = l1_data_ecc_status &
			(gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_uncorrected_err_el1_0_m() |
				gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_uncorrected_err_tbm_m());

	if ((corr_err == 0U) && (uncorr_err == 0U)) {
		return false;
	}

	ecc_status->err_count = 0U;

	if ((l1_data_ecc_status &
		gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_uncorrected_err_el1_0_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SM,
					GPU_SM_L1_DATA_ECC_UNCORRECTED);
		nvgpu_err(g, "sm_l1_tag_el1_0_ecc_uncorrected");
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_L1_DATA_ECC_UNCORRECTED;
		ecc_status->err_count =
			nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}

	if ((l1_data_ecc_status &
		gr_pri_gpc0_tpc0_sm_l1_data_ecc_status_uncorrected_err_tbm_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SM,
					GPU_SM_L1_DATA_TBM_ECC_UNCORRECTED);
		nvgpu_err(g, "sm_l1_tag_tbm_ecc_uncorrected");
		ecc_status->err_id[ecc_status->err_count] =
				GPU_SM_L1_DATA_TBM_ECC_UNCORRECTED;
		ecc_status->err_count =
			nvgpu_wrapping_add_u32(ecc_status->err_count, 1U);
	}

	ecc_status->corrected_err_status = corr_err;
	ecc_status->uncorrected_err_status = uncorr_err;

	return true;
}

void gb10b_gr_intr_enable_gpc_crop_hww(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");
	nvgpu_writel(g, gr_gpcs_rops_crop_hww_esr_r(),
			gr_gpcs_rops_crop_hww_esr_reset_active_f() |
			gr_gpcs_rops_crop_hww_esr_en_enable_f());
}

void gb10b_gr_intr_handle_gcc_exception(struct gk20a *g, u32 gpc,
				u32 gpc_exception,
				u32 *corrected_err, u32 *uncorrected_err)
{
	u32 offset = nvgpu_gr_gpc_offset(g, gpc);
	u32 gcc_hww_esr = 0U;

	if (gr_gpc0_gpccs_gpc_exception_gcc_v(gpc_exception) == 0U) {
		return;
	}
	/*
	 * Check for poison bit sent from XBAR in case
	 * parity is detected in data packet sent from L2.
	 */
	if ((nvgpu_safe_add_u32((gr_gpc0_gcc_hww_esr_r()), offset) &
					gr_gpc0_gcc_hww_esr_poison_m()) != 0U) {
		nvgpu_err(g, "GCC: Poisoned bit received from L2, "
					"gcc_hww_esr: 0x%08x", gcc_hww_esr);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GCC,
				GPU_GCC_POISON_ERROR);
		if (g->ops.xbar.handle_xbar_read_ecc_err != NULL) {
			g->ops.xbar.handle_xbar_read_ecc_err(g);
		}
	}
	gv11b_gr_intr_handle_gcc_exception(g, gpc, gpc_exception,
						corrected_err, uncorrected_err);
	/* Clear the interrupt */
	nvgpu_writel(g, gr_gpc0_gcc_hww_esr_r(),
			gr_gpc0_gcc_hww_esr_reset_active_f());

}

void gb10b_gr_intr_enable_gpc_zrop_hww(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");
	nvgpu_writel(g, gr_gpcs_rops_zrop_hww_esr_r(),
			gr_gpcs_rops_zrop_hww_esr_reset_active_f() |
			gr_gpcs_rops_zrop_hww_esr_en_enable_f());
}

static u32 gb10b_gr_intr_check_gr_rastwod_exception(struct gk20a *g,
				u32 exception)
{
	if ((exception & gr_exception_rastwod_m()) != 0U) {
		u32 rastwod_hww_esr = nvgpu_readl(g, gr_rstr2d_hww_esr_r());
		u32 info = nvgpu_readl(g, gr_rstr2d_hww_esr_info_r());

		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_PGRAPH,
				GPU_PGRAPH_RASTWOD_EXCEPTION);
		nvgpu_err(g, "rastwod exception: esr: 0x%08x, info: %08x",
				rastwod_hww_esr, info);

		/* Clear the interrupt. */
		nvgpu_writel(g, gr_rstr2d_hww_esr_r(),
				gr_rstr2d_hww_esr_reset_active_f());

		/* TODO: Return 1 to trigger recovery in case of
		 * rastwod exception. Currently returning 0 to keep
		 * recoverable errors at parity for Orin and Thor.
		 */
		return 0U;
	}
	return 0U;
}

bool gb10b_gr_intr_handle_exceptions(struct gk20a *g, bool *is_gpc_exception)
{
	u32 gpc_reset = 0U;
	u32 exception = nvgpu_readl(g, gr_exception_r());

	nvgpu_log(g, gpu_dbg_intr | gpu_dbg_gpu_dbg,
				"exception 0x%08x", exception);

	gpc_reset |= gb10b_gr_intr_check_gr_rastwod_exception(g, exception);
	gpc_reset |= ga10b_gr_intr_handle_exceptions(g, is_gpc_exception);

	return (gpc_reset != 0U) ? true : false;
}

void gb10b_gr_intr_enable_hww_exceptions(struct gk20a *g)
{
	/* Enable rastwod exception. */
	nvgpu_writel(g, gr_rstr2d_hww_esr_r(),
		gr_rstr2d_hww_esr_en_enable_f() |
		gr_rstr2d_hww_esr_reset_active_f());

	gv11b_gr_intr_enable_hww_exceptions(g);
}

void gb10b_gr_intr_enable_exceptions(struct gk20a *g,
				struct nvgpu_gr_config *gr_config, bool enable)
{
	u32 reg_val = 0U;

	if (!enable) {
		nvgpu_writel(g, gr_exception_en_r(), reg_val);
		nvgpu_writel(g, gr_exception1_en_r(), reg_val);
		return;
	}

	/*
	 * clear exceptions :
	 * other than SM : hww_esr are reset in *enable_hww_excetpions*
	 * SM            : cleared in *set_hww_esr_report_mask*
	 */

	/* enable exceptions */
	reg_val = BIT32(nvgpu_gr_config_get_gpc_count(gr_config));
	nvgpu_writel(g, gr_exception1_en_r(),
				nvgpu_safe_sub_u32(reg_val, 1U));

	reg_val = gr_exception_en_fe_enabled_f() |
		  gr_exception_en_memfmt_enabled_f() |
		  gr_exception_en_pd_enabled_f() |
		  gr_exception_en_scc_enabled_f() |
		  gr_exception_en_ds_enabled_f() |
		  gr_exception_en_ssync_enabled_f() |
		  gr_exception_en_rastwod_enabled_f() |
		  gr_exception_en_mme_enabled_f() |
		  gr_exception_en_sked_enabled_f() |
		  gr_exception_en_mme_fe1_enabled_f() |
		  gr_exception_en_gpc_enabled_f();

	nvgpu_log(g, gpu_dbg_info, "gr_exception_en 0x%08x", reg_val);

	nvgpu_writel(g, gr_exception_en_r(), reg_val);
}

static int gb10b_intr_gr_stall_isr(struct gk20a *g)
{
	int err = 0;

#ifdef CONFIG_NVGPU_POWER_PG
	/* Disable ELPG before handling stall isr */
	if (!g->in_isr) {
		err = nvgpu_pg_elpg_disable(g);
		if (err != 0) {
			nvgpu_err(g, "ELPG disable failed."
				"Going ahead with stall_isr handling");
		}
	}
#endif
	/* handle stall isr */
	err = g->ops.gr.intr.stall_isr(g);
	if (err != 0) {
		nvgpu_err(g, "GR intr stall_isr failed");
		return err;
	}

	err = g->ops.gr.intr.retrigger(g);
	if (err != 0) {
		nvgpu_err(g, "GR intr retrigger failed");
		return err;
	}

#ifdef CONFIG_NVGPU_POWER_PG
	/* enable elpg again */
	if (!g->in_isr) {
		err = nvgpu_pg_elpg_enable(g);
		if (err != 0) {
			nvgpu_err(g, "ELPG enable failed.");
		}
	}
#endif
	return err;
}

static void gb10b_gr_intr_stall_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);
	u32 gr_instance_id =
		nvgpu_grmgr_get_gr_instance_id_for_syspipe(
				g, inst_id);

	(void) nvgpu_gr_exec_with_err_for_instance(g,
			gr_instance_id, gb10b_intr_gr_stall_isr(g));
}

static u32 gb10b_gr_intr_nonstall_handler(struct gk20a *g, u64 cookie)
{
	(void)g;
	(void)cookie;
	return (NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE |
			NVGPU_CIC_NONSTALL_OPS_POST_EVENTS);
}

void gb10b_gr_intr_enable_interrupts(struct gk20a *g, bool enable)
{
	u32 stall_vector = 0U;
	u32 nonstall_vector = 0U;
	u32 intr_ctrl = 0U;
	u32 intr_notify_ctrl = 0U;
	const struct nvgpu_device *dev =
		nvgpu_device_get(g, NVGPU_DEVTYPE_GRAPHICS,
				nvgpu_gr_get_syspipe_id(g, g->mig.cur_gr_instance));

	nvgpu_assert(dev != NULL);

	stall_vector = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_GR, dev->inst_id);
	nonstall_vector = nvgpu_gin_get_unit_nonstall_vector(g,
			NVGPU_GIN_INTR_UNIT_GR, dev->inst_id);

	if (enable) {
		nvgpu_log(g, gpu_dbg_intr, "gr intr stall vector(%d), nonstall vector(%d)",
				stall_vector, nonstall_vector);
		/* Mask intr */
		nvgpu_writel(g, gr_intr_en_r(), 0U);
		/* Clear interrupt */
		nvgpu_writel(g, gr_intr_r(), U32_MAX);
		/* Set stall and nonstall interrupt handlers */
		nvgpu_gin_set_stall_handler(g, stall_vector, &gb10b_gr_intr_stall_handler,
				dev->inst_id);
		nvgpu_gin_set_nonstall_handler(g, nonstall_vector, &gb10b_gr_intr_nonstall_handler,
				dev->inst_id);
		/* Enable stalling interrupt to cpu, disable stalling interrupt to gsp */
		intr_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, stall_vector, true, false);
		/* Enable notifying interrupt to cpu, disable notifying interrupt to gsp */
		intr_notify_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, nonstall_vector, true, false);
		nvgpu_writel(g, gr_intr_ctrl_r(), intr_ctrl);
		nvgpu_writel(g, gr_intr_notify_ctrl_r(), intr_notify_ctrl);
		/* Enable gr interrupts */
		nvgpu_writel(g, gr_intr_en_r(), g->ops.gr.intr.enable_mask(g));
	} else {
		/* Mask intr */
		nvgpu_writel(g, gr_intr_en_r(), 0U);
		/* Disable stalling interrupt to cpu, disable stalling interrupt to gsp */
		intr_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, stall_vector, false, false);
		/* Disable notifying interrupt to cpu, disable notifying interrupt to gsp */
		intr_notify_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, nonstall_vector, false, false);
		nvgpu_writel(g, gr_intr_ctrl_r(), intr_ctrl);
		nvgpu_writel(g, gr_intr_notify_ctrl_r(), intr_notify_ctrl);
		/* Clear intr */
		nvgpu_writel(g, gr_intr_r(), U32_MAX);
	}
}

