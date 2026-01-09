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
#ifndef NVGPU_HW_XAL_EP_GB10B_H
#define NVGPU_HW_XAL_EP_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define xal_ep_intr_0_r()                                          (0x0010f100U)
#define xal_ep_intr_0_pri_fecserr_m()                          (U32(0x1U) << 1U)
#define xal_ep_intr_0_pri_req_timeout_m()                      (U32(0x1U) << 2U)
#define xal_ep_intr_0_pri_rsp_timeout_m()                      (U32(0x1U) << 3U)
#define xal_ep_intr_0_fb_req_timeout_m()                       (U32(0x1U) << 4U)
#define xal_ep_intr_0_fb_ack_timeout_m()                       (U32(0x1U) << 5U)
#define xal_ep_intr_0_fb_rdata_timeout_m()                     (U32(0x1U) << 6U)
#define xal_ep_intr_0_ecc_correctable_m()                     (U32(0x1U) << 21U)
#define xal_ep_intr_0_ecc_uncorrectable_m()                   (U32(0x1U) << 22U)
#define xal_ep_intr_0_parity_error_m()                        (U32(0x1U) << 24U)
#define xal_ep_intr_0_jtag_timeout_m()                         (U32(0x1U) << 0U)
#define xal_ep_intr_0_ingress_poison_m()                      (U32(0x1U) << 18U)
#define xal_ep_intr_0_egress_poison_m()                       (U32(0x1U) << 19U)
#define xal_ep_intr_0_decoupler_error_m()                     (U32(0x1U) << 23U)
#define xal_ep_intr_en_0_r()                                       (0x0010f104U)
#define xal_ep_intr_en_0_pri_fecserr_m()                       (U32(0x1U) << 1U)
#define xal_ep_intr_en_0_pri_req_timeout_m()                   (U32(0x1U) << 2U)
#define xal_ep_intr_en_0_pri_rsp_timeout_m()                   (U32(0x1U) << 3U)
#define xal_ep_intr_en_0_fb_req_timeout_m()                    (U32(0x1U) << 4U)
#define xal_ep_intr_en_0_fb_ack_timeout_m()                    (U32(0x1U) << 5U)
#define xal_ep_intr_en_0_fb_rdata_timeout_m()                  (U32(0x1U) << 6U)
#define xal_ep_intr_en_0_ecc_uncorrectable_m()                (U32(0x1U) << 22U)
#define xal_ep_intr_en_0_parity_error_m()                     (U32(0x1U) << 24U)
#define xal_ep_intr_en_0_jtag_timeout_m()                      (U32(0x1U) << 0U)
#define xal_ep_intr_en_0_ingress_poison_m()                   (U32(0x1U) << 18U)
#define xal_ep_intr_en_0_egress_poison_m()                    (U32(0x1U) << 19U)
#define xal_ep_intr_en_0_decoupler_error_m()                  (U32(0x1U) << 23U)
#define xal_ep_intr_en_1_r()                                       (0x0010f134U)
#define xal_ep_intr_en_1_ecc_correctable_m()                  (U32(0x1U) << 21U)
#define xal_ep_jtag_timeout_r()                                    (0x0010f924U)
#define xal_ep_jtag_timeout_en_m()                            (U32(0x1U) << 31U)
#define xal_ep_jtag_timeout_upper_r()                              (0x0010f950U)
#define xal_ep_jtag_timeout_lower_r()                              (0x0010f94cU)
#define xal_ep_pcie_rsp_pri_timeout_r()                            (0x0010f204U)
#define xal_ep_pcie_rsp_pri_timeout_detection_m()             (U32(0x1U) << 31U)
#define xal_ep_poison_control_r()                                  (0x0010f314U)
#define xal_ep_poison_control_ingress_report_m()               (U32(0x1U) << 0U)
#define xal_ep_poison_control_egress_report_m()                (U32(0x1U) << 2U)
#define xal_ep_poison_ingress_log0_r()                             (0x0010f320U)
#define xal_ep_poison_ingress_log1_r()                             (0x0010f324U)
#define xal_ep_poison_ingress_log2_r()                             (0x0010f328U)
#define xal_ep_poison_egress_log0_r()                              (0x0010f330U)
#define xal_ep_poison_egress_log1_r()                              (0x0010f334U)
#define xal_ep_poison_egress_log2_r()                              (0x0010f338U)
#define xal_ep_decoupler_err_status_r()                            (0x0010f390U)
#define xal_ep_decoupler_err_log1_r()                              (0x0010f394U)
#define xal_ep_decoupler_err_log2_r()                              (0x0010f398U)
#define xal_ep_ecc_uncorrectable_status_r()                        (0x0010fd14U)
#define xal_ep_ecc_uncorrectable_status_dpfifo_m()             (U32(0x1U) << 0U)
#define xal_ep_ecc_uncorrectable_status_dnpfifo_m()            (U32(0x1U) << 1U)
#define xal_ep_ecc_uncorrectable_status_upcplfifo_m()          (U32(0x1U) << 2U)
#define xal_ep_ecc_uncorrectable_status_prireqpfifo_m()        (U32(0x1U) << 3U)
#define xal_ep_ecc_uncorrectable_status_prireqnpfifo_m()       (U32(0x1U) << 4U)
#define xal_ep_ecc_uncorrectable_status_pritrackerfifo_m()     (U32(0x1U) << 5U)
#define xal_ep_ecc_uncorrectable_status_prireorder_m()         (U32(0x1U) << 6U)
#define xal_ep_ecc_uncorrectable_status_prirspcplfifo_m()      (U32(0x1U) << 7U)
#define xal_ep_ecc_uncorrectable_status_xdcegressfifo_m()      (U32(0x1U) << 8U)
#define xal_ep_ecc_uncorrectable_status_xdcingressfifo_m()     (U32(0x1U) << 9U)
#define xal_ep_ecc_correctable_status_r()                          (0x0010fd10U)
#define xal_ep_ecc_correctable_status_dpfifo_m()               (U32(0x1U) << 0U)
#define xal_ep_ecc_correctable_status_dnpfifo_m()              (U32(0x1U) << 1U)
#define xal_ep_ecc_correctable_status_upcplfifo_m()            (U32(0x1U) << 2U)
#define xal_ep_ecc_correctable_status_prireqpfifo_m()          (U32(0x1U) << 3U)
#define xal_ep_ecc_correctable_status_prireqnpfifo_m()         (U32(0x1U) << 4U)
#define xal_ep_ecc_correctable_status_pritrackerfifo_m()       (U32(0x1U) << 5U)
#define xal_ep_ecc_correctable_status_prireorder_m()           (U32(0x1U) << 6U)
#define xal_ep_ecc_correctable_status_prirspcplfifo_m()        (U32(0x1U) << 7U)
#define xal_ep_ecc_correctable_status_xdcegressfifo_m()        (U32(0x1U) << 8U)
#define xal_ep_ecc_correctable_status_xdcingressfifo_m()       (U32(0x1U) << 9U)
#define xal_ep_dpfifo_ecc_status_r()                               (0x0010fc0cU)
#define xal_ep_dpfifo_ecc_corrected_err_count_r()                  (0x0010fc10U)
#define xal_ep_dpfifo_ecc_uncorrected_err_count_r()                (0x0010fc14U)
#define xal_ep_dpfifo_ecc_address_r()                              (0x0010fc18U)
#define xal_ep_dnpfifo_ecc_status_r()                              (0x0010fc24U)
#define xal_ep_dnpfifo_ecc_corrected_err_count_r()                 (0x0010fc28U)
#define xal_ep_dnpfifo_ecc_uncorrected_err_count_r()               (0x0010fc2cU)
#define xal_ep_dnpfifo_ecc_address_r()                             (0x0010fc30U)
#define xal_ep_upcplfifo_ecc_status_r()                            (0x0010fc3cU)
#define xal_ep_upcplfifo_ecc_corrected_err_count_r()               (0x0010fc40U)
#define xal_ep_upcplfifo_ecc_uncorrected_err_count_r()             (0x0010fc44U)
#define xal_ep_upcplfifo_ecc_address_r()                           (0x0010fc4cU)
#define xal_ep_prireqpfifo_ecc_status_r()                          (0x0010fc58U)
#define xal_ep_prireqpfifo_ecc_corrected_err_count_r()             (0x0010fc5cU)
#define xal_ep_prireqpfifo_ecc_uncorrected_err_count_r()           (0x0010fc60U)
#define xal_ep_prireqpfifo_ecc_address_r()                         (0x0010fc64U)
#define xal_ep_prireqnpfifo_ecc_status_r()                         (0x0010fc70U)
#define xal_ep_prireqnpfifo_ecc_corrected_err_count_r()            (0x0010fc74U)
#define xal_ep_prireqnpfifo_ecc_uncorrected_err_count_r()          (0x0010fc78U)
#define xal_ep_prireqnpfifo_ecc_address_r()                        (0x0010fc7cU)
#define xal_ep_pritrackerfifo_ecc_status_r()                       (0x0010fc88U)
#define xal_ep_pritrackerfifo_ecc_corrected_err_count_r()          (0x0010fc8cU)
#define xal_ep_pritrackerfifo_ecc_uncorrected_err_count_r()        (0x0010fc90U)
#define xal_ep_pritrackerfifo_ecc_address_r()                      (0x0010fc94U)
#define xal_ep_prireorder_ecc_status_r()                           (0x0010fca4U)
#define xal_ep_prireorder_ecc_corrected_err_count_r()              (0x0010fca8U)
#define xal_ep_prireorder_ecc_uncorrected_err_count_r()            (0x0010fcacU)
#define xal_ep_prireorder_ecc_address_r()                          (0x0010fcb0U)
#define xal_ep_prirspcplfifo_ecc_status_r()                        (0x0010fcbcU)
#define xal_ep_prirspcplfifo_ecc_corrected_err_count_r()           (0x0010fcd0U)
#define xal_ep_prirspcplfifo_ecc_uncorrected_err_count_r()         (0x0010fcd4U)
#define xal_ep_prirspcplfifo_ecc_address_r()                       (0x0010fcd8U)
#define xal_ep_xdcegressfifo_ecc_status_r()                        (0x0010fce4U)
#define xal_ep_xdcegressfifo_ecc_corrected_err_count_r()           (0x0010fce8U)
#define xal_ep_xdcegressfifo_ecc_uncorrected_err_count_r()         (0x0010fcecU)
#define xal_ep_xdcegressfifo_ecc_address_r()                       (0x0010fcf0U)
#define xal_ep_xdcingressfifo_ecc_status_r()                       (0x0010fcfcU)
#define xal_ep_xdcingressfifo_ecc_corrected_err_count_r()          (0x0010fd04U)
#define xal_ep_xdcingressfifo_ecc_uncorrected_err_count_r()        (0x0010fd08U)
#define xal_ep_xdcingressfifo_ecc_address_r()                      (0x0010fd0cU)
#define xal_ep_intr_ctrl_r()                                       (0x0010f10cU)
#define xal_ep_intr_ctrl_1_r()                                     (0x0010f13cU)
#define xal_ep_intr_retrigger_r()                                  (0x0010f114U)
#define xal_ep_intr_retrigger_trigger_true_f()                            (0x1U)
#define xal_ep_intr_retrigger_1_r()                                (0x0010f144U)
#define xal_ep_intr_retrigger_1_trigger_true_f()                          (0x1U)
#define xal_ep_pcie_rsp_fb_timeout_r()                             (0x0010f208U)
#define xal_ep_pcie_rsp_fb_timeout_period_v(r)       (((r) >> 0U) & 0x3fffffffU)
#define xal_ep_intr_triggered_pri_req_timeout_r()                  (0x0010f954U)
#define xal_ep_intr_triggered_pri_req_timeout_adr_v(r)\
				(((r) >> 0U) & 0x3ffffffU)
#define xal_ep_intr_triggered_pri_req_timeout_write_v(r)   (((r) >> 29U) & 0x1U)
#define xal_ep_intr_triggered_pri_rsp_timeout_r()                  (0x0010f958U)
#define xal_ep_intr_triggered_pri_rsp_timeout_adr_v(r)\
				(((r) >> 0U) & 0x3ffffffU)
#define xal_ep_intr_triggered_pri_rsp_timeout_write_v(r)   (((r) >> 29U) & 0x1U)
#define xal_ep_intr_triggered_pri_error_r()                        (0x0010f95cU)
#define xal_ep_intr_triggered_pri_error_adr_v(r)      (((r) >> 0U) & 0x3ffffffU)
#define xal_ep_intr_triggered_pri_error_write_v(r)         (((r) >> 29U) & 0x1U)
#define xal_ep_intr_triggered_pri_error_data_r()                   (0x0010f960U)
#define xal_ep_uflush_fb_flush_r()                                 (0x0010f800U)
#define xal_ep_uflush_fb_flush_token_s()                                   (31U)
#define xal_ep_uflush_fb_flush_token_v(r)            (((r) >> 0U) & 0x7fffffffU)
#define xal_ep_fb_flush_completed_r()                              (0x0010f804U)
#define xal_ep_fb_flush_completed_token_v(r)         (((r) >> 0U) & 0x7fffffffU)
#define xal_ep_fb_flush_completed_status_v(r)              (((r) >> 31U) & 0x1U)
#define xal_ep_fb_flush_completed_status_idle_v()                  (0x00000000U)
#define xal_ep_uflush_l2_flush_dirty_r()                           (0x0010f810U)
#define xal_ep_uflush_l2_flush_dirty_token_s()                             (31U)
#define xal_ep_uflush_l2_flush_dirty_token_v(r)      (((r) >> 0U) & 0x7fffffffU)
#define xal_ep_l2_flush_dirty_completed_r()                        (0x0010f814U)
#define xal_ep_l2_flush_dirty_completed_token_v(r)   (((r) >> 0U) & 0x7fffffffU)
#define xal_ep_l2_flush_dirty_completed_status_v(r)        (((r) >> 31U) & 0x1U)
#define xal_ep_l2_flush_dirty_completed_status_idle_v()            (0x00000000U)
#define xal_ep_uflush_l2_clean_comptags_r()                        (0x0010f808U)
#define xal_ep_uflush_l2_clean_comptags_token_s()                          (31U)
#define xal_ep_uflush_l2_clean_comptags_token_v(r)   (((r) >> 0U) & 0x7fffffffU)
#define xal_ep_l2_clean_comptags_completed_r()                     (0x0010f80cU)
#define xal_ep_l2_clean_comptags_completed_token_v(r)\
				(((r) >> 0U) & 0x7fffffffU)
#define xal_ep_l2_clean_comptags_completed_status_v(r)     (((r) >> 31U) & 0x1U)
#define xal_ep_l2_clean_comptags_completed_status_idle_v()         (0x00000000U)
#endif
