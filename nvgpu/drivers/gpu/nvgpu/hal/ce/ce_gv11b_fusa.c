// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/log.h>
#include <nvgpu/bitops.h>
#include <nvgpu/device.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/nvgpu_init.h>

#include "ce_gp10b.h"
#include "ce_gv11b.h"

#include <nvgpu/hw/gv11b/hw_ce_gv11b.h>

u32 gv11b_ce_get_num_pce(struct gk20a *g)
{
	/*
	 * register contains a bitmask indicating which physical copy
	 * engines are present (and not floorswept).
	 */
	u32 num_pce;
	u32 ce_pce_map = nvgpu_readl(g, ce_pce_map_r());

	num_pce = U32(hweight32(ce_pce_map));
	nvgpu_log_info(g, "num PCE: %d", num_pce);
	return num_pce;
}

void gv11b_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
				bool *needs_rc, bool *needs_quiesce)
{
	u32 ce_intr = nvgpu_readl(g, ce_intr_status_r(inst_id));
	u32 clear_intr = 0U;
	u32 reg_val;
	u32 err_code;

	nvgpu_log(g, gpu_dbg_intr, "ce isr 0x%08x 0x%08x", ce_intr, inst_id);

	if ((ce_intr & ce_intr_status_launcherr_pending_f()) != 0U) {
		nvgpu_err(g, "ce launch error interrupt");
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_CE,
				GPU_CE_LAUNCH_ERROR);

		/* INVALID_CONFIG and METHOD_BUFFER_FAULT may still be
		 * reported via LAUNCHERR bit, but using different
		 * error code. Check the LAUNCHERR errorcode to
		 * check if above two interrupts are routed to
		 * LAUNCHERR bit and handle as per error handling
		 * policy.
		 */
		reg_val = nvgpu_readl(g, ce_lce_launcherr_r(inst_id));
		err_code = ce_lce_launcherr_report_v(reg_val);
		nvgpu_err(g, "ce launch error interrupt with errcode:0x%x", err_code);
		if ((err_code == ce_lce_launcherr_report_method_buffer_access_fault_v()) ||
				(err_code == ce_lce_launcherr_report_invalid_config_v())) {
			*needs_quiesce = *needs_quiesce || true;
		} else {
			*needs_rc = *needs_rc || true;
		}
		clear_intr |= ce_intr_status_launcherr_pending_f();
	}

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	/*
	 * An INVALID_CONFIG interrupt will be generated if a floorswept
	 * PCE is assigned to a valid LCE in the NV_CE_PCE2LCE_CONFIG
	 * registers. This is a fatal error and the LCE will have to be
	 * reset to get back to a working state.
	 */
	if ((ce_intr & ce_intr_status_invalid_config_pending_f()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_CE,
				GPU_CE_INVALID_CONFIG);
		nvgpu_err(g, "ce: inst %d: invalid config", inst_id);
		*needs_quiesce = true;
		clear_intr |= ce_intr_status_invalid_config_reset_f();
	}

	/*
	 * A MTHD_BUFFER_FAULT interrupt will be triggered if any access
	 * to a method buffer during context load or save encounters a fault.
	 * This is a fatal interrupt and will require at least the LCE to be
	 * reset before operations can start again, if not the entire GPU.
	 */
	if ((ce_intr & ce_intr_status_mthd_buffer_fault_pending_f()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_CE,
				GPU_CE_METHOD_BUFFER_FAULT);
		nvgpu_err(g, "ce: inst %d: mthd buffer fault", inst_id);
		*needs_quiesce = true;
		clear_intr |= ce_intr_status_mthd_buffer_fault_reset_f();
	}
#endif

	nvgpu_writel(g, ce_intr_status_r(inst_id), clear_intr);

	gp10b_ce_stall_isr(g, inst_id, pri_base, needs_rc, needs_quiesce);
}

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gv11b_ce_mthd_buffer_fault_in_bar2_fault(struct gk20a *g)
{
	u32 reg_val, num_lce, lce, clear_intr;

	num_lce = g->ops.top.get_num_lce(g);

	for (lce = 0U; lce < num_lce; lce++) {
		reg_val = nvgpu_readl(g, ce_intr_status_r(lce));
		if ((reg_val &
			ce_intr_status_mthd_buffer_fault_pending_f()) != 0U) {
			nvgpu_err(g, "ce: lce %d: mthd buffer fault", lce);
			nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_CE,
				GPU_CE_METHOD_BUFFER_FAULT);
			/* This is a fatal interrupt as it implies a kernel bug.
			 * Needs quiesce.
			 */
			nvgpu_sw_quiesce(g);
			clear_intr = ce_intr_status_mthd_buffer_fault_reset_f();
			nvgpu_writel(g, ce_intr_status_r(lce), clear_intr);
		}
	}
}
#endif

void gv11b_ce_init_prod_values(struct gk20a *g)
{
	u32 reg_val;
	u32 num_lce, lce;

	num_lce = g->ops.top.get_num_lce(g);

	for (lce = 0U; lce < num_lce; lce++) {
		reg_val = nvgpu_readl(g, ce_lce_opt_r(lce));
		reg_val |= ce_lce_opt_force_barriers_npl__prod_f();
		nvgpu_writel(g, ce_lce_opt_r(lce), reg_val);
	}
}

void gv11b_ce_halt_engine(struct gk20a *g, const struct nvgpu_device *dev)
{
	u32 reg_val;
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	reg_val = nvgpu_readl(g, ce_lce_engctl_r(dev->inst_id));
	reg_val |= ce_lce_engctl_stallreq_true_f();
	nvgpu_writel(g, ce_lce_engctl_r(dev->inst_id), reg_val);

	if (g->ops.fifo.wait_for_preempt_before_reset != NULL) {
		g->ops.fifo.wait_for_preempt_before_reset(g, dev->runlist_id);
	}

	do {
		reg_val = nvgpu_readl(g, ce_lce_engctl_r(dev->inst_id));

		if ((reg_val & ce_lce_engctl_stallack_true_f()) != 0U) {
			return;
		}

		nvgpu_usleep_range(delay, nvgpu_safe_mult_u32(delay, 2U));
		delay = min_t(u32, delay << 1U, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	nvgpu_err(g, "Timed out waiting for CE%u engine idle", dev->inst_id);
}

u64 gv11b_ce_get_inst_ptr_from_lce(struct gk20a *g, u32 inst_id)
{
	u32 reg_val;

	reg_val = nvgpu_readl(g, ce_lce_bind_status_r(inst_id));
	if (ce_lce_bind_status_bound_v(reg_val) ==
			ce_lce_bind_status_bound_false_v()) {
		/* CE appears to have never been bound -- ignore */
		return 0U;
	}

	return ce_lce_bind_status_ctx_ptr_v((u64)reg_val) <<
			g->ops.ramin.base_shift();
}
