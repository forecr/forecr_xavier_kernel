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
#ifndef NVGPU_HW_PRI_RINGSTATION_SYS_GB20C_H
#define NVGPU_HW_PRI_RINGSTATION_SYS_GB20C_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

#define pri_ringstation_sys_bar0_to_pri_window_r()                 (0x00122144U)
#define pri_ringstation_sys_bar0_to_pri_window_index_f(v)\
				((U32(v) & 0x3fU) << 0U)
#define pri_ringstation_sys_bar0_to_pri_window_index_m()      (U32(0x3fU) << 0U)
#define pri_ringstation_sys_bar0_to_pri_window_enable_f(v)\
				((U32(v) & 0x1U) << 31U)
#define pri_ringstation_sys_bar0_to_pri_window_enable_m()     (U32(0x1U) << 31U)
#define pri_ringstation_sys_bar0_to_pri_window_enable_enable_v()   (0x00000001U)
#define pri_ringstation_sys_bar0_to_pri_window_enable_disable_v()  (0x00000000U)
#define pri_ringstation_sys_pri_ring_init_r()                      (0x00122274U)
#define pri_ringstation_sys_pri_ring_init_status_v(r)       (((r) >> 0U) & 0x7U)
#define pri_ringstation_sys_pri_ring_init_status_dead_v()          (0x00000000U)
#define pri_ringstation_sys_pri_ring_init_status_alive_v()         (0x00000004U)
#define pri_ringstation_sys_pri_ring_init_status_cmd_rdy_v()       (0x00000001U)
#define pri_ringstation_sys_pri_ring_init_cmd_v(r)          (((r) >> 8U) & 0x3U)
#define pri_ringstation_sys_pri_ring_init_cmd_enumerate_and_start_f()   (0x100U)
#define pri_ringstation_sys_pri_ring_init_cmd_none_v()             (0x00000000U)
#define pri_ringstation_sys_vrlid_to_epid_lookup_table_ctrl_r()    (0x00122280U)
#define pri_ringstation_sys_vrlid_to_epid_lookup_table_ctrl_epid_div_32_hw_f(v)\
				((U32(v) & 0x1U) << 5U)
#define pri_ringstation_sys_vrlid_to_epid_lookup_table_ctrl_epid_div_32_hw_v(r)\
				(((r) >> 5U) & 0x1U)
#define pri_ringstation_sys_vrlid_to_epid_lookup_table_ctrl_gfid_hw_f(v)\
				((U32(v) & 0x3fU) << 16U)
#define pri_ringstation_sys_vrlid_to_epid_lookup_table_ctrl_gfid_hw_v(r)\
				(((r) >> 16U) & 0x3fU)
#define pri_ringstation_sys_vrlid_to_epid_lookup_table_data_r()    (0x00122284U)
#define pri_ringstation_sys_epid_to_esched_map_ctrl_r()            (0x00122288U)
#define pri_ringstation_sys_epid_to_esched_map_ctrl_epid_hw_f(v)\
				((U32(v) & 0x3fU) << 0U)
#define pri_ringstation_sys_epid_to_esched_map_ctrl_epid_hw_v(r)\
				(((r) >> 0U) & 0x3fU)
#define pri_ringstation_sys_epid_to_esched_map_data_r()            (0x0012228cU)
#define pri_ringstation_sys_epid_to_esched_map_data_runlist_pri_base_f(v)\
				((U32(v) & 0x3ffffffU) << 0U)
#define pri_ringstation_sys_epid_to_esched_map_data_runlist_pri_base_v(r)\
				(((r) >> 0U) & 0x3ffffffU)
#define pri_ringstation_sys_epid_info_r()                          (0x00122310U)
#define pri_ringstation_sys_epid_info_num_epids_v(r) (((r) >> 0U) & 0xffffffffU)
#endif
