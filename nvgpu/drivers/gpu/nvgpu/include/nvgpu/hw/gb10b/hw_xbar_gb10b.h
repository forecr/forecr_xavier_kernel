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
#ifndef NVGPU_HW_XBAR_GB10B_H
#define NVGPU_HW_XBAR_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define xbar_mxbar_pri_gpc0_gnic0_base_v()                         (0x00b20000U)
#define xbar_mxbar_pri_gpc0_gnic1_base_v()                         (0x00b21000U)
#define xbar_mxbar_pri_gpc0_gnic2_base_v()                         (0x00b22000U)
#define xbar_mxbar_pri_gpc0_gnic3_base_v()                         (0x00b23000U)
#define xbar_mxbar_pri_gpc0_gnic4_base_v()                         (0x00b24000U)
#define xbar_mxbar_pri_gpc0_gnic0_preg_pm_ctrl_r()                 (0x00b20014U)
#define xbar_mxbar_pri_gpc1_gnic0_preg_pm_ctrl_r()                 (0x00b20094U)
#define xbar_gpc0_gnic0_preg_ecc_status_r()                        (0x00b2004cU)
#define xbar_gpc0_gnic0_preg_ecc_status_uncorrected_err_e2e_m()\
				(U32(0x1U) << 0U)
#define xbar_gpc0_gnic0_preg_ecc_status_corrected_err_e2e_m()  (U32(0x1U) << 1U)
#define xbar_gpc0_gnic0_preg_ecc_status_uncorrected_err_total_counter_overflow_m()\
				(U32(0x1U) << 18U)
#define xbar_gpc0_gnic0_preg_ecc_status_reset_m()             (U32(0x1U) << 30U)
#define xbar_gpc0_gnic0_preg_ecc_status_reset_clear_f()            (0x40000000U)
#define xbar_gpc0_gnic0_preg_ecc_address_r()                       (0x00b20058U)
#define xbar_gpc0_gnic0_preg_ecc_uncorrected_err_count_r()         (0x00b20054U)
#define xbar_gpc0_gnic0_preg_ecc_uncorrected_err_count_total_s()           (16U)
#define xbar_gpc0_gnic0_preg_ecc_uncorrected_err_count_total_v(r)\
				(((r) >> 0U) & 0xffffU)
#endif
