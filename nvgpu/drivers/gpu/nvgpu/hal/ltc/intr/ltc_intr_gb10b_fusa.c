// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gin.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/nvgpu_init.h>

#include "ltc_intr_gb10b.h"

#include <nvgpu/hw/gb10b/hw_func_gb10b.h>
#include <nvgpu/hw/gb10b/hw_ltc_gb10b.h>

u32 gb10b_ltc_intr_read_intr1(struct gk20a *g)
{
	return nvgpu_readl(g, ltc_ltcs_ltss_intr_r());
}

u32 gb10b_ltc_intr_read_intr2(struct gk20a *g)
{
	return nvgpu_readl(g, ltc_ltcs_ltss_intr2_r());
}

u32 gb10b_ltc_intr_read_intr3(struct gk20a *g)
{
	return nvgpu_readl(g, ltc_ltcs_ltss_intr3_r());
}

void gb10b_ltc_intr_write_intr1(struct gk20a *g, u32 reg_val)
{
	nvgpu_writel(g, ltc_ltcs_ltss_intr_r(), reg_val);
}

void gb10b_ltc_intr_write_intr2(struct gk20a *g, u32 reg_val)
{
	nvgpu_writel(g, ltc_ltcs_ltss_intr2_r(), reg_val);
}

void gb10b_ltc_intr_write_intr3(struct gk20a *g, u32 reg_val)
{
	nvgpu_writel(g, ltc_ltcs_ltss_intr3_r(), reg_val);
}

void gb10b_ltc_intr_report_non_ecc_uncorrected_err(struct gk20a *g)
{
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_LTC,
			GPU_LTC_CACHE_UNCORRECTED_ERROR);
}

static void gb10b_ltc_intr_stall_handler(struct gk20a *g, u64 cookie)
{
	(void)cookie;
	g->ops.mc.ltc_isr(g);
}

void gb10b_ltc_intr_configure(struct gk20a *g)
{
	u32 legacy_vector = 0U;
	u32 ue_vector = 0U;
	u32 ce_vector = 0U;
	u32 intr_ctrl_msg = 0U;
	u32 intr2_ctrl_msg = 0U;
	u32 intr3_ctrl_msg = 0U;

	if (g->ops.ltc.intr.configure_intr1 != NULL) {
		g->ops.ltc.intr.configure_intr1(g);
	}

	if (g->ops.ltc.intr.configure_intr2 != NULL) {
		g->ops.ltc.intr.configure_intr2(g);
	}

	if (g->ops.ltc.intr.configure_intr3 != NULL) {
		g->ops.ltc.intr.configure_intr3(g);
	}

	legacy_vector = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_LTC, NVGPU_LTC_LEGACY_VECTOR_OFFSET);
	ue_vector = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_LTC, NVGPU_LTC_UE_VECTOR_OFFSET);
	ce_vector = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_LTC, NVGPU_LTC_CE_VECTOR_OFFSET);

	nvgpu_gin_set_stall_handler(g, legacy_vector, &gb10b_ltc_intr_stall_handler, 0);
	nvgpu_gin_set_stall_handler(g, ue_vector, &gb10b_ltc_intr_stall_handler, 0);
	nvgpu_gin_set_stall_handler(g, ce_vector, &gb10b_ltc_intr_stall_handler, 0);

	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, legacy_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	/* intr2_ctrl is tied to corrected error tree in HW */
	intr2_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, ce_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	/* intr3_ctrl is tied to uncorrected error tree in HW */
	intr3_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);

	nvgpu_writel(g, ltc_ltcs_intr_ctrl_r(), intr_ctrl_msg);
	nvgpu_writel(g, ltc_ltcs_intr2_ctrl_r(), intr2_ctrl_msg);
	nvgpu_writel(g, ltc_ltcs_intr3_ctrl_r(), intr3_ctrl_msg);
}

void gb10b_ltc_intr_retrigger(struct gk20a *g)
{
	nvgpu_writel(g, ltc_ltcs_intr_retrigger_r(),
			ltc_ltcs_intr_retrigger_trigger_true_f());
}

void gb10b_ltc_intr1_configure(struct gk20a *g)
{
	u32 reg;

	reg = g->ops.ltc.intr.read_intr1(g);

	/* IDLE_ERROR_IQ - unused in HWW, hence disabled */
	reg = set_field(reg, ltc_ltcs_ltss_intr_en_idle_error_iq_m(),
		ltc_ltcs_ltss_intr_en_idle_error_iq_disabled_f());

	/*
	 * IDLE_ERROR_CBC - flag if cbc gets a request while slcg clock is
	 * disabled
	 */
	reg = set_field(reg, ltc_ltcs_ltss_intr_en_idle_error_cbc_m(),
		ltc_ltcs_ltss_intr_en_idle_error_cbc_enabled_f());
	/*
	 * IDLE_ERROR_TSTG - flag if tstg gets a request while slcg clock is
	 * disabled
	 */
	reg = set_field(reg, ltc_ltcs_ltss_intr_en_idle_error_tstg_m(),
		ltc_ltcs_ltss_intr_en_idle_error_tstg_enabled_f());
	/*
	 * IDLE_ERROR_DSTG - flag if dstg gets a request while slcg clock is
	 * disabled
	 */
	reg = set_field(reg, ltc_ltcs_ltss_intr_en_idle_error_dstg_m(),
		ltc_ltcs_ltss_intr_en_idle_error_dstg_enabled_f());

	/*
	 * RECEIVED_NDERR_RD - ND error received on read response.
	 */

	reg = set_field(reg,
		ltc_ltcs_ltss_intr_en_received_nderr_rd_m(),
		ltc_ltcs_ltss_intr_en_received_nderr_rd_enabled_f());

	/*
	 * ECC_SEC_ERROR: now reported as ECC DSTG corrected errors
	 * ECC_DED ERROR: now reported as ECC DSTG uncorrected errors
	 *
	 * Note: SEC_ERROR and DED_ERROR bits are unused in ga10b, ga100.
	 * INTR_EN_ECC_SEC_ERROR and INTR_EN_ECC_DED_ERROR priv regs no
	 * longer control enabling/disabling SEC/DED error interrupt.
	 * These are now ECC fuse controlled.
	 */
	reg = set_field(reg, ltc_ltcs_ltss_intr_en_ecc_sec_error_m(),
		ltc_ltcs_ltss_intr_en_ecc_sec_error_disable_f());

	reg = set_field(reg, ltc_ltcs_ltss_intr_en_ecc_ded_error_m(),
		ltc_ltcs_ltss_intr_en_ecc_ded_error_disable_f());

	/* DEBUG: Unused in HW, default disabled in RTL. */
	reg = set_field(reg, ltc_ltcs_ltss_intr_en_debug_m(),
		ltc_ltcs_ltss_intr_en_debug_disable_f());

	/*
	 * ATOMIC_TO_Z - atomic to packing Z or S8.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr_en_atomic_to_z_m(),
		ltc_ltcs_ltss_intr_en_atomic_to_z_enabled_f());

	/*
	 * ILLEGAL_ATOMIC - unsupported atomic op and/or size received.
	 */
	reg = set_field(reg, ltc_ltcs_ltss_intr_en_illegal_atomic_m(),
		ltc_ltcs_ltss_intr_en_illegal_atomic_enabled_f());

	/*
	 * ILLEGAL_COMPSTAT_ACCESS - indicates that some memory access
	 * read/wrote into the memory space reserved for the compression bit
	 * carveout.
	 */
	reg = set_field(reg, ltc_ltcs_ltss_intr_en_illegal_compstat_access_m(),
		ltc_ltcs_ltss_intr_en_illegal_compstat_access_enabled_f());

	/*
	 * RECEIVED_NDERR_WR - ND error received on write response.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr_en_received_nderr_wr_m(),
		ltc_ltcs_ltss_intr_en_received_nderr_wr_enabled_f());

	/*
	* Following interrupts are debug interrupts valid for GB10B,
	* and should be enabled only for non-safety use.
	* Initially disabled here and re-enabled later under
	* NON-FUSA for clarity.
	*/

	/*
	 * EVICTED_CB - indicates that a CB was demoted.
	 * Normally this should not happen because the CBs should be flushed
	 * during context switch and/or invalidated when no longer used.
	 * This interrupt is disabled by default in RTL.
	 *
	 * Note: this occurs more frequently than expected, so is being left
	 * disabled as on previous chips and consistent with HW POR value.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr_en_evicted_cb_m(),
		ltc_ltcs_ltss_intr_en_evicted_cb_disabled_f());
	/*
	 * CBC_OUTSIDE_SAFE_SIZE -  indicates a
	 * CBC request has accessed outside the considered
	 * safe region of memory.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr_en_cbc_outside_safe_size_m(),
		ltc_ltcs_ltss_intr_en_cbc_outside_safe_size_disabled_f());

	/*
	 * BLKACTIVITY_ERR - internal error in power sensing block activity
	 * monitor.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr_en_blkactivity_err_m(),
		ltc_ltcs_ltss_intr_en_blkactivity_err_disable_f());

	/* Enable above debug interrupts under NON FUSA */

	#ifdef CONFIG_NVGPU_NON_FUSA
	reg = set_field(reg,
		ltc_ltcs_ltss_intr_en_cbc_outside_safe_size_m(),
		ltc_ltcs_ltss_intr_en_cbc_outside_safe_size_enabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr_en_blkactivity_err_m(),
		ltc_ltcs_ltss_intr_en_blkactivity_err_enabled_f());
	#endif

	g->ops.ltc.intr.write_intr1(g, reg);

	/* Read back register for write synchronization */
	g->ops.ltc.intr.read_intr1(g);
}

void gb10b_ltc_intr2_configure(struct gk20a *g)
{
	u32 reg;

	reg = g->ops.ltc.intr.read_intr2(g);

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_cache_coh_access_compressible_m(),
		ltc_ltcs_ltss_intr2_en_cache_coh_access_compressible_enabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_unsupported_chi_req_m(),
		ltc_ltcs_ltss_intr2_en_unsupported_chi_req_enabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_snp_sys_aperture_mismatch_m(),
		ltc_ltcs_ltss_intr2_en_snp_sys_aperture_mismatch_enabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_invalid_rs_subid_m(),
		ltc_ltcs_ltss_intr2_en_invalid_rs_subid_enabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_unexpected_rs_type_m(),
		ltc_ltcs_ltss_intr2_en_unexpected_rs_type_enabled_f());

	/* This intr is unused and disabled by default in HW */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_data_mismatch_mask_subid_m(),
		ltc_ltcs_ltss_intr2_en_data_mismatch_mask_subid_disabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_hsh2ltc_unexpected_data_m(),
		ltc_ltcs_ltss_intr2_en_hsh2ltc_unexpected_data_enabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_sysfill_bypass_invalid_subid_m(),
		ltc_ltcs_ltss_intr2_en_sysfill_bypass_invalid_subid_enabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_unexpected_sysfill_bypass_m(),
		ltc_ltcs_ltss_intr2_en_unexpected_sysfill_bypass_enabled_f());

	/*
	* Following interrupts are debug interrupts valid for GB10B,
	* and should be enabled only for non-safety use.
	* Initially disabled here and re-enabled later under
	* NON-FUSA for clarity.
	*/
	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_cache_coh_access_packed_kind_m(),
		ltc_ltcs_ltss_intr2_en_cache_coh_access_packed_kind_disable_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_sys_aperture_mismatch_m(),
		ltc_ltcs_ltss_intr2_en_sys_aperture_mismatch_disable_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_prefetch_sys_aperture_mismatch_m(),
		ltc_ltcs_ltss_intr2_en_prefetch_sys_aperture_mismatch_disable_f());

	/* Enable above debug interrupts under NON FUSA */

	#ifdef CONFIG_NVGPU_NON_FUSA
	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_cache_coh_access_packed_kind_m(),
		ltc_ltcs_ltss_intr2_en_cache_coh_access_packed_kind_enabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_sys_aperture_mismatch_m(),
		ltc_ltcs_ltss_intr2_en_sys_aperture_mismatch_enabled_f());

	reg = set_field(reg,
		ltc_ltcs_ltss_intr2_en_prefetch_sys_aperture_mismatch_m(),
		ltc_ltcs_ltss_intr2_en_prefetch_sys_aperture_mismatch_enabled_f());
	#endif

	g->ops.ltc.intr.write_intr2(g, reg);

	/* Read back register for write synchronization */
	reg = g->ops.ltc.intr.read_intr2(g);
}

void gb10b_ltc_intr3_configure(struct gk20a *g)
{
	u32 reg;

	reg = g->ops.ltc.intr.read_intr3(g);
	/*
	 * ECC_CORRECTED and ECC_UNCORRECTED error bits are
	 * now fuse controlled and not controllable by SW.
	 */
	/*
	 * Following interrupts are debug interrupts.
	 * These are disabled by default in HW.
	 * Should be enabled only on non-safety.
	 */
	 #ifdef CONFIG_NVGPU_NON_FUSA
	/*
	 * VOLATILE_TO_COMPRESSED - A volatile transaction to compressed data
	 * detected.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr3_en_volatile_to_compressed_m(),
		ltc_ltcs_ltss_intr3_en_volatile_to_compressed_enabled_f());

	/*
	 * ILLEGAL_ACCESS_KIND_TYPE1 - A read request kind does not match the
	 * CBC stored kind type1.
	 * 1. Access kind is not color and stored kind is color with compression
	 *    status non-zero.
	 * 2. Access kind is color and stored kind is not color and the request
	 *    is from CROP.
	 * 3. Access kind is z and stored kind is different z.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr3_en_illegal_access_kind_type1_m(),
		ltc_ltcs_ltss_intr3_en_illegal_access_kind_type1_enabled_f());

	/*
	 * ILLEGAL_ACCESS_KIND_TYPE2 - A read request kind does not match the
	 * CBC stored kind type2.
	 * 1. Access kind is not color and stored kind is color with compression
	 *    status zero.
	 * 2. Access kind is color and stored kind is not color and the request
	 *    is from XBAR.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr3_en_illegal_access_kind_type2_m(),
		ltc_ltcs_ltss_intr3_en_illegal_access_kind_type2_enabled_f());

	/*
	 * DTM_KIND_INVALID - If the kind of a comp stat req read or packed
	 * read is invalid or pitch, the inter3_dtm_kind_invalid interrupt will
	 * assert if enabled.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr3_en_dtm_kind_invalid_m(),
		ltc_ltcs_ltss_intr3_en_dtm_kind_invalid_enabled_f());

	/*
	 * DTM_COMPTAG_INVALID - If the comptag_vld of a comp stat req read or
	 * packed read is invalid, the inter3_dtm_comptag_invalid interrupt will
	 * assert if enabled.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr3_en_dtm_comptag_invalid_m(),
		ltc_ltcs_ltss_intr3_en_dtm_comptag_invalid_enabled_f());
	#endif

	/* Following interrupts are not valid for GB10B, hence disabled */
	/*
	 * CHECKEDOUT_OTHER_NCNP_REQ - NCNP request on other vc (not NINB or CREQ),
	 * mainly ROP on BL,U_CREQ, to a cpu checked out line.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr3_en_checkedout_other_ncnp_req_m(),
		ltc_ltcs_ltss_intr3_en_checkedout_other_ncnp_req_disabled_f());

	/*
	 * CHECKEDOUT_ACCESS_PACKED_KIND - packed kind access to cpu checked out
	 * line.
	 */
	reg = set_field(reg,
		ltc_ltcs_ltss_intr3_en_checkedout_access_packed_kind_m(),
		ltc_ltcs_ltss_intr3_en_checkedout_access_packed_kind_disabled_f());

	g->ops.ltc.intr.write_intr3(g, reg);

	/* Read back register for write synchronization */
	reg = g->ops.ltc.intr.read_intr3(g);
}

void gb10b_ltc_intr_handle_lts_intr2(struct gk20a *g, u32 ltc, u32 slice, bool *needs_quiesce)
{
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);

	u32 offset = nvgpu_safe_add_u32(nvgpu_safe_mult_u32(ltc_stride, ltc),
					nvgpu_safe_mult_u32(lts_stride, slice));
	u32 ltc_intr2 = nvgpu_readl(g, nvgpu_safe_add_u32(
					ltc_ltc0_lts0_intr2_r(), offset));
	u32 ltc_intr2_mask = 0U;

	ltc_intr2_mask =
#ifdef CONFIG_NVGPU_NON_FUSA
			GB10B_DEBUG_LTC_INTR2 |
#endif
			GB10B_FATAL_LTC_INTR2;

	/* LSB 16 bits represent the interrupt status and MSB 16 bits denote
	 * interrupt enable state. Filter LSB 16 bits to check for actual
	 * interrupt status.
	 * Based on HW design, all LTC interrupts can set their corresponding
	 * status bits but SW can control the propagation of interrupts to
	 * the CPU using its corresponding LTC_INTR_EN bit.
	 * Return early if no enabled interrupts are active
	 * in the intr status bits.
	 */

	if (((ltc_intr2 & ltc_ltcs_ltss_intr2_reset_bit_mask_v()) &
			ltc_intr2_mask) == 0U) {
		return;
	}

	nvgpu_log(g, gpu_dbg_intr,
		"LTC:%d, LTS:%d LTC_INTR2:0x%x, LTC_INTR2_MASK: 0x%08x",
		ltc, slice, ltc_intr2, ltc_intr2_mask);

	/* Check for fatal LTC errors */
	if ((ltc_intr2 & GB10B_FATAL_LTC_INTR2) != 0U) {
		nvgpu_err(g, "LTC Uncorrected Error: LTC [%d] Slice [%d] "
			"ltc_lts_intr2 reg [0x%08x]", ltc, slice, ltc_intr2);

		/* Report uncorrected LTC errors */
		if (g->ops.ltc.intr.report_non_ecc_uncorrected_err != NULL) {
			g->ops.ltc.intr.report_non_ecc_uncorrected_err(g);
		}
		*needs_quiesce |= true;
	}

	/* Log debug interrupt */
	#ifdef CONFIG_NVGPU_NON_FUSA
	if ((ltc_intr2 & GB10B_DEBUG_LTC_INTR2) != 0U) {
		nvgpu_log(g, gpu_dbg_intr,
			"ltc:%d lts:%d LTC debug interrupt, ltc_intr2: 0x%08x",
			ltc, slice, ltc_intr2);
	}
	#endif

	/* reset interrupts*/
	nvgpu_writel(g, nvgpu_safe_add_u32(ltc_ltc0_lts0_intr2_r(), offset),
			ltc_intr2);
}

void gb10b_xbar_handle_intfc_ecc_intr(struct gk20a *g, u32 ltc, u32 slice, u32 ecc_status,
				u32 ecc_addr, bool *needs_quiesce)
{
	bool is_intfc_ecc_addr = (ltc_ltc0_lts0_l2_cache_ecc_address_subunit_v(ecc_addr) ==
				 ltc_ltc0_lts0_l2_cache_ecc_address_subunit_intfc_v());

	if ((ecc_status & ltc_ltc0_lts0_l2_cache_ecc_status_uncorrected_err_intfc_m()) != 0U) {
		nvgpu_err(g, "LTC: INTFC ECC Uncorrected Error on LTC%d LTS%d", ltc, slice);

		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_LTC,
			GPU_LTC_CACHE_XBAR_INTFC_ECC_UNCORRECTED);
		*needs_quiesce |= true;

		if (!is_intfc_ecc_addr) {
			nvgpu_err(g, "ECC address doesn't belong to L2 INTFC");
			/*
			 * Above nvgpu_report_err_to_sdl() call covers
			 * error reporting in case of BUG().
			 */
			BUG();
		}
	}
	if ((ecc_status & ltc_ltc0_lts0_l2_cache_ecc_status_corrected_err_intfc_m()) != 0U) {
		nvgpu_err(g, "LTC: INTFC ECC Corrected Error");
		/*
		 * This error is not expected to occur in gb10b and hence,
		 * this scenario is considered as a fatal error.
		 */
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_LTC,
			GPU_LTC_CACHE_UNSUPPORTED_ECC_ERROR);
		BUG();
	}
}
void gb10b_ltc_intr_handle_lts_intr3(struct gk20a *g, u32 ltc, u32 slice, bool *needs_quiesce)
{
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);

	u32 offset = nvgpu_safe_add_u32(nvgpu_safe_mult_u32(ltc_stride, ltc),
					nvgpu_safe_mult_u32(lts_stride, slice));
	u32 ltc_intr3 = nvgpu_readl(g, nvgpu_safe_add_u32(
					ltc_ltc0_lts0_intr3_r(), offset));
	u32 ltc_intr3_mask = 0U;

	ltc_intr3_mask =
#ifdef CONFIG_NVGPU_NON_FUSA
			GB10B_DEBUG_LTC_INTR3 |
#endif
			GB10B_FATAL_LTC_INTR3;

	/* LSB 16 bits represent the interrupt status and MSB 16 bits denote
	 * interrupt enable state. Filter LSB 16 bits to check for actual
	 * interrupt status.
	 * Based on HW design, all LTC interrupts can set their corresponding
	 * status bits but SW can control the propagation of interrupts to
	 * the CPU using its corresponding LTC_INTR_EN bit.
	 * Return early if no enabled interrupts are active
	 * in the intr status bits.
	 */

	if (((ltc_intr3 & ltc_ltcs_ltss_intr3_reset_bit_mask_v()) &
			ltc_intr3_mask) == 0U) {
		return;
	}

	nvgpu_log(g, gpu_dbg_intr,
		"LTC:%d, LTS:%d LTC_INTR3:0x%x, LTC_INTR3_MASK: 0x%08x",
		ltc, slice, ltc_intr3, ltc_intr3_mask);

	if (g->ops.ltc.intr.handle_ecc_intr3 != NULL) {
		g->ops.ltc.intr.handle_ecc_intr3(g, ltc, slice, offset, ltc_intr3,
				needs_quiesce);
	}

	#ifdef CONFIG_NVGPU_NON_FUSA
	/* All the interrupts below are debug interrupts */
	if ((ltc_intr3 & GB10B_DEBUG_LTC_INTR3) != 0U) {
		nvgpu_log(g, gpu_dbg_intr,
		"ltc:%d lts:%d LTC debug interrupt, ltc_intr3: 0x%08x",
		ltc, slice, ltc_intr3);
	}
	#endif

	/* Reset interrupts */
	nvgpu_writel(g, nvgpu_safe_add_u32(ltc_ltc0_lts0_intr3_r(), offset),
			ltc_intr3);
}
void gb10b_ltc_intr_handle_lts_intr(struct gk20a *g, u32 ltc, u32 slice, bool *needs_quiesce)
{
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);

	u32 offset = nvgpu_safe_add_u32(nvgpu_safe_mult_u32(ltc_stride, ltc),
					nvgpu_safe_mult_u32(lts_stride, slice));
	u32 ltc_intr = nvgpu_readl(g, nvgpu_safe_add_u32(
					ltc_ltc0_lts0_intr_r(), offset));
	u32 ltc_intr_mask = 0U;

	ltc_intr_mask =
#ifdef CONFIG_NVGPU_NON_FUSA
			GB10B_DEBUG_LTC_INTR |
#endif
			GB10B_FATAL_LTC_INTR;

	/* LSB 16 bits represent the interrupt status and MSB 16 bits denote
	 * interrupt enable state. Filter LSB 16 bits to check for actual
	 * interrupt status.
	 * Based on HW design, all LTC interrupts can set their corresponding
	 * status bits but SW can control the propagation of interrupts to
	 * the CPU using its corresponding LTC_INTR_EN bit.
	 * Return early if no enabled interrupts are active
	 * in the intr status bits.
	 */

	if (((ltc_intr & ltc_ltcs_ltss_intr_reset_bit_mask_v()) &
			ltc_intr_mask) == 0U) {
		return;
	}

	nvgpu_log(g, gpu_dbg_intr,
		"LTC:%d, LTS: %d, LTC_INTR:0x%x, ltc_intr_mask: 0x%08x",
		ltc, slice, ltc_intr, ltc_intr_mask);

	/* Check for fatal LTC errors */
	if ((ltc_intr & GB10B_FATAL_LTC_INTR) != 0U) {
		nvgpu_err(g, "LTC Uncorrected Error: LTC [%d] Slice [%d] "
			"ltc_lts_intr reg [0x%08x]", ltc, slice, ltc_intr);

		/* Report uncorrected LTC errors */
		if (g->ops.ltc.intr.report_non_ecc_uncorrected_err != NULL) {
			g->ops.ltc.intr.report_non_ecc_uncorrected_err(g);
		}
		*needs_quiesce |= true;
	}

	/* Log debug interrupts */
	#ifdef CONFIG_NVGPU_NON_FUSA
	if ((ltc_intr & GB10B_DEBUG_LTC_INTR) != 0U) {
		nvgpu_log(g, gpu_dbg_intr, "LTC Debug Interrupt: LTC [%d] Slice [%d] "
			"ltc_lts_intr reg [0x%08x]", ltc, slice, ltc_intr);
	}
	#endif

	/* reset interrupts*/
	nvgpu_writel(g, nvgpu_safe_add_u32(ltc_ltc0_lts0_intr_r(), offset),
		ltc_intr);
}
