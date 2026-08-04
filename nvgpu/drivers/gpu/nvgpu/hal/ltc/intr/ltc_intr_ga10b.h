/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_INTR_GA10B_H
#define NVGPU_LTC_INTR_GA10B_H

#include <nvgpu/types.h>

#define LTC_INTR_RESET_BIT_MASK		(0xFFFF)

struct gk20a;

#define GA10B_FATAL_LTC_INTR \
	(ltc_ltcs_ltss_intr_idle_error_cbc_m() | \
	ltc_ltcs_ltss_intr_idle_error_tstg_m() | \
	ltc_ltcs_ltss_intr_idle_error_dstg_m() | \
	ltc_ltcs_ltss_intr_illegal_atomic_m() | \
	ltc_ltcs_ltss_intr_illegal_compstat_access_m())

#define GA10B_FATAL_LTC_INTR2 \
	(ltc_ltcs_ltss_intr2_sysfill_bypass_invalid_subid_m() | \
	ltc_ltcs_ltss_intr2_unexpected_sysfill_bypass_m())

#define GA10B_FATAL_LTC_ECC_INTR3 \
	(ltc_ltcs_ltss_intr3_ecc_uncorrected_m() | \
	ltc_ltcs_ltss_intr3_ecc_corrected_m())

#define GA10B_FATAL_LTC_NON_ECC_INTR3 \
	(ltc_ltcs_ltss_intr3_rmwrs_invalid_subid_m() | \
	ltc_ltcs_ltss_intr3_unexpected_rmwrs_m())

#define GA10B_DEBUG_LTC_INTR3 \
    (ltc_ltcs_ltss_intr3_volatile_to_compressed_m() | \
     ltc_ltcs_ltss_intr3_illegal_access_kind_type1_m() | \
     ltc_ltcs_ltss_intr3_illegal_access_kind_type2_m() | \
     ltc_ltcs_ltss_intr3_dtm_kind_invalid_m() | \
     ltc_ltcs_ltss_intr3_dtm_comptag_invalid_m())

void ga10b_ltc_intr_configure(struct gk20a *g);
void ga10b_ltc_intr1_configure(struct gk20a *g);
void ga10b_ltc_intr2_configure(struct gk20a *g);
void ga10b_ltc_intr3_configure(struct gk20a *g);
int ga10b_ltc_intr_isr(struct gk20a *g, u32 ltc);
void ga10b_ltc_intr3_ecc_interrupts(struct gk20a *g, u32 ltc, u32 slice,
				u32 offset, u32 ltc_intr3, bool *needs_quiesce);
void ga10b_ltc_intr3_interrupts(struct gk20a *g, u32 ltc, u32 slice,
				u32 ltc_intr3, bool *needs_quiesce);
void ga10b_ltc_intr_handle_lts_intr(struct gk20a *g, u32 ltc, u32 slice,
	bool *needs_quiesce);
void ga10b_ltc_intr_handle_lts_intr2(struct gk20a *g, u32 ltc, u32 slice,
	bool *needs_quiesce);
void ga10b_ltc_intr_handle_lts_intr3(struct gk20a *g, u32 ltc, u32 slice,
	bool *needs_quiesce);
u32 ga10b_ltc_intr_read_intr1(struct gk20a *g);
u32 ga10b_ltc_intr_read_intr2(struct gk20a *g);
u32 ga10b_ltc_intr_read_intr3(struct gk20a *g);
void ga10b_ltc_intr_write_intr1(struct gk20a *g, u32 reg_val);
void ga10b_ltc_intr_write_intr2(struct gk20a *g, u32 reg_val);
void ga10b_ltc_intr_write_intr3(struct gk20a *g, u32 reg_val);

#endif /* NVGPU_LTC_INTR_GA10B_H */
