// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/firmware.h>
#include <nvgpu/acr.h>
#include <nvgpu/nvgpu_err.h>

#include "ofa_gb10b.h"
#include "hal/riscv/riscv_gb10b.h"

#include <nvgpu/hw/gb10b/hw_pofa_gb10b.h>

u32 gb10b_ofa_base_addr(u32 inst_id)
{
	nvgpu_assert(inst_id == 0U);
	return pofa_falcon_irqsset_r();
}

u32 gb10b_ofa_falcon2_base_addr(u32 inst_id)
{
	nvgpu_assert(inst_id == 0U);
	/*
	 * OFA risc-v base address is calculated by subtracting
	 * risc-v cpuctl offset from OFA cpuctl address.
	 */
	return pofa_riscv_cpuctl_r() - gb10b_riscv_cpuctl_offset();
}

void gb10b_ofa_setup_boot_config(struct gk20a *g, u32 inst_id)
{
	u32 data = 0;

	nvgpu_assert(inst_id == 0U);

	data = nvgpu_readl(g, pofa_fbif_ctl_r());
	data |= pofa_fbif_ctl_allow_phys_no_ctx_allow_f();
	nvgpu_writel(g, pofa_fbif_ctl_r(), data);

	/* Setup aperture (physical) for ucode loading */
	data = nvgpu_readl(g, pofa_fbif_transcfg_r(UCODE_DMA_ID));
	data |= pofa_fbif_transcfg_mem_type_physical_f() |
			pofa_fbif_transcfg_target_noncoherent_sysmem_f();
	nvgpu_writel(g, pofa_fbif_transcfg_r(UCODE_DMA_ID), data);
}

void gb10b_ofa_halt_engine(struct gk20a *g, u32 inst_id)
{
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;
	u32 data;

	nvgpu_assert(inst_id == 0U);

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	data = nvgpu_readl(g, pofa_falcon_engctl_r()) |
			pofa_falcon_engctl_set_stallreq_true_f();
	nvgpu_writel(g, pofa_falcon_engctl_r(), data);

	do {
		data = nvgpu_readl(g, pofa_falcon_engctl_r());
		if (pofa_falcon_engctl_stallack_v(data) ==
			pofa_falcon_engctl_stallack_true_v()) {
			nvgpu_log(g, gpu_dbg_mme, "ofa engine halted");
			return;
		}

		nvgpu_usleep_range(delay, nvgpu_safe_mult_u32(delay, 2U));
		delay = min_t(u32, delay << 1U, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	nvgpu_err(g, "Timed out waiting for ofa engine stall ack");
}

int gb10b_ofa_reset_engine(struct gk20a *g, u32 inst_id)
{
	u32 data;

	nvgpu_assert(inst_id == 0U);

	nvgpu_writel(g, pofa_falcon_engine_r(),
			pofa_falcon_engine_reset_true_f());
	nvgpu_udelay(10);

	data = nvgpu_readl(g, pofa_falcon_engine_r());
	if (pofa_falcon_engine_reset_status_v(data) ==
		pofa_falcon_engine_reset_status_asserted_v()) {
		nvgpu_log(g, gpu_dbg_mme, "ofa engine reset asserted");
	}

	nvgpu_writel(g, pofa_falcon_engine_r(),
			pofa_falcon_engine_reset_false_f());
	nvgpu_udelay(10);

	data = nvgpu_readl(g, pofa_falcon_engine_r());
	if (pofa_falcon_engine_reset_status_v(data) ==
		pofa_falcon_engine_reset_status_deasserted_v()) {
		nvgpu_log(g, gpu_dbg_mme, "ofa engine reset deasserted");
	}

	return 0;
}

static void gb10b_ofa_intr_stall_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	if (g->ops.ofa.ofa_isr != NULL) {
		g->ops.ofa.ofa_isr(g, inst_id);
	} else {
		nvgpu_err(g, "ofa isr not set");
	}
}

static void gb10b_ofa_dump_ec_regs(struct gk20a *g)
{
	nvgpu_err(g, "ofa errslice0_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice0_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice1_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice1_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice2_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice2_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice3_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice3_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice4_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice4_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice5_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice5_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice6_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice6_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice7_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice7_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice8_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice8_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice9_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice9_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice10_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice10_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice11_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice11_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice12_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice12_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice13_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice13_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice14_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice14_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice15_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice15_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice16_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice16_missionerr_status_r()));
	nvgpu_err(g, "ofa errslice17_missionerr_status: 0x%x",
			nvgpu_readl(g, pofa_errslice17_missionerr_status_r()));
}

static void gb10b_ofa_clear_ec_regs(struct gk20a *g)
{
	nvgpu_writel(g, pofa_errslice0_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice1_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice2_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice3_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice4_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice5_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice6_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice7_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice8_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice9_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice10_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice11_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice12_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice13_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice14_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice15_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice16_missionerr_status_r(), 0xFFFFFFFF);
	nvgpu_writel(g, pofa_errslice17_missionerr_status_r(), 0xFFFFFFFF);
}

static void gb10b_ofa_ue_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "ofa uncorrected error interrupt");

	nvgpu_assert(inst_id == 0U);

	/* Report the error to safety services */
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_OFA,
						GPU_OFA_UNCORRECTED_ERR);

	/* Print all Error collator regs in OFA */
	gb10b_ofa_dump_ec_regs(g);

	nvgpu_err(g, "FALCON_SAFETY_MAILBOX: 0x%x",
			nvgpu_readl(g, pofa_falcon_safety_mailbox_r()));

	/* TODO: Trigger Recovery for Engine Reset*/
}

static void gb10b_ofa_ce_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "ofa corrected error interrupt");

	nvgpu_assert(inst_id == 0U);

	/* Report the error to safety services */
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_OFA,
						GPU_OFA_CORRECTED_ERR);
	/* Read and clear the error status registers */
	gb10b_ofa_dump_ec_regs(g);
	nvgpu_err(g, "FALCON_SAFETY_MAILBOX: 0x%x",
			nvgpu_readl(g, pofa_falcon_safety_mailbox_r()));
	gb10b_ofa_clear_ec_regs(g);
}

static u32 gb10b_ofa_intr_nonstall_handler(struct gk20a *g, u64 cookie)
{
	u32 inst_id = nvgpu_safe_cast_u64_to_u32(cookie);

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "ofa%u nonstall irq stat: 0x%08x",
			inst_id, nvgpu_readl(g, pofa_falcon_irqstat_r()));

	return (NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE |
			NVGPU_CIC_NONSTALL_OPS_POST_EVENTS);
}

void gb10b_ofa_get_intr_ctrl_msg(struct gk20a *g, u32 inst_id, bool enable,
							u32 *intr_ctrl, u32 *intr_notify_ctrl)
{
	u32 stall_vector = 0U;
	u32 nonstall_vector = 0U;

	nvgpu_assert(inst_id == 0U);

	stall_vector = nvgpu_gin_get_unit_stall_vector(g,
				NVGPU_GIN_INTR_UNIT_OFA,
				inst_id + NVGPU_OFA_LEGACY_VECTOR_OFFSET);
	nonstall_vector = nvgpu_gin_get_unit_nonstall_vector(g,
				NVGPU_GIN_INTR_UNIT_OFA, inst_id);

	if (enable) {
		/* Set stall and nonstall interrupt handlers */
		nvgpu_gin_set_stall_handler(g, stall_vector,
				&gb10b_ofa_intr_stall_handler, inst_id);
		nvgpu_gin_set_nonstall_handler(g, nonstall_vector,
				&gb10b_ofa_intr_nonstall_handler, inst_id);
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

void gb10b_ofa_set_ecc_intr(struct gk20a *g, u32 inst_id, bool enable)
{
	u32 ue_vector = 0U;
	u32 ce_vector = 0U;
	u32 intr_ctrl2 = 0U;
	u32 intr_ctrl3 = 0U;

	ue_vector = nvgpu_gin_get_unit_stall_vector(g,
				NVGPU_GIN_INTR_UNIT_OFA, inst_id + NVGPU_OFA_UE_VECTOR_OFFSET);
	ce_vector = nvgpu_gin_get_unit_stall_vector(g,
				NVGPU_GIN_INTR_UNIT_OFA, inst_id + NVGPU_OFA_CE_VECTOR_OFFSET);

	if (enable) {
		/* Set corrected and uncorrected interrupt handlers */
		nvgpu_gin_set_stall_handler(g, ue_vector,
				&gb10b_ofa_ue_handler, inst_id);
		nvgpu_gin_set_stall_handler(g, ce_vector,
				&gb10b_ofa_ce_handler, inst_id);
		/* Enable interrupt to cpu, disable stalling interrupt to gsp */
		intr_ctrl2 = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
				NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
		intr_ctrl3 = nvgpu_gin_get_intr_ctrl_msg(g, ce_vector,
				NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	} else {
		intr_ctrl2 = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
				NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
		intr_ctrl3 = nvgpu_gin_get_intr_ctrl_msg(g, ce_vector,
				NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
	}

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr,
			"ofa intr %s ctrl2 0x%x ctrl3 0x%x",
			enable ? "enable" : "disable",
			intr_ctrl2, intr_ctrl3);

	nvgpu_writel(g, pofa_falcon_intr_ctrl2_r(), intr_ctrl2);
	nvgpu_writel(g, pofa_falcon_intr_ctrl3_r(), intr_ctrl3);
}

void gb10b_ofa_enable_irq(struct gk20a *g, u32 inst_id, bool enable)
{
	u32 stall_offset = 0U;
	u32 nonstall_offset = 1U;
	u32 intr_ctrl = 0U;
	u32 intr_notify_ctrl = 0U;

	nvgpu_assert(inst_id == 0U);

	/* For every instance of OFA, we have 3 stalling vectors -
	 * 1. Legacy stall line
	 * 2. Uncorrected(UE) line
	 * 3. Corrected(CE) line.
	 */

	if (g->ops.ofa.get_intr_ctrl_msg != NULL) {
		g->ops.ofa.get_intr_ctrl_msg(g, inst_id, enable,
		&intr_ctrl, &intr_notify_ctrl);
	} else {
		nvgpu_err(g, "Un-supported OFA interrupt msg ctrl");
	}

	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr,
			"ofa intr %s ctrl 0x%x notify_ctrl 0x%x",
			enable ? "enable" : "disable", intr_ctrl,
			intr_notify_ctrl);

	nvgpu_writel(g, pofa_falcon_intr_ctrl_r(stall_offset), intr_ctrl);
	nvgpu_writel(g, pofa_falcon_intr_ctrl_r(nonstall_offset), intr_notify_ctrl);

	if (g->ops.ofa.set_ecc_intr != NULL) {
		g->ops.ofa.set_ecc_intr(g, inst_id, enable);
	} else {
		nvgpu_err(g, "Un-supported OFA ECC interrupt ctrl");
	}
}

void gb10b_ofa_isr(struct gk20a *g, u32 inst_id)
{
	u32 intr_stat = 0U;

	nvgpu_assert(inst_id == 0U);

	intr_stat = nvgpu_readl(g, pofa_falcon_irqstat_r());
	nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr, "ofa irqstat 0x%08x", intr_stat);

	nvgpu_writel(g, pofa_falcon_irqsclr_r(), intr_stat);
	if ((intr_stat & (pofa_falcon_irqstat_swgen0_true_f() |
				pofa_falcon_irqstat_swgen1_true_f())) != 0U) {
		nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr,
			"ofa%u swgen0/1 interrupt", inst_id);
		nvgpu_log(g, gpu_dbg_mme | gpu_dbg_intr,
			"ofa%u mailbox0: 0x%x mailbox1: 0x%x", inst_id,
			nvgpu_readl(g, pofa_falcon_mailbox1_r()),
			nvgpu_readl(g, pofa_falcon_mailbox0_r()));
	} else if ((intr_stat & pofa_falcon_irqstat_halt_true_f()) != 0U) {
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_OFA,
						GPU_OFA_UNCORRECTED_ERR);
		nvgpu_err(g, "ofa%u fatal halt interrupt", inst_id);
		nvgpu_err(g, "ofa%u mailbox0: 0x%x mailbox1: 0x%x", inst_id,
			nvgpu_readl(g, pofa_falcon_mailbox1_r()),
			nvgpu_readl(g, pofa_falcon_mailbox0_r()));
		nvgpu_multimedia_debug_dump(g, nvgpu_device_get(g, NVGPU_DEVTYPE_OFA, inst_id));
		/* TODO: Trigger recovery (engine reset) */
	} else {
		/* Report the error to safety services */
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_OFA,
						GPU_OFA_UNCORRECTED_ERR);
		nvgpu_err(g, "Un-supported or Un-handled ofa interrupts: stat:0x%08x", intr_stat);
		nvgpu_multimedia_debug_dump(g, nvgpu_device_get(g, NVGPU_DEVTYPE_OFA, inst_id));
		/* TODO: Error notifier, quiesce or recovery/reset as needed */
	}
}

void gb10b_ofa_fw(struct gk20a *g,
			struct nvgpu_firmware **desc_fw,
			struct nvgpu_firmware **image_fw,
			struct nvgpu_firmware **pkc_sig)
{

	*desc_fw = nvgpu_request_firmware(g, GB10B_OFA_RISCV_EB_UCODE_DESC,
					NVGPU_REQUEST_FIRMWARE_NO_WARN);

	if (nvgpu_acr_is_lsb_v3_supported(g)) {
		*pkc_sig = nvgpu_request_firmware(g,
				GB10B_OFA_RISCV_EB_UCODE_LSB_PKC_SIG,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	} else {
		*pkc_sig = nvgpu_request_firmware(g,
				GB10B_OFA_RISCV_EB_UCODE_PKC_SIG,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	}

	if (g->ops.pmu.is_debug_mode_enabled(g)) {
		*image_fw = nvgpu_request_firmware(g,
				GB10B_OFA_RISCV_EB_UCODE_IMAGE,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);

	} else {
		*image_fw = nvgpu_request_firmware(g,
				GB10B_OFA_RISCV_EB_UCODE_PROD_IMAGE,
				NVGPU_REQUEST_FIRMWARE_NO_WARN);
	}
}
