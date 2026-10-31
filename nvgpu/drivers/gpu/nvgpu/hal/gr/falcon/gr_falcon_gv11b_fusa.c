// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/gr/gr_falcon.h>

#include "gr_falcon_gp10b.h"
#include "gr_falcon_gm20b.h"
#include "gr_falcon_gv11b.h"
#include "common/gr/gr_falcon_priv.h"

#include <nvgpu/hw/gv11b/hw_gr_gv11b.h>

static void gr_falcon_set_fecs_ecc_error_status(u32 ecc_status,
		struct nvgpu_fecs_ecc_status *fecs_ecc_status)
{
	if ((ecc_status &
	     gr_fecs_falcon_ecc_status_corrected_err_imem_m()) != 0U) {
		fecs_ecc_status->imem_corrected_err = true;
	}
	if ((ecc_status &
	     gr_fecs_falcon_ecc_status_uncorrected_err_imem_m()) != 0U) {
		fecs_ecc_status->imem_uncorrected_err = true;
	}
	if ((ecc_status &
	     gr_fecs_falcon_ecc_status_corrected_err_dmem_m()) != 0U) {
		fecs_ecc_status->dmem_corrected_err = true;
	}
	if ((ecc_status &
	     gr_fecs_falcon_ecc_status_uncorrected_err_dmem_m()) != 0U) {
		fecs_ecc_status->dmem_uncorrected_err = true;
	}
}

void gv11b_gr_falcon_handle_fecs_ecc_error(struct gk20a *g,
			struct nvgpu_fecs_ecc_status *fecs_ecc_status)
{
	u32 ecc_status, ecc_addr, corrected_cnt, uncorrected_cnt;
	u32 corrected_delta, uncorrected_delta;
	u32 corrected_overflow, uncorrected_overflow;
	u32 gr_fecs_intr = nvgpu_readl(g, gr_fecs_host_int_status_r());

	if ((gr_fecs_intr & (gr_fecs_host_int_status_ecc_uncorrected_m() |
		    gr_fecs_host_int_status_ecc_corrected_m())) != 0U) {
		ecc_status = nvgpu_readl(g, gr_fecs_falcon_ecc_status_r());
		ecc_addr = nvgpu_readl(g, gr_fecs_falcon_ecc_address_r());
		corrected_cnt = nvgpu_readl(g,
			gr_fecs_falcon_ecc_corrected_err_count_r());
		uncorrected_cnt = nvgpu_readl(g,
			gr_fecs_falcon_ecc_uncorrected_err_count_r());
		corrected_delta =
			gr_fecs_falcon_ecc_corrected_err_count_total_v(
							corrected_cnt);
		uncorrected_delta =
			gr_fecs_falcon_ecc_uncorrected_err_count_total_v(
							uncorrected_cnt);

		corrected_overflow = ecc_status &
			gr_fecs_falcon_ecc_status_corrected_err_total_counter_overflow_m();
		uncorrected_overflow = ecc_status &
			gr_fecs_falcon_ecc_status_uncorrected_err_total_counter_overflow_m();

		/* clear the interrupt */
		if ((corrected_delta > 0U) || (corrected_overflow != 0U)) {
			nvgpu_writel(g,
				gr_fecs_falcon_ecc_corrected_err_count_r(), 0);
		}
		if ((uncorrected_delta > 0U) || (uncorrected_overflow != 0U)) {
			nvgpu_writel(g,
				gr_fecs_falcon_ecc_uncorrected_err_count_r(),
				0);
		}

		/* clear the interrupt */
		nvgpu_writel(g, gr_fecs_falcon_ecc_uncorrected_err_count_r(),
				0);
		nvgpu_writel(g, gr_fecs_falcon_ecc_corrected_err_count_r(), 0);

		/* clear the interrupt */
		nvgpu_writel(g, gr_fecs_falcon_ecc_status_r(),
				gr_fecs_falcon_ecc_status_reset_task_f());

		fecs_ecc_status->corrected_delta = corrected_delta;
		fecs_ecc_status->uncorrected_delta = uncorrected_delta;
		fecs_ecc_status->ecc_addr = ecc_addr;

		nvgpu_log(g, gpu_dbg_intr,
			"fecs ecc interrupt intr: 0x%x", gr_fecs_intr);

		gr_falcon_set_fecs_ecc_error_status(ecc_status,
							fecs_ecc_status);

		if ((corrected_overflow != 0U) || (uncorrected_overflow != 0U)) {
			nvgpu_info(g, "fecs ecc counter overflow!");
		}

		nvgpu_log(g, gpu_dbg_intr,
			"ecc error row address: 0x%x",
			gr_fecs_falcon_ecc_address_row_address_v(ecc_addr));
	}
}

int gv11b_gr_falcon_ctrl_ctxsw(struct gk20a *g, u32 fecs_method,
		u32 data, u32 *ret_val)
{
	struct nvgpu_fecs_method_op op = {
		.mailbox = { .id = 0U, .data = 0U, .ret = NULL,
			     .clr = ~U32(0U), .ok = 0U, .fail = 0U},
		.method.data = 0U,
		.cond.ok = GR_IS_UCODE_OP_NOT_EQUAL,
		.cond.fail = GR_IS_UCODE_OP_SKIP,
	};
	u32 flags = 0;
	int ret;

	nvgpu_log_info(g, "fecs method %d data 0x%x ret_val %p",
				fecs_method, data, ret_val);

	switch (fecs_method) {
	case NVGPU_GR_FALCON_METHOD_SET_WATCHDOG_TIMEOUT:
		op.method.addr =
			gr_fecs_method_push_adr_set_watchdog_timeout_f();
		op.method.data = data;
		if (nvgpu_platform_is_silicon(g)) {
			op.cond.ok = GR_IS_UCODE_OP_EQUAL;
			op.mailbox.ok = gr_fecs_ctxsw_mailbox_value_pass_v();
		} else {
			op.cond.ok = GR_IS_UCODE_OP_SKIP;
		}
		flags |= NVGPU_GR_FALCON_SUBMIT_METHOD_F_LOCKED;

		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags, fecs_method);
		break;

	default:
		ret = gp10b_gr_falcon_ctrl_ctxsw(g, fecs_method,
				data, ret_val);
		break;
	}
	return ret;
}

void gv11b_gr_falcon_fecs_host_int_enable(struct gk20a *g)
{
	/*
	 * ctxsw_intr1 (bit 1 of HOST_INT_SET_CTXSW) indicates that a CILP
	 * ctxsw save has finished. A write this bit
	 * sets the HOST_INT_STATUS register.
	 * This bit is disabled by default in HW and only enabled
	 * under non safety CILP config.
	 */

	nvgpu_writel(g, gr_fecs_host_int_enable_r(),
		     gr_fecs_host_int_enable_ctxsw_intr0_enable_f() |
#ifdef CONFIG_NVGPU_CILP
		     gr_fecs_host_int_enable_ctxsw_intr1_enable_f() |
#endif
		     gr_fecs_host_int_enable_fault_during_ctxsw_enable_f() |
		     gr_fecs_host_int_enable_umimp_firmware_method_enable_f() |
		     gr_fecs_host_int_enable_umimp_illegal_method_enable_f() |
		     gr_fecs_host_int_enable_watchdog_enable_f() |
		     gr_fecs_host_int_enable_flush_when_busy_enable_f() |
		     gr_fecs_host_int_enable_ecc_corrected_enable_f() |
		     gr_fecs_host_int_enable_ecc_uncorrected_enable_f());
}

u32 gv11b_gr_falcon_get_current_ctx_id(struct gk20a *g)
{
	return nvgpu_readl(g, gr_fecs_ctxsw_current_ctx_id_r());
}
