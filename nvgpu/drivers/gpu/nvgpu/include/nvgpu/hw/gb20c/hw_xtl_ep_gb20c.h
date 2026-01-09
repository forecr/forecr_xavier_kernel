/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#ifndef NVGPU_HW_XTL_EP_GB20C_H
#define NVGPU_HW_XTL_EP_GB20C_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define xtl_ep_xtl_base_v()                                        (0x00091000U)
#define xtl_ep_pri_intr_enable_r(i)\
		(nvgpu_safe_add_u32(0x00000338U, nvgpu_safe_mult_u32((i), 4U)))
#define xtl_ep_pri_intr_enable_ep_ec_corr_err_m()             (U32(0x1U) << 17U)
#define xtl_ep_pri_intr_enable_ep_ec_uncorr_err_m()           (U32(0x1U) << 16U)
#define xtl_ep_pri_intr_status_r(i)\
		(nvgpu_safe_add_u32(0x000002d8U, nvgpu_safe_mult_u32((i), 4U)))
#define xtl_ep_pri_intr_status_ep_ec_corr_err_m()             (U32(0x1U) << 17U)
#define xtl_ep_pri_intr_status_ep_ec_uncorr_err_m()           (U32(0x1U) << 16U)
#define xtl_ep_pri_intr_mask_clr_r(i)\
		(nvgpu_safe_add_u32(0x00000318U, nvgpu_safe_mult_u32((i), 4U)))
#define xtl_ep_pri_intr_mask_clr_ep_ec_corr_err_m()           (U32(0x1U) << 17U)
#define xtl_ep_pri_intr_mask_clr_ep_ec_uncorr_err_m()         (U32(0x1U) << 16U)
#define xtl_ep_pri_intr_ctrl_r(i)\
		(nvgpu_safe_add_u32(0x000002a0U, nvgpu_safe_mult_u32((i), 4U)))
#define xtl_ep_pri_intr_retrigger_r(i)\
		(nvgpu_safe_add_u32(0x000002c0U, nvgpu_safe_mult_u32((i), 4U)))
#define xtl_ep_pri_intr_retrigger_trigger_true_f()                        (0x1U)
#endif
