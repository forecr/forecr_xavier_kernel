// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/firmware.h>
#include <nvgpu/acr.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/gin.h>

#include "nvenc_gb10b.h"
#include "hal/riscv/riscv_gb10b.h"

#include <nvgpu/hw/gb10b/hw_pnvenc_gb10b.h>

u32 gb10b_nvenc_base_addr(u32 inst_id)
{
	return pnvenc_falcon_irqsset_r(inst_id);
}

u32 gb10b_nvenc_falcon2_base_addr(u32 inst_id)
{
	/*
	 * NVENC risc-v base address is calculated by subtracting
	 * risc-v cpuctl offset from NVENC cpuctl address.
	 */
	return pnvenc_riscv_cpuctl_r(inst_id) - gb10b_riscv_cpuctl_offset();
}

void gb10b_nvenc_setup_boot_config(struct gk20a *g, u32 inst_id)
{
	u32 data = 0;

	data = nvgpu_readl(g, pnvenc_fbif_ctl_r(inst_id));
	data |= pnvenc_fbif_ctl_allow_phys_no_ctx_allow_f();
	nvgpu_writel(g, pnvenc_fbif_ctl_r(inst_id), data);

	/* Setup aperture (physical) for ucode loading */
	data = nvgpu_readl(g, pnvenc_fbif_transcfg_r(inst_id, UCODE_DMA_ID));
	data |= pnvenc_fbif_transcfg_mem_type_physical_f() |
			pnvenc_fbif_transcfg_target_noncoherent_sysmem_f();
	nvgpu_writel(g, pnvenc_fbif_transcfg_r(inst_id, UCODE_DMA_ID), data);
}

void gb10b_nvenc_halt_engine(struct gk20a *g, u32 inst_id)
{
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;
	u32 data;

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	data = nvgpu_readl(g, pnvenc_falcon_engctl_r(inst_id)) |
			pnvenc_falcon_engctl_set_stallreq_true_f();
	nvgpu_writel(g, pnvenc_falcon_engctl_r(inst_id), data);

	do {
		data = nvgpu_readl(g, pnvenc_falcon_engctl_r(inst_id));
		if (pnvenc_falcon_engctl_stallack_v(data) ==
			pnvenc_falcon_engctl_stallack_true_v()) {
			nvgpu_log(g, gpu_dbg_mme, "nvenc%u engine halted", inst_id);
			return;
		}

		nvgpu_usleep_range(delay, nvgpu_safe_mult_u32(delay, 2U));
		delay = min_t(u32, delay << 1U, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	nvgpu_err(g, "Timed out waiting for nvenc%u engine stall ack", inst_id);
}

int gb10b_nvenc_reset_engine(struct gk20a *g, u32 inst_id)
{
	u32 data;

	nvgpu_writel(g, pnvenc_falcon_engine_r(inst_id),
			pnvenc_falcon_engine_reset_true_f());
	nvgpu_udelay(10);

	data = nvgpu_readl(g, pnvenc_falcon_engine_r(inst_id));
	if (pnvenc_falcon_engine_reset_status_v(data) ==
		pnvenc_falcon_engine_reset_status_asserted_v()) {
		nvgpu_log(g, gpu_dbg_mme, "nvenc%u engine reset asserted", inst_id);
	}

	nvgpu_writel(g, pnvenc_falcon_engine_r(inst_id),
			pnvenc_falcon_engine_reset_false_f());
	nvgpu_udelay(10);

	data = nvgpu_readl(g, pnvenc_falcon_engine_r(inst_id));
	if (pnvenc_falcon_engine_reset_status_v(data) ==
		pnvenc_falcon_engine_reset_status_deasserted_v()) {
		nvgpu_log(g, gpu_dbg_mme, "nvenc%u engine reset deasserted", inst_id);
	}

	return 0;
}

static void gb10b_nvenc_ue_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	/* Report fatal error to safety services. */
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_NVENC,
						GPU_NVENC_UNCORRECTED_ERR);

	nvgpu_err(g, "NVENC%u uncorrected error interrupt", inst_id);
	nvgpu_err(g, "ERRSLICE0_MISSIONERR_STATUS: 0x%x",
			 nvgpu_readl(g,
				pnvenc_errslice0_missionerr_status_r(inst_id)));
	nvgpu_err(g, "FALCON_SAFETY_MAILBOX: 0x%x",
			 nvgpu_readl(g,
				pnvenc_falcon_safety_mailbox_r(inst_id)));

	/* TODO: Trigger recovery for NVENC uncorrected errors */
}

static void gb10b_nvenc_ce_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	/* Report fatal error to safety services. */
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_NVENC,
						GPU_NVENC_CORRECTED_ERR);

	nvgpu_err(g, "NVENC corrected error interrupt");
	nvgpu_err(g, "ERRSLICE0_MISSIONERR_STATUS: 0x%x",
			 nvgpu_readl(g,
				pnvenc_errslice0_missionerr_status_r(inst_id)));
	nvgpu_err(g, "FALCON_SAFETY_MAILBOX: 0x%x",
			 nvgpu_readl(g,
				pnvenc_falcon_safety_mailbox_r(inst_id)));

	/* Corrected error is mapped to only bit 4 in below register */
	nvgpu_writel(g, pnvenc_errslice0_missionerr_status_r(inst_id),
			0x10);
}

static void gb10b_nvenc_intr_stall_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	if (g->ops.nvenc.nvenc_isr != NULL) {
		g->ops.nvenc.nvenc_isr(g, inst_id);
	} else {
		nvgpu_err(g, "nvenc isr not set");
	}
}

static u32 gb10b_nvenc_intr_nonstall_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "nvenc%u nonstall irq stat: 0x%08x",
			inst_id, nvgpu_readl(g, pnvenc_falcon_irqstat_r(inst_id)));

	return (NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE |
			NVGPU_CIC_NONSTALL_OPS_POST_EVENTS);
}

void gb10b_nvenc_set_safety_intr(struct gk20a *g, u32 inst_id, bool enable)
{
	u32 ue_vector = 0U;
	u32 ce_vector = 0U;
	u32 intr_ctrl_ue = 0U;
	u32 intr_ctrl_ce = 0U;

	ue_vector = nvgpu_gin_get_unit_stall_vector(g,
				NVGPU_GIN_INTR_UNIT_NVENC, NVGPU_NVENC_UE_VECTOR_OFFSET_0 + inst_id);
	ce_vector = nvgpu_gin_get_unit_stall_vector(g,
				NVGPU_GIN_INTR_UNIT_NVENC, NVGPU_NVENC_CE_VECTOR_OFFSET_0 + inst_id);

	if (enable) {
		/* Set UE and CE interrupt handlers */
		nvgpu_gin_set_stall_handler(g, ue_vector,
				&gb10b_nvenc_ue_handler, inst_id);
		nvgpu_gin_set_stall_handler(g, ce_vector,
				&gb10b_nvenc_ce_handler, inst_id);
		/* Enable UE interrupt to cpu, disable UE interrupt to gsp */
		intr_ctrl_ue = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
				NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
		/* Enable CE interrupt to cpu, disabling CE interrupt to gsp */
		intr_ctrl_ce = nvgpu_gin_get_intr_ctrl_msg(g, ce_vector,
				NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	} else {
		/* Disable UE interrupt to cpu, disable UE interrupt to gsp */
		intr_ctrl_ue = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
				NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
		/* Disable CE interrupt to cpu, disabling CE interrupt to gsp */
		intr_ctrl_ce = nvgpu_gin_get_intr_ctrl_msg(g, ce_vector,
				NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
	}

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "nvenc%u intr %s intr_ctrl_ue 0x%x intr_ctrl_ce 0x%x",
			inst_id, enable ? "enable" : "disable", intr_ctrl_ue, intr_ctrl_ce);

	nvgpu_writel(g, pnvenc_falcon_intr_ctrl2_r(inst_id), intr_ctrl_ue);
	nvgpu_writel(g, pnvenc_falcon_intr_ctrl3_r(inst_id), intr_ctrl_ce);
}

void gb10b_nvenc_get_intr_ctrl_msg(struct gk20a *g, u32 inst_id, bool enable,
							u32 *intr_ctrl, u32 *intr_notify_ctrl)
{
	u32 stall_vector = 0U;
	u32 nonstall_vector = 0U;

	stall_vector = nvgpu_gin_get_unit_stall_vector(g,
				NVGPU_GIN_INTR_UNIT_NVENC, inst_id);
	nonstall_vector = nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_NVENC, inst_id);

	if (enable) {
		/* Set stall and nonstall interrupt handlers */
		nvgpu_gin_set_stall_handler(g, stall_vector,
				&gb10b_nvenc_intr_stall_handler, inst_id);
		nvgpu_gin_set_nonstall_handler(g, nonstall_vector,
				&gb10b_nvenc_intr_nonstall_handler, inst_id);
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

void gb10b_nvenc_enable_irq(struct gk20a *g, u32 inst_id, bool enable)
{
	u32 stall_offset = 0U;
	u32 nonstall_offset = 1U;
	u32 intr_ctrl = 0U;
	u32 intr_notify_ctrl = 0U;

	if (g->ops.nvenc.get_intr_ctrl_msg != NULL) {
		g->ops.nvenc.get_intr_ctrl_msg(g, inst_id, enable,
			&intr_ctrl, &intr_notify_ctrl);
	} else {
		nvgpu_err(g, "Un-supported NVENC interrupt msg ctrl");
	}

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "nvenc%u intr %s ctrl 0x%x notify_ctrl 0x%x",
			inst_id, enable ? "enable" : "disable", intr_ctrl, intr_notify_ctrl);

	nvgpu_writel(g, pnvenc_falcon_intr_ctrl_r(inst_id, stall_offset), intr_ctrl);
	nvgpu_writel(g, pnvenc_falcon_intr_ctrl_r(inst_id, nonstall_offset), intr_notify_ctrl);
}

void gb10b_nvenc_isr(struct gk20a *g, u32 inst_id)
{
	u32 intr_stat = 0U;

	intr_stat = nvgpu_readl(g, pnvenc_falcon_irqstat_r(inst_id));
	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr,
			"nvenc%u irqstat 0x%08x", inst_id, intr_stat);

	nvgpu_writel(g, pnvenc_falcon_irqsclr_r(inst_id), intr_stat);
	if ((intr_stat & (pnvenc_falcon_irqstat_swgen0_true_f() |
				pnvenc_falcon_irqstat_swgen1_true_f())) != 0U) {
		nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr,
			"nvenc%u swgen0/1 interrupt", inst_id);
		nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr,
			"nvenc%u mailbox0: 0x%x mailbox1: 0x%x", inst_id,
			nvgpu_readl(g, pnvenc_falcon_mailbox1_r(inst_id)),
			nvgpu_readl(g, pnvenc_falcon_mailbox0_r(inst_id)));
	} else if ((intr_stat & pnvenc_falcon_irqstat_halt_true_f()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_NVENC,
						GPU_NVENC_UNCORRECTED_ERR);
		nvgpu_err(g, "nvenc%u fatal halt interrupt", inst_id);
		nvgpu_err(g, "nvenc%u mailbox0: 0x%x mailbox1: 0x%x", inst_id,
			nvgpu_readl(g, pnvenc_falcon_mailbox1_r(inst_id)),
			nvgpu_readl(g, pnvenc_falcon_mailbox0_r(inst_id)));
		nvgpu_multimedia_debug_dump(g, nvgpu_device_get(g,
						NVGPU_DEVTYPE_NVENC, inst_id));
		/* TODO: Trigger recovery (engine reset) */
	} else {
		/* Report the error to safety services */
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_NVENC,
						GPU_NVENC_UNCORRECTED_ERR);
		nvgpu_err(g, "Un-handled nvenc%u interrupts: stat:0x%08x",
				inst_id, intr_stat);
		nvgpu_multimedia_debug_dump(g, nvgpu_device_get(g,
						NVGPU_DEVTYPE_NVENC, inst_id));
		/* TODO: Error notifier, quiesce or recovery/reset as needed */
	}
}

void gb10b_nvenc_fw(struct gk20a *g, struct nvgpu_firmware **desc_fw,
			struct nvgpu_firmware **image_fw,
			struct nvgpu_firmware **pkc_sig)
{

	*desc_fw = nvgpu_request_firmware(g, GB10B_NVENC_RISCV_EB_UCODE_DESC,
					NVGPU_REQUEST_FIRMWARE_NO_WARN);

	if (nvgpu_acr_is_lsb_v3_supported(g)) {
		*pkc_sig = nvgpu_request_firmware(g,
				GB10B_NVENC_RISCV_EB_UCODE_LSB_PKC_SIG,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	} else {
		*pkc_sig = nvgpu_request_firmware(g,
				GB10B_NVENC_RISCV_EB_UCODE_PKC_SIG,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	}

	if (g->ops.pmu.is_debug_mode_enabled(g)) {
		*image_fw = nvgpu_request_firmware(g,
				GB10B_NVENC_RISCV_EB_UCODE_IMAGE,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);

	} else {
		*image_fw = nvgpu_request_firmware(g,
				GB10B_NVENC_RISCV_EB_UCODE_PROD_IMAGE,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	}
}
