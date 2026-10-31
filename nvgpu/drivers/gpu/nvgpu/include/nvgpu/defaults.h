/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_DEFAULTS_H
#define NVGPU_DEFAULTS_H

/**
 * @defgroup NVGPU_COMMON_DEFAULTS
 *
 * Default values used in NvGPU for timeouts, inits, etc.
 * @page defaults defaults
 *
 * Overview
 * ========
 * For |nvgpu-rm| defaults unit, there is no dynamic initialization
 * done. The default values of timeouts, inits, etc are statically defined as
 * part of unit headers. These defaults are used by other units during their
 * initialization.
 *
 * Data Structures
 * ===============
 * The defaults unit exports following default values.
 *   + @ref NVGPU_COMMON_DEFAULTS
 */

/**
 * @ingroup NVGPU_COMMON_DEFAULTS
 *
 * The default timeout value defined in msec and used on a silicon
 * platform. This timeout value is used for channel watchdog, ctxsw timeouts, gr
 * timeouts, etc.
 */
#define NVGPU_DEFAULT_POLL_TIMEOUT_MS	3000

/**
 * @ingroup NVGPU_COMMON_DEFAULTS
 *
 * The default timeout value defined in msec and used for railgate delay. This
 * defines a value for auto-suspend delay.
 */
#define NVGPU_DEFAULT_RAILGATE_IDLE_TIMEOUT 500

/**
 * @ingroup NVGPU_COMMON_DEFAULTS
 *
 * The default timeout value defined in msec and used on FPGA platform. This
 * timeout value is used at places similar to NVGPU_DEFAULT_POLL_TIMEOUT_MS.
 */
#define NVGPU_DEFAULT_FPGA_TIMEOUT_MS 100000U /* 100 sec */

#endif /* NVGPU_DEFAULTS_H */
