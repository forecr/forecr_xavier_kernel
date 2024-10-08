/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <nvgpu/falcon.h>
#include <nvgpu/mm.h>
#include <nvgpu/io.h>
#include <nvgpu/timers.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/gsp.h>
#ifdef CONFIG_NVGPU_GSP_SCHEDULER
#include <nvgpu/gsp.h>
#include <nvgpu/string.h>
#endif
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

int ga10b_gsp_engine_reset(struct gk20a *g)
{
	gk20a_writel(g, pgsp_falcon_engine_r(),
		pgsp_falcon_engine_reset_true_f());
	nvgpu_udelay(10);
	gk20a_writel(g, pgsp_falcon_engine_r(),
		pgsp_falcon_engine_reset_false_f());

	/* Load SLCG prod values for GSP */
	nvgpu_cg_slcg_gsp_load_enable(g, true);

	return 0;
}

static int ga10b_gsp_handle_ecc(struct gk20a *g, u32 ecc_status, u32 err_module)
{
	int ret = 0;

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
	u32 ecc_status;

	ecc_status = nvgpu_readl(g, pgsp_falcon_ecc_status_r());

	return ((ga10b_gsp_handle_ecc(g, ecc_status, NVGPU_ERR_MODULE_GSP_ACR) == 0) ? true :
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

	if (tag_width_shift > 31) {
		nvgpu_err(g, "Invalid tag width shift, %u", tag_width_shift);
		status = -EINVAL;
		goto exit;
	}

	*start_emem = BIT32(tag_width_shift);


	if (end_emem == NULL) {
		goto exit;
	}

	*end_emem = *start_emem +
		((u32)pgsp_hwcfg_emem_size_f(gk20a_readl(g, pgsp_hwcfg_r()))
		* (u32)256U);

exit:
	return status;
}

#ifdef CONFIG_NVGPU_GSP_SCHEDULER
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

static u32 ga10b_gsp_get_irqmask(struct gk20a *g)
{
	return (gk20a_readl(g, pgsp_riscv_irqmask_r()) &
			gk20a_readl(g, pgsp_riscv_irqdest_r()));
}

static bool ga10b_gsp_is_interrupted(struct gk20a *g, u32 *intr)
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

static void ga10b_gsp_handle_swgen1_irq(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_FALCON_DEBUG
	int err = 0;
#endif
	struct nvgpu_falcon *flcn = NULL;

	nvgpu_log_fn(g, " ");

	flcn = nvgpu_gsp_falcon_instance(g);
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

static void ga10b_gsp_handle_halt_irq(struct gk20a *g)
{
	nvgpu_err(g, "GSP Halt Interrupt Fired");

#ifdef CONFIG_NVGPU_GSP_STRESS_TEST
	nvgpu_gsp_set_test_fail_status(g, true);
#endif
}

static void ga10b_gsp_clr_intr(struct gk20a *g, u32 intr)
{
	gk20a_writel(g, pgsp_falcon_irqsclr_r(), intr);
}

static void ga10b_gsp_handle_interrupts(struct gk20a *g, u32 intr)
{
#ifndef CONFIG_NVGPU_MON_PRESENT
	int err = 0;
#endif
	u32 ecc_status = 0U;

	nvgpu_log_fn(g, " ");

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
		ecc_status = nvgpu_readl(g, pgsp_falcon_ecc_status_r());
		if (ga10b_gsp_handle_ecc(g, ecc_status, NVGPU_ERR_MODULE_GSP_SCHED) != 0) {
			nvgpu_err(g, "nvgpu ecc error handling failed err=");
		}
	}

	if ((intr & pgsp_falcon_irqstat_iopmp_true_f()) != 0U) {
		nvgpu_err(g, "GSP Pri access failure IOPMP");
	}

	if ((intr & pgsp_falcon_irqstat_memerr_true_f()) != 0U) {
		nvgpu_err(g, "GSP Pri access failure MEMERR");
	}
}

void ga10b_gsp_isr(struct gk20a *g, struct nvgpu_gsp *gsp)
{
	u32 intr = 0U;
	u32 mask = 0U;

	nvgpu_log_fn(g, " ");

	if (!ga10b_gsp_is_interrupted(g, &intr)) {
		nvgpu_err(g, "GSP interrupt not supported stat:0x%08x", intr);
		return;
	}

	nvgpu_gsp_isr_mutex_acquire(g, gsp);
	if (!nvgpu_gsp_is_isr_enable(g, gsp)) {
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
	nvgpu_gsp_isr_mutex_release(g, gsp);
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
	dmem_addr -= start_emem;

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
#endif /* CONFIG_NVGPU_GSP_SCHEDULER */
