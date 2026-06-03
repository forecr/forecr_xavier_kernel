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
#ifndef NVGPU_HW_PBDMA_GB20C_H
#define NVGPU_HW_PBDMA_GB20C_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define pbdma_gp_entry1_r()                                        (0x10000004U)
#define pbdma_gp_entry1_length_f(v)                ((U32(v) & 0x1fffffU) << 10U)
#define pbdma_gp_base_r()                                          (0x00000090U)
#define pbdma_gp_base_offset_f(v)                 ((U32(v) & 0x1fffffffU) << 3U)
#define pbdma_gp_base_rsvd_s()                                              (3U)
#define pbdma_gp_base_hi_r()                                       (0x00000094U)
#define pbdma_gp_base_hi_offset_f(v)               ((U32(v) & 0x1ffffffU) << 0U)
#define pbdma_gp_fetch_r()                                         (0x000000c8U)
#define pbdma_gp_get_r()                                           (0x00000098U)
#define pbdma_gp_put_r()                                           (0x000000c0U)
#define pbdma_get_r()                                              (0x00000080U)
#define pbdma_get_hi_r()                                           (0x00000084U)
#define pbdma_put_r()                                              (0x00000088U)
#define pbdma_put_hi_r()                                           (0x0000008cU)
#define pbdma_pb_header_r()                                        (0x00000420U)
#define pbdma_pb_header_method_zero_f()                                   (0x0U)
#define pbdma_pb_header_subchannel_zero_f()                               (0x0U)
#define pbdma_pb_header_level_main_f()                                    (0x0U)
#define pbdma_pb_header_first_true_f()                               (0x400000U)
#define pbdma_misc_fetch_state_r()                                 (0x000000b4U)
#define pbdma_misc_fetch_state_pb_header_type_inc_f()                     (0x8U)
#define pbdma_misc_fetch_state_pb_header_type_non_inc_f()                (0x18U)
#define pbdma_misc_fetch_state_gp_info_limit2_s()                           (5U)
#define pbdma_misc_fetch_state_gp_info_limit2_f(v)     ((U32(v) & 0x1fU) << 24U)
#define pbdma_pb_segment_extended_base_r()                         (0x0000005cU)
#define pbdma_pb_segment_extended_base_value_f(v)    ((U32(v) & 0x1ffffU) << 8U)
#define pbdma_hdr_shadow_r()                                       (0x000000ccU)
#define pbdma_gp_shadow_r(i)\
		(nvgpu_safe_add_u32(0x00000110U, nvgpu_safe_mult_u32((i), 4U)))
#define pbdma_gp_shadow_size_v()                                   (0x00000002U)
#define pbdma_subdevice_r()                                        (0x0000003cU)
#define pbdma_subdevice_id_f(v)                        ((U32(v) & 0xfffU) << 0U)
#define pbdma_misc_fetch_state_r()                                 (0x000000b4U)
#define pbdma_misc_fetch_state_subdevice_status_active_f()            (0x20000U)
#define pbdma_misc_fetch_state_subdevice_channel_dma_enable_f()       (0x40000U)
#define pbdma_method_r(i)\
		(nvgpu_safe_add_u32(0x00000400U, nvgpu_safe_mult_u32((i), 8U)))
#define pbdma_method_size_v()                                      (0x00000004U)
#define pbdma_method_addr_f(v)                         ((U32(v) & 0xfffU) << 2U)
#define pbdma_method_addr_v(r)                            (((r) >> 2U) & 0xfffU)
#define pbdma_method_subch_v(r)                            (((r) >> 16U) & 0x7U)
#define pbdma_method_first_true_f()                                  (0x400000U)
#define pbdma_method_valid_true_f()                                (0x80000000U)
#define pbdma_signature_r()                                        (0x00000038U)
#define pbdma_signature_sw_zero_f()                                       (0x0U)
#define pbdma_hce_ctrl_r()                                         (0x00000054U)
#define pbdma_hce_ctrl_hce_priv_mode_yes_f()                             (0x20U)
#define pbdma_hce_dbg0_r()                                         (0x00000150U)
#define pbdma_hce_dbg1_r()                                         (0x00000154U)
#define pbdma_intr_0_r()                                           (0x00000108U)
#define pbdma_intr_0_gpfifo_pending_f()                                (0x2000U)
#define pbdma_intr_0_gpptr_pending_f()                                 (0x4000U)
#define pbdma_intr_0_pbptr_pending_f()                                (0x20000U)
#define pbdma_intr_0_pbentry_pending_f()                              (0x40000U)
#define pbdma_intr_0_method_pending_f()                              (0x200000U)
#define pbdma_intr_0_device_pending_f()                              (0x800000U)
#define pbdma_intr_0_semaphore_pending_f()                          (0x2000000U)
#define pbdma_intr_0_pri_pending_f()                                (0x8000000U)
#define pbdma_intr_0_pbseg_pending_f()                             (0x40000000U)
#define pbdma_intr_0_signature_pending_f()                         (0x80000000U)
#define pbdma_intr_1_r()                                           (0x00000148U)
#define pbdma_intr_1_ctxnotvalid_pending_f()                       (0x80000000U)
#define pbdma_intr_0_en_set_tree_r(i)\
		(nvgpu_safe_add_u32(0x00000170U, nvgpu_safe_mult_u32((i), 4U)))
#define pbdma_intr_0_en_set_tree__size_1_v()                       (0x00000002U)
#define pbdma_intr_0_en_set_tree_gpfifo_enabled_f()                    (0x2000U)
#define pbdma_intr_0_en_set_tree_gpptr_enabled_f()                     (0x4000U)
#define pbdma_intr_0_en_set_tree_gpentry_enabled_f()                   (0x8000U)
#define pbdma_intr_0_en_set_tree_pbptr_enabled_f()                    (0x20000U)
#define pbdma_intr_0_en_set_tree_pbentry_enabled_f()                  (0x40000U)
#define pbdma_intr_0_en_set_tree_method_enabled_f()                  (0x200000U)
#define pbdma_intr_0_en_set_tree_device_enabled_f()                  (0x800000U)
#define pbdma_intr_0_en_set_tree_semaphore_enabled_f()              (0x2000000U)
#define pbdma_intr_0_en_set_tree_pri_enabled_f()                    (0x8000000U)
#define pbdma_intr_0_en_set_tree_pbseg_enabled_f()                 (0x40000000U)
#define pbdma_intr_0_en_set_tree_signature_enabled_f()             (0x80000000U)
#define pbdma_intr_0_en_clear_tree_r(i)\
		(nvgpu_safe_add_u32(0x00000190U, nvgpu_safe_mult_u32((i), 4U)))
#define pbdma_intr_0_en_clear_tree__size_1_v()                     (0x00000002U)
#define pbdma_intr_0_en_clear_tree_gpfifo_enabled_f()                  (0x2000U)
#define pbdma_intr_0_en_clear_tree_gpptr_enabled_f()                   (0x4000U)
#define pbdma_intr_0_en_clear_tree_gpentry_enabled_f()                 (0x8000U)
#define pbdma_intr_0_en_clear_tree_pbptr_enabled_f()                  (0x20000U)
#define pbdma_intr_0_en_clear_tree_pbentry_enabled_f()                (0x40000U)
#define pbdma_intr_0_en_clear_tree_method_enabled_f()                (0x200000U)
#define pbdma_intr_0_en_clear_tree_device_enabled_f()                (0x800000U)
#define pbdma_intr_0_en_clear_tree_semaphore_enabled_f()            (0x2000000U)
#define pbdma_intr_0_en_clear_tree_pri_enabled_f()                  (0x8000000U)
#define pbdma_intr_0_en_clear_tree_pbseg_enabled_f()               (0x40000000U)
#define pbdma_intr_0_en_clear_tree_signature_enabled_f()           (0x80000000U)
#define pbdma_intr_1_en_set_tree_r(i)\
		(nvgpu_safe_add_u32(0x00000180U, nvgpu_safe_mult_u32((i), 4U)))
#define pbdma_intr_1_en_set_tree_hce_re_illegal_op_enabled_f()            (0x1U)
#define pbdma_intr_1_en_set_tree_hce_re_alignb_enabled_f()                (0x2U)
#define pbdma_intr_1_en_set_tree_hce_priv_enabled_f()                     (0x4U)
#define pbdma_intr_1_en_set_tree_hce_illegal_mthd_enabled_f()             (0x8U)
#define pbdma_intr_1_en_set_tree_hce_illegal_class_enabled_f()           (0x10U)
#define pbdma_intr_1_en_set_tree_ctxnotvalid_enabled_f()           (0x80000000U)
#define pbdma_intr_1_en_clear_tree_r(i)\
		(nvgpu_safe_add_u32(0x000001a0U, nvgpu_safe_mult_u32((i), 4U)))
#define pbdma_intr_1_en_clear_tree_hce_re_illegal_op_enabled_f()          (0x1U)
#define pbdma_intr_1_en_clear_tree_hce_re_alignb_enabled_f()              (0x2U)
#define pbdma_intr_1_en_clear_tree_hce_priv_enabled_f()                   (0x4U)
#define pbdma_intr_1_en_clear_tree_hce_illegal_mthd_enabled_f()           (0x8U)
#define pbdma_intr_1_en_clear_tree_hce_illegal_class_enabled_f()         (0x10U)
#define pbdma_intr_1_en_clear_tree_ctxnotvalid_enabled_f()         (0x80000000U)
#define pbdma_udma_nop_r()                                         (0x00000008U)
#define pbdma_misc_execute_state_r()                               (0x00000050U)
#define pbdma_misc_execute_state_target_engine_f(v)      ((U32(v) & 0x1U) << 0U)
#define pbdma_misc_execute_state_target_eng_ctx_valid_true_f()            (0x2U)
#define pbdma_misc_execute_state_target_ce_ctx_valid_true_f()             (0x4U)
#define pbdma_misc_execute_state_config_auth_level_privileged_f()      (0x1000U)
#define pbdma_status_sched_r()                                     (0x0000015cU)
#define pbdma_status_sched_tsgid_v(r)                     (((r) >> 0U) & 0xfffU)
#define pbdma_status_sched_chan_status_v(r)                (((r) >> 13U) & 0x7U)
#define pbdma_status_sched_chan_status_valid_v()                   (0x00000001U)
#define pbdma_status_sched_chan_status_chsw_save_v()               (0x00000005U)
#define pbdma_status_sched_chan_status_chsw_load_v()               (0x00000006U)
#define pbdma_status_sched_chan_status_chsw_switch_v()             (0x00000007U)
#define pbdma_status_sched_next_tsgid_v(r)               (((r) >> 16U) & 0xfffU)
#define pbdma_channel_r()                                          (0x00000120U)
#define pbdma_channel_gfid_v(r)                           (((r) >> 16U) & 0x3fU)
#define pbdma_next_channel_r()                                     (0x00000124U)
#define pbdma_next_channel_valid_v(r)                      (((r) >> 13U) & 0x1U)
#define pbdma_next_channel_gfid_v(r)                      (((r) >> 16U) & 0x3fU)
#define pbdma_cfg0_r()                                             (0x00000104U)
#define pbdma_cfg0_pbdma_fault_id_v(r)                    (((r) >> 0U) & 0x3ffU)
#endif
