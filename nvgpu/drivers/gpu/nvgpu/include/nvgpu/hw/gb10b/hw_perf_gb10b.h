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
#ifndef NVGPU_HW_PERF_GB10B_H
#define NVGPU_HW_PERF_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define perf_hem_sys0_base_v()                                     (0x002b6c00U)
#define perf_hem_sys0_extent_v()                                   (0x002b6c7fU)
#define perf_hem_sys0_perhem_offset_v()                            (0x00000040U)
#define perf_sys_streaming_dg_id_hemcwd0_v()                       (0x00000012U)
#define perf_sys_streaming_dg_id_hemcwd1_v()                       (0x00000013U)
#define perf_sys_streaming_dg_id_nvdec0_v()                        (0x00000000U)
#define perf_sys_streaming_dg_id_nvdec1_v()                        (0x00000001U)
#define perf_sys_streaming_dg_id_nvenc0_v()                        (0x00000002U)
#define perf_sys_streaming_dg_id_nvenc1_v()                        (0x00000003U)
#define perf_sys_streaming_dg_id_nvjpg_v()                         (0x00000004U)
#define perf_sys_streaming_dg_id_nvjpg1_v()                        (0x00000005U)
#define perf_sys_streaming_dg_id_ofa_v()                           (0x00000006U)
#define perf_sys_streaming_dg_id_pcie0_v()                         (0x00000007U)
#define perf_sys_streaming_dg_id_pwr0_v()                          (0x00000008U)
#define perf_sys_streaming_dg_id_smca0_v()                         (0x00000009U)
#define perf_sys_streaming_dg_id_smca1_v()                         (0x0000000aU)
#define perf_sys_streaming_dg_id_smcb0_v()                         (0x0000000bU)
#define perf_sys_streaming_dg_id_smcb1_v()                         (0x0000000cU)
#define perf_sys_streaming_dg_id_smga0_v()                         (0x0000000dU)
#define perf_sys_streaming_dg_id_sys0_v()                          (0x0000000eU)
#define perf_sys_streaming_dg_id_sys1_v()                          (0x0000000fU)
#define perf_sys_streaming_dg_id_sys2_v()                          (0x00000010U)
#define perf_sys_streaming_dg_id_xbar0_v()                         (0x00000011U)
#define perf_gpc_streaming_dg_id_tpc0cau0_v()                      (0x00000014U)
#define perf_gpc_streaming_dg_id_tpc1cau0_v()                      (0x00000015U)
#define perf_gpc_streaming_dg_id_tpc2cau0_v()                      (0x00000016U)
#define perf_gpc_streaming_dg_id_gpctpca0_v()                      (0x00000002U)
#define perf_gpc_streaming_dg_id_gpctpcd3_v()                      (0x00000011U)
#define perf_pmmgpc_perdomain_offset_v()                           (0x00000200U)
#define perf_pmmsys_perdomain_offset_v()                           (0x00000200U)
#define perf_pmmgpc_base_v()                                       (0x00180000U)
#define perf_pmmgpc_extent_v()                                     (0x00183fffU)
#define perf_pmmsys_base_v()                                       (0x00240000U)
#define perf_pmmsys_extent_v()                                     (0x00243fffU)
#define perf_pmmfbp_base_v()                                       (0x00200000U)
#define perf_pmmfbp_extent_v()                                     (0x00203fffU)
#define perf_pmmfbprouter_base_v()                                 (0x002a0000U)
#define perf_pmmfbprouter_extent_v()                               (0x002a03ffU)
#define perf_pmmgpcrouter_base_v()                                 (0x002a4000U)
#define perf_pmmgpcrouter_extent_v()                               (0x002a43ffU)
#define perf_pmasys_channel_mem_bytes_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a38U, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_mem_bytes__size_1_v()                  (0x00000003U)
#define perf_pmasys_channel_mem_bytes__size_2_v()                  (0x00000002U)
#define perf_pmasys_channel_status_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a00U, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_status__size_1_v()                     (0x00000003U)
#define perf_pmasys_channel_status__size_2_v()                     (0x00000002U)
#define perf_pmasys_channel_status_membuf_status_overflowed_f()       (0x10000U)
#define perf_pmasys_channel_mem_bump_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a14U, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_mem_bump__size_1_v()                   (0x00000003U)
#define perf_pmasys_channel_mem_bump__size_2_v()                   (0x00000002U)
#define perf_pmasys_channel_mem_bump_numbytes_f(v) ((U32(v) & 0xfffffffU) << 4U)
#define perf_pmasys_enginestatus_r()                               (0x002b0818U)
#define perf_pmasys_enginestatus_status_f(v)             ((U32(v) & 0x7U) << 0U)
#define perf_pmasys_enginestatus_status_empty_v()                  (0x00000000U)
#define perf_pmasys_enginestatus_status_active_v()                 (0x00000001U)
#define perf_pmasys_channel_control_user_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a10U, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_control_user__size_1_v()               (0x00000003U)
#define perf_pmasys_channel_control_user__size_2_v()               (0x00000002U)
#define perf_pmasys_channel_control_user_update_bytes_m()     (U32(0x1U) << 16U)
#define perf_pmasys_channel_control_user_update_bytes_doit_f()        (0x10000U)
#define perf_pmasys_channel_control_user_membuf_clear_status_doit_f()     (0x2U)
#define perf_pmasys_channel_outbase_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a28U, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_outbase__size_1_v()                    (0x00000003U)
#define perf_pmasys_channel_outbase__size_2_v()                    (0x00000002U)
#define perf_pmasys_channel_outbaseupper_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a2cU, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_outbaseupper__size_1_v()               (0x00000003U)
#define perf_pmasys_channel_outbaseupper__size_2_v()               (0x00000002U)
#define perf_pmasys_channel_outbaseupper_ptr_f(v)  ((U32(v) & 0x1ffffffU) << 0U)
#define perf_pmasys_channel_outsize_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a30U, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_outsize__size_1_v()                    (0x00000003U)
#define perf_pmasys_channel_outsize__size_2_v()                    (0x00000002U)
#define perf_pmasys_channel_mem_bytes_addr_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a3cU, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_mem_bytes_addr__size_1_v()             (0x00000003U)
#define perf_pmasys_channel_mem_bytes_addr__size_2_v()             (0x00000002U)
#define perf_pmasys_channel_mem_bytes_addr_ptr_f(v)\
				((U32(v) & 0x3fffffffU) << 2U)
#define perf_pmasys_channel_mem_bytes_addr_ptr_b()                          (2U)
#define perf_pmasys_cblock_bpc_gfid_r(i)\
		(nvgpu_safe_add_u32(0x002b1e00U, nvgpu_safe_mult_u32((i), 32U)))
#define perf_pmasys_cblock_bpc_gfid__size_1_v()                    (0x00000003U)
#define perf_pmasys_cblock_bpc_mem_block_r(i)\
		(nvgpu_safe_add_u32(0x002b1e04U, nvgpu_safe_mult_u32((i), 32U)))
#define perf_pmasys_cblock_bpc_mem_block__size_1_v()               (0x00000003U)
#define perf_pmasys_cblock_bpc_mem_blockupper_r(i)\
		(nvgpu_safe_add_u32(0x002b1e08U, nvgpu_safe_mult_u32((i), 32U)))
#define perf_pmasys_cblock_bpc_mem_blockupper__size_1_v()          (0x00000003U)
#define perf_pmasys_cblock_bpc_mem_blockupper_base_f(v) ((U32(v) & 0xffU) << 0U)
#define perf_pmasys_cblock_bpc_mem_blockupper_target_f(v)\
				((U32(v) & 0x3U) << 28U)
#define perf_pmasys_cblock_bpc_mem_blockupper_target_lfb_f()              (0x0U)
#define perf_pmasys_cblock_bpc_mem_blockupper_target_p2p_f()       (0x10000000U)
#define perf_pmasys_cblock_bpc_mem_blockupper_target_sys_coh_f()   (0x20000000U)
#define perf_pmasys_cblock_bpc_mem_blockupper_target_sys_ncoh_f()  (0x30000000U)
#define perf_pmasys_cblock_bpc_mem_blockupper_valid_true_f()       (0x80000000U)
#define perf_pmasys_cblock_bpc_mem_blockupper_valid_false_f()             (0x0U)
#define perf_pmasys_channel_config_user_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a24U, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_config_user__size_1_v()                (0x00000003U)
#define perf_pmasys_channel_config_user__size_2_v()                (0x00000002U)
#define perf_pmasys_channel_config_user_stream_m()             (U32(0x1U) << 0U)
#define perf_pmasys_channel_config_user_stream_enable_f()                 (0x1U)
#define perf_pmasys_channel_config_user_stream_disable_f()                (0x0U)
#define perf_pmasys_channel_config_user_coalesce_timeout_cycles_m()\
				(U32(0x7U) << 24U)
#define perf_pmasys_channel_config_user_coalesce_timeout_cycles__prod_f()\
				(0x4000000U)
#define perf_pmasys_channel_mem_head_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a34U, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_mem_head__size_1_v()                   (0x00000003U)
#define perf_pmasys_channel_mem_head__size_2_v()                   (0x00000002U)
#define perf_pmasys_cblock_status_r(i)\
		(nvgpu_safe_add_u32(0x002b1e18U, nvgpu_safe_mult_u32((i), 32U)))
#define perf_pmasys_cblock_status__size_1_v()                      (0x00000003U)
#define perf_pmasys_cblock_status_bpc_status_v(r)           (((r) >> 3U) & 0x7U)
#define perf_pmasys_cblock_status_bpc_status_empty_v()             (0x00000000U)
#define perf_pmasys_cblock_status_bpc_status_active_v()            (0x00000001U)
#define perf_pmasys_cblock_status_bpc_status_paused_v()            (0x00000002U)
#define perf_pmasys_cblock_status_bpc_status_quiescent_v()         (0x00000003U)
#define perf_pmasys_cblock_status_bpc_status_undefined_v()         (0x00000004U)
#define perf_pmasys_cblock_status_bpc_status_stalled_v()           (0x00000005U)
#define perf_pmasys_cblock_status_bpc_status_faulted_v()           (0x00000006U)
#define perf_pmasys_cblock_status_bpc_status_halted_v()            (0x00000007U)
#define perf_pmasys_cblock_status_bpc_state_v(r)            (((r) >> 6U) & 0x3U)
#define perf_pmasys_cblock_status_bpc_state_unbound_v()            (0x00000000U)
#define perf_pmasys_cblock_status_bpc_state_bound_v()              (0x00000001U)
#define perf_pmasys_cblock_status_bpc_state_faulted_v()            (0x00000002U)
#define perf_pmasys_channel_config_secure_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b0a20U, \
		nvgpu_safe_mult_u32((i), 128U)), nvgpu_safe_mult_u32((j), 64U)))
#define perf_pmasys_channel_config_secure__size_1_v()              (0x00000003U)
#define perf_pmasys_channel_config_secure__size_2_v()              (0x00000002U)
#define perf_pmasys_channel_config_secure_mbu_membar_type_f(v)\
				((U32(v) & 0x3U) << 29U)
#define perf_pmasys_channel_config_secure_mbu_membar_type_none_v() (0x00000000U)
#define perf_pmasys_channel_config_secure_mbu_membar_type_sys_v()  (0x00000001U)
#define perf_pmasys_channel_config_secure_mbu_membar_type_gl_v()   (0x00000002U)
#define perf_pmmsys_engine_sel_r(i)\
		(nvgpu_safe_add_u32(0x0024006cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmsys_engine_sel__size_1_v()                         (0x00000012U)
#define perf_pmmfbp_engine_sel_r(i)\
		(nvgpu_safe_add_u32(0x0020006cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmfbp_engine_sel__size_1_v()                         (0x00000009U)
#define perf_pmmgpc_engine_sel_r(i)\
		(nvgpu_safe_add_u32(0x0018006cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmgpc_engine_sel__size_1_v()                         (0x00000014U)
#define perf_pmmsys_control_r(i)\
		(nvgpu_safe_add_u32(0x0024009cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmfbp_fbps_control_r(i)\
		(nvgpu_safe_add_u32(0x0028009cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmgpc_gpcs_control_r(i)\
		(nvgpu_safe_add_u32(0x0028809cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmsys_secure_config_r(i)\
		(nvgpu_safe_add_u32(0x0024012cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmsys_secure_config_cblock_id_m()               (U32(0xfU) << 11U)
#define perf_pmmsys_secure_config_cblock_id_f(v)        ((U32(v) & 0xfU) << 11U)
#define perf_pmmsys_secure_config_channel_id_m()               (U32(0x3U) << 8U)
#define perf_pmmsys_secure_config_channel_id_f(v)        ((U32(v) & 0x3U) << 8U)
#define perf_pmmsys_secure_config_cmd_slice_id_m()            (U32(0x1fU) << 0U)
#define perf_pmmsys_secure_config_cmd_slice_id_f(v)     ((U32(v) & 0x1fU) << 0U)
#define perf_pmmsys_secure_config_mapped_m()                  (U32(0x1U) << 28U)
#define perf_pmmsys_secure_config_mapped_true_f()                  (0x10000000U)
#define perf_pmmsys_secure_config_mapped_false_f()                        (0x0U)
#define perf_pmmsys_secure_config_command_pkt_decoder_m()     (U32(0x1U) << 31U)
#define perf_pmmsys_secure_config_command_pkt_decoder_enable_f()   (0x80000000U)
#define perf_pmmsys_secure_config_use_prog_dg_idx_m()         (U32(0x1U) << 30U)
#define perf_pmmsys_secure_config_use_prog_dg_idx_false_f()               (0x0U)
#define perf_pmmsys_secure_config_use_prog_dg_idx_true_f()         (0x40000000U)
#define perf_pmmsys_secure_config_dg_idx_m()                 (U32(0xffU) << 16U)
#define perf_pmmsys_secure_config_dg_idx_f(v)          ((U32(v) & 0xffU) << 16U)
#define perf_pmmsys_secure_config_ignore_cmd_pkt_reset_m()    (U32(0x1U) << 24U)
#define perf_pmmsys_secure_config_ignore_cmd_pkt_reset_false_f()          (0x0U)
#define perf_pmmsys_secure_config_ignore_cmd_pkt_reset_true_f()     (0x1000000U)
#define perf_pmmgpc_secure_config_r(i)\
		(nvgpu_safe_add_u32(0x0018012cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmfbp_secure_config_r(i)\
		(nvgpu_safe_add_u32(0x0020012cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmsysrouter_user_channel_enginestatus_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b40b0U, \
		nvgpu_safe_mult_u32((i), 32U)), nvgpu_safe_mult_u32((j), 16U)))
#define perf_pmmsysrouter_user_channel_enginestatus_status_v(r)\
				(((r) >> 0U) & 0x7U)
#define perf_pmmsysrouter_user_channel_enginestatus_status_empty_v()\
				(0x00000000U)
#define perf_pmmsysrouter_user_channel_enginestatus_status_quiescent_v()\
				(0x00000003U)
#define perf_pmmsysrouter_user_channel_enginestatus_merged_perfmon_status_v(r)\
				(((r) >> 3U) & 0x7U)
#define perf_pmmsysrouter_user_channel_enginestatus_merged_perfmon_status_empty_v()\
				(0x00000000U)
#define perf_pmmsysrouter_user_channel_enginestatus_merged_perfmon_status_quiescent_v()\
				(0x00000003U)
#define perf_pmmgpcrouter_user_channel_enginestatus_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002a40b0U, \
		nvgpu_safe_mult_u32((i), 32U)), nvgpu_safe_mult_u32((j), 16U)))
#define perf_pmmfbprouter_user_channel_enginestatus_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002a00b0U, \
		nvgpu_safe_mult_u32((i), 32U)), nvgpu_safe_mult_u32((j), 16U)))
#define perf_pmmsysrouter_user_dgmap_status_secure_r(i)\
		(nvgpu_safe_add_u32(0x002b4050U, nvgpu_safe_mult_u32((i), 4U)))
#define perf_pmmsysrouter_user_dgmap_status_secure__size_1_v()     (0x00000008U)
#define perf_pmmsysrouter_user_dgmap_status_secure_dg0_mapped_v()  (0x00000001U)
#define perf_pmmsysrouter_user_dgmap_status_secure_dg0_not_mapped_v()\
				(0x00000000U)
#define perf_pmmgpcrouter_user_dgmap_status_secure_r(i)\
		(nvgpu_safe_add_u32(0x002a4050U, nvgpu_safe_mult_u32((i), 4U)))
#define perf_pmmgpcrouter_user_dgmap_status_secure__size_1_v()     (0x00000008U)
#define perf_pmmfbprouter_user_dgmap_status_secure_r(i)\
		(nvgpu_safe_add_u32(0x002a0050U, nvgpu_safe_mult_u32((i), 4U)))
#define perf_pmmfbprouter_user_dgmap_status_secure__size_1_v()     (0x00000008U)
#define perf_hem_sys0_secure_config_r(i)\
		(nvgpu_safe_add_u32(0x002b6c00U, nvgpu_safe_mult_u32((i), 64U)))
#define perf_hem_sys0_secure_config__size_1_v()                    (0x00000002U)
#define perf_hem_sys0_secure_config_cblock_id_m()             (U32(0xfU) << 11U)
#define perf_hem_sys0_secure_config_cblock_id_f(v)      ((U32(v) & 0xfU) << 11U)
#define perf_hem_sys0_secure_config_channel_id_m()             (U32(0x3U) << 8U)
#define perf_hem_sys0_secure_config_channel_id_f(v)      ((U32(v) & 0x3U) << 8U)
#define perf_hem_sys0_secure_config_cmd_slice_id_m()          (U32(0x1fU) << 0U)
#define perf_hem_sys0_secure_config_cmd_slice_id_f(v)   ((U32(v) & 0x1fU) << 0U)
#define perf_hem_sys0_secure_config_mapped_m()                (U32(0x1U) << 28U)
#define perf_hem_sys0_secure_config_mapped_true_f()                (0x10000000U)
#define perf_hem_sys0_secure_config_mapped_false_f()                      (0x0U)
#define perf_hem_sys0_secure_config_command_pkt_decoder_m()   (U32(0x1U) << 31U)
#define perf_hem_sys0_secure_config_command_pkt_decoder_enable_f() (0x80000000U)
#define perf_hem_sys0_secure_config_use_prog_dg_idx_m()       (U32(0x1U) << 30U)
#define perf_hem_sys0_secure_config_use_prog_dg_idx_false_f()             (0x0U)
#define perf_hem_sys0_secure_config_use_prog_dg_idx_true_f()       (0x40000000U)
#define perf_hem_sys0_secure_config_dg_idx_m()               (U32(0xffU) << 16U)
#define perf_hem_sys0_secure_config_dg_idx_f(v)        ((U32(v) & 0xffU) << 16U)
#define perf_hem_sys0_secure_config_ignore_cmd_pkt_reset_m()  (U32(0x1U) << 24U)
#define perf_hem_sys0_secure_config_ignore_cmd_pkt_reset_false_f()        (0x0U)
#define perf_hem_sys0_secure_config_ignore_cmd_pkt_reset_true_f()   (0x1000000U)
#define perf_hem_sys0_config_r(i)\
		(nvgpu_safe_add_u32(0x002b6c08U, nvgpu_safe_mult_u32((i), 64U)))
#define perf_hem_sys0_perfmonid_r(i)\
		(nvgpu_safe_add_u32(0x002b6c0cU, nvgpu_safe_mult_u32((i), 64U)))
#define perf_hem_sys0_ctxsw_r(i)\
		(nvgpu_safe_add_u32(0x002b6c10U, nvgpu_safe_mult_u32((i), 64U)))
#define perf_hem_sys0_status_r(i)\
		(nvgpu_safe_add_u32(0x002b6c20U, nvgpu_safe_mult_u32((i), 64U)))
#define perf_hem_sys0_data_gen_r(i)\
		(nvgpu_safe_add_u32(0x002b6c24U, nvgpu_safe_mult_u32((i), 64U)))
#define perf_hem_sys0_pm_r(i)\
		(nvgpu_safe_add_u32(0x002b6c38U, nvgpu_safe_mult_u32((i), 64U)))
#define perf_hem_sys0_debug_r(i)\
		(nvgpu_safe_add_u32(0x002b6c28U, nvgpu_safe_mult_u32((i), 64U)))
#define perf_hem_sys0_action_r(i)\
		(nvgpu_safe_add_u32(0x002b6c34U, nvgpu_safe_mult_u32((i), 64U)))
#define perf_hem_sys0_cg2_r(i)\
		(nvgpu_safe_add_u32(0x002b6c3cU, nvgpu_safe_mult_u32((i), 64U)))
#define perf_pmasys_command_slice_trigger_mask_secure0_r(i)\
		(nvgpu_safe_add_u32(0x002b1110U, nvgpu_safe_mult_u32((i), 144U)))
#define perf_pmasys_command_slice_trigger_mask_secure1_r(i)\
		(nvgpu_safe_add_u32(0x002b1114U, nvgpu_safe_mult_u32((i), 144U)))
#define perf_pmasys_engine_index_gr_v()                            (0x00000003U)
#define perf_pmasys_engine_index_gr1_v()                           (0x00000004U)
#define perf_pmasys_engine_index_grevents_v()                      (0x00000005U)
#define perf_pmasys_engine_index_grevents1_v()                     (0x00000006U)
#define perf_pmasys_engine_index_pma_v()                           (0x00000010U)
#define perf_pmmfbp_fbpgs_ltcs_base_v()                            (0x002a8200U)
#define perf_pmmfbp_fbpgs_ltcs_extent_v()                          (0x002a83ffU)
#define perf_pmmgpc_gpcgs_gpctpca_base_v()                         (0x002a8800U)
#define perf_pmmgpc_gpcgs_gpctpca_extent_v()                       (0x002a89ffU)
#define perf_pmmgpc_gpcgs_gpctpcb_base_v()                         (0x002a8a00U)
#define perf_pmmgpc_gpcgs_gpctpcb_extent_v()                       (0x002a8bffU)
#define perf_pmmgpc_gpcs_base_v()                                  (0x00288000U)
#define perf_pmmgpc_gpcs_extent_v()                                (0x0028bfffU)
#define perf_pmmgpc_gpcsrouter_base_v()                            (0x002aa400U)
#define perf_pmmgpc_gpcsrouter_extent_v()                          (0x002aa7ffU)
#define perf_pmmfbp_fbps_base_v()                                  (0x00280000U)
#define perf_pmmfbp_fbps_extent_v()                                (0x00283fffU)
#define perf_pmmfbp_fbpsrouter_base_v()                            (0x002aa000U)
#define perf_pmmfbp_fbpsrouter_extent_v()                          (0x002aa3ffU)
#define perf_pmmgpc_control_r(i)\
		(nvgpu_safe_add_u32(0x0018009cU, nvgpu_safe_mult_u32((i), 512U)))
#define perf_pmmgpc_control__size_1_v()                            (0x00000014U)
#define perf_pmmgpc_gpc0_config_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002a40b8U, \
		nvgpu_safe_mult_u32((i), 32U)), nvgpu_safe_mult_u32((j), 16U)))
#define perf_pmmgpc_gpc0_config__size_1_v()                        (0x00000003U)
#define perf_pmmgpc_gpc0_config__size_2_v()                        (0x00000002U)
#define perf_pmmgpc_gpc0_config_value_m()                    (U32(0x1ffU) << 0U)
#define perf_pmmgpc_gpc0_config_value_f(v)             ((U32(v) & 0x1ffU) << 0U)
#define perf_pmmfbp_fbp0_config_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002a00b8U, \
		nvgpu_safe_mult_u32((i), 32U)), nvgpu_safe_mult_u32((j), 16U)))
#define perf_pmmsys_sys0_config_r(i,j)\
		(nvgpu_safe_add_u32(nvgpu_safe_add_u32(0x002b40b8U, \
		nvgpu_safe_mult_u32((i), 32U)), nvgpu_safe_mult_u32((j), 16U)))
#endif
