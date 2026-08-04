// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/falcon.h>
#include <nvgpu/mm.h>
#include <nvgpu/io.h>
#include <nvgpu/timers.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/gsp.h>
#include <nvgpu/acr.h>
#include <nvgpu/string.h>
#ifdef CONFIG_NVGPU_GSP_STRESS_TEST
#include <nvgpu/gsp/gsp_test.h>
#endif

#include "gsp_ga10b.h"

#include <nvgpu/hw/ga10b/hw_pgsp_ga10b.h>

u32 ga10b_gsp_falcon2_base_addr(void)
{
	return pgsp_falcon2_gsp_base_r();
}

u32 ga10b_gsp_falcon_base_addr(void)
{
	return pgsp_falcon_irqsset_r();
}

int ga10b_gsp_engine_reset(struct gk20a *g, u32 inst_id)
{
	nvgpu_assert(inst_id == 0U);

	gk20a_writel(g, pgsp_falcon_engine_r(),
		pgsp_falcon_engine_reset_true_f());
	nvgpu_udelay(10);
	gk20a_writel(g, pgsp_falcon_engine_r(),
		pgsp_falcon_engine_reset_false_f());

	/* Load SLCG prod values for GSP */
	nvgpu_cg_slcg_gsp_load_enable(g, true);

	return 0;
}

int ga10b_gsp_handle_ecc(struct gk20a *g, u32 err_module)
{
	int ret = 0;
	u32 ecc_status;

	ecc_status = nvgpu_readl(g, pgsp_falcon_ecc_status_r());

	if ((ecc_status &
		pgsp_falcon_ecc_status_uncorrected_err_imem_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, err_module,
					GPU_GSP_ACR_IMEM_ECC_UNCORRECTED);
		nvgpu_err(g, "imem ecc error uncorrected");
		ret = -EFAULT;
	}

	if ((ecc_status &
		pgsp_falcon_ecc_status_uncorrected_err_dmem_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, err_module,
					GPU_GSP_ACR_DMEM_ECC_UNCORRECTED);
		nvgpu_err(g, "dmem ecc error uncorrected");
		ret = -EFAULT;
	}

	if ((ecc_status &
		pgsp_falcon_ecc_status_uncorrected_err_dcls_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, err_module,
					GPU_GSP_ACR_DCLS_UNCORRECTED);
		nvgpu_err(g, "dcls ecc error uncorrected");
		ret = -EFAULT;
	}

	if ((ecc_status &
		pgsp_falcon_ecc_status_uncorrected_err_reg_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, err_module,
					GPU_GSP_ACR_REG_ECC_UNCORRECTED);
		nvgpu_err(g, "reg ecc error uncorrected");
		ret = -EFAULT;
	}

	if ((ecc_status &
		pgsp_falcon_ecc_status_uncorrected_err_emem_m()) != 0U) {
		nvgpu_report_err_to_sdl(g, err_module,
					GPU_GSP_ACR_EMEM_ECC_UNCORRECTED);
		nvgpu_err(g, "emem ecc error uncorrected");
		ret = -EFAULT;
	}

	return ret;
}

bool ga10b_gsp_validate_mem_integrity(struct gk20a *g)
{
	return ((ga10b_gsp_handle_ecc(g, NVGPU_ERR_MODULE_GSP_ACR) == 0) ? true :
			false);
}

bool ga10b_gsp_is_debug_mode_en(struct gk20a *g)
{
	u32 ctl_stat =  nvgpu_readl(g, pgsp_falcon_hwcfg2_r());

	if (pgsp_falcon_hwcfg2_dbgmode_v(ctl_stat) ==
		pgsp_falcon_hwcfg2_dbgmode_enable_v()) {
		nvgpu_gsp_dbg(g, "DEBUG MODE");
		return true;
	} else {
		nvgpu_gsp_dbg(g, "PROD MODE");
		return false;
	}
}

s32 ga10b_gsp_get_emem_boundaries(struct gk20a *g,
	u32 *start_emem, u32 *end_emem)
{
	u32 tag_width_shift = 0;
	int status = 0;
	/*
	 * EMEM is mapped at the top of DMEM VA space
	 * START_EMEM = DMEM_VA_MAX = 2^(DMEM_TAG_WIDTH + 8)
	 */
	if (start_emem == NULL) {
		status = -EINVAL;
		goto exit;
	}

	tag_width_shift = ((u32)pgsp_falcon_hwcfg1_dmem_tag_width_v(
			gk20a_readl(g, pgsp_falcon_hwcfg1_r())) + (u32)8U);

	if (tag_width_shift > 31U) {
		nvgpu_err(g, "Invalid tag width shift, %u", tag_width_shift);
		status = -EINVAL;
		goto exit;
	}

	*start_emem = BIT32(tag_width_shift);


	if (end_emem == NULL) {
		goto exit;
	}

	*end_emem = nvgpu_safe_add_u32(*start_emem,
				       nvgpu_safe_mult_u32((u32)pgsp_hwcfg_emem_size_f(gk20a_readl(g, pgsp_hwcfg_r())),
							    (u32)256U));

exit:
	return status;
}

/**
 * @brief Computes the mask of enabled GSP interrupts.
 *
 * This function calculates the mask of currently enabled GSP interrupts by
 * reading the GSP RISCV IRQ mask and destination registers and performing a
 * bitwise AND operation between them. The result indicates which interrupts
 * are both enabled and have a destination set.
 *
 * The steps performed by the function are as follows:
 * -# Read the GSP RISCV IRQ mask register using the gk20a_readl() function.
 * -# Read the GSP RISCV IRQ destination register using the gk20a_readl() function.
 * -# Perform a bitwise AND operation between the IRQ mask and IRQ destination
 *    values to determine the active IRQ mask.
 *
 * @param [in]  g  Pointer to the GPU driver struct.
 *
 * @return The mask of enabled GSP interrupts.
 */
static u32 ga10b_gsp_get_irqmask(struct gk20a *g)
{
	u32 pgsp_riscv_irqmask = gk20a_readl(g, pgsp_riscv_irqmask_r());
	u32 pgsp_riscv_irqdest = gk20a_readl(g, pgsp_riscv_irqdest_r());
	return (pgsp_riscv_irqmask & pgsp_riscv_irqdest);
}

bool ga10b_gsp_is_interrupted(struct gk20a *g, u32 *intr)
{
	u32 supported_gsp_int = 0U;
	u32 intr_stat = gk20a_readl(g, pgsp_falcon_irqstat_r());

	supported_gsp_int = pgsp_falcon_irqstat_halt_true_f() |
			pgsp_falcon_irqstat_swgen1_true_f()		|
			pgsp_falcon_irqstat_swgen0_true_f()		|
			pgsp_falcon_irqstat_wdtmr_true_f()		|
			pgsp_falcon_irqstat_extirq7_true_f()	|
			pgsp_falcon_irqstat_memerr_true_f()		|
			pgsp_falcon_irqstat_iopmp_true_f()		|
			pgsp_falcon_irqstat_exterr_true_f();

	*intr = intr_stat;

	if ((intr_stat & supported_gsp_int) != 0U) {
		return true;
	}

	return false;
}

/**
 * @brief Handles the SWGEN1 interrupt for the GSP.
 *
 * This function is responsible for handling the SWGEN1 (software generated interrupt 1)
 * for the GSP (Graphics System Processor). It retrieves the GSP falcon instance and
 * logs an error if the instance is not found.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GSP falcon instance using nvgpu_gsp_falcon_instance().
 * -# Check if the retrieved falcon instance is NULL.
 *    - If it is NULL, log an error message indicating that the GSP falcon instance
 *      was not found and return from the function.
 *
 * @param [in]  g  Pointer to the GPU driver struct.
 */
static void ga10b_gsp_handle_swgen1_irq(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_FALCON_DEBUG
	int err = 0;
#endif
	struct nvgpu_falcon *flcn = NULL;

	nvgpu_log_fn(g, " ");

	/*
	 * Currently in GSP ACR with IPC enabled, print buffer
	 * is processed using cmd queue and EMEM instead of
	 * DMEM based buffers. So, till we shift to DMEM buffers
	 * just set the msg queue head and tail as same, such that
	 * there's no queue related sanity check failures in GSP FMC
	 */
	if (g->is_acr_ipc_supported == true) {
		gk20a_writel(g, pgsp_queue_tail_r(7U),
			gk20a_readl(g, pgsp_queue_head_r(7U)));
		return;
	}

	flcn = &g->gsp_flcn;
	if (flcn == NULL) {
		nvgpu_err(g, "GSP falcon instance not found");
		return;
	}

#ifdef CONFIG_NVGPU_FALCON_DEBUG
	err = nvgpu_falcon_dbg_buf_display(flcn);
	if (err != 0) {
		nvgpu_err(g, "nvgpu_falcon_debug_buffer_display failed err=%d",
		err);
	}
#endif
}

/**
 * @brief Handles the halt interrupt for the GSP.
 *
 * This function is invoked when a halt interrupt is fired by the GSP (Graphics System Processor).
 * It logs the occurrence of the halt interrupt and, if GSP stress testing is enabled, sets the
 * test fail status.
 *
 * The steps performed by the function are as follows:
 * -# Log the message "GSP Halt Interrupt Fired" to indicate that a halt interrupt has occurred.
 * -# If the GSP stress test configuration is enabled (checked via #CONFIG_NVGPU_GSP_STRESS_TEST),
 *    call nvgpu_gsp_set_test_fail_status() to set the test fail status to true.
 *
 * @param [in]  g  Pointer to the GPU driver struct.
 */
static void ga10b_gsp_handle_halt_irq(struct gk20a *g)
{
	nvgpu_err(g, "GSP Halt Interrupt Fired");

#ifdef CONFIG_NVGPU_GSP_STRESS_TEST
	nvgpu_gsp_set_test_fail_status(g, true);
#endif
}

/**
 * @brief Clears the specified GSP interrupt.
 *
 * This function clears a GSP (Graphics System Processor) interrupt by writing to the
 * GSP falcon interrupt clear register. The specific interrupt to clear is indicated
 * by the 'intr' parameter.
 *
 * The steps performed by the function are as follows:
 * -# Write the interrupt identifier to the GSP falcon interrupt clear register using
 *    the gk20a_writel() function. This action clears the specified interrupt in the GSP.
 *
 * @param [in]  g     Pointer to the GPU driver struct.
 * @param [in]  intr  The identifier of the interrupt to clear.
 */
static void ga10b_gsp_clr_intr(struct gk20a *g, u32 intr)
{
	gk20a_writel(g, pgsp_falcon_irqsclr_r(), intr);
}

/**
 * @brief Handles various GSP interrupts.
 *
 * This function processes different types of interrupts generated by the GSP
 * (Graphics System Processor). It handles software-generated interrupts, halt
 * interrupts, external error interrupts, watchdog timer interrupts, ECC error
 * interrupts, and memory error interrupts. For each type of interrupt, it performs
 * specific actions, including logging errors, clearing interrupts, and processing
 * messages.
 *
 * The steps performed by the function are as follows:
 * -# Check if the SWGEN1 interrupt is set and if so, handle it using
 *    ga10b_gsp_handle_swgen1_irq().
 * -# Check if the halt interrupt is set and if so, handle it using
 *    ga10b_gsp_handle_halt_irq().
 * -# Check if the external error interrupt is set and if so, log an error message
 *    and clear the interrupt by writing to the external error status register.
 * -# Check if the watchdog timer interrupt is set and if so, log an error message
 *    and report the error using nvgpu_report_err_to_sdl().
 * -# If the MON is not present, check if the SWGEN0 interrupt is set and if so,
 *    handle it by processing the message using nvgpu_gsp_process_message().
 * -# Check if the ECC error interrupt is set and if so, log an error message,
 *    read the ECC status register, and handle the ECC error using
 *    ga10b_gsp_handle_ecc().
 * -# Check if the IOPMP error interrupt is set and if so, log an error message.
 * -# Check if the memory error interrupt is set and if so, log an error message.
 *
 * @param [in]  g     Pointer to the GPU driver struct.
 * @param [in]  intr  The bitmask of the current GSP interrupts.
 */
static void ga10b_gsp_handle_interrupts(struct gk20a *g, u32 intr)
{
#ifndef CONFIG_NVGPU_MON_PRESENT
	int err = 0;
#endif

	nvgpu_log_fn(g, " ");
	nvgpu_gsp_dbg(g, "acr_ipc_supported: stat:0x%x",
			g->is_acr_ipc_supported);
	/* swgen1 interrupt handle */
	if ((intr & pgsp_falcon_irqstat_swgen1_true_f()) != 0U) {
		ga10b_gsp_handle_swgen1_irq(g);
	}

	/* halt interrupt handle */
	if ((intr & pgsp_falcon_irqstat_halt_true_f()) != 0U) {
		ga10b_gsp_handle_halt_irq(g);
	}

	/* exterr interrupt handle */
	if ((intr & pgsp_falcon_irqstat_exterr_true_f()) != 0U) {
		nvgpu_err(g,
			"gsp exterr intr not implemented. Clearing interrupt.");

		nvgpu_writel(g, pgsp_falcon_exterrstat_r(),
			nvgpu_readl(g, pgsp_falcon_exterrstat_r()) &
				~pgsp_falcon_exterrstat_valid_m());
	}

	/* watchdog timer interrupt handle */
	if ((intr & pgsp_falcon_irqstat_wdtmr_true_f()) != 0U) {
		nvgpu_err(g, "gsp watchdog timeout.");
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_GSP_SCHED,
			GPU_GSP_SCHED_WDT_UNCORRECTED);
	}

#ifndef CONFIG_NVGPU_MON_PRESENT
	/* swgen0 interrupt handle */
	if ((intr & pgsp_falcon_irqstat_swgen0_true_f()) != 0U) {
		nvgpu_log_info(g, "Received swgen0 gsp intr: stat:0x%x",
			g->is_acr_ipc_supported);
		err = nvgpu_gsp_process_message(g);
		if (err != 0) {
			nvgpu_err(g, "nvgpu_gsp_process_message failed err=%d",
				err);
		}
	}
#endif

	/* handling ecc error by extirq7 */
	if ((intr & pgsp_falcon_irqstat_extirq7_true_f()) != 0U) {
		nvgpu_err(g, "ECC error detected.");
		if (g->ops.gsp.gsp_handle_ecc != NULL) {
			if (g->ops.gsp.gsp_handle_ecc(g, NVGPU_ERR_MODULE_GSP_SCHED) != 0) {
				nvgpu_err(g, "nvgpu ecc error handling failed err=");
			}
		}
	}

	if ((intr & pgsp_falcon_irqstat_iopmp_true_f()) != 0U) {
		nvgpu_err(g, "GSP Pri access failure IOPMP");
	}

	if ((intr & pgsp_falcon_irqstat_memerr_true_f()) != 0U) {
		nvgpu_err(g, "GSP Pri access failure MEMERR");
	}
}

void ga10b_gsp_isr(struct gk20a *g)
{
	u32 intr = 0U;
	u32 mask = 0U;

	nvgpu_log_fn(g, " ");

	/*
	 * Acquire the GSP ISR mutex to ensure exclusive access to the GSP
	 * interrupt handling process.
	 * This is done before any IRQ related register operation happens,
	 * such that there is no discrepancy in the IRQ state when the interrupt
	 * is being processed.
	 */
	nvgpu_gsp_isr_mutex_acquire(g, &g->gsp_flcn);

	/*
	 * If GSP is in privlockdown then IRQ registers will be locked
	 * and we should not process any interrupts when this happens
	 * as it can lead to 0xBADF5620 priv error
	 */
	if (g->ops.falcon.is_priv_lockdown(&g->gsp_flcn)) {
		nvgpu_gsp_dbg(g, "GSP privlockdown in ISR, returning");
		goto exit;
	}

	if (!ga10b_gsp_is_interrupted(g, &intr)) {
		if (intr != 0U) {
			nvgpu_err(g, "GSP interrupt not supported stat:0x%08x", intr);
		}
		goto exit;
	}

	if (!nvgpu_gsp_is_isr_enable(g, &g->gsp_flcn)) {
		goto exit;
	}

	mask = ga10b_gsp_get_irqmask(g);
	nvgpu_log_info(g, "received gsp interrupt: stat:0x%08x mask:0x%08x",
			intr, mask);

	if ((intr & mask) == 0U) {
		nvgpu_log_info(g,
			"clearing unhandled interrupt: stat:0x%08x mask:0x%08x",
			intr, mask);
		nvgpu_writel(g, pgsp_riscv_irqmclr_r(), intr);
		goto exit;
	}

	intr = intr & mask;
	ga10b_gsp_clr_intr(g, intr);

	ga10b_gsp_handle_interrupts(g, intr);

exit:
	nvgpu_gsp_isr_mutex_release(g, &g->gsp_flcn);
}

void ga10b_gsp_enable_irq(struct gk20a *g, bool enable)
{
	nvgpu_log_fn(g, " ");

	nvgpu_cic_mon_intr_stall_unit_config(g,
			NVGPU_CIC_INTR_UNIT_GSP, NVGPU_CIC_INTR_DISABLE);

	if (enable) {
		nvgpu_cic_mon_intr_stall_unit_config(g,
				NVGPU_CIC_INTR_UNIT_GSP, NVGPU_CIC_INTR_ENABLE);

		/* Configuring RISCV interrupts is expected to be done inside firmware */
	}
}

u32 ga10b_gsp_queue_head_r(u32 i)
{
	return pgsp_queue_head_r(i);
}

u32 ga10b_gsp_queue_head__size_1_v(void)
{
	return pgsp_queue_head__size_1_v();
}

u32 ga10b_gsp_queue_tail_r(u32 i)
{
	return pgsp_queue_tail_r(i);
}

u32 ga10b_gsp_queue_tail__size_1_v(void)
{
	return pgsp_queue_tail__size_1_v();
}

/**
 * @brief Validates the parameters for a GSP memory copy operation.
 *
 * This function checks if the parameters provided for a GSP memory copy operation
 * are valid. It verifies the size of the copy, the port number, the alignment of
 * the DMEM address, and whether the copy operation lies within the EMEM aperture.
 *
 * The steps performed by the function are as follows:
 * -# Check if the size of the copy is zero bytes. If so, log an error and set the
 *    status to -EINVAL, then go to exit.
 * -# Retrieve the maximum number of EMEM ports and check if the provided port number
 *    is within the supported range. If not, log an error and set the status to
 *    -EINVAL, then go to exit.
 * -# Check if the DMEM address is 4-byte aligned. If not, log an error and set the
 *    status to -EINVAL, then go to exit.
 * -# Retrieve the start and end boundaries of the EMEM by calling ga10b_gsp_get_emem_boundaries().
 *    If the function returns a non-zero status, go to exit.
 * -# Check if the DMEM address is within the EMEM aperture boundaries. If not, log an
 *    error and set the status to -EINVAL, then go to exit.
 * -# If all checks pass, return 0 indicating success.
 *
 * @param [in]  g              Pointer to the GPU driver struct.
 * @param [in]  dmem_addr      The DMEM address where the copy operation starts.
 * @param [in]  size_in_bytes  The size of the copy operation in bytes.
 * @param [in]  port           The port number to be used for the copy operation.
 *
 * @return 0 if all parameters are valid, -EINVAL if any parameter check fails.
 */
static int gsp_memcpy_params_check(struct gk20a *g, u32 dmem_addr,
	u32 size_in_bytes, u8 port)
{
	u8 max_emem_ports = (u8)pgsp_ememc__size_1_v();
	u32 start_emem = 0;
	u32 end_emem = 0;
	int status = 0;

	if (size_in_bytes == 0U) {
		nvgpu_err(g, "zero-byte copy requested");
		status = -EINVAL;
		goto exit;
	}

	if (port >= max_emem_ports) {
		nvgpu_err(g, "only %d ports supported. Accessed port=%d",
			max_emem_ports, port);
		status = -EINVAL;
		goto exit;
	}

	if ((dmem_addr & 0x3U) != 0U) {
		nvgpu_err(g, "offset (0x%08x) not 4-byte aligned", dmem_addr);
		status = -EINVAL;
		goto exit;
	}

	status = ga10b_gsp_get_emem_boundaries(g, &start_emem, &end_emem);
	if (status != 0) {
		goto exit;
	}

	if (dmem_addr < start_emem ||
		(dmem_addr + size_in_bytes) > end_emem) {
		nvgpu_err(g, "copy must be in emem aperature [0x%x, 0x%x]",
			start_emem, end_emem);
		status = -EINVAL;
		goto exit;
	}

	return 0;

exit:
	return status;
}

/**
 * @brief Transfers data to or from GSP EMEM.
 *
 * This function handles the transfer of data between system memory and GSP's External Memory (EMEM).
 * It performs parameter validation, calculates the EMEM offsets, and executes the memory transfer
 * operation either from system memory to EMEM or vice versa, based on the 'is_copy_from' flag.
 *
 * The steps performed by the function are as follows:
 * -# Validate the memory copy parameters by calling gsp_memcpy_params_check().
 *    - If validation fails, set the status to the error code and exit the function.
 * -# Retrieve the EMEMC and EMEMD register addresses for the specified port.
 * -# Retrieve the start boundary of the EMEM by calling ga10b_gsp_get_emem_boundaries().
 *    - If the function returns a non-zero status, exit the function.
 * -# Convert the DMEM address to an EMEM offset for use by EMEMC/EMEMD registers.
 * -# Prepare the EMEMC register value by masking with OFFSET and BLOCK masks and
 *    setting the auto-increment mode based on the direction of the copy.
 * -# Write the prepared EMEMC register value to the EMEMC register.
 * -# Calculate the number of words and remaining bytes to be copied.
 * -# Perform the word-wise copy operation:
 *    - If copying from EMEM, read each word from EMEM and store in the buffer.
 *    - If copying to EMEM, write each word from the buffer to EMEM.
 * -# If there are leftover bytes after the word-wise copy, handle them:
 *    - If copying from EMEM, read the remaining bytes, store them in the buffer.
 *    - If copying to EMEM, write the remaining bytes from the buffer to EMEM.
 * -# Return the status of the transfer operation.
 *
 * @param [in]  g              Pointer to the GPU driver struct.
 * @param [in]  dmem_addr      The DMEM address for the transfer.
 * @param [in]  buf            Pointer to the data buffer for the transfer.
 * @param [in]  size_in_bytes  The size of the data to be transferred in bytes.
 * @param [in]  port           The port number to be used for the transfer.
 * @param [in]  is_copy_from   Flag indicating the direction of the transfer.
 *
 * @return The status of the memory transfer operation. A zero value indicates success,
 *         while a non-zero value indicates an error.
 */
static int ga10b_gsp_emem_transfer(struct gk20a *g, u32 dmem_addr, u8 *buf,
	u32 size_in_bytes, u8 port, bool is_copy_from)
{
	u32 *data = (u32 *)(void *)buf;
	u32 num_words = 0;
	u32 num_bytes = 0;
	u32 start_emem = 0;
	u32 reg = 0;
	u32 i = 0;
	u32 emem_c_offset = 0;
	u32 emem_d_offset = 0;
	int status = 0;

	status = gsp_memcpy_params_check(g, dmem_addr, size_in_bytes, port);
	if (status != 0) {
		goto exit;
	}

	/*
	 * Get the EMEMC/D register addresses
	 * for the specified port
	 */
	emem_c_offset = pgsp_ememc_r(port);
	emem_d_offset = pgsp_ememd_r(port);

	/* Only start address needed */
	status = ga10b_gsp_get_emem_boundaries(g, &start_emem, NULL);
	if (status != 0) {
		goto exit;
	}

	/* Convert to emem offset for use by EMEMC/EMEMD */
	dmem_addr = nvgpu_safe_sub_u32(dmem_addr, start_emem);

	/* Mask off all but the OFFSET and BLOCK in EMEM offset */
	reg = dmem_addr & (pgsp_ememc_offs_m() |
		pgsp_ememc_blk_m());

	if (is_copy_from) {
		/* mark auto-increment on read */
		reg |= pgsp_ememc_aincr_m();
	} else {
		/* mark auto-increment on write */
		reg |= pgsp_ememc_aincw_m();
	}

	gk20a_writel(g, emem_c_offset, reg);

	/* Calculate the number of words and bytes */
	num_words = size_in_bytes >> 2U;
	num_bytes = size_in_bytes & 0x3U;

	/* Directly copy words to emem*/
	for (i = 0; i < num_words; i++) {
		if (is_copy_from) {
			data[i] = gk20a_readl(g, emem_d_offset);
		} else {
			gk20a_writel(g, emem_d_offset, data[i]);
		}
	}

	/* Check if there are leftover bytes to copy */
	if (num_bytes > 0U) {
		u32 bytes_copied = num_words << 2U;

		reg = gk20a_readl(g, emem_d_offset);
		if (is_copy_from) {
			nvgpu_memcpy((buf + bytes_copied), ((u8 *)&reg),
					num_bytes);
		} else {
			nvgpu_memcpy(((u8 *)&reg), (buf + bytes_copied),
					num_bytes);
			gk20a_writel(g, emem_d_offset, reg);
		}
	}

exit:
	return status;
}

int ga10b_gsp_flcn_copy_to_emem(struct gk20a *g,
	u32 dst, u8 *src, u32 size, u8 port)
{
	return ga10b_gsp_emem_transfer(g, dst, src, size, port, false);
}

int ga10b_gsp_flcn_copy_from_emem(struct gk20a *g,
	u32 src, u8 *dst, u32 size, u8 port)
{
	return ga10b_gsp_emem_transfer(g, src, dst, size, port, true);
}

void ga10b_gsp_flcn_setup_boot_config(struct gk20a *g)
{
	struct mm_gk20a *mm = &g->mm;
	u32 inst_block_ptr;

	nvgpu_log_fn(g, " ");

	/*
	 * The instance block address to write is the lower 32-bits of the 4K-
	 * aligned physical instance block address.
	 */
	inst_block_ptr = nvgpu_inst_block_ptr(g, &mm->gsp.inst_block);

	gk20a_writel(g, pgsp_falcon_nxtctx_r(),
		pgsp_falcon_nxtctx_ctxptr_f(inst_block_ptr) |
		pgsp_falcon_nxtctx_ctxvalid_f(1) |
		nvgpu_aperture_mask(g, &mm->gsp.inst_block,
			pgsp_falcon_nxtctx_ctxtgt_sys_ncoh_f(),
			pgsp_falcon_nxtctx_ctxtgt_sys_coh_f(),
			pgsp_falcon_nxtctx_ctxtgt_fb_f()));
}

int ga10b_gsp_queue_head(struct gk20a *g, u32 queue_id, u32 queue_index,
	u32 *head, bool set)
{
	u32 queue_head_size = 8;

	if (queue_id <= nvgpu_gsp_get_last_cmd_id(g)) {
		if (queue_index >= queue_head_size) {
			return -EINVAL;
		}

		if (!set) {
			*head = pgsp_queue_head_address_v(
				gk20a_readl(g, pgsp_queue_head_r(queue_index)));
		} else {
			gk20a_writel(g, pgsp_queue_head_r(queue_index),
				pgsp_queue_head_address_f(*head));
		}
	} else {
		if (!set) {
			*head = pgsp_msgq_head_val_v(
				gk20a_readl(g, pgsp_msgq_head_r(0U)));
		} else {
			gk20a_writel(g,
				pgsp_msgq_head_r(0U),
				pgsp_msgq_head_val_f(*head));
		}
	}

	return 0;
}

int ga10b_gsp_queue_tail(struct gk20a *g, u32 queue_id, u32 queue_index,
	u32 *tail, bool set)
{
	u32 queue_tail_size = 8;

	if (queue_id == nvgpu_gsp_get_last_cmd_id(g)) {
		if (queue_index >= queue_tail_size) {
			return -EINVAL;
		}

		if (!set) {
			*tail = pgsp_queue_tail_address_v(
				gk20a_readl(g, pgsp_queue_tail_r(queue_index)));
		} else {
			gk20a_writel(g,
				pgsp_queue_tail_r(queue_index),
				pgsp_queue_tail_address_f(*tail));
		}
	} else {
		if (!set) {
			*tail = pgsp_msgq_tail_val_v(
				gk20a_readl(g, pgsp_msgq_tail_r(0U)));
		} else {
			gk20a_writel(g, pgsp_msgq_tail_r(0U),
				pgsp_msgq_tail_val_f(*tail));
		}
	}

	return 0;
}

void ga10b_gsp_msgq_tail(struct gk20a *g, struct nvgpu_gsp *gsp,
	u32 *tail, bool set)
{
	if (!set) {
		*tail = gk20a_readl(g, pgsp_msgq_tail_r(0U));
	} else {
		gk20a_writel(g, pgsp_msgq_tail_r(0U), *tail);
	}
	(void)gsp;
}

void ga10b_gsp_set_msg_intr(struct gk20a *g)
{
	gk20a_writel(g, pgsp_riscv_irqmset_r(),
		pgsp_riscv_irqmset_swgen0_f(1));
}

