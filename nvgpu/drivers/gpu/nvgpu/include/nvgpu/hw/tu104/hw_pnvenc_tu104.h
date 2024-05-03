/*
 * Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
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
#ifndef NVGPU_HW_PNVENC_TU104_H
#define NVGPU_HW_PNVENC_TU104_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define pnvenc_falcon_irqsset_r(i)\
		(nvgpu_safe_add_u32(0x001c8000U, nvgpu_safe_mult_u32((i), 16384U)))
#define pnvenc_falcon_itfen_r()                                      (0x001c8048U)
#define pnvenc_falcon_itfen_ctxen_enable_f()                                (0x1U)
#define pnvenc_falcon_itfen_mthden_enable_f()                               (0x2U)
#define pnvenc_falcon_engctl_r()                                     (0x001c80a4U)
#define pnvenc_falcon_engctl_stallreq_true_f()                            (0x100U)
#define pnvenc_falcon_engctl_stallack_v(r)                    (((r) >> 9U) & 0x1U)
#define pnvenc_falcon_engctl_stallack_true_f()                            (0x200U)
#define pnvenc_fbif_transcfg_r(i)\
		(nvgpu_safe_add_u32(0x001c8800U, nvgpu_safe_mult_u32((i), 4U)))
#define pnvenc_fbif_transcfg_target_local_fb_f()                            (0x0U)
#define pnvenc_fbif_transcfg_target_coherent_sysmem_f()                     (0x1U)
#define pnvenc_fbif_transcfg_target_noncoherent_sysmem_f()                  (0x2U)
#define pnvenc_fbif_transcfg_mem_type_s()                                     (1U)
#define pnvenc_fbif_transcfg_mem_type_f(v)                 ((U32(v) & 0x1U) << 2U)
#define pnvenc_fbif_transcfg_mem_type_m()                        (U32(0x1U) << 2U)
#define pnvenc_fbif_transcfg_mem_type_v(r)                    (((r) >> 2U) & 0x1U)
#define pnvenc_fbif_transcfg_mem_type_virtual_f()                           (0x0U)
#define pnvenc_fbif_transcfg_mem_type_physical_f()                          (0x4U)
#define pnvenc_fbif_ctl_r()                                          (0x001c8824U)
#define pnvenc_fbif_ctl_allow_phys_no_ctx_init_f()                          (0x0U)
#define pnvenc_fbif_ctl_allow_phys_no_ctx_disallow_f()                      (0x0U)
#define pnvenc_fbif_ctl_allow_phys_no_ctx_allow_f()                        (0x80U)
#define pnvenc_falcon_irqmset_r()                                    (0x001c8010U)
#define pnvenc_falcon_irqmset_gptmr_f(v)                   ((U32(v) & 0x1U) << 0U)
#define pnvenc_falcon_irqmset_wdtmr_f(v)                   ((U32(v) & 0x1U) << 1U)
#define pnvenc_falcon_irqmset_mthd_f(v)                    ((U32(v) & 0x1U) << 2U)
#define pnvenc_falcon_irqmset_ctxsw_f(v)                   ((U32(v) & 0x1U) << 3U)
#define pnvenc_falcon_irqmset_halt_f(v)                    ((U32(v) & 0x1U) << 4U)
#define pnvenc_falcon_irqmset_exterr_f(v)                  ((U32(v) & 0x1U) << 5U)
#define pnvenc_falcon_irqmset_swgen0_f(v)                  ((U32(v) & 0x1U) << 6U)
#define pnvenc_falcon_irqmset_swgen1_f(v)                  ((U32(v) & 0x1U) << 7U)
#define pnvenc_falcon_irqmset_ext_f(v)                    ((U32(v) & 0xffU) << 8U)
#define pnvenc_falcon_irqdest_r()                                    (0x001c801cU)
#define pnvenc_falcon_irqdest_host_gptmr_f(v)              ((U32(v) & 0x1U) << 0U)
#define pnvenc_falcon_irqdest_host_wdtmr_f(v)              ((U32(v) & 0x1U) << 1U)
#define pnvenc_falcon_irqdest_host_mthd_f(v)               ((U32(v) & 0x1U) << 2U)
#define pnvenc_falcon_irqdest_host_ctxsw_f(v)              ((U32(v) & 0x1U) << 3U)
#define pnvenc_falcon_irqdest_host_halt_f(v)               ((U32(v) & 0x1U) << 4U)
#define pnvenc_falcon_irqdest_host_exterr_f(v)             ((U32(v) & 0x1U) << 5U)
#define pnvenc_falcon_irqdest_host_swgen0_f(v)             ((U32(v) & 0x1U) << 6U)
#define pnvenc_falcon_irqdest_host_swgen1_f(v)             ((U32(v) & 0x1U) << 7U)
#define pnvenc_falcon_irqdest_host_ext_f(v)               ((U32(v) & 0xffU) << 8U)
#define pnvenc_falcon_irqdest_target_gptmr_f(v)           ((U32(v) & 0x1U) << 16U)
#define pnvenc_falcon_irqdest_target_wdtmr_f(v)           ((U32(v) & 0x1U) << 17U)
#define pnvenc_falcon_irqdest_target_mthd_f(v)            ((U32(v) & 0x1U) << 18U)
#define pnvenc_falcon_irqdest_target_ctxsw_f(v)           ((U32(v) & 0x1U) << 19U)
#define pnvenc_falcon_irqdest_target_halt_f(v)            ((U32(v) & 0x1U) << 20U)
#define pnvenc_falcon_irqdest_target_exterr_f(v)          ((U32(v) & 0x1U) << 21U)
#define pnvenc_falcon_irqdest_target_swgen0_f(v)          ((U32(v) & 0x1U) << 22U)
#define pnvenc_falcon_irqdest_target_swgen1_f(v)          ((U32(v) & 0x1U) << 23U)
#define pnvenc_falcon_irqdest_target_ext_f(v)            ((U32(v) & 0xffU) << 24U)
#endif
