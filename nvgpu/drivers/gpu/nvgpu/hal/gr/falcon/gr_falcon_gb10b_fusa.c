// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/io.h>
#include <nvgpu/debug.h>
#include <nvgpu/power_features/pg.h>
#include <nvgpu/soc.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/string.h>
#include <nvgpu/gr/gr_intr.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/gr/fecs_trace.h>
#include "hal/gr/falcon/gr_falcon_gm20b.h"
#include "hal/gr/intr/gr_intr_gv11b.h"

#include "gr_falcon_gb10b.h"
#include "common/gr/gr_falcon_priv.h"
#include "common/gr/gr_intr_priv.h"
#include "hal/riscv/riscv_gb10b.h"

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>
#include <nvgpu/hw/gb10b/hw_ctrl_gb10b.h>
#include <nvgpu/hw/gb10b/hw_xbar_gb10b.h>

#define GR_FECS_POLL_INTERVAL	5U /* usec */

#define FECS_ARB_CMD_TIMEOUT_MAX_US 40U
#define FECS_ARB_CMD_TIMEOUT_DEFAULT_US 2U
#define CTXSW_MEM_SCRUBBING_TIMEOUT_MAX_US 1000U
#define CTXSW_MEM_SCRUBBING_TIMEOUT_DEFAULT_US 10U

#ifdef CONFIG_NVGPU_CTXSW_FW_ERROR_WDT_TESTING
#define CTXSW_WDT_DEFAULT_VALUE 0x1U
#else
#define CTXSW_WDT_DEFAULT_VALUE 0x3FFFFFFFU
#endif
#define CTXSW_INTR0 BIT32(0)
#define CTXSW_INTR1 BIT32(1)
#define CTXSW_INTR2 BIT32(2)

#define NVGPU_NULL_METHOD_DATA 0xDEADCA11U

u32 gb10b_gr_falcon_get_gpccs_start_reg_offset(void)
{
	return (gr_gpcs_gpccs_falcon_hwcfg_r() - gr_fecs_falcon_hwcfg_r());
}

int gb10b_gr_falcon_wait_mem_scrubbing(struct gk20a *g)
{
	struct nvgpu_timeout timeout;
	bool fecs_scrubbing;
	bool gpccs_scrubbing;

	nvgpu_log_fn(g, " ");

	nvgpu_timeout_init_retry(g, &timeout,
			   CTXSW_MEM_SCRUBBING_TIMEOUT_MAX_US /
				CTXSW_MEM_SCRUBBING_TIMEOUT_DEFAULT_US);

	do {
		fecs_scrubbing = (nvgpu_readl(g, gr_fecs_dmactl_r()) &
			(gr_fecs_dmactl_imem_scrubbing_m() |
			 gr_fecs_dmactl_dmem_scrubbing_m())) != 0U;

		gpccs_scrubbing = (nvgpu_readl(g, gr_gpccs_dmactl_r()) &
			(gr_gpccs_dmactl_imem_scrubbing_m() |
			 gr_gpccs_dmactl_imem_scrubbing_m())) != 0U;

		if (!fecs_scrubbing && !gpccs_scrubbing) {
			nvgpu_log_fn(g, "done");
			return 0;
		}

		nvgpu_udelay(CTXSW_MEM_SCRUBBING_TIMEOUT_DEFAULT_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	nvgpu_err(g, "Falcon mem scrubbing timeout");
	return -ETIMEDOUT;
}

u32 gb10b_gr_falcon_fecs_base_addr(void)
{
	return gr_fecs_irqsset_r();
}

u32 gb10b_gr_falcon_fecs2_base_addr(void)
{
	return gr_fecs_riscv_cpuctl_r() - gb10b_riscv_cpuctl_offset();
}

u32 gb10b_gr_falcon_gpccs_base_addr(void)
{
	return gr_gpcs_gpccs_irqsset_r();
}

u32 gb10b_gr_falcon_gpccs2_base_addr(void)
{
	return gr_gpcs_gpccs_riscv_cpuctl_r() - gb10b_riscv_cpuctl_offset();
}

void gb10b_gr_falcon_reset_ctxsw_mailbox_scratch(struct gk20a *g)
{
	u32 i;

	for (i = 0; i < ctrl_mailbox_scratch__size_1_v(); i++) {
		nvgpu_writel(g, ctrl_mailbox_scratch_r(i), 0x0);
	}
}

void gb10b_gr_falcon_start_gpccs(struct gk20a *g)
{
	u32 reg_offset = gr_gpcs_gpccs_falcon_hwcfg_r() -
					gr_fecs_falcon_hwcfg_r();

#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
	if (!nvgpu_is_enabled(g, NVGPU_SEC_SECUREGPCCS)) {
		nvgpu_writel(g, gr_gpccs_dmactl_r(),
			gr_gpccs_dmactl_require_ctx_f(0U));
		nvgpu_writel(g, gr_gpccs_cpuctl_r(),
			gr_gpccs_cpuctl_startcpu_f(1U));
	} else
#endif
	{
		nvgpu_writel(g, reg_offset +
			gr_fecs_cpuctl_alias_r(),
			gr_gpccs_cpuctl_startcpu_f(1U));
	}
}

void gb10b_gr_falcon_start_fecs(struct gk20a *g)
{
	g->ops.gr.falcon.fecs_ctxsw_clear_mailbox(g, 0U, ~U32(0U));
	nvgpu_writel(g, gr_fecs_ctxsw_mailbox_r(1U), 1U);
	g->ops.gr.falcon.fecs_ctxsw_clear_mailbox(g, 6U, 0xffffffffU);
	nvgpu_writel(g, gr_fecs_cpuctl_alias_r(),
			gr_fecs_cpuctl_startcpu_f(1U));
}

u32 gb10b_gr_falcon_get_compute_preemption_mode(struct gk20a *g)
{
	u32 ctxsw_interface_ctl = 0U;
	u32 compute_preemption_control = 0U;

	/*
	 * Starting in GB10B, FECS ucode programs the context's requested
	 * preemption modes into the FE_*_PREEMPTION_CONTROL fields of the
	 * NV_PGRAPH_PRI_FECS_CTXSW_INTERFACE_CTL register during ctxsw restore.
	 */
	ctxsw_interface_ctl = nvgpu_readl(g, gr_fecs_ctxsw_interface_ctl_r());
	compute_preemption_control =
		gr_fecs_ctxsw_interface_ctl_fe_compute_preemption_control_v(
				ctxsw_interface_ctl);

	switch (compute_preemption_control) {
	case gr_fecs_ctxsw_interface_ctl_fe_compute_preemption_control_wfi_v():
		return NVGPU_PREEMPTION_MODE_COMPUTE_WFI;
	case gr_fecs_ctxsw_interface_ctl_fe_compute_preemption_control_cta_v():
		return NVGPU_PREEMPTION_MODE_COMPUTE_CTA;
	case gr_fecs_ctxsw_interface_ctl_fe_compute_preemption_control_cilp_v():
		return NVGPU_PREEMPTION_MODE_COMPUTE_CILP;
	default:
		nvgpu_err(g, "invalid compute preemption mode %u",
				compute_preemption_control);
	}

	return 0U;
}

void gb10b_gr_falcon_set_null_fecs_method_data(struct gk20a *g,
		struct nvgpu_fecs_method_op *op,
		u32 fecs_method)
{
	switch (fecs_method) {
#if defined(CONFIG_NVGPU_DEBUGGER) || defined(CONFIG_NVGPU_RECOVERY)
	case NVGPU_GR_FALCON_METHOD_CTXSW_STOP:
	case NVGPU_GR_FALCON_METHOD_CTXSW_START:
#endif
	case NVGPU_GR_FALCON_METHOD_HALT_PIPELINE:
	case NVGPU_GR_FALCON_METHOD_CTXSW_DISCOVER_IMAGE_SIZE:
#if defined(CONFIG_NVGPU_DEBUGGER) || \
defined(CONFIG_NVGPU_CTXSW_FW_ERROR_CODE_TESTING)
	case NVGPU_GR_FALCON_METHOD_CTXSW_DISCOVER_PM_IMAGE_SIZE:
#endif
#ifdef CONFIG_NVGPU_GRAPHICS
	case NVGPU_GR_FALCON_METHOD_PREEMPT_IMAGE_SIZE:
#endif
#if defined(CONFIG_NVGPU_DEBUGGER) || defined(CONFIG_NVGPU_PROFILER)
	case NVGPU_GR_FALCON_METHOD_START_SMPC_GLOBAL_MODE:
	case NVGPU_GR_FALCON_METHOD_STOP_SMPC_GLOBAL_MODE:
#endif
		op->method.data = NVGPU_NULL_METHOD_DATA;
		break;
	default:
		nvgpu_log(g, gpu_dbg_gpu_dbg, "fecs method: %d", fecs_method);
		break;
	}
}

#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
static void gb10b_gr_falcon_program_fecs_dmem_data(struct gk20a *g,
			u32 reg_offset, u32 addr_code32, u32 addr_data32,
			u32 code_size, u32 data_size)
{
	u32 offset = nvgpu_safe_add_u32(reg_offset, gr_fecs_dmemd_r(0));

	nvgpu_writel(g, offset, 0);
	nvgpu_writel(g, offset, 0);
	nvgpu_writel(g, offset, 0);
	nvgpu_writel(g, offset, 0);
	nvgpu_writel(g, offset, 4);
	nvgpu_writel(g, offset, addr_code32);
	nvgpu_writel(g, offset, 0);
	nvgpu_writel(g, offset, code_size);
	nvgpu_writel(g, offset, 0);
	nvgpu_writel(g, offset, 0);
	nvgpu_writel(g, offset, 0);
	nvgpu_writel(g, offset, addr_data32);
	nvgpu_writel(g, offset, data_size);
}

void gb10b_gr_falcon_load_ctxsw_ucode_boot(struct gk20a *g, u32 reg_offset,
			u32 boot_entry, u32 addr_load32, u32 blocks, u32 dst)
{
	u32 b;

	nvgpu_log(g, gpu_dbg_gr, " ");

	/*
	 * Set the base FB address for the DMA transfer. Subtract off the 256
	 * byte IMEM block offset such that the relative FB and IMEM offsets
	 * match, allowing the IMEM tags to be properly created.
	 */

	nvgpu_writel(g, nvgpu_safe_add_u32(reg_offset,
				gr_fecs_dmatrfbase_r()),
			nvgpu_safe_sub_u32(addr_load32, (dst >> 8)));

	for (b = 0; b < blocks; b++) {
		/* Setup destination IMEM offset */
		nvgpu_writel(g, nvgpu_safe_add_u32(reg_offset,
					gr_fecs_dmatrfmoffs_r()),
				nvgpu_safe_add_u32(dst, (b << 8)));

		/* Setup source offset (relative to BASE) */
		nvgpu_writel(g, nvgpu_safe_add_u32(reg_offset,
					gr_fecs_dmatrffboffs_r()),
				nvgpu_safe_add_u32(dst, (b << 8)));

		nvgpu_writel(g, nvgpu_safe_add_u32(reg_offset,
						gr_fecs_dmatrfcmd_r()),
				gr_fecs_dmatrfcmd_imem_f(0x01) |
				gr_fecs_dmatrfcmd_write_f(0x00) |
				gr_fecs_dmatrfcmd_size_f(0x06) |
				gr_fecs_dmatrfcmd_ctxdma_f(0));
	}

	/* Specify the falcon boot vector */
	nvgpu_writel(g, nvgpu_safe_add_u32(reg_offset, gr_fecs_bootvec_r()),
			gr_fecs_bootvec_vec_f(boot_entry));

	/* start the falcon immediately if PRIV security is disabled*/
	if (!nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		nvgpu_writel(g, nvgpu_safe_add_u32(reg_offset,
						gr_fecs_cpuctl_r()),
				gr_fecs_cpuctl_startcpu_f(0x01));
	}
}

void gb10b_gr_falcon_fecs_dmemc_write(struct gk20a *g, u32 reg_offset, u32 port,
	u32 offs, u32 blk, u32 ainc)
{
	nvgpu_writel(g, nvgpu_safe_add_u32(reg_offset, gr_fecs_dmemc_r(port)),
			gr_fecs_dmemc_offs_f(offs) |
			gr_fecs_dmemc_blk_f(blk) |
			gr_fecs_dmemc_aincw_f(ainc));
}

void gb10b_gr_falcon_load_ctxsw_ucode_header(struct gk20a *g,
	u32 reg_offset, u32 boot_signature, u32 addr_code32,
	u32 addr_data32, u32 code_size, u32 data_size)
{
	u32 offset = nvgpu_safe_add_u32(reg_offset, gr_fecs_dmemd_r(0));

	nvgpu_writel(g, nvgpu_safe_add_u32(reg_offset, gr_fecs_dmactl_r()),
			gr_fecs_dmactl_require_ctx_f(0));

	/*
	 * Copy falcon bootloader header into dmem at offset 0.
	 * Configure dmem port 0 for auto-incrementing writes starting at dmem
	 * offset 0.
	 */
	g->ops.gr.falcon.fecs_dmemc_write(g, reg_offset, 0U, 0U, 0U, 1U);

	/* Write out the actual data */
	switch (boot_signature) {
	case FALCON_UCODE_SIG_T18X_GPCCS_WITH_RESERVED:
	case FALCON_UCODE_SIG_T21X_FECS_WITH_DMEM_SIZE:
	case FALCON_UCODE_SIG_T21X_FECS_WITH_RESERVED:
	case FALCON_UCODE_SIG_T21X_GPCCS_WITH_RESERVED:
	case FALCON_UCODE_SIG_T12X_FECS_WITH_RESERVED:
	case FALCON_UCODE_SIG_T12X_GPCCS_WITH_RESERVED:
		nvgpu_writel(g, offset, 0);
		nvgpu_writel(g, offset, 0);
		nvgpu_writel(g, offset, 0);
		nvgpu_writel(g, offset, 0);
		gb10b_gr_falcon_program_fecs_dmem_data(g, reg_offset,
			addr_code32, addr_data32, code_size, data_size);
		break;
	case FALCON_UCODE_SIG_T12X_FECS_WITHOUT_RESERVED:
	case FALCON_UCODE_SIG_T12X_GPCCS_WITHOUT_RESERVED:
	case FALCON_UCODE_SIG_T21X_FECS_WITHOUT_RESERVED:
	case FALCON_UCODE_SIG_T21X_FECS_WITHOUT_RESERVED2:
	case FALCON_UCODE_SIG_T21X_GPCCS_WITHOUT_RESERVED:
		gb10b_gr_falcon_program_fecs_dmem_data(g, reg_offset,
			addr_code32, addr_data32, code_size, data_size);
		break;
	case FALCON_UCODE_SIG_T12X_FECS_OLDER:
	case FALCON_UCODE_SIG_T12X_GPCCS_OLDER:
		nvgpu_writel(g, offset, 0);
		nvgpu_writel(g, offset, addr_code32);
		nvgpu_writel(g, offset, 0);
		nvgpu_writel(g, offset, code_size);
		nvgpu_writel(g, offset, 0);
		nvgpu_writel(g, offset, addr_data32);
		nvgpu_writel(g, offset, data_size);
		nvgpu_writel(g, offset, addr_code32);
		nvgpu_writel(g, offset, 0);
		nvgpu_writel(g, offset, 0);
		break;
	default:
		nvgpu_err(g,
				"unknown falcon ucode boot signature 0x%08x"
				" with reg_offset 0x%08x",
				boot_signature, reg_offset);
		BUG();
		break;
	}
}

#endif

u32 gb10b_gr_falcon_fecs_host_intr_status(struct gk20a *g,
			struct nvgpu_fecs_host_intr_status *fecs_host_intr)
{

	u32 gr_fecs_intr = nvgpu_readl(g, gr_fecs_host_int_status_r());

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg | gpu_dbg_intr, " ");

	gr_fecs_intr |= gm20b_gr_falcon_fecs_host_intr_status(g, fecs_host_intr);

	if ((gr_fecs_intr &
			gr_fecs_host_int_status_poisoned_read_active_f()) != 0U) {
		fecs_host_intr->poisoned_read = true;
		nvgpu_err(g, "fecs_host_int_status_poisoned_read err handled");
	}

	if ((gr_fecs_intr &
			gr_fecs_host_int_status_ctx_load_after_halt_active_f()) != 0U) {
			fecs_host_intr->ctx_load_after_halt = true;
			nvgpu_err(g, "fecs_host_int_status_ctx_load_after_halt err handled");
	}

	if ((gr_fecs_intr &
			gr_fecs_host_int_status_ctxsw_intr_f(CTXSW_INTR2)) != 0U) {
		fecs_host_intr->ctxsw_intr2 =
			gr_fecs_host_int_status_ctxsw_intr_f(CTXSW_INTR2);
	}

	return gr_fecs_intr;
}

int gb10b_gr_intr_handle_fecs_error(struct gk20a *g,
				struct nvgpu_channel *ch_ptr,
				struct nvgpu_gr_isr_data *isr_data)
{
	int ret = 0;
	u32 mailbox_value;
	u32 mailbox_id = NVGPU_GR_FALCON_FECS_CTXSW_MAILBOX4;
	struct nvgpu_fecs_host_intr_status *fecs_host_intr;
	u32 gr_fecs_intr = isr_data->fecs_intr;

	if (gr_fecs_intr == 0U) {
		return ret;
	}

	fecs_host_intr = &isr_data->fecs_host_intr_status;
	if (fecs_host_intr->poisoned_read) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_FECS,
						GPU_FECS_CTXSW_POISON_ERROR);
		nvgpu_err(g, "FECS: Poisoned Read Data Error");

		g->ops.gr.falcon.dump_stats(g);
		gk20a_fecs_gr_debug_dump(g);

		/* TODO: Return -1 to trigger recovery for FECS
		 * poison error. Currently return 0 to keep
		 * recoverable errors at parity with Orin.
		 */
		ret = 0;
	}
	if (fecs_host_intr->ctx_load_after_halt) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_FECS,
						GPU_FECS_CTX_LOAD_AFTER_HALT);
		nvgpu_err(g, "FECS: Context load requested after a halt!");

		g->ops.gr.falcon.dump_stats(g);
		gk20a_fecs_gr_debug_dump(g);

		/* TODO: Return -1 to trigger recovery for FECS context
		 * load after halt error. Currently return 0 to keep
		 * recoverable errors at parity with Orin.
		 */
		ret = 0;
	}

	if (fecs_host_intr->ctxsw_intr2 != 0U) {
		mailbox_value = g->ops.gr.falcon.read_fecs_ctxsw_mailbox(g,
								mailbox_id);
#ifdef CONFIG_NVGPU_FECS_TRACE
		if (mailbox_value ==
			g->ops.gr.fecs_trace.get_buffer_full_mailbox_val()) {
			nvgpu_info(g, "ctxsw intr2 set by ucode, "
					"timestamp buffer full");
			nvgpu_gr_fecs_trace_reset_buffer(g);
		}
#endif
	}
	ret |= gv11b_gr_intr_handle_fecs_error(g, ch_ptr, isr_data);
	return ret;
}

void gb10b_gr_falcon_fecs_host_int_enable(struct gk20a *g)
{
	nvgpu_writel(g, gr_fecs_host_int_enable_r(),
		gr_fecs_host_int_enable_ctxsw_intr0_enable_f() |
#ifdef CONFIG_NVGPU_CILP
		gr_fecs_host_int_enable_ctxsw_intr1_enable_f() |
#endif
		gr_fecs_host_int_enable_ctxsw_intr2_enable_f() |
		gr_fecs_host_int_enable_fault_during_ctxsw_enable_f() |
		gr_fecs_host_int_enable_umimp_firmware_method_enable_f() |
		gr_fecs_host_int_enable_umimp_illegal_method_enable_f() |
		gr_fecs_host_int_enable_watchdog_enable_f() |
		gr_fecs_host_int_enable_flush_when_busy_enable_f() |
		gr_fecs_host_int_enable_ecc_corrected_enable_f() |
		gr_fecs_host_int_enable_ecc_uncorrected_enable_f() |
		gr_fecs_host_int_enable_poisoned_read_enable_f() |
		gr_fecs_host_int_enable_ctx_load_after_halt_enable_f());
}

static void gb10b_gr_falcon_fecs_dump_stats(struct gk20a *g)
{
	unsigned int i;

#ifdef CONFIG_NVGPU_FALCON_DEBUG
	nvgpu_falcon_dump_stats(&g->fecs_flcn);
#endif

	for (i = 0U; i < g->ops.gr.falcon.fecs_ctxsw_mailbox_size(); i++) {
		nvgpu_err(g, "gr_fecs_ctxsw_mailbox_%d_r: 0x%x",
			i, nvgpu_readl(g, gr_fecs_ctxsw_mailbox_r(i)));
	}

	for (i = 0U; i < gr_fecs_ctxsw_func_tracing_mailbox__size_1_v(); i++) {
		nvgpu_err(g, "gr_fecs_ctxsw_func_tracing_mailbox_%d_r(): 0x%x",
			i, nvgpu_readl(g,
				gr_fecs_ctxsw_func_tracing_mailbox_r(i)));
	}
	for (i = 0; i < gr_fecs_ctxsw_error_info__size_1_v(); i++) {
		nvgpu_err(g, "gr_fecs_ctxsw_error_info_%d_r(): 0x%08x",
			i, nvgpu_readl(g, gr_fecs_ctxsw_error_info_r(i)));
	}
	nvgpu_err(g, "gr_fecs_current_ctx_r(): 0x%08x",
			nvgpu_readl(g, gr_fecs_current_ctx_r()));
	nvgpu_err(g, "gr_fecs_current_ctx_hi_r(): 0x%08x",
			nvgpu_readl(g, gr_fecs_current_ctx_hi_r()));
	nvgpu_err(g, "gr_fecs_new_ctx_r(): 0x%08x",
			nvgpu_readl(g, gr_fecs_new_ctx_r()));
	nvgpu_err(g, "gr_fecs_new_ctx_hi_r(): 0x%08x",
			nvgpu_readl(g, gr_fecs_new_ctx_hi_r()));
	nvgpu_err(g, "gr_pri_fecs_fs_r(): 0x%08x",
			nvgpu_readl(g, gr_pri_fecs_fs_r()));
	nvgpu_err(g, "gr_pri_fecs_arb_base_addr_r(): 0x%08x",
			nvgpu_readl(g, gr_pri_fecs_arb_base_addr_r()));
	nvgpu_err(g, "gr_pri_fecs_arb_status_r(): 0x%08x",
			nvgpu_readl(g, gr_pri_fecs_arb_status_r()));
	nvgpu_err(g, "gr_fecs_host_int_status_r(): 0x%08x",
			nvgpu_readl(g, gr_fecs_host_int_status_r()));
	nvgpu_err(g, "gr_fecs_host_ctxsw_status_fe_0_r(): 0x%08x",
			nvgpu_readl(g, gr_fecs_ctxsw_status_fe_0_r()));
	nvgpu_err(g, "gr_fecs_host_ctxsw_status_1_r(): 0x%08x",
			nvgpu_readl(g, gr_fecs_ctxsw_status_1_r()));
}

static void gb10b_gr_falcon_gpccs_dump_stats(struct gk20a *g)
{
	unsigned int i;
	struct nvgpu_gr_config *gr_config = nvgpu_gr_get_config_ptr(g);
	u32 gpc_count = nvgpu_gr_config_get_gpc_count(gr_config);
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	u32 gpc = 0U, offset = 0U;
	u32 tpc = 0U;

	for (gpc = 0U; gpc < gpc_count; gpc++) {
		offset = nvgpu_safe_mult_u32(gpc_stride, gpc);
		for (i = 0U; i < gr_gpccs_ctxsw_mailbox__size_1_v(); i++) {
			nvgpu_err(g,
				"gr_gpc%d_gpccs_ctxsw_mailbox_%d_r(): 0x%x",
				gpc, i,
				nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_gpc0_gpccs_ctxsw_mailbox_r(i),
					offset)));
		}
	}

	for (gpc = 0U; gpc < gpc_count; gpc++) {
		offset = nvgpu_safe_mult_u32(gpc_stride, gpc);
		for (i = 0U;
			i < gr_gpc0_gpccs_ctxsw_func_tracing_mailbox__size_1_v();
			i++) {
			nvgpu_err(g,
			"gr_gpc%d_gpccs_ctxsw_func_tracing_mailbox_%d_r(): 0x%x",
				gpc, i,
				nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_gpc0_gpccs_ctxsw_func_tracing_mailbox_r(i),
					offset)));
		}
	}

	for (gpc = 0U; gpc < gpc_count; gpc++) {
		offset = nvgpu_safe_mult_u32(gpc_stride, gpc);
		for (i = 0U;
			i < gr_gpc0_gpccs_ctxsw_error_info__size_1_v();
			i++) {
			nvgpu_err(g,
			"gr_gpc%d_gpccs_ctxsw_error_info_%d_r(): 0x%x",
				gpc, i,
				nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_gpc0_gpccs_ctxsw_error_info_r(i),
					offset)));
		}
	}

	for (gpc = 0U; gpc < gpc_count; gpc++) {
		offset = nvgpu_safe_mult_u32(gpc_stride, gpc);
		nvgpu_err(g, "gr_pri_gpc%d_gpccs_gpc_activity0_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_pri_gpc0_gpccs_gpc_activity0_r(), offset)));
		nvgpu_err(g, "gr_pri_gpc%d_gpccs_gpc_activity1_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_pri_gpc0_gpccs_gpc_activity1_r(), offset)));
		nvgpu_err(g, "gr_pri_gpc%d_gpccs_gpc_activity2_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_pri_gpc0_gpccs_gpc_activity2_r(), offset)));
		nvgpu_err(g, "gr_pri_gpc%d_gpccs_gpc_activity3_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_pri_gpc0_gpccs_gpc_activity3_r(), offset)));
		nvgpu_err(g, "gr_pri_gpc%d_gpccs_gpc_activity4_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_pri_gpc0_gpccs_gpc_activity4_r(), offset)));
		nvgpu_err(g, "gr_pri_gpc%d_gpccs_gpc_activity5_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_pri_gpc0_gpccs_gpc_activity5_r(), offset)));
		nvgpu_err(g, "gr_pri_gpc%d_gpccs_gpc_activity6_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_pri_gpc0_gpccs_gpc_activity6_r(), offset)));
	}

	for (gpc = 0U; gpc < gpc_count; gpc++) {
		offset = nvgpu_safe_mult_u32(gpc_stride, gpc);
		nvgpu_err(g, "gr_gpc%d_gpccs_ctxsw_status_1_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_gpccs_ctxsw_status_1_r(), offset)));
		nvgpu_err(g, "gr_gpc%d_gpccs_ctxsw_status_gpc_0_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_gpccs_ctxsw_status_gpc_0_r(), offset)));
		nvgpu_err(g, "gr_gpc%d_gpccs_fs_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_gpccs_fs_r(), offset)));
		nvgpu_err(g, "gr_gpc%d_gpccs_fs_gpc_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_gpc0_fs_gpc_r(), offset)));
		nvgpu_err(g, "gr_gpc%d_gpccs_tpcs_usable_by_gfx_r(): 0x%x",
				gpc, nvgpu_readl(g, nvgpu_safe_add_u32(
				gr_pri_gpc0_gpccs_tpcs_usable_by_gfx_r(), offset)));
	}

	for (gpc = 0U; gpc < gpc_count; gpc++) {
		for (tpc = 0U;
			 tpc < nvgpu_gr_config_get_gpc_tpc_count(gr_config, gpc);
			 tpc++) {
			offset = nvgpu_safe_add_u32(
				nvgpu_safe_mult_u32(gpc_stride, gpc),
				nvgpu_safe_mult_u32(tpc_stride, tpc));
			nvgpu_err(g,
				"gr_gpc%d_tpc%d_tpccs_tpc_activity0_r(): 0x%x",
				gpc, tpc,
				nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_pri_gpc0_tpc0_tpccs_tpc_activity_0_r(),
					offset)));
		}
	}
}

void gb10b_gr_falcon_dump_stats(struct gk20a *g)
{
	/* Dump FECS, GPCCS registers to aid debug */
	gb10b_gr_falcon_fecs_dump_stats(g);
	gb10b_gr_falcon_gpccs_dump_stats(g);
}
