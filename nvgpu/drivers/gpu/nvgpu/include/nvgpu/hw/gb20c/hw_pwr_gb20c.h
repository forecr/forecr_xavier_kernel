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
#ifndef NVGPU_HW_PWR_GB20C_H
#define NVGPU_HW_PWR_GB20C_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define pwr_falcon2_pwr_base_r()                                   (0x008f5000U)
#define pwr_falcon_intr_ctrl_r(i)\
		(nvgpu_safe_add_u32(0x008f43e0U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_falcon_irqsset_r()                                     (0x008f4000U)
#define pwr_falcon_irqsset_swgen0_set_f()                                (0x40U)
#define pwr_falcon_irqsclr_r()                                     (0x008f4004U)
#define pwr_falcon_irqstat_r()                                     (0x008f4008U)
#define pwr_falcon_irqstat_wdt_true_f()                                   (0x2U)
#define pwr_falcon_irqstat_halt_true_f()                                 (0x10U)
#define pwr_falcon_irqstat_exterr_true_f()                               (0x20U)
#define pwr_falcon_irqstat_swgen0_true_f()                               (0x40U)
#define pwr_falcon_irqstat_ext_ecc_parity_true_f()                      (0x400U)
#define pwr_falcon_irqstat_swgen1_true_f()                               (0x80U)
#define pwr_falcon_irqstat_memerr_true_f()                            (0x40000U)
#define pwr_falcon_irqstat_iopmp_true_f()                            (0x800000U)
#define pwr_falcon_irqmset_r()                                     (0x008f4010U)
#define pwr_falcon_irqmset_gptmr_f(v)                    ((U32(v) & 0x1U) << 0U)
#define pwr_falcon_irqmset_wdtmr_f(v)                    ((U32(v) & 0x1U) << 1U)
#define pwr_falcon_irqmset_mthd_f(v)                     ((U32(v) & 0x1U) << 2U)
#define pwr_falcon_irqmset_ctxsw_f(v)                    ((U32(v) & 0x1U) << 3U)
#define pwr_falcon_irqmset_halt_f(v)                     ((U32(v) & 0x1U) << 4U)
#define pwr_falcon_irqmset_exterr_f(v)                   ((U32(v) & 0x1U) << 5U)
#define pwr_falcon_irqmset_swgen0_f(v)                   ((U32(v) & 0x1U) << 6U)
#define pwr_falcon_irqmset_swgen1_f(v)                   ((U32(v) & 0x1U) << 7U)
#define pwr_falcon_irqmset_ext_ecc_parity_f(v)          ((U32(v) & 0x1U) << 10U)
#define pwr_falcon_irqmask_r()                                     (0x008f4018U)
#define pwr_falcon_irqdest_r()                                     (0x008f401cU)
#define pwr_falcon_irqdest_host_gptmr_f(v)               ((U32(v) & 0x1U) << 0U)
#define pwr_falcon_irqdest_host_wdtmr_f(v)               ((U32(v) & 0x1U) << 1U)
#define pwr_falcon_irqdest_host_mthd_f(v)                ((U32(v) & 0x1U) << 2U)
#define pwr_falcon_irqdest_host_ctxsw_f(v)               ((U32(v) & 0x1U) << 3U)
#define pwr_falcon_irqdest_host_halt_f(v)                ((U32(v) & 0x1U) << 4U)
#define pwr_falcon_irqdest_host_exterr_f(v)              ((U32(v) & 0x1U) << 5U)
#define pwr_falcon_irqdest_host_swgen0_f(v)              ((U32(v) & 0x1U) << 6U)
#define pwr_falcon_irqdest_host_swgen1_f(v)              ((U32(v) & 0x1U) << 7U)
#define pwr_falcon_irqdest_host_ext_ecc_parity_f(v)     ((U32(v) & 0x1U) << 10U)
#define pwr_falcon_irqdest_target_gptmr_f(v)            ((U32(v) & 0x1U) << 16U)
#define pwr_falcon_irqdest_target_wdtmr_f(v)            ((U32(v) & 0x1U) << 17U)
#define pwr_falcon_irqdest_target_mthd_f(v)             ((U32(v) & 0x1U) << 18U)
#define pwr_falcon_irqdest_target_ctxsw_f(v)            ((U32(v) & 0x1U) << 19U)
#define pwr_falcon_irqdest_target_halt_f(v)             ((U32(v) & 0x1U) << 20U)
#define pwr_falcon_irqdest_target_exterr_f(v)           ((U32(v) & 0x1U) << 21U)
#define pwr_falcon_irqdest_target_swgen0_f(v)           ((U32(v) & 0x1U) << 22U)
#define pwr_falcon_irqdest_target_swgen1_f(v)           ((U32(v) & 0x1U) << 23U)
#define pwr_falcon_irqdest_target_ext_ecc_parity_f(v)   ((U32(v) & 0x1U) << 26U)
#define pwr_falcon_mailbox1_r()                                    (0x008f4044U)
#define pwr_falcon_itfen_r()                                       (0x008f4048U)
#define pwr_falcon_itfen_ctxen_enable_f()                                 (0x1U)
#define pwr_falcon_os_r()                                          (0x008f4080U)
#define pwr_falcon_cpuctl_r()                                      (0x008f4100U)
#define pwr_falcon_cpuctl_startcpu_f(v)                  ((U32(v) & 0x1U) << 1U)
#define pwr_falcon_cpuctl_alias_r()                                (0x008f4130U)
#define pwr_falcon_hwcfg2_r()                                      (0x008f40f4U)
#define pwr_falcon_hwcfg2_dbgmode_v(r)                      (((r) >> 3U) & 0x1U)
#define pwr_falcon_hwcfg2_dbgmode_enable_v()                       (0x00000001U)
#define pwr_falcon_dmatrfbase_r()                                  (0x008f4110U)
#define pwr_falcon_dmatrfbase1_r()                                 (0x008f4128U)
#define pwr_falcon_dmatrfmoffs_r()                                 (0x008f4114U)
#define pwr_falcon_dmatrfcmd_r()                                   (0x008f4118U)
#define pwr_falcon_dmatrfcmd_imem_f(v)                   ((U32(v) & 0x1U) << 4U)
#define pwr_falcon_dmatrfcmd_write_f(v)                  ((U32(v) & 0x1U) << 5U)
#define pwr_falcon_dmatrfcmd_size_f(v)                   ((U32(v) & 0x7U) << 8U)
#define pwr_falcon_dmatrfcmd_ctxdma_f(v)                ((U32(v) & 0x7U) << 12U)
#define pwr_falcon_dmatrffboffs_r()                                (0x008f411cU)
#define pwr_falcon_exterraddr_r()                                  (0x008f4168U)
#define pwr_falcon_exterrstat_r()                                  (0x008f416cU)
#define pwr_falcon_exterrstat_valid_m()                       (U32(0x1U) << 31U)
#define pwr_falcon_exterrstat_valid_v(r)                   (((r) >> 31U) & 0x1U)
#define pwr_falcon_exterrstat_valid_true_v()                       (0x00000001U)
#define pwr_falcon_dmemc_r(i)\
		(nvgpu_safe_add_u32(0x008f41c0U, nvgpu_safe_mult_u32((i), 8U)))
#define pwr_falcon_dmemc_offs_f(v)                      ((U32(v) & 0x3fU) << 2U)
#define pwr_falcon_dmemc_blk_f(v)                     ((U32(v) & 0xffffU) << 8U)
#define pwr_falcon_dmemc_aincw_f(v)                     ((U32(v) & 0x1U) << 24U)
#define pwr_falcon_dmemd_r(i)\
		(nvgpu_safe_add_u32(0x008f41c4U, nvgpu_safe_mult_u32((i), 8U)))
#define pwr_pmu_new_instblk_r()                                    (0x008f6480U)
#define pwr_pmu_new_instblk_ptr_f(v)               ((U32(v) & 0xfffffffU) << 0U)
#define pwr_pmu_new_instblk_target_sys_coh_f()                     (0x20000000U)
#define pwr_pmu_new_instblk_target_sys_ncoh_f()                    (0x30000000U)
#define pwr_pmu_new_instblk_valid_f(v)                  ((U32(v) & 0x1U) << 30U)
#define pwr_pmu_mutex_id_r()                                       (0x008f6488U)
#define pwr_pmu_mutex_id_value_v(r)                        (((r) >> 0U) & 0xffU)
#define pwr_pmu_mutex_id_value_init_v()                            (0x00000000U)
#define pwr_pmu_mutex_id_value_not_avail_v()                       (0x000000ffU)
#define pwr_pmu_mutex_id_release_r()                               (0x008f648cU)
#define pwr_pmu_mutex_id_release_value_f(v)             ((U32(v) & 0xffU) << 0U)
#define pwr_pmu_mutex_id_release_value_m()                    (U32(0xffU) << 0U)
#define pwr_pmu_mutex_r(i)\
		(nvgpu_safe_add_u32(0x008f6580U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_mutex__size_1_v()                                  (0x00000010U)
#define pwr_pmu_mutex_value_f(v)                        ((U32(v) & 0xffU) << 0U)
#define pwr_pmu_mutex_value_v(r)                           (((r) >> 0U) & 0xffU)
#define pwr_pmu_mutex_value_initial_lock_f()                              (0x0U)
#define pwr_pmu_queue_head_r(i)\
		(nvgpu_safe_add_u32(0x008f6800U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_queue_head__size_1_v()                             (0x00000008U)
#define pwr_pmu_queue_head_address_f(v)           ((U32(v) & 0xffffffffU) << 0U)
#define pwr_pmu_queue_head_address_v(r)              (((r) >> 0U) & 0xffffffffU)
#define pwr_pmu_queue_tail_r(i)\
		(nvgpu_safe_add_u32(0x008f6820U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_queue_tail__size_1_v()                             (0x00000008U)
#define pwr_pmu_queue_tail_address_f(v)           ((U32(v) & 0xffffffffU) << 0U)
#define pwr_pmu_queue_tail_address_v(r)              (((r) >> 0U) & 0xffffffffU)
#define pwr_pmu_msgq_head_r()                                      (0x008f64c8U)
#define pwr_pmu_msgq_head_val_f(v)                ((U32(v) & 0xffffffffU) << 0U)
#define pwr_pmu_msgq_head_val_v(r)                   (((r) >> 0U) & 0xffffffffU)
#define pwr_pmu_msgq_tail_r()                                      (0x008f64ccU)
#define pwr_pmu_msgq_tail_val_f(v)                ((U32(v) & 0xffffffffU) << 0U)
#define pwr_pmu_msgq_tail_val_v(r)                   (((r) >> 0U) & 0xffffffffU)
#define pwr_pmu_idle_mask_r(i)\
		(nvgpu_safe_add_u32(0x008f7e40U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_idle_mask_gr_enabled_f()                                  (0x1U)
#define pwr_pmu_idle_mask_nvenc_0_enabled_f()                         (0x20000U)
#define pwr_pmu_idle_mask_nvdec_0_enabled_f()                            (0x20U)
#define pwr_pmu_idle_mask_1_r(i)\
		(nvgpu_safe_add_u32(0x008f7e80U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_idle_mask_2_r(i)\
		(nvgpu_safe_add_u32(0x008f7ec0U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_idle_mask_2_ofa_enabled_f()                        (0x10000000U)
#define pwr_pmu_idle_mask_3_r(i)\
		(nvgpu_safe_add_u32(0x008f7c80U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_idle_count_r(i)\
		(nvgpu_safe_add_u32(0x008f7f80U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_idle_count_value_v(r)                (((r) >> 0U) & 0x7fffffffU)
#define pwr_pmu_idle_count_reset_f(v)                   ((U32(v) & 0x1U) << 31U)
#define pwr_pmu_idle_ctrl_r(i)\
		(nvgpu_safe_add_u32(0x008f7fc0U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_idle_ctrl_value_m()                            (U32(0x3U) << 0U)
#define pwr_pmu_idle_ctrl_value_busy_f()                                  (0x2U)
#define pwr_pmu_idle_ctrl_value_always_f()                                (0x3U)
#define pwr_pmu_idle_ctrl_filter_m()                           (U32(0x1U) << 2U)
#define pwr_pmu_idle_ctrl_filter_disabled_f()                             (0x0U)
#define pwr_pmu_idle_threshold_r(i)\
		(nvgpu_safe_add_u32(0x008f7e00U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_idle_threshold_value_f(v)         ((U32(v) & 0x7fffffffU) << 0U)
#define pwr_pmu_idle_intr_r()                                      (0x008f69e8U)
#define pwr_pmu_idle_intr_en_f(v)                        ((U32(v) & 0x1U) << 0U)
#define pwr_pmu_idle_intr_status_r()                               (0x008f69ecU)
#define pwr_pmu_idle_intr_status_intr_f(v)               ((U32(v) & 0x1U) << 0U)
#define pwr_pmu_idle_intr_status_intr_v(r)                  (((r) >> 0U) & 0x1U)
#define pwr_pmu_debug_r(i)\
		(nvgpu_safe_add_u32(0x008f65c0U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_debug__size_1_v()                                  (0x00000004U)
#define pwr_pmu_mailbox_r(i)\
		(nvgpu_safe_add_u32(0x008f6450U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_mailbox__size_1_v()                                (0x0000000cU)
#define pwr_pmu_bar0_addr_r()                                      (0x008f67a0U)
#define pwr_pmu_bar0_data_r()                                      (0x008f67a4U)
#define pwr_pmu_bar0_ctl_r()                                       (0x008f67acU)
#define pwr_pmu_bar0_timeout_r()                                   (0x008f67a8U)
#define pwr_pmu_bar0_fecs_error_r()                                (0x008f6988U)
#define pwr_pmu_bar0_host_error_r()                                (0x008f6990U)
#define pwr_pmu_bar0_error_status_r()                              (0x008f67b0U)
#define pwr_pmu_bar0_error_status_timeout_host_m()             (U32(0x1U) << 0U)
#define pwr_pmu_bar0_error_status_timeout_fecs_m()             (U32(0x1U) << 1U)
#define pwr_pmu_bar0_error_status_cmd_hwerr_m()                (U32(0x1U) << 2U)
#define pwr_pmu_bar0_error_status_err_cmd_m()                  (U32(0x1U) << 3U)
#define pwr_pmu_bar0_error_status_hosterr_m()                 (U32(0x1U) << 30U)
#define pwr_pmu_bar0_error_status_fecserr_m()                 (U32(0x1U) << 31U)
#define pwr_pmu_pg_idlefilth_r(i)\
		(nvgpu_safe_add_u32(0x008f7100U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_pg_ppuidlefilth_r(i)\
		(nvgpu_safe_add_u32(0x008f7180U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_pg_idle_cnt_r(i)\
		(nvgpu_safe_add_u32(0x008f7200U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_pmu_pg_intren_r(i)\
		(nvgpu_safe_add_u32(0x008f7280U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_fbif_transcfg_r(i)\
		(nvgpu_safe_add_u32(0x008f4e00U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_fbif_transcfg_target_local_fb_f()                             (0x0U)
#define pwr_fbif_transcfg_target_coherent_sysmem_f()                      (0x1U)
#define pwr_fbif_transcfg_target_noncoherent_sysmem_f()                   (0x2U)
#define pwr_fbif_transcfg_mem_type_virtual_f()                            (0x0U)
#define pwr_fbif_transcfg_mem_type_physical_f()                           (0x4U)
#define pwr_falcon_engine_r()                                      (0x008f43c0U)
#define pwr_falcon_engine_reset_true_f()                                  (0x1U)
#define pwr_falcon_engine_reset_false_f()                                 (0x0U)
#define pwr_gate_ctrl_r(i)\
		(nvgpu_safe_add_u32(0x008f7b00U, nvgpu_safe_mult_u32((i), 4U)))
#define pwr_gate_ctrl_eng_clk_m()                              (U32(0x3U) << 0U)
#define pwr_gate_ctrl_eng_clk_run_f()                                     (0x0U)
#define pwr_gate_ctrl_eng_clk_auto_f()                                    (0x1U)
#define pwr_gate_ctrl_eng_clk_stop_f()                                    (0x2U)
#define pwr_gate_ctrl_blk_clk_m()                              (U32(0x3U) << 2U)
#define pwr_gate_ctrl_blk_clk_run_f()                                     (0x0U)
#define pwr_gate_ctrl_blk_clk_auto_f()                                    (0x4U)
#define pwr_gate_ctrl_idle_holdoff_m()                         (U32(0x1U) << 4U)
#define pwr_gate_ctrl_idle_holdoff_on_f()                                (0x10U)
#define pwr_gate_ctrl_eng_idle_filt_exp_m()                   (U32(0x1fU) << 8U)
#define pwr_gate_ctrl_eng_idle_filt_exp__prod_f()                       (0x200U)
#define pwr_gate_ctrl_eng_idle_filt_mant_m()                  (U32(0x7U) << 13U)
#define pwr_gate_ctrl_eng_idle_filt_mant__prod_f()                     (0x2000U)
#define pwr_gate_ctrl_eng_delay_before_m()                    (U32(0xfU) << 16U)
#define pwr_gate_ctrl_eng_delay_before__prod_f()                      (0x60000U)
#define pwr_gate_ctrl_eng_delay_after_m()                     (U32(0xfU) << 20U)
#define pwr_gate_ctrl_eng_delay_after__prod_f()                           (0x0U)
#define pwr_sys_pri_hub_idle_filter_r()                            (0x008f6870U)
#define pwr_sys_pri_hub_idle_filter_value_m()           (U32(0xffffffffU) << 0U)
#define pwr_sys_pri_hub_idle_filter_value__prod_f()                       (0x0U)
#define pwr_hubmmu_idle_filter_r()                                 (0x008f69d8U)
#define pwr_hubmmu_idle_filter_value_m()                (U32(0xffffffffU) << 0U)
#define pwr_hubmmu_idle_filter_value__prod_f()                            (0x0U)
#define pwr_riscv_irqmask_r()                                      (0x008f5528U)
#define pwr_riscv_irqdest_r()                                      (0x008f552cU)
#define pwr_pmu_residency_ctrl_global_r()                          (0x008f6a68U)
#define pwr_pmu_residency_ctrl_global_en_dis_f()                          (0x0U)
#define pwr_pmu_residency_ctrl_global_en_en_f()                           (0x1U)
#define pwr_pmu_residency_ctrl_index_r()                           (0x008f6a58U)
#define pwr_pmu_residency_ctrl_index_index_m()                (U32(0x7fU) << 0U)
#define pwr_pmu_residency_ctrl_index_index_min_f()                        (0x0U)
#define pwr_pmu_residency_ctrl_index_index_max_f()                       (0x20U)
#define pwr_pmu_residency_ctrl_index_readincr_m()              (U32(0x1U) << 7U)
#define pwr_pmu_residency_ctrl_index_readincr_disabled_f()                (0x0U)
#define pwr_pmu_residency_ctrl_index_readincr_enabled_f()                (0x80U)
#define pwr_pmu_residency_ctrl_index_writeincr_m()             (U32(0x1U) << 8U)
#define pwr_pmu_residency_ctrl_index_writeincr_disabled_f()               (0x0U)
#define pwr_pmu_residency_ctrl_index_writeincr_enabled_f()              (0x100U)
#define pwr_pmu_residency_ctrl_r()                                 (0x008f6a5cU)
#define pwr_pmu_residency_ctrl_source_m()                     (U32(0x7fU) << 0U)
#define pwr_pmu_residency_ctrl_source_none_f()                            (0x0U)
#define pwr_pmu_residency_ctrl_source_elcg_gr_f()                         (0x1U)
#define pwr_pmu_residency_ctrl_source_elcg_nvenc0_f()                     (0x2U)
#define pwr_pmu_residency_ctrl_source_fgrppg_nvenc0_f()                  (0x41U)
#define pwr_pmu_residency_ctrl_source_elcg_nvdec_f()                      (0x5U)
#define pwr_pmu_residency_ctrl_source_fgrppg_nvdec_f()                   (0x44U)
#define pwr_pmu_residency_ctrl_source_elcg_sec_f()                       (0x16U)
#define pwr_pmu_residency_ctrl_source_elcg_ce0_f()                       (0x17U)
#define pwr_pmu_residency_ctrl_source_elcg_ce1_f()                       (0x18U)
#define pwr_pmu_residency_ctrl_source_elcg_ofa0_f()                      (0x21U)
#define pwr_pmu_residency_ctrl_source_fgrppg_ofa0_f()                    (0x54U)
#define pwr_pmu_residency_ctrl_source_fg_rppg_f()                        (0x22U)
#define pwr_pmu_residency_ctrl_source_root_gate_gpcclk_f()               (0x23U)
#define pwr_pmu_residency_ctrl_source_root_gate_nvdclk_f()               (0x24U)
#define pwr_pmu_residency_ctrl_source_hubmmu_power_req_f()               (0x25U)
#define pwr_pmu_residency_ctrl_source_sys0_hubmmu_power_req_f()          (0x25U)
#define pwr_pmu_residency_ctrl_source_rmt_priv_req_f()                   (0x26U)
#define pwr_pmu_residency_ctrl_source_fecs_power_req_f()                 (0x27U)
#define pwr_pmu_residency_ctrl_source_sys0a_pri_hub_power_req_f()        (0x27U)
#define pwr_pmu_residency_ctrl_polarity_m()                   (U32(0x1U) << 28U)
#define pwr_pmu_residency_ctrl_polarity_low_active_f()                    (0x0U)
#define pwr_pmu_residency_ctrl_polarity_high_active_f()            (0x10000000U)
#define pwr_pmu_residency_ctrl_type_m()                       (U32(0x1U) << 29U)
#define pwr_pmu_residency_ctrl_type_edge_f()                              (0x0U)
#define pwr_pmu_residency_ctrl_type_level_f()                      (0x20000000U)
#define pwr_pmu_residency_ctrl_clear_m()                      (U32(0x1U) << 30U)
#define pwr_pmu_residency_ctrl_clear_done_f()                             (0x0U)
#define pwr_pmu_residency_ctrl_clear_not_done_f()                  (0x40000000U)
#define pwr_pmu_residency_ctrl_clear_trigger_f()                   (0x40000000U)
#define pwr_pmu_residency_ctrl_en_m()                         (U32(0x1U) << 31U)
#define pwr_pmu_residency_ctrl_en_disable_f()                             (0x0U)
#define pwr_pmu_residency_ctrl_en_enable_f()                       (0x80000000U)
#define pwr_pmu_residency_state_index_r()                          (0x008f6a60U)
#define pwr_pmu_residency_state_index_index_m()               (U32(0x7fU) << 0U)
#define pwr_pmu_residency_state_index_index_min_f()                       (0x0U)
#define pwr_pmu_residency_state_index_index_max_f()                      (0x20U)
#define pwr_pmu_residency_state_index_readincr_m()             (U32(0x1U) << 7U)
#define pwr_pmu_residency_state_index_readincr_disabled_f()               (0x0U)
#define pwr_pmu_residency_state_index_readincr_enabled_f()               (0x80U)
#define pwr_pmu_residency_state_index_writeincr_m()            (U32(0x1U) << 8U)
#define pwr_pmu_residency_state_index_writeincr_disabled_f()              (0x0U)
#define pwr_pmu_residency_state_index_writeincr_enabled_f()             (0x100U)
#define pwr_pmu_residency_state_r()                                (0x008f6a64U)
#define pwr_pmu_residency_state_value_m()               (U32(0xffffffffU) << 0U)
#define pwr_pmu_residency_state_value_init_f()                            (0x0U)
#define pwr_pmu_residency_state_value_zero_f()                            (0x0U)
#define pwr_pmu_residency_state_value_max_f()                      (0xffffffffU)
#define pwr_falcon_common_scratch_group_0_r(i)\
		(nvgpu_safe_add_u32(0x008f4300U, nvgpu_safe_mult_u32((i), 4U)))
							    /* NEEDS_IP_AUDIT */
#endif
