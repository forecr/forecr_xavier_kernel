// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/firmware.h>
#include <nvgpu/acr.h>
#include <nvgpu/nvgpu_err.h>

#include "nvjpg_gb10b.h"
#include "hal/riscv/riscv_gb10b.h"

#include <nvgpu/hw/gb10b/hw_pnvjpg_gb10b.h>

u32 gb10b_nvjpg_base_addr(u32 inst_id)
{
	return pnvjpg_falcon_irqsset_r(inst_id);
}

u32 gb10b_nvjpg_falcon2_base_addr(u32 inst_id)
{
	/*
	 * NVJPG risc-v base address is calculated by subtracting
	 * risc-v cpuctl offset from NVJPG cpuctl address.
	 */
	return pnvjpg_riscv_cpuctl_r(inst_id) - gb10b_riscv_cpuctl_offset();
}

void gb10b_nvjpg_setup_boot_config(struct gk20a *g, u32 inst_id)
{
	u32 data = 0;

	data = nvgpu_readl(g, pnvjpg_fbif_ctl_r(inst_id));
	data |= pnvjpg_fbif_ctl_allow_phys_no_ctx_allow_f();
	nvgpu_writel(g, pnvjpg_fbif_ctl_r(inst_id), data);

	/* Setup aperture (physical) for ucode loading */
	data = nvgpu_readl(g, pnvjpg_fbif_transcfg_r(inst_id, UCODE_DMA_ID));
	data |= pnvjpg_fbif_transcfg_mem_type_physical_f() |
			pnvjpg_fbif_transcfg_target_noncoherent_sysmem_f();
	nvgpu_writel(g, pnvjpg_fbif_transcfg_r(inst_id, UCODE_DMA_ID), data);
}

void gb10b_nvjpg_halt_engine(struct gk20a *g, u32 inst_id)
{
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;
	u32 data;

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	data = nvgpu_readl(g, pnvjpg_falcon_engctl_r(inst_id)) |
			pnvjpg_falcon_engctl_set_stallreq_true_f();
	nvgpu_writel(g, pnvjpg_falcon_engctl_r(inst_id), data);

	do {
		data = nvgpu_readl(g, pnvjpg_falcon_engctl_r(inst_id));
		if (pnvjpg_falcon_engctl_stallack_v(data) ==
			pnvjpg_falcon_engctl_stallack_true_v()) {
			nvgpu_log(g, gpu_dbg_mme, "nvjpg%u engine halted", inst_id);
			return;
		}

		nvgpu_usleep_range(delay, nvgpu_safe_mult_u32(delay, 2U));
		delay = min_t(u32, delay << 1U, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	nvgpu_err(g, "Timed out waiting for nvjpg%u engine stall ack", inst_id);
}

int gb10b_nvjpg_reset_engine(struct gk20a *g, u32 inst_id)
{
	u32 data;

	nvgpu_writel(g, pnvjpg_falcon_engine_r(inst_id),
			pnvjpg_falcon_engine_reset_true_f());
	nvgpu_udelay(10);

	data = nvgpu_readl(g, pnvjpg_falcon_engine_r(inst_id));
	if (pnvjpg_falcon_engine_reset_status_v(data) ==
		pnvjpg_falcon_engine_reset_status_asserted_v()) {
		nvgpu_log(g, gpu_dbg_mme, "nvjpg%u engine reset asserted", inst_id);
	}

	nvgpu_writel(g, pnvjpg_falcon_engine_r(inst_id),
			pnvjpg_falcon_engine_reset_false_f());
	nvgpu_udelay(10);

	data = nvgpu_readl(g, pnvjpg_falcon_engine_r(inst_id));
	if (pnvjpg_falcon_engine_reset_status_v(data) ==
		pnvjpg_falcon_engine_reset_status_deasserted_v()) {
		nvgpu_log(g, gpu_dbg_mme, "nvjpg%u engine reset deasserted", inst_id);
	}

	return 0;
}

static void gb10b_nvjpg_intr_stall_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	if (g->ops.nvjpg.nvjpg_isr != NULL) {
		g->ops.nvjpg.nvjpg_isr(g, inst_id);
	} else {
		nvgpu_err(g, "nvjpg isr not set");
	}
}

static u32 gb10b_nvjpg_intr_nonstall_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "nvjpg%u nonstall irq stat: 0x%08x",
			inst_id, nvgpu_readl(g, pnvjpg_falcon_irqstat_r(inst_id)));

	return (NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE |
			NVGPU_CIC_NONSTALL_OPS_POST_EVENTS);
}

void gb10b_nvjpg_get_intr_ctrl_msg(struct gk20a *g, u32 inst_id, bool enable,
							u32 *intr_ctrl, u32 *intr_notify_ctrl)
{
	u32 stall_vector = 0U;
	u32 nonstall_vector = 0U;

	stall_vector = nvgpu_gin_get_unit_stall_vector(g,
				NVGPU_GIN_INTR_UNIT_NVJPG, inst_id);
	nonstall_vector = nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_NVJPG, inst_id);

	if (enable) {
		/* Set stall and nonstall interrupt handlers */
		nvgpu_gin_set_stall_handler(g, stall_vector,
				&gb10b_nvjpg_intr_stall_handler, inst_id);
		nvgpu_gin_set_nonstall_handler(g, nonstall_vector,
				&gb10b_nvjpg_intr_nonstall_handler, inst_id);
		/* Enable stalling interrupt to cpu, disable stalling interrupt to gsp */
		*intr_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, stall_vector,
				NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
		/* Enable notifying interrupt to cpu, disable notifying interrupt to gsp */
		*intr_notify_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, nonstall_vector,
				NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	} else {
		/* Disable stalling interrupt to cpu, disable stalling interrupt to gsp */
		*intr_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, stall_vector,
				NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
		/* Disable notifying interrupt to cpu, disable notifying interrupt to gsp */
		*intr_notify_ctrl = nvgpu_gin_get_intr_ctrl_msg(g, nonstall_vector,
				NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
	}
	return;
}

void gb10b_nvjpg_enable_irq(struct gk20a *g, u32 inst_id, bool enable)
{
	u32 stall_offset = 0U;
	u32 nonstall_offset = 1U;
	u32 intr_ctrl = 0U;
	u32 intr_notify_ctrl = 0U;

	if (g->ops.nvjpg.get_intr_ctrl_msg != NULL) {
		g->ops.nvjpg.get_intr_ctrl_msg(g, inst_id, enable,
			&intr_ctrl, &intr_notify_ctrl);
	} else {
		nvgpu_err(g, "Un-supported NVJPG interrupt msg ctrl");
	}

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "nvjpg%u intr %s ctrl 0x%x notify_ctrl 0x%x",
			inst_id, enable ? "enable" : "disable", intr_ctrl, intr_notify_ctrl);

	nvgpu_writel(g, pnvjpg_falcon_intr_ctrl_r(inst_id, stall_offset), intr_ctrl);
	nvgpu_writel(g, pnvjpg_falcon_intr_ctrl_r(inst_id, nonstall_offset), intr_notify_ctrl);
}

void gb10b_nvjpg_isr(struct gk20a *g, u32 inst_id)
{
	u32 intr_stat = 0U;

	intr_stat = nvgpu_readl(g, pnvjpg_falcon_irqstat_r(inst_id));
	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "nvjpg%u irqstat 0x%08x", inst_id, intr_stat);

	nvgpu_writel(g, pnvjpg_falcon_irqsclr_r(inst_id), intr_stat);
	if ((intr_stat & (pnvjpg_falcon_irqstat_swgen0_true_f() |
				pnvjpg_falcon_irqstat_swgen1_true_f())) != 0U) {
		nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr,
			"nvjpg%u swgen0/1 interrupt", inst_id);
		nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr,
			"nvjpg%u mailbox0: 0x%x mailbox1: 0x%x", inst_id,
			nvgpu_readl(g, pnvjpg_falcon_mailbox1_r(inst_id)),
			nvgpu_readl(g, pnvjpg_falcon_mailbox0_r(inst_id)));
	} else if ((intr_stat & pnvjpg_falcon_irqstat_halt_true_f()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_NVJPG,
						GPU_NVJPG_UNCORRECTED_ERR);
		nvgpu_err(g, "nvjpg%u fatal halt interrupt", inst_id);
		nvgpu_err(g, "nvjpg%u mailbox0: 0x%x mailbox1: 0x%x", inst_id,
			nvgpu_readl(g, pnvjpg_falcon_mailbox1_r(inst_id)),
			nvgpu_readl(g, pnvjpg_falcon_mailbox0_r(inst_id)));
		nvgpu_multimedia_debug_dump(g, nvgpu_device_get(g, NVGPU_DEVTYPE_NVJPG, inst_id));
		/* TODO: Trigger recovery (engine reset) */
	} else {
		/* Report the error to safety services */
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_NVJPG,
						GPU_NVJPG_UNCORRECTED_ERR);
		nvgpu_err(g, "Un-supported or Un-handled nvjpg%u interrupts: stat:0x%08x",
				inst_id, intr_stat);
		nvgpu_multimedia_debug_dump(g, nvgpu_device_get(g, NVGPU_DEVTYPE_NVJPG, inst_id));
		/* TODO: Error notifier, quiesce or recovery/reset as needed */
	}
}

void gb10b_nvjpg_fw(struct gk20a *g, struct nvgpu_firmware **desc_fw,
            struct nvgpu_firmware **image_fw,
            struct nvgpu_firmware **pkc_sig)
{

	*desc_fw = nvgpu_request_firmware(g, GB10B_NVJPG_RISCV_EB_UCODE_DESC,
					NVGPU_REQUEST_FIRMWARE_NO_WARN);

	if (nvgpu_acr_is_lsb_v3_supported(g)) {
		*pkc_sig = nvgpu_request_firmware(g,
				GB10B_NVJPG_RISCV_EB_UCODE_LSB_PKC_SIG,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	} else {
		*pkc_sig = nvgpu_request_firmware(g,
				GB10B_NVJPG_RISCV_EB_UCODE_PKC_SIG,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	}

	if (g->ops.pmu.is_debug_mode_enabled(g)) {
		*image_fw = nvgpu_request_firmware(g,
				GB10B_NVJPG_RISCV_EB_UCODE_IMAGE,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	} else {
		*image_fw = nvgpu_request_firmware(g,
				GB10B_NVJPG_RISCV_EB_UCODE_PROD_IMAGE,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	}
}
