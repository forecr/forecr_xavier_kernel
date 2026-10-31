/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#ifndef NVGPU_HW_PNVDEC_GB20C_H
#define NVGPU_HW_PNVDEC_GB20C_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define pnvdec_falcon_irqsset_r(i)\
		(nvgpu_safe_add_u32(0x00848000U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_falcon_irqsclr_r(i)\
		(nvgpu_safe_add_u32(0x00848004U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_falcon_irqstat_r(i)\
		(nvgpu_safe_add_u32(0x00848008U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_falcon_irqstat_halt_f(v)                  ((U32(v) & 0x1U) << 4U)
#define pnvdec_falcon_irqstat_halt_true_f()                              (0x10U)
#define pnvdec_falcon_irqstat_swgen0_f(v)                ((U32(v) & 0x1U) << 6U)
#define pnvdec_falcon_irqstat_swgen0_true_f()                            (0x40U)
#define pnvdec_falcon_irqstat_swgen1_f(v)                ((U32(v) & 0x1U) << 7U)
#define pnvdec_falcon_irqstat_swgen1_true_f()                            (0x80U)
#define pnvdec_falcon_intr_ctrl_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x008483e0U, \
		nvgpu_safe_mult_u32((i), 16384U)), nvgpu_safe_mult_u32((j), 4U)))
#define pnvdec_falcon_irqmclr_r(i)\
		(nvgpu_safe_add_u32(0x00848014U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_falcon_engctl_r(i)\
		(nvgpu_safe_add_u32(0x008480a4U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_falcon_engctl_set_stallreq_true_f()                        (0x2U)
#define pnvdec_falcon_engctl_stallack_v(r)                  (((r) >> 9U) & 0x1U)
#define pnvdec_falcon_engctl_stallack_true_v()                     (0x00000001U)
#define pnvdec_falcon_engine_r(i)\
		(nvgpu_safe_add_u32(0x008483c0U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_falcon_engine_reset_false_f()                              (0x0U)
#define pnvdec_falcon_engine_reset_true_f()                               (0x1U)
#define pnvdec_falcon_engine_reset_status_v(r)              (((r) >> 8U) & 0x7U)
#define pnvdec_falcon_engine_reset_status_asserted_v()             (0x00000000U)
#define pnvdec_falcon_engine_reset_status_deasserted_v()           (0x00000002U)
#define pnvdec_fbif_transcfg_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x0084be00U, \
		nvgpu_safe_mult_u32((i), 16384U)), nvgpu_safe_mult_u32((j), 4U)))
#define pnvdec_fbif_transcfg_target_local_fb_f()                          (0x0U)
#define pnvdec_fbif_transcfg_target_coherent_sysmem_f()                   (0x1U)
#define pnvdec_fbif_transcfg_target_noncoherent_sysmem_f()                (0x2U)
#define pnvdec_fbif_transcfg_mem_type_virtual_f()                         (0x0U)
#define pnvdec_fbif_transcfg_mem_type_physical_f()                        (0x4U)
#define pnvdec_fbif_ctl_r(i)\
		(nvgpu_safe_add_u32(0x0084be24U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_fbif_ctl_allow_phys_no_ctx_allow_f()                      (0x80U)
#define pnvdec_falcon_debuginfo_r(i)\
		(nvgpu_safe_add_u32(0x00848094U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_riscv_cpuctl_r(i)\
		(nvgpu_safe_add_u32(0x0084b388U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_falcon_mailbox0_r(i)\
		(nvgpu_safe_add_u32(0x00848040U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvdec_falcon_mailbox1_r(i)\
		(nvgpu_safe_add_u32(0x00848044U, nvgpu_safe_mult_u32((i), 16384U)))
#endif
