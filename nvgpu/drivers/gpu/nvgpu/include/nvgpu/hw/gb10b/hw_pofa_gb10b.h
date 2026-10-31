/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#ifndef NVGPU_HW_POFA_GB10B_H
#define NVGPU_HW_POFA_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define pofa_falcon_irqsset_r()                                    (0x00844000U)
#define pofa_falcon_irqsclr_r()                                    (0x00844004U)
#define pofa_falcon_irqstat_r()                                    (0x00844008U)
#define pofa_falcon_irqstat_halt_f(v)                    ((U32(v) & 0x1U) << 4U)
#define pofa_falcon_irqstat_halt_true_f()                                (0x10U)
#define pofa_falcon_irqstat_swgen0_f(v)                  ((U32(v) & 0x1U) << 6U)
#define pofa_falcon_irqstat_swgen0_true_f()                              (0x40U)
#define pofa_falcon_irqstat_swgen1_f(v)                  ((U32(v) & 0x1U) << 7U)
#define pofa_falcon_irqstat_swgen1_true_f()                              (0x80U)
#define pofa_falcon_intr_ctrl_r(i)\
		(nvgpu_safe_add_u32(0x008443e0U, nvgpu_safe_mult_u32((i), 4U)))
#define pofa_falcon_irqmclr_r()                                    (0x00844014U)
#define pofa_falcon_engctl_r()                                     (0x008440a4U)
#define pofa_falcon_engctl_set_stallreq_true_f()                          (0x2U)
#define pofa_falcon_engctl_stallack_v(r)                    (((r) >> 9U) & 0x1U)
#define pofa_falcon_engctl_stallack_true_v()                       (0x00000001U)
#define pofa_falcon_engine_r()                                     (0x008443c0U)
#define pofa_falcon_engine_reset_false_f()                                (0x0U)
#define pofa_falcon_engine_reset_true_f()                                 (0x1U)
#define pofa_falcon_engine_reset_status_v(r)                (((r) >> 8U) & 0x7U)
#define pofa_falcon_engine_reset_status_asserted_v()               (0x00000000U)
#define pofa_falcon_engine_reset_status_deasserted_v()             (0x00000002U)
#define pofa_fbif_transcfg_r(i)\
		(nvgpu_safe_add_u32(0x00844400U, nvgpu_safe_mult_u32((i), 4U)))
#define pofa_fbif_transcfg_target_local_fb_f()                            (0x0U)
#define pofa_fbif_transcfg_target_coherent_sysmem_f()                     (0x1U)
#define pofa_fbif_transcfg_target_noncoherent_sysmem_f()                  (0x2U)
#define pofa_fbif_transcfg_mem_type_virtual_f()                           (0x0U)
#define pofa_fbif_transcfg_mem_type_physical_f()                          (0x4U)
#define pofa_fbif_ctl_r()                                          (0x00844424U)
#define pofa_fbif_ctl_allow_phys_no_ctx_allow_f()                        (0x80U)
#define pofa_falcon_debuginfo_r()                                  (0x00844094U)
#define pofa_riscv_cpuctl_r()                                      (0x00844b88U)
#define pofa_errslice0_missionerr_status_r()                       (0x00847038U)
#define pofa_errslice1_missionerr_status_r()                       (0x00847068U)
#define pofa_errslice2_missionerr_status_r()                       (0x00847098U)
#define pofa_errslice3_missionerr_status_r()                       (0x008470c8U)
#define pofa_errslice4_missionerr_status_r()                       (0x008470f8U)
#define pofa_errslice5_missionerr_status_r()                       (0x00847128U)
#define pofa_errslice6_missionerr_status_r()                       (0x00847158U)
#define pofa_errslice7_missionerr_status_r()                       (0x00847188U)
#define pofa_errslice8_missionerr_status_r()                       (0x008471b8U)
#define pofa_errslice9_missionerr_status_r()                       (0x008471e8U)
#define pofa_errslice10_missionerr_status_r()                      (0x00847218U)
#define pofa_errslice11_missionerr_status_r()                      (0x00847248U)
#define pofa_errslice12_missionerr_status_r()                      (0x00847278U)
#define pofa_errslice13_missionerr_status_r()                      (0x008472a8U)
#define pofa_errslice14_missionerr_status_r()                      (0x008472d8U)
#define pofa_errslice15_missionerr_status_r()                      (0x00847308U)
#define pofa_errslice16_missionerr_status_r()                      (0x00847338U)
#define pofa_errslice17_missionerr_status_r()                      (0x00847368U)
#define pofa_falcon_intr_ctrl2_r()                                 (0x00844a70U)
#define pofa_falcon_intr_ctrl3_r()                                 (0x00844a78U)
#define pofa_falcon_mailbox0_r()                                   (0x00844040U)
#define pofa_falcon_mailbox1_r()                                   (0x00844044U)
#define pofa_falcon_safety_mailbox_r()                             (0x008442f0U)
#endif
