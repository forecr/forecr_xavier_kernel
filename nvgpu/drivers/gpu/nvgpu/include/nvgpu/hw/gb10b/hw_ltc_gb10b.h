/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
/*
 * Function/Macro naming determines intended use:
 *
 *     <x>_r(void) : Returns the offset for register <x>.
 *
 *     <x>_o(void) : Returns the offset for element <x>.
 *
 *     <x>_w(void) : Returns the word offset for word (4 byte) element <x>.
 *
 *     <x>_<y>_s(void) : Returns size of field <y> of register <x> in bits.
 *
 *     <x>_<y>_f(u32 v) : Returns a value based on 'v' which has been shifted
 *         and masked to place it at field <y> of register <x>.  This value
 *         can be |'d with others to produce a full register value for
 *         register <x>.
 *
 *     <x>_<y>_m(void) : Returns a mask for field <y> of register <x>.  This
 *         value can be ~'d and then &'d to clear the value of field <y> for
 *         register <x>.
 *
 *     <x>_<y>_<z>_f(void) : Returns the constant value <z> after being shifted
 *         to place it at field <y> of register <x>.  This value can be |'d
 *         with others to produce a full register value for <x>.
 *
 *     <x>_<y>_v(u32 r) : Returns the value of field <y> from a full register
 *         <x> value 'r' after being shifted to place its LSB at bit 0.
 *         This value is suitable for direct comparison with other unshifted
 *         values appropriate for use in field <y> of register <x>.
 *
 *     <x>_<y>_<z>_v(void) : Returns the constant value for <z> defined for
 *         field <y> of register <x>.  This value is suitable for direct
 *         comparison with unshifted values appropriate for use in field <y>
 *         of register <x>.
 */
#ifndef NVGPU_HW_LTC_GB10B_H
#define NVGPU_HW_LTC_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define ltc_pltcg_base_v()                                         (0x00140000U)
#define ltc_pltcg_extent_v()                                       (0x0017ffffU)
#define ltc_pltcg_ltcs_base_v()                                    (0x001c6000U)
#define ltc_ltc0_lts0_v()                                          (0x00140400U)
#define ltc_ltcs_ltss_v()                                          (0x001c6200U)
#define ltc_ltcs_ltss_tstg_set_mgmt0_r()                           (0x001c62acU)
#define ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_f(v)\
				((U32(v) & 0x1fU) << 16U)
#define ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_m()      (U32(0x1fU) << 16U)
#define ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_v(r)  (((r) >> 16U) & 0x1fU)
#define ltc_ltcs_ltss_tstg_cfg1_r()                                (0x001c6294U)
#define ltc_ltcs_ltss_tstg_cfg1_active_sets_v(r)           (((r) >> 16U) & 0x3U)
#define ltc_ltcs_ltss_tstg_cfg1_active_sets_all_v()                (0x00000000U)
#define ltc_ltcs_ltss_tstg_cfg1_active_sets_half_v()               (0x00000001U)
#define ltc_ltcs_ltss_tstg_cfg2_r()                                (0x001c6298U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_l1_promote_f(v)  ((U32(v) & 0x3U) << 16U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_l1_promote_m()         (U32(0x3U) << 16U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_l1_promote_v(r)     (((r) >> 16U) & 0x3U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_l1_promote_none_v()         (0x00000000U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_l1_promote_64b_v()          (0x00000001U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_l1_promote_128b_v()         (0x00000002U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_t1_promote_f(v)  ((U32(v) & 0x3U) << 18U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_t1_promote_m()         (U32(0x3U) << 18U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_t1_promote_v(r)     (((r) >> 18U) & 0x3U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_t1_promote_none_v()         (0x00000000U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_t1_promote_64b_v()          (0x00000001U)
#define ltc_ltcs_ltss_tstg_cfg2_vidmem_t1_promote_128b_v()         (0x00000002U)
#define ltc_ltcs_ltss_tstg_cfg3_r()                                (0x001c629cU)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_l1_promote_f(v)  ((U32(v) & 0x3U) << 16U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_l1_promote_m()         (U32(0x3U) << 16U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_l1_promote_v(r)     (((r) >> 16U) & 0x3U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_l1_promote_none_v()         (0x00000000U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_l1_promote_64b_v()          (0x00000001U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_l1_promote_128b_v()         (0x00000002U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_t1_promote_f(v)  ((U32(v) & 0x3U) << 18U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_t1_promote_m()         (U32(0x3U) << 18U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_t1_promote_v(r)     (((r) >> 18U) & 0x3U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_t1_promote_none_v()         (0x00000000U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_t1_promote_64b_v()          (0x00000001U)
#define ltc_ltcs_ltss_tstg_cfg3_sysmem_t1_promote_128b_v()         (0x00000002U)
#define ltc_ltcs_ltss_cbc_ctrl1_r()                                (0x001c626cU)
#define ltc_ltcs_ltss_cbc_ctrl1_clean_active_f()                          (0x1U)
#define ltc_ltcs_ltss_cbc_ctrl1_invalidate_active_f()                     (0x2U)
#define ltc_ltcs_ltss_cbc_ctrl1_clear_active_f()                          (0x4U)
#define ltc_ltc0_lts0_cbc_ctrl1_r()                                (0x0014046cU)
#define ltc_ltcs_ltss_cbc_ctrl2_r()                                (0x001c6270U)
#define ltc_ltcs_ltss_cbc_ctrl2_clear_lower_bound_f(v)\
				((U32(v) & 0xfffffU) << 0U)
#define ltc_ltcs_ltss_cbc_ctrl3_r()                                (0x001c6274U)
#define ltc_ltcs_ltss_cbc_ctrl3_clear_upper_bound_f(v)\
				((U32(v) & 0xfffffU) << 0U)
#define ltc_ltcs_ltss_cbc_ctrl3_clear_upper_bound_init_v()         (0x000fffffU)
#define ltc_ltcs_ltss_cbc_base_r()                                 (0x001c6278U)
#define ltc_ltcs_ltss_cbc_base_alignment_shift_v()                 (0x0000000bU)
#define ltc_ltc0_lts0_cbc_num_active_ltcs_r()                      (0x0014047cU)
#define ltc_ltc0_lts0_cbc_num_active_ltcs_v_f(v)        ((U32(v) & 0x3fU) << 0U)
#define ltc_ltc0_lts0_cbc_num_active_ltcs_v_m()               (U32(0x3fU) << 0U)
#define ltc_ltc0_lts0_cbc_num_active_ltcs_v_v(r)           (((r) >> 0U) & 0x3fU)
#define ltc_ltcs_ltss_cbc_cfg_0_r()                                (0x001c6264U)
#define ltc_ltcs_ltss_cbc_cfg_0_swizid_f(v)             ((U32(v) & 0x1U) << 31U)
#define ltc_ltcs_ltss_cbc_cfg_0_swizid_m()                    (U32(0x1U) << 31U)
#define ltc_ltcs_ltss_cbc_cfg_0_swizid_v(r)                (((r) >> 31U) & 0x1U)
#define ltc_ltcs_ltss_cbc_cfg_0_swizid_swizid_zero_v()             (0x00000000U)
#define ltc_ltcs_ltss_cbc_cfg_0_swizid_swizid_one_v()              (0x00000001U)
#define ltc_ltcs_ltss_cbc_param_r()                                (0x001c6280U)
#define ltc_ltcs_ltss_cbc_param_bytes_per_comptagline_per_slice_v(r)\
				(((r) >> 0U) & 0x3ffU)
#define ltc_ltc0_ltss_cbc_param_r()                                (0x00140280U)
#define ltc_ltc0_ltss_cbc_param_slices_per_ltc_v(r)        (((r) >> 28U) & 0xfU)
#define ltc_ltc0_lts0_cbc_base_r()                                 (0x00140478U)
#define ltc_ltcs_ltss_cbc_param2_r()                               (0x001c63f4U)
#define ltc_ltcs_ltss_cbc_param2_gobs_per_comptagline_per_slice_v(r)\
				(((r) >> 0U) & 0xffffU)
#define ltc_ltcs_ltss_cbc_param2_cache_line_size_v(r)      (((r) >> 24U) & 0xfU)
#define ltc_ltcs_ltss_cbc_param2_slices_per_ltc_v(r)       (((r) >> 28U) & 0xfU)
#define ltc_ltcs_ltss_tstg_set_mgmt_3_r()                          (0x001c62b4U)
#define ltc_ltcs_ltss_tstg_set_mgmt_3_disallow_clean_ce_imm_m()\
				(U32(0x1U) << 23U)
#define ltc_ltcs_ltss_tstg_set_mgmt_3_disallow_clean_ce_imm_enabled_f()\
				(0x800000U)
#define ltc_ltcs_ltss_tstg_set_mgmt_3_disallow_clean_fclr_imm_m()\
				(U32(0x1U) << 21U)
#define ltc_ltcs_ltss_tstg_set_mgmt_3_disallow_clean_fclr_imm_enabled_f()\
				(0x200000U)
#define ltc_ltcs_ltss_tstg_set_mgmt_5_r()                          (0x001c62bcU)
#define ltc_ltcs_ltss_dstg_zbc_index_r()                           (0x001c6338U)
#define ltc_ltcs_ltss_dstg_zbc_index_address_f(v)       ((U32(v) & 0x1fU) << 0U)
#define ltc_ltcs_ltss_dstg_zbc_color_clear_value_r(i)\
		(nvgpu_safe_add_u32(0x001c633cU, nvgpu_safe_mult_u32((i), 4U)))
#define ltc_ltcs_ltss_dstg_zbc_color_clear_value__size_1_v()       (0x00000004U)
#define ltc_ltcs_ltss_dstg_zbc_depth_clear_value_r()               (0x001c634cU)
#define ltc_ltcs_ltss_dstg_zbc_stencil_clear_value_r()             (0x001c6204U)
#define ltc_ltcs_ltss_tstg_set_mgmt_2_r()                          (0x001c62b0U)
#define ltc_ltcs_ltss_tstg_set_mgmt_2_l2_bypass_mode_enabled_f()   (0x10000000U)
#define ltc_ltcs_ltss_intr_r()                                     (0x001c620cU)
#define ltc_ltcs_ltss_intr_reset_bit_mask_v()                      (0x0000ffffU)
#define ltc_ltcs_ltss_intr_en_idle_error_iq_m()               (U32(0x1U) << 16U)
#define ltc_ltcs_ltss_intr_en_idle_error_iq_disabled_f()                  (0x0U)
#define ltc_ltcs_ltss_intr_idle_error_cbc_m()                  (U32(0x1U) << 1U)
#define ltc_ltcs_ltss_intr_idle_error_cbc_reset_f()                       (0x2U)
#define ltc_ltcs_ltss_intr_en_idle_error_cbc_m()              (U32(0x1U) << 17U)
#define ltc_ltcs_ltss_intr_en_idle_error_cbc_enabled_f()              (0x20000U)
#define ltc_ltcs_ltss_intr_en_ecc_sec_error_m()               (U32(0x1U) << 24U)
#define ltc_ltcs_ltss_intr_en_ecc_sec_error_disable_f()                   (0x0U)
							    /* NEEDS_IP_AUDIT */
#define ltc_ltcs_ltss_intr_en_ecc_ded_error_m()               (U32(0x1U) << 25U)
#define ltc_ltcs_ltss_intr_en_ecc_ded_error_disable_f()                   (0x0U)
							    /* NEEDS_IP_AUDIT */
#define ltc_ltcs_ltss_intr_en_debug_m()                       (U32(0x1U) << 26U)
							    /* NEEDS_IP_AUDIT */
#define ltc_ltcs_ltss_intr_en_debug_disable_f()                           (0x0U)
							    /* NEEDS_IP_AUDIT */
#define ltc_ltcs_ltss_intr_idle_error_tstg_m()                 (U32(0x1U) << 2U)
#define ltc_ltcs_ltss_intr_idle_error_tstg_reset_f()                      (0x4U)
#define ltc_ltcs_ltss_intr_en_idle_error_tstg_m()             (U32(0x1U) << 18U)
#define ltc_ltcs_ltss_intr_en_idle_error_tstg_enabled_f()             (0x40000U)
#define ltc_ltcs_ltss_intr_idle_error_dstg_m()                 (U32(0x1U) << 3U)
#define ltc_ltcs_ltss_intr_idle_error_dstg_reset_f()                      (0x8U)
#define ltc_ltcs_ltss_intr_en_idle_error_dstg_m()             (U32(0x1U) << 19U)
#define ltc_ltcs_ltss_intr_en_idle_error_dstg_enabled_f()             (0x80000U)
#define ltc_ltcs_ltss_intr_evicted_cb_m()                      (U32(0x1U) << 4U)
#define ltc_ltcs_ltss_intr_evicted_cb_reset_f()                          (0x10U)
#define ltc_ltcs_ltss_intr_en_evicted_cb_m()                  (U32(0x1U) << 20U)
#define ltc_ltcs_ltss_intr_en_evicted_cb_enabled_f()                 (0x100000U)
#define ltc_ltcs_ltss_intr_en_evicted_cb_disabled_f()                     (0x0U)
#define ltc_ltcs_ltss_intr_illegal_atomic_m()                 (U32(0x1U) << 12U)
#define ltc_ltcs_ltss_intr_illegal_atomic_reset_f()                    (0x1000U)
#define ltc_ltcs_ltss_intr_en_illegal_atomic_m()              (U32(0x1U) << 28U)
#define ltc_ltcs_ltss_intr_en_illegal_atomic_enabled_f()           (0x10000000U)
#define ltc_ltcs_ltss_intr_atomic_to_z_m()                    (U32(0x1U) << 11U)
#define ltc_ltcs_ltss_intr_atomic_to_z_reset_f()                        (0x800U)
#define ltc_ltcs_ltss_intr_en_atomic_to_z_m()                 (U32(0x1U) << 27U)
#define ltc_ltcs_ltss_intr_en_atomic_to_z_enabled_f()               (0x8000000U)
#define ltc_ltcs_ltss_intr_blkactivity_err_m()                (U32(0x1U) << 13U)
#define ltc_ltcs_ltss_intr_blkactivity_err_reset_f()                   (0x2000U)
#define ltc_ltcs_ltss_intr_en_blkactivity_err_m()             (U32(0x1U) << 29U)
#define ltc_ltcs_ltss_intr_en_blkactivity_err_enabled_f()          (0x20000000U)
#define ltc_ltcs_ltss_intr_en_blkactivity_err_disable_f()                 (0x0U)
#define ltc_ltcs_ltss_intr_illegal_compstat_access_m()        (U32(0x1U) << 14U)
#define ltc_ltcs_ltss_intr_illegal_compstat_access_reset_f()           (0x4000U)
#define ltc_ltcs_ltss_intr_en_illegal_compstat_access_m()     (U32(0x1U) << 30U)
#define ltc_ltcs_ltss_intr_en_illegal_compstat_access_enabled_f()  (0x40000000U)
#define ltc_ltcs_ltss_intr_cbc_outside_safe_size_m()           (U32(0x1U) << 5U)
#define ltc_ltcs_ltss_intr_cbc_outside_safe_size_reset_f()               (0x20U)
#define ltc_ltcs_ltss_intr_en_cbc_outside_safe_size_m()       (U32(0x1U) << 21U)
#define ltc_ltcs_ltss_intr_en_cbc_outside_safe_size_enabled_f()      (0x200000U)
#define ltc_ltcs_ltss_intr_en_cbc_outside_safe_size_disabled_f()          (0x0U)
#define ltc_ltcs_ltss_intr_received_nderr_rd_m()               (U32(0x1U) << 7U)
#define ltc_ltcs_ltss_intr_received_nderr_rd_reset_f()                   (0x80U)
#define ltc_ltcs_ltss_intr_en_received_nderr_rd_m()           (U32(0x1U) << 23U)
#define ltc_ltcs_ltss_intr_en_received_nderr_rd_enabled_f()          (0x800000U)
#define ltc_ltcs_ltss_intr_en_received_nderr_rd_disabled_f()              (0x0U)
#define ltc_ltcs_ltss_intr_received_nderr_wr_m()              (U32(0x1U) << 15U)
#define ltc_ltcs_ltss_intr_received_nderr_wr_reset_f()                 (0x8000U)
#define ltc_ltcs_ltss_intr_en_received_nderr_wr_m()           (U32(0x1U) << 31U)
#define ltc_ltcs_ltss_intr_en_received_nderr_wr_enabled_f()        (0x80000000U)
#define ltc_ltcs_ltss_intr_en_received_nderr_wr_disabled_f()              (0x0U)
#define ltc_ltcs_ltss_intr2_r()                                    (0x001c6208U)
#define ltc_ltcs_ltss_intr2_reset_bit_mask_v()                     (0x0000ffffU)
#define ltc_ltcs_ltss_intr2_sysfill_bypass_invalid_subid_m()  (U32(0x1U) << 12U)
#define ltc_ltcs_ltss_intr2_sysfill_bypass_invalid_subid_reset_f()     (0x1000U)
#define ltc_ltcs_ltss_intr2_en_sysfill_bypass_invalid_subid_m()\
				(U32(0x1U) << 28U)
#define ltc_ltcs_ltss_intr2_en_sysfill_bypass_invalid_subid_enabled_f()\
				(0x10000000U)
#define ltc_ltcs_ltss_intr2_unexpected_sysfill_bypass_m()     (U32(0x1U) << 13U)
#define ltc_ltcs_ltss_intr2_unexpected_sysfill_bypass_reset_f()        (0x2000U)
#define ltc_ltcs_ltss_intr2_en_unexpected_sysfill_bypass_m()  (U32(0x1U) << 29U)
#define ltc_ltcs_ltss_intr2_en_unexpected_sysfill_bypass_enabled_f()\
				(0x20000000U)
#define ltc_ltcs_ltss_intr2_cache_coh_access_packed_kind_m()   (U32(0x1U) << 0U)
#define ltc_ltcs_ltss_intr2_cache_coh_access_packed_kind_reset_f()        (0x1U)
#define ltc_ltcs_ltss_intr2_en_cache_coh_access_packed_kind_m()\
				(U32(0x1U) << 16U)
#define ltc_ltcs_ltss_intr2_en_cache_coh_access_packed_kind_enabled_f()\
				(0x10000U)
#define ltc_ltcs_ltss_intr2_en_cache_coh_access_packed_kind_disable_f()   (0x0U)
#define ltc_ltcs_ltss_intr2_cache_coh_access_compressible_m()  (U32(0x1U) << 1U)
#define ltc_ltcs_ltss_intr2_cache_coh_access_compressible_reset_f()       (0x2U)
#define ltc_ltcs_ltss_intr2_en_cache_coh_access_compressible_m()\
				(U32(0x1U) << 17U)
#define ltc_ltcs_ltss_intr2_en_cache_coh_access_compressible_enabled_f()\
				(0x20000U)
#define ltc_ltcs_ltss_intr2_unsupported_chi_req_m()            (U32(0x1U) << 2U)
#define ltc_ltcs_ltss_intr2_unsupported_chi_req_reset_f()                 (0x4U)
#define ltc_ltcs_ltss_intr2_en_unsupported_chi_req_m()        (U32(0x1U) << 18U)
#define ltc_ltcs_ltss_intr2_en_unsupported_chi_req_enabled_f()        (0x40000U)
#define ltc_ltcs_ltss_intr2_sys_aperture_mismatch_m()          (U32(0x1U) << 3U)
#define ltc_ltcs_ltss_intr2_sys_aperture_mismatch_reset_f()               (0x8U)
#define ltc_ltcs_ltss_intr2_en_sys_aperture_mismatch_m()      (U32(0x1U) << 19U)
#define ltc_ltcs_ltss_intr2_en_sys_aperture_mismatch_enabled_f()      (0x80000U)
#define ltc_ltcs_ltss_intr2_en_sys_aperture_mismatch_disable_f()          (0x0U)
#define ltc_ltcs_ltss_intr2_prefetch_sys_aperture_mismatch_m() (U32(0x1U) << 4U)
#define ltc_ltcs_ltss_intr2_prefetch_sys_aperture_mismatch_reset_f()     (0x10U)
#define ltc_ltcs_ltss_intr2_en_prefetch_sys_aperture_mismatch_m()\
				(U32(0x1U) << 20U)
#define ltc_ltcs_ltss_intr2_en_prefetch_sys_aperture_mismatch_enabled_f()\
				(0x100000U)
#define ltc_ltcs_ltss_intr2_en_prefetch_sys_aperture_mismatch_disable_f() (0x0U)
#define ltc_ltcs_ltss_intr2_snp_sys_aperture_mismatch_m()      (U32(0x1U) << 5U)
#define ltc_ltcs_ltss_intr2_snp_sys_aperture_mismatch_reset_f()          (0x20U)
#define ltc_ltcs_ltss_intr2_en_snp_sys_aperture_mismatch_m()  (U32(0x1U) << 21U)
#define ltc_ltcs_ltss_intr2_en_snp_sys_aperture_mismatch_enabled_f() (0x200000U)
#define ltc_ltcs_ltss_intr2_invalid_rs_subid_m()               (U32(0x1U) << 6U)
#define ltc_ltcs_ltss_intr2_invalid_rs_subid_reset_f()                   (0x40U)
#define ltc_ltcs_ltss_intr2_en_invalid_rs_subid_m()           (U32(0x1U) << 22U)
#define ltc_ltcs_ltss_intr2_en_invalid_rs_subid_enabled_f()          (0x400000U)
#define ltc_ltcs_ltss_intr2_unexpected_rs_type_m()             (U32(0x1U) << 7U)
#define ltc_ltcs_ltss_intr2_unexpected_rs_type_reset_f()                 (0x80U)
#define ltc_ltcs_ltss_intr2_en_unexpected_rs_type_m()         (U32(0x1U) << 23U)
#define ltc_ltcs_ltss_intr2_en_unexpected_rs_type_enabled_f()        (0x800000U)
#define ltc_ltcs_ltss_intr2_data_mismatch_mask_subid_m()       (U32(0x1U) << 9U)
#define ltc_ltcs_ltss_intr2_data_mismatch_mask_subid_reset_f()          (0x200U)
#define ltc_ltcs_ltss_intr2_en_data_mismatch_mask_subid_m()   (U32(0x1U) << 25U)
#define ltc_ltcs_ltss_intr2_en_data_mismatch_mask_subid_enabled_f() (0x2000000U)
#define ltc_ltcs_ltss_intr2_en_data_mismatch_mask_subid_disabled_f()      (0x0U)
#define ltc_ltcs_ltss_intr2_hsh2ltc_unexpected_data_m()       (U32(0x1U) << 10U)
#define ltc_ltcs_ltss_intr2_hsh2ltc_unexpected_data_reset_f()           (0x400U)
#define ltc_ltcs_ltss_intr2_en_hsh2ltc_unexpected_data_m()    (U32(0x1U) << 26U)
#define ltc_ltcs_ltss_intr2_en_hsh2ltc_unexpected_data_enabled_f()  (0x4000000U)
#define ltc_ltcs_ltss_intr3_r()                                    (0x001c6388U)
#define ltc_ltcs_ltss_intr3_reset_bit_mask_v()                     (0x0000ffffU)
#define ltc_ltcs_ltss_intr3_ecc_corrected_m()                  (U32(0x1U) << 7U)
#define ltc_ltcs_ltss_intr3_ecc_uncorrected_m()                (U32(0x1U) << 8U)
#define ltc_ltcs_ltss_intr3_illegal_access_kind_type1_m()     (U32(0x1U) << 10U)
#define ltc_ltcs_ltss_intr3_illegal_access_kind_type1_reset_f()         (0x400U)
#define ltc_ltcs_ltss_intr3_en_illegal_access_kind_type1_m()  (U32(0x1U) << 26U)
#define ltc_ltcs_ltss_intr3_en_illegal_access_kind_type1_enabled_f()\
				(0x4000000U)
#define ltc_ltcs_ltss_intr3_illegal_access_kind_type2_m()     (U32(0x1U) << 11U)
#define ltc_ltcs_ltss_intr3_illegal_access_kind_type2_reset_f()         (0x800U)
#define ltc_ltcs_ltss_intr3_en_illegal_access_kind_type2_m()  (U32(0x1U) << 27U)
#define ltc_ltcs_ltss_intr3_en_illegal_access_kind_type2_enabled_f()\
				(0x8000000U)
#define ltc_ltcs_ltss_intr3_dtm_kind_invalid_m()              (U32(0x1U) << 12U)
#define ltc_ltcs_ltss_intr3_dtm_kind_invalid_reset_f()                 (0x1000U)
#define ltc_ltcs_ltss_intr3_en_dtm_kind_invalid_m()           (U32(0x1U) << 28U)
#define ltc_ltcs_ltss_intr3_en_dtm_kind_invalid_enabled_f()        (0x10000000U)
#define ltc_ltcs_ltss_intr3_dtm_comptag_invalid_m()           (U32(0x1U) << 13U)
#define ltc_ltcs_ltss_intr3_dtm_comptag_invalid_reset_f()              (0x2000U)
#define ltc_ltcs_ltss_intr3_en_dtm_comptag_invalid_m()        (U32(0x1U) << 29U)
#define ltc_ltcs_ltss_intr3_en_dtm_comptag_invalid_enabled_f()     (0x20000000U)
#define ltc_ltcs_ltss_intr3_en_checkedout_other_ncnp_req_m()  (U32(0x1U) << 30U)
#define ltc_ltcs_ltss_intr3_en_checkedout_other_ncnp_req_enabled_f()\
				(0x40000000U)
#define ltc_ltcs_ltss_intr3_en_checkedout_other_ncnp_req_disabled_f()     (0x0U)
#define ltc_ltcs_ltss_intr3_en_checkedout_access_packed_kind_m()\
				(U32(0x1U) << 31U)
#define ltc_ltcs_ltss_intr3_en_checkedout_access_packed_kind_enabled_f()\
				(0x80000000U)
#define ltc_ltcs_ltss_intr3_en_checkedout_access_packed_kind_disabled_f() (0x0U)
#define ltc_ltcs_ltss_intr3_volatile_to_compressed_m()         (U32(0x1U) << 9U)
#define ltc_ltcs_ltss_intr3_volatile_to_compressed_reset_f()            (0x200U)
#define ltc_ltcs_ltss_intr3_en_volatile_to_compressed_m()     (U32(0x1U) << 25U)
#define ltc_ltcs_ltss_intr3_en_volatile_to_compressed_enabled_f()   (0x2000000U)
#define ltc_ltc0_lts0_intr_r()                                     (0x0014040cU)
#define ltc_ltc0_lts0_intr2_r()                                    (0x00140408U)
#define ltc_ltc0_lts0_intr3_r()                                    (0x00140588U)
#define ltc_ltcs_ltss_tstg_cmgmt0_r()                              (0x001c62a0U)
#define ltc_ltcs_ltss_tstg_cmgmt0_invalidate_pending_f()                  (0x1U)
#define ltc_ltcs_ltss_tstg_cmgmt0_max_cycles_between_invalidates_3_f()  (0x300U)
#define ltc_ltcs_ltss_tstg_cmgmt0_invalidate_evict_last_class_true_f()\
				(0x10000000U)
#define ltc_ltcs_ltss_tstg_cmgmt0_invalidate_evict_normal_class_true_f()\
				(0x20000000U)
#define ltc_ltcs_ltss_tstg_cmgmt0_invalidate_evict_first_class_true_f()\
				(0x40000000U)
#define ltc_ltcs_ltss_tstg_cmgmt1_r()                              (0x001c62a4U)
#define ltc_ltcs_ltss_tstg_cmgmt1_clean_pending_f()                       (0x1U)
#define ltc_ltcs_ltss_tstg_cmgmt1_max_cycles_between_cleans_3_f()       (0x300U)
#define ltc_ltcs_ltss_tstg_cmgmt1_clean_wait_for_fb_to_pull_true_f()  (0x10000U)
#define ltc_ltcs_ltss_tstg_cmgmt1_clean_evict_last_class_true_f()  (0x10000000U)
#define ltc_ltcs_ltss_tstg_cmgmt1_clean_evict_normal_class_true_f()\
				(0x20000000U)
#define ltc_ltcs_ltss_tstg_cmgmt1_clean_evict_first_class_true_f() (0x40000000U)
#define ltc_ltc0_ltss_tstg_cmgmt0_r()                              (0x001402a0U)
#define ltc_ltc0_ltss_tstg_cmgmt0_invalidate_pending_f()                  (0x1U)
#define ltc_ltc0_ltss_tstg_cmgmt1_r()                              (0x001402a4U)
#define ltc_ltc0_ltss_tstg_cmgmt1_clean_pending_f()                       (0x1U)
#define ltc_ltc0_lts0_tstg_info_1_r()                              (0x0014058cU)
#define ltc_ltc0_lts0_tstg_info_1_slice_size_in_kb_v(r)  (((r) >> 0U) & 0xffffU)
#define ltc_ltc0_lts0_tstg_info_1_slices_per_l2_v(r)      (((r) >> 16U) & 0x1fU)
#define ltc_ltcs_ltss_tstg_set_mgmt_1_r()                          (0x001c639cU)
#define ltc_ltcs_ltss_tstg_set_mgmt_1_plc_recompress_plc_m()   (U32(0x1U) << 7U)
#define ltc_ltcs_ltss_tstg_set_mgmt_1_plc_recompress_plc_disabled_f()     (0x0U)
#define ltc_ltcs_ltss_tstg_set_mgmt_1_plc_recompress_plc_enabled_f()     (0x80U)
#define ltc_ltcs_ltss_tstg_set_mgmt_1_plc_recompress_rmw_m()  (U32(0x1U) << 29U)
#define ltc_ltcs_ltss_tstg_set_mgmt_1_plc_recompress_rmw_disabled_f()     (0x0U)
#define ltc_ltcs_ltss_tstg_set_mgmt_1_plc_recompress_rmw_enabled_f()\
				(0x20000000U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_r()                     (0x001404fcU)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_v(r)       (((r) >> 22U) & 0xffU)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_clrbe_trlram0_v()\
				(0x00000000U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_clrbe_trlram1_v()\
				(0x00000001U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_clrbe_trlram2_v()\
				(0x00000002U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_clrbe_trlram3_v()\
				(0x00000003U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_clrbe_trlram4_v()\
				(0x00000004U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_clrbe_trlram5_v()\
				(0x00000005U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_clrbe_trlram6_v()\
				(0x00000006U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_clrbe_trlram7_v()\
				(0x00000007U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_bank0_v()   (0x00000008U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_bank1_v()   (0x00000009U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_bank2_v()   (0x0000000aU)
#define ltc_ltc0_lts0_l2_cache_ecc_address_ram_dstg_db_bank3_v()   (0x0000000bU)
#define ltc_ltc0_lts0_l2_cache_ecc_address_subunit_v(r)    (((r) >> 30U) & 0x3U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_subunit_rstg_v()        (0x00000000U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_subunit_tstg_v()        (0x00000001U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_subunit_dstg_v()        (0x00000002U)
#define ltc_ltc0_lts0_l2_cache_ecc_address_subunit_intfc_v()       (0x00000003U)
#define ltc_ltc0_lts0_l2_cache_ecc_corrected_err_count_r()         (0x001404f4U)
#define ltc_ltc0_lts0_l2_cache_ecc_corrected_err_count_total_s()           (16U)
#define ltc_ltc0_lts0_l2_cache_ecc_corrected_err_count_total_v(r)\
				(((r) >> 0U) & 0xffffU)
#define ltc_ltc0_lts0_l2_cache_ecc_uncorrected_err_count_r()       (0x001404f8U)
#define ltc_ltc0_lts0_l2_cache_ecc_uncorrected_err_count_total_s()         (16U)
#define ltc_ltc0_lts0_l2_cache_ecc_uncorrected_err_count_total_v(r)\
				(((r) >> 0U) & 0xffffU)
#define ltc_ltc0_lts0_l2_cache_ecc_control_r()                     (0x001404ecU)
#define ltc_ltc0_lts0_l2_cache_ecc_control_inject_corrected_err_f(v)\
				((U32(v) & 0x1U) << 4U)
#define ltc_ltc0_lts0_l2_cache_ecc_control_inject_uncorrected_err_f(v)\
				((U32(v) & 0x1U) << 5U)
#define ltc_ltc0_lts0_l2_cache_ecc_control_inject_corrected_err_rstg_f(v)\
				((U32(v) & 0x1U) << 6U)
#define ltc_ltc0_lts0_l2_cache_ecc_control_inject_uncorrected_err_rstg_f(v)\
				((U32(v) & 0x1U) << 7U)
#define ltc_ltc0_lts0_l2_cache_ecc_control_inject_corrected_err_tstg_f(v)\
				((U32(v) & 0x1U) << 8U)
#define ltc_ltc0_lts0_l2_cache_ecc_control_inject_uncorrected_err_tstg_f(v)\
				((U32(v) & 0x1U) << 9U)
#define ltc_ltc0_lts0_l2_cache_ecc_control_inject_corrected_err_dstg_f(v)\
				((U32(v) & 0x1U) << 10U)
#define ltc_ltc0_lts0_l2_cache_ecc_control_inject_uncorrected_err_dstg_f(v)\
				((U32(v) & 0x1U) << 11U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_r()                      (0x001404f0U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_uncorrected_err_rstg_m()\
				(U32(0x1U) << 0U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_corrected_err_rstg_m()\
				(U32(0x1U) << 1U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_uncorrected_err_tstg_m()\
				(U32(0x1U) << 2U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_corrected_err_tstg_m()\
				(U32(0x1U) << 3U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_uncorrected_err_dstg_m()\
				(U32(0x1U) << 4U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_corrected_err_dstg_m()\
				(U32(0x1U) << 5U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_uncorrected_err_intfc_m()\
				(U32(0x1U) << 6U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_corrected_err_intfc_m()\
				(U32(0x1U) << 7U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_corrected_err_total_counter_overflow_m()\
				(U32(0x1U) << 16U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_uncorrected_err_total_counter_overflow_m()\
				(U32(0x1U) << 18U)
#define ltc_ltc0_lts0_l2_cache_ecc_status_reset_task_f()           (0x40000000U)
#define ltc_ltcs_intr_ctrl_r()                                     (0x001c6068U)
#define ltc_ltcs_intr2_ctrl_r()                                    (0x001c6078U)
#define ltc_ltcs_intr3_ctrl_r()                                    (0x001c607cU)
#define ltc_ltcs_intr_retrigger_r()                                (0x001c606cU)
#define ltc_ltcs_intr_retrigger_trigger_true_f()                          (0x1U)
#endif
