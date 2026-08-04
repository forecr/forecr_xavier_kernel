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
#ifndef NVGPU_HW_FUNC_GB10B_H
#define NVGPU_HW_FUNC_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define func_full_phys_offset_v()                                  (0x00b80000U)
#define func_doorbell_r()                                          (0x00030090U)
#define func_cfg0_r()                                              (0x00030000U)
#define func_priv_func_l2_system_invalidate_r()                    (0x00000f10U)
#define func_priv_func_l2_system_invalidate_token_s()                      (31U)
#define func_priv_func_l2_system_invalidate_token_v(r)\
				(((r) >> 0U) & 0x7fffffffU)
#define func_priv_func_l2_system_invalidate_completed_r()          (0x00000f14U)
#define func_priv_func_l2_system_invalidate_completed_token_v(r)\
				(((r) >> 0U) & 0x7fffffffU)
#define func_priv_func_l2_system_invalidate_completed_status_v(r)\
				(((r) >> 31U) & 0x1U)
#define func_priv_func_l2_system_invalidate_completed_status_idle_v()\
				(0x00000000U)
#define func_priv_func_l2_system_invalidate_completed_status_busy_v()\
				(0x00000001U)
#define func_priv_cpu_intr_top_en_set_r(i)\
		(nvgpu_safe_add_u32(0x00001608U, nvgpu_safe_mult_u32((i), 4U)))
#define func_priv_cpu_intr_top_en_clear_r(i)\
		(nvgpu_safe_add_u32(0x00001610U, nvgpu_safe_mult_u32((i), 4U)))
#define func_priv_cpu_intr_top_en_clear__size_1_v()                (0x00000001U)
#define func_priv_cpu_intr_top_en_set_r(i)\
		(nvgpu_safe_add_u32(0x00001608U, nvgpu_safe_mult_u32((i), 4U)))
#define func_priv_cpu_intr_leaf_en_set_r(i)\
		(nvgpu_safe_add_u32(0x00001200U, nvgpu_safe_mult_u32((i), 4U)))
#define func_priv_cpu_intr_leaf_en_clear_r(i)\
		(nvgpu_safe_add_u32(0x00001400U, nvgpu_safe_mult_u32((i), 4U)))
#define func_priv_cpu_intr_top_r(i)\
		(nvgpu_safe_add_u32(0x00001600U, nvgpu_safe_mult_u32((i), 4U)))
#define func_priv_cpu_intr_top__size_1_v()                         (0x00000001U)
#define func_priv_cpu_intr_leaf_r(i)\
		(nvgpu_safe_add_u32(0x00001000U, nvgpu_safe_mult_u32((i), 4U)))
#define func_priv_cpu_intr_leaf__size_1_v()                        (0x00000010U)
#define func_priv_intr_count_r()                                   (0x00001644U)
#define func_priv_intr_count_size_v(r)                    (((r) >> 0U) & 0xfffU)
#define func_bar1_block_low_addr_r()                               (0x00000f60U)
#define func_bar1_block_low_addr_target_vid_mem_f()                       (0x0U)
#define func_bar1_block_low_addr_target_sys_mem_coh_f()                 (0x800U)
#define func_bar1_block_low_addr_target_sys_mem_ncoh_f()                (0xc00U)
#define func_bar1_block_low_addr_bar1_pending_v(r)          (((r) >> 0U) & 0x1U)
#define func_bar1_block_low_addr_bar1_pending_empty_f()                   (0x0U)
#define func_bar1_block_low_addr_bar1_outstanding_v(r)      (((r) >> 1U) & 0x1U)
#define func_bar1_block_low_addr_bar1_outstanding_false_f()               (0x0U)
#define func_bar1_block_low_addr_mode_virtual_f()                       (0x200U)
#define func_bar1_block_low_addr_ptr_f(v)           ((U32(v) & 0xfffffU) << 12U)
#define func_bar1_block_high_addr_r()                              (0x00000f64U)
#define func_bar1_block_high_addr_ptr_f(v)        ((U32(v) & 0xffffffffU) << 0U)
#define func_bar1_block_ptr_shift_v()                              (0x0000000cU)
#define func_bar2_block_low_addr_r()                               (0x00000f70U)
#define func_bar2_block_low_addr_target_vid_mem_f()                       (0x0U)
#define func_bar2_block_low_addr_target_sys_mem_coh_f()                 (0x800U)
#define func_bar2_block_low_addr_target_sys_mem_ncoh_f()                (0xc00U)
#define func_bar2_block_low_addr_bar2_pending_v(r)          (((r) >> 0U) & 0x1U)
#define func_bar2_block_low_addr_bar2_pending_empty_f()                   (0x0U)
#define func_bar2_block_low_addr_bar2_outstanding_v(r)      (((r) >> 1U) & 0x1U)
#define func_bar2_block_low_addr_bar2_outstanding_false_f()               (0x0U)
#define func_bar2_block_low_addr_mode_virtual_f()                       (0x200U)
#define func_bar2_block_low_addr_ptr_f(v)           ((U32(v) & 0xfffffU) << 12U)
#define func_bar2_block_high_addr_r()                              (0x00000f74U)
#define func_bar2_block_high_addr_ptr_f(v)           ((U32(v) & 0xfffffU) << 0U)
#define func_bar2_block_ptr_shift_v()                              (0x0000000cU)
#define func_priv_cpu_intr_pfb_vector_v()                          (0x0000008dU)
#define func_priv_cpu_intr_pmu_vector_v()                          (0x00000098U)
#define func_priv_cpu_intr_ltc_all_vector_v()                      (0x00000099U)
#define func_priv_cpu_intr_pbus_vector_v()                         (0x0000009cU)
#define func_priv_cpu_intr_priv_ring_vector_v()                    (0x0000009eU)
#define func_priv_mmu_fault_buffer_lo_r(i)\
		(nvgpu_safe_add_u32(0x00003000U, nvgpu_safe_mult_u32((i), 32U)))
#define func_priv_mmu_fault_buffer_lo_addr_f(v)     ((U32(v) & 0xfffffU) << 12U)
#define func_priv_mmu_fault_buffer_hi_r(i)\
		(nvgpu_safe_add_u32(0x00003004U, nvgpu_safe_mult_u32((i), 32U)))
#define func_priv_mmu_fault_buffer_hi_addr_f(v)   ((U32(v) & 0xffffffffU) << 0U)
#define func_priv_mmu_fault_buffer_get_r(i)\
		(nvgpu_safe_add_u32(0x00003008U, nvgpu_safe_mult_u32((i), 32U)))
#define func_priv_mmu_fault_buffer_get_ptr_f(v)      ((U32(v) & 0xfffffU) << 0U)
#define func_priv_mmu_fault_buffer_get_ptr_m()             (U32(0xfffffU) << 0U)
#define func_priv_mmu_fault_buffer_get_ptr_v(r)         (((r) >> 0U) & 0xfffffU)
#define func_priv_mmu_fault_buffer_get_getptr_corrupted_m()   (U32(0x1U) << 30U)
#define func_priv_mmu_fault_buffer_get_getptr_corrupted_clear_f()  (0x40000000U)
#define func_priv_mmu_fault_buffer_get_overflow_m()           (U32(0x1U) << 31U)
#define func_priv_mmu_fault_buffer_get_overflow_clear_f()          (0x80000000U)
#define func_priv_mmu_fault_buffer_put_r(i)\
		(nvgpu_safe_add_u32(0x0000300cU, nvgpu_safe_mult_u32((i), 32U)))
#define func_priv_mmu_fault_buffer_put_ptr_v(r)         (((r) >> 0U) & 0xfffffU)
#define func_priv_mmu_fault_buffer_size_r(i)\
		(nvgpu_safe_add_u32(0x00003010U, nvgpu_safe_mult_u32((i), 32U)))
#define func_priv_mmu_fault_buffer_size_val_f(v)     ((U32(v) & 0xfffffU) << 0U)
#define func_priv_mmu_fault_buffer_size_val_v(r)        (((r) >> 0U) & 0xfffffU)
#define func_priv_mmu_fault_buffer_size_overflow_intr_enable_f()   (0x20000000U)
#define func_priv_mmu_fault_buffer_size_enable_m()            (U32(0x1U) << 31U)
#define func_priv_mmu_fault_buffer_size_enable_true_f()            (0x80000000U)
#define func_priv_mmu_fault_addr_lo_r()                            (0x00003080U)
#define func_priv_mmu_fault_addr_lo_phys_aperture_v(r)      (((r) >> 0U) & 0x3U)
#define func_priv_mmu_fault_addr_lo_addr_v(r)          (((r) >> 12U) & 0xfffffU)
#define func_priv_mmu_fault_addr_hi_r()                            (0x00003084U)
#define func_priv_mmu_fault_addr_hi_addr_v(r)        (((r) >> 0U) & 0xffffffffU)
#define func_priv_mmu_fault_inst_lo_r()                            (0x00003088U)
#define func_priv_mmu_fault_inst_lo_engine_id_v(r)        (((r) >> 0U) & 0x1ffU)
#define func_priv_mmu_fault_inst_lo_aperture_v(r)          (((r) >> 10U) & 0x3U)
#define func_priv_mmu_fault_inst_lo_addr_v(r)          (((r) >> 12U) & 0xfffffU)
#define func_priv_mmu_fault_inst_hi_r()                            (0x0000308cU)
#define func_priv_mmu_fault_inst_hi_addr_v(r)        (((r) >> 0U) & 0xffffffffU)
#define func_priv_mmu_fault_info_r()                               (0x00003090U)
#define func_priv_mmu_fault_info_fault_type_v(r)           (((r) >> 0U) & 0x1fU)
#define func_priv_mmu_fault_info_replayable_fault_v(r)      (((r) >> 7U) & 0x1U)
#define func_priv_mmu_fault_info_client_v(r)               (((r) >> 8U) & 0x7fU)
#define func_priv_mmu_fault_info_access_type_v(r)          (((r) >> 16U) & 0xfU)
#define func_priv_mmu_fault_info_client_type_v(r)          (((r) >> 20U) & 0x1U)
#define func_priv_mmu_fault_info_gpc_id_v(r)              (((r) >> 24U) & 0x1fU)
#define func_priv_mmu_fault_info_protected_mode_v(r)       (((r) >> 29U) & 0x1U)
#define func_priv_mmu_fault_info_replayable_fault_en_v(r)  (((r) >> 30U) & 0x1U)
#define func_priv_mmu_fault_info_valid_v(r)                (((r) >> 31U) & 0x1U)
#define func_priv_mmu_fault_status_r()                             (0x00003094U)
#define func_priv_mmu_fault_status_dropped_bar1_phys_set_f()              (0x1U)
#define func_priv_mmu_fault_status_dropped_bar1_virt_set_f()              (0x2U)
#define func_priv_mmu_fault_status_dropped_bar2_phys_set_f()              (0x4U)
#define func_priv_mmu_fault_status_dropped_bar2_virt_set_f()              (0x8U)
#define func_priv_mmu_fault_status_dropped_ifb_phys_set_f()              (0x10U)
#define func_priv_mmu_fault_status_dropped_ifb_virt_set_f()              (0x20U)
#define func_priv_mmu_fault_status_dropped_other_phys_set_f()            (0x40U)
#define func_priv_mmu_fault_status_dropped_other_virt_set_f()            (0x80U)
#define func_priv_mmu_fault_status_replayable_m()              (U32(0x1U) << 8U)
#define func_priv_mmu_fault_status_replayable_error_m()       (U32(0x1U) << 10U)
#define func_priv_mmu_fault_status_non_replayable_error_m()   (U32(0x1U) << 11U)
#define func_priv_mmu_fault_status_replayable_overflow_m()    (U32(0x1U) << 12U)
#define func_priv_mmu_fault_status_non_replayable_overflow_m()\
				(U32(0x1U) << 13U)
#define func_priv_mmu_fault_status_replayable_getptr_corrupted_m()\
				(U32(0x1U) << 14U)
#define func_priv_mmu_fault_status_non_replayable_getptr_corrupted_m()\
				(U32(0x1U) << 15U)
#define func_priv_mmu_fault_status_vab_error_m()              (U32(0x1U) << 16U)
#define func_priv_mmu_fault_status_busy_true_f()                   (0x40000000U)
#define func_priv_mmu_fault_status_valid_m()                  (U32(0x1U) << 31U)
#define func_priv_mmu_fault_status_valid_set_f()                   (0x80000000U)
#define func_priv_mmu_fault_status_valid_clear_f()                 (0x80000000U)
#define func_priv_mmu_invalidate_pdb_r()                           (0x000030a0U)
#define func_priv_mmu_invalidate_pdb_aperture_vid_mem_f()                 (0x0U)
#define func_priv_mmu_invalidate_pdb_aperture_sys_mem_f()                 (0x2U)
#define func_priv_mmu_invalidate_pdb_addr_f(v)     ((U32(v) & 0xfffffffU) << 4U)
#define func_priv_mmu_invalidate_pdb_addr_alignment_v()            (0x0000000cU)
#define func_priv_mmu_invalidate_upper_pdb_r()                     (0x000030a4U)
#define func_priv_mmu_invalidate_upper_pdb_addr_f(v) ((U32(v) & 0xfffffU) << 0U)
#define func_priv_mmu_invalidate_r()                               (0x000030b0U)
#define func_priv_mmu_invalidate_all_va_true_f()                          (0x1U)
#define func_priv_mmu_invalidate_hubtlb_only_true_f()                     (0x4U)
#define func_priv_mmu_invalidate_scope_non_link_tlbs_f()              (0x10000U)
#define func_priv_mmu_invalidate_trigger_v(r)              (((r) >> 31U) & 0x1U)
#define func_priv_mmu_invalidate_trigger_true_v()                  (0x00000001U)
#define func_priv_mmu_invalidate_trigger_true_f()                  (0x80000000U)
#define func_priv_mmu_invalidate_trigger_false_v()                 (0x00000000U)
#define func_priv_mmu_invalidate_trigger_false_f()                        (0x0U)
#define func_priv_mailbox_scratch_r(i)\
		(nvgpu_safe_add_u32(0x00002100U, nvgpu_safe_mult_u32((i), 4U)))
#endif
