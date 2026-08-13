/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_INTR_GB10B_H
#define NVGPU_LTC_INTR_GB10B_H

#include <nvgpu/types.h>

struct gk20a;

#define GB10B_FATAL_LTC_INTR \
	(ltc_ltcs_ltss_intr_idle_error_cbc_m() | \
	ltc_ltcs_ltss_intr_idle_error_tstg_m() | \
	ltc_ltcs_ltss_intr_idle_error_dstg_m() | \
	ltc_ltcs_ltss_intr_received_nderr_rd_m() | \
	ltc_ltcs_ltss_intr_atomic_to_z_m() | \
	ltc_ltcs_ltss_intr_illegal_atomic_m() | \
	ltc_ltcs_ltss_intr_illegal_compstat_access_m() | \
	ltc_ltcs_ltss_intr_received_nderr_wr_m())

#define GB10B_DEBUG_LTC_INTR \
	(ltc_ltcs_ltss_intr_evicted_cb_m() | \
	ltc_ltcs_ltss_intr_blkactivity_err_m() | \
	ltc_ltcs_ltss_intr_cbc_outside_safe_size_m())

#define GB10B_FATAL_LTC_INTR2 \
	(ltc_ltcs_ltss_intr2_cache_coh_access_compressible_m() | \
	ltc_ltcs_ltss_intr2_unsupported_chi_req_m() | \
	ltc_ltcs_ltss_intr2_snp_sys_aperture_mismatch_m() | \
	ltc_ltcs_ltss_intr2_invalid_rs_subid_m() | \
	ltc_ltcs_ltss_intr2_unexpected_rs_type_m() | \
	ltc_ltcs_ltss_intr2_hsh2ltc_unexpected_data_m() | \
	ltc_ltcs_ltss_intr2_sysfill_bypass_invalid_subid_m() | \
	ltc_ltcs_ltss_intr2_unexpected_sysfill_bypass_m())

#define GB10B_DEBUG_LTC_INTR2 \
	(ltc_ltcs_ltss_intr2_cache_coh_access_packed_kind_m() | \
	ltc_ltcs_ltss_intr2_sys_aperture_mismatch_m() | \
	ltc_ltcs_ltss_intr2_prefetch_sys_aperture_mismatch_m())

#define GB10B_FATAL_LTC_INTR3 \
	(ltc_ltcs_ltss_intr3_ecc_uncorrected_m() | \
	ltc_ltcs_ltss_intr3_ecc_corrected_m())

#define GB10B_DEBUG_LTC_INTR3 \
    (ltc_ltcs_ltss_intr3_volatile_to_compressed_m() | \
     ltc_ltcs_ltss_intr3_illegal_access_kind_type1_m() | \
     ltc_ltcs_ltss_intr3_illegal_access_kind_type2_m() | \
     ltc_ltcs_ltss_intr3_dtm_kind_invalid_m() | \
     ltc_ltcs_ltss_intr3_dtm_comptag_invalid_m())

void gb10b_ltc_intr_handle_lts_intr(struct gk20a *g, u32 ltc, u32 slice,
		bool *needs_quiesce);
void gb10b_ltc_intr_handle_lts_intr2(struct gk20a *g, u32 ltc, u32 slice,
		bool *needs_quiesce);
void gb10b_ltc_intr_handle_lts_intr3(struct gk20a *g, u32 ltc, u32 slice,
		bool *needs_quiesce);
void gb10b_ltc_intr1_configure(struct gk20a *g);
void gb10b_ltc_intr2_configure(struct gk20a *g);
void gb10b_ltc_intr3_configure(struct gk20a *g);
u32 gb10b_ltc_intr_read_intr1(struct gk20a *g);
u32 gb10b_ltc_intr_read_intr2(struct gk20a *g);
u32 gb10b_ltc_intr_read_intr3(struct gk20a *g);
void gb10b_ltc_intr_write_intr1(struct gk20a *g, u32 reg_val);
void gb10b_ltc_intr_write_intr2(struct gk20a *g, u32 reg_val);
void gb10b_ltc_intr_write_intr3(struct gk20a *g, u32 reg_val);
void gb10b_ltc_intr_configure(struct gk20a *g);
void gb10b_ltc_intr_retrigger(struct gk20a *g);
void gb10b_xbar_handle_intfc_ecc_intr(struct gk20a *g, u32 ltc, u32 slice,
		u32 ecc_status, u32 ecc_addr, bool *needs_quiesce);
void gb10b_ltc_intr_report_non_ecc_uncorrected_err(struct gk20a *g);
#endif /* NVGPU_LTC_INTR_GB10B_H */
