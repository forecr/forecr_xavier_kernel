// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/io.h>
#include <nvgpu/log.h>
#include <nvgpu/device.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/gin.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/power_features/pg.h>
#include <nvgpu/ce.h>

#include "hal/ce/ce_gb10b.h"
#include "hal/ce/ce_ga10b.h"

#include <nvgpu/hw/gb10b/hw_ce_gb10b.h>

static u32 gb10b_ce_lce_intr_mask(void)
{
	u32 mask =
#ifdef CONFIG_NVGPU_NONSTALL_INTR
		ce_lce_intr_en_nonblockpipe_m() |
#endif
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
		ce_lce_intr_en_stalling_debug_m() |
		ce_lce_intr_en_blockpipe_m() |
		ce_lce_intr_en_invalid_config_m() |
		ce_lce_intr_en_mthd_buffer_fault_m() |
		ce_lce_intr_en_fbuf_crc_fail_m() |
		ce_lce_intr_en_fbuf_magic_chk_fail_m() |
		ce_lce_intr_en_ctx_load_after_halt_m() |
		ce_lce_intr_en_cc_no_resources_m() |
#endif
		ce_lce_intr_en_launcherr_m();
	return mask;
}

static void gb10b_ce_intr_stall_handler(struct gk20a *g, u64 cookie)
{
	int err = 0;
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);
	const struct nvgpu_device *dev = nvgpu_device_get(g,
			NVGPU_DEVTYPE_LCE, inst_id);

#ifdef CONFIG_NVGPU_POWER_PG
	/* disable elpg before accessing CE registers */
	err = nvgpu_pg_elpg_disable(g);
	if (err != 0) {
		nvgpu_err(g, "ELPG disable failed");
		/* enable ELPG again so that PG SM is in known state */
		(void)nvgpu_pg_elpg_enable(g);
	}
#endif

	nvgpu_ce_stall_isr(g, inst_id, dev->pri_base);
	g->ops.ce.intr_retrigger(g, dev->inst_id);

#ifdef CONFIG_NVGPU_POWER_PG
	/* enable elpg again */
	(void)nvgpu_pg_elpg_enable(g);
#endif
}

static u32 gb10b_ce_intr_get_stall_vector(struct gk20a *g,
		 const struct nvgpu_device *dev)
{
	return nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_CE, dev->inst_id);
}

static u32 gb10b_ce_intr_get_nonstall_vector(struct gk20a *g,
		const struct nvgpu_device *dev)
{
	return nvgpu_gin_get_unit_nonstall_vector(g,
			NVGPU_GIN_INTR_UNIT_CE, dev->inst_id);
}

static u32 gb10b_ce_intr_nonstall_handler(struct gk20a *g, u64 cookie)
{
	(void)g;
	(void)cookie;
	return (NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE |
			NVGPU_CIC_NONSTALL_OPS_POST_EVENTS);
}

static void gb10b_ce_intr_stall_nonstall_enable(struct gk20a *g,
		const struct nvgpu_device *dev, bool enable)
{
	u32 intr_en_mask = 0U;
	u32 intr_ctrl = 0U;
	u32 intr_notify_ctrl = 0U;
	u32 inst_id = dev->inst_id;
	u32 stall_vector = 0U;
	u32 nonstall_vector = 0U;

	stall_vector = gb10b_ce_intr_get_stall_vector(g, dev);
	nonstall_vector = gb10b_ce_intr_get_nonstall_vector(g, dev);

	if (enable) {
		bool nonstall_cpu_enable = NVGPU_GIN_CPU_ENABLE;

#ifndef CONFIG_NVGPU_NONSTALL_INTR
		nonstall_cpu_enable = NVGPU_GIN_CPU_DISABLE;
#endif

		/*
		 * Enable reporting all stall and nonstall (if enabled)
		 * interrupts to CPU. Disable reporting all interrupts to GSP.
		 */
		nvgpu_gin_set_stall_handler(g, stall_vector,
				&gb10b_ce_intr_stall_handler, inst_id);
		nvgpu_gin_set_nonstall_handler(g, nonstall_vector,
				&gb10b_ce_intr_nonstall_handler, inst_id);
		intr_en_mask = gb10b_ce_lce_intr_mask();
		intr_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, stall_vector,
				NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
		intr_notify_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, nonstall_vector,
				nonstall_cpu_enable, NVGPU_GIN_GSP_DISABLE);
	} else {
		/*
		 * Mask all interrupts from the engine and disable
		 * reporting to both CPU, GSP.
		 */
		intr_en_mask = 0U;
		intr_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, stall_vector,
				NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
		intr_notify_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, nonstall_vector,
				NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
	}

	nvgpu_log(g, gpu_dbg_intr, "ce(%d) intr_ctrl(0x%x) "\
			"intr_notify_ctrl(0x%x) intr_en_mask(0x%x)",
			inst_id, intr_ctrl, intr_notify_ctrl,
			intr_en_mask);

	nvgpu_writel(g, ce_lce_intr_ctrl_r(inst_id), intr_ctrl);
	nvgpu_writel(g, ce_lce_intr_notify_ctrl_r(inst_id),
			intr_notify_ctrl);
	nvgpu_writel(g, ce_lce_intr_en_r(inst_id), intr_en_mask);
}

void gb10b_ce_intr_enable(struct gk20a *g, bool enable)
{
	const struct nvgpu_device *dev;

	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_LCE) {
		gb10b_ce_intr_stall_nonstall_enable(g, dev, enable);
	}
}

void gb10b_ce_set_engine_reset(struct gk20a *g,
		const struct nvgpu_device *dev, bool assert_reset)
{
	u32 inst_id = dev->inst_id;
	u32 reg_val = 0U;
	u32 expected_status = 0U;

	if (assert_reset) {
		reg_val = ce_lce_ctrl_reset_assert_f();
		expected_status = ce_lce_ctrl_status_asserted_v();
	} else {
		reg_val = ce_lce_ctrl_reset_deassert_f();
		expected_status = ce_lce_ctrl_status_deasserted_v();
	}

	nvgpu_writel(g, ce_lce_ctrl_r(inst_id), reg_val);
	/* Read same register back to ensure hw propagation of write */
	reg_val = nvgpu_readl(g, ce_lce_ctrl_r(inst_id));

	nvgpu_udelay(10);
	reg_val = nvgpu_readl(g, ce_lce_ctrl_r(inst_id));

	if (ce_lce_ctrl_status_v(reg_val) != expected_status) {
		nvgpu_err(g, "timed out waiting for LCE%u reset %s",
				inst_id, assert_reset ? "assert" : "deassert");
	}
}

void gb10b_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
				bool *needs_rc, bool *needs_quiesce)
{
	u32 ce_intr = nvgpu_readl(g, ce_intr_status_r(inst_id));
	u32 clear_intr = 0U;

	nvgpu_log(g, gpu_dbg_intr, "ce(%u) isr 0x%08x 0x%08x", inst_id,
			ce_intr, inst_id);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	/* All other error bits in ce_intr_status are invalid for GB10B. */
	/*
	 * CTX_LOAD_AFTER_HALT: SW did not disable scheduling
	 * before telling the engine to halt.
	 */
	if ((ce_intr & ce_intr_status_ctx_load_after_halt_pending_f()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_CE,
				GPU_CE_CTX_LOAD_AFTER_HALT);
		nvgpu_err(g, "ce: inst %d, ctx load received from host while "
			"engine is being halted/reset", inst_id);
		*needs_rc = true;
		clear_intr |= ce_intr_status_ctx_load_after_halt_reset_f();
	}

	/*
	 * CC_NO_RESOURCES: a secure copy is received by CE
	 * but no AES capable PCE is available to the LCE engine.
	 */
	if ((ce_intr & ce_intr_status_cc_no_resources_pending_f()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_CE,
				GPU_CE_CC_NO_RESOURCES);
		nvgpu_err(g, "ce: inst %d, cc no resources pending!", inst_id);
		*needs_rc = true;
		clear_intr |= ce_intr_status_cc_no_resources_reset_f();
	}
#endif
	nvgpu_writel(g, ce_intr_status_r(inst_id), clear_intr);

	/*
	 * The remaining legacy interrupts are handled by legacy interrupt
	 * handler.
	 */
	ga10b_ce_stall_isr(g, inst_id, pri_base, needs_rc, needs_quiesce);
}