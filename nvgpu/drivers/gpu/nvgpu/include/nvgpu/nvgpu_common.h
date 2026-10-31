/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_COMMON_H
#define NVGPU_COMMON_H

/**
 * @file
 *
 * @page nvgpu nvgpu
 *
 * Overview
 * ========
 *
 * The |nvgpu-rm| unit nvgpu provides basic global APIs and data types that
 * are used throughout the |nvgpu-rm| driver. There are three components of the
 * nvgpu unit that each provides a distinct functionality:
 *
 * - gk20a.h
 * - static_analysis.h
 * - nvgpu_common.h
 *
 * gk20a.h
 * -------
 *
 * This component provides the gk20a struct that is used throughout the driver
 * to represent the GPU state. Along with state information, the gk20a struct also
 * contains the HAL interface function pointers for the driver units. The HAL
 * interfaces are initialized during |nvgpu-rm-init|.
 *
 * In addition to the gk20a struct, this component contains a number of accessor
 * functions for some specific members of the gk20a struct.
 *
 * static_analysis.h
 * -----------------
 *
 * This component provides APIs for satisfying MISRA and CERT-C compliance, such
 * as functions to prevent arithmetic overflow/underflow.
 *
 * nvgpu_common.h
 * --------------
 *
 * This component provides interfaces to environment-specific actions such as
 * restarting the driver.
 *
 * Data Structures
 * ===============
 *
 *   + struct gk20a
 *   + struct gpu_ops
 *   + struct nvgpu_gpu_params
 *
 * Static Design
 * =============
 *
 * External APIs
 * -------------
 *   + nvgpu_get_poll_timeout()
 *   + nvgpu_has_syncpoints()
 *   + nvgpu_is_timeouts_enabled()
 *   + nvgpu_kernel_restart()
 *   + nvgpu_safe_add_u32()
 *   + nvgpu_safe_add_s32()
 *   + nvgpu_safe_add_u64()
 *   + nvgpu_safe_add_s64()
 *   + nvgpu_safe_sub_u8()
 *   + nvgpu_safe_sub_u32()
 *   + nvgpu_safe_sub_s32()
 *   + nvgpu_safe_sub_u64()
 *   + nvgpu_safe_sub_s64()
 *   + nvgpu_safe_mult_u32()
 *   + nvgpu_safe_mult_u64()
 *   + nvgpu_safe_mult_s64()
 *   + nvgpu_safe_cast_u64_to_u16()
 *   + nvgpu_safe_cast_u64_to_u32()
 *   + nvgpu_safe_cast_u64_to_u8()
 *   + nvgpu_safe_cast_s64_to_u32()
 *   + nvgpu_safe_cast_s64_to_u64()
 *   + nvgpu_safe_cast_bool_to_u32()
 *   + nvgpu_safe_cast_s8_to_u8()
 *   + nvgpu_safe_cast_s32_to_u32()
 *   + nvgpu_safe_cast_s32_to_u64()
 *   + nvgpu_safe_cast_u32_to_u16()
 *   + nvgpu_safe_cast_u32_to_u8()
 *   + nvgpu_safe_cast_u32_to_s8()
 *   + nvgpu_safe_cast_u32_to_s32()
 *   + nvgpu_safe_cast_u64_to_s32()
 *   + nvgpu_safe_cast_u64_to_s64()
 *   + nvgpu_safe_cast_s64_to_s32()
 *   + nvgpu_safety_checks()
 *
 * @addtogroup unit-common-nvgpu
 * @{
 */

#ifdef CONFIG_NVGPU_NON_FUSA
/**
 * @brief Restart driver as implemented for OS.
 *
 * @param cmd [in]	Pointer to command to execute before restart, if
 *			possible. Pass NULL for no command.
 *
 * This is a very OS-dependent interface.
 * - On Linux, this will request the kernel to execute the command if not NULL,
 *   then the kernel will reboot the OS.
 * - On QNX, this simply calls BUG() which will restart the driver.
 */
void nvgpu_kernel_restart(void *cmd);
#endif

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *nvgpu_nvgpu_get_fault_injection(void);
#endif

/**
 * @}
 */
#endif
