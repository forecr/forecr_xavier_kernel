/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LOG_H
#define NVGPU_LOG_H

#ifdef __KERNEL__
#include <nvgpu/linux/log.h>
#elif defined(__NVGPU_POSIX__)
#include <nvgpu/posix/log.h>
#elif defined(NVGPU_HVRTOS)
#include <nvgpu_hvrtos/log.h>
#else
#include <nvgpu/qnx/log.h>
#endif

#ifdef CONFIG_NVGPU_NON_FUSA
/**
 * nvgpu_log_mask_enabled - Check if logging is enabled
 *
 * @g        - The GPU.
 * @log_mask - The mask to check against.
 *
 * Check if, given the passed mask, logging would actually happen. This is
 * useful for avoiding calling the logging function many times when we know that
 * said prints would not happen. For example for-loops of log statements in
 * critical paths.
 */
bool nvgpu_log_mask_enabled(struct gk20a *g, u64 log_mask);
#endif

/**
 * nvgpu_log - Print a debug message
 *
 * @g        - The GPU.
 * @log_mask - A mask defining when the print should happen. See enum
 *             %nvgpu_log_categories.
 * @fmt      - A format string (printf style).
 * @arg...   - Arguments for the format string.
 *
 * Print a message if the log_mask matches the enabled debugging.
 */
#define nvgpu_log(g, log_mask, fmt, arg...)				\
	nvgpu_log_impl(g, log_mask, fmt, ##arg)

/**
 * nvgpu_err - Print an error
 *
 * @g        - The GPU.
 * @fmt      - A format string (printf style).
 * @arg...   - Arguments for the format string.
 *
 * Unconditionally print an error message.
 */
#define nvgpu_err(g, fmt, arg...)					\
	nvgpu_err_impl(g, fmt, ##arg)

/**
 * nvgpu_warn - Print a warning
 *
 * @g        - The GPU.
 * @fmt      - A format string (printf style).
 * @arg...   - Arguments for the format string.
 *
 * Unconditionally print a warning message.
 */
#define nvgpu_warn(g, fmt, arg...)					\
	nvgpu_warn_impl(g, fmt, ##arg)

/**
 * nvgpu_info - Print an info message
 *
 * @g        - The GPU.
 * @fmt      - A format string (printf style).
 * @arg...   - Arguments for the format string.
 *
 * Unconditionally print an information message.
 */
#define nvgpu_info(g, fmt, arg...)					\
	nvgpu_info_impl(g, fmt, ##arg)

/*
 * Some convenience macros.
 */
#define nvgpu_log_fn(g, fmt, arg...)	nvgpu_log(g, gpu_dbg_fn, fmt, ##arg)
#define nvgpu_log_info(g, fmt, arg...)	nvgpu_log(g, gpu_dbg_info, fmt, ##arg)

/******************************************************************************
 * The old legacy debugging API minus some parts that are unnecessary.        *
 * Please, please, please do not use this!!! This is still around to aid      *
 * transitioning to the new API.                                              *
 *                                                                            *
 * This changes up the print formats to be closer to the new APIs formats.    *
 * Also it removes the dev_warn() and dev_err() usage. Those arguments are    *
 * ignored now.                                                               *
 ******************************************************************************/

/*
 * This exist for backwards compatibility with the old debug/logging API. If you
 * want ftrace support use the new API!
 */

#define gk20a_dbg(log_mask, fmt, arg...)				\
	gk20a_dbg_impl(log_mask, fmt, ##arg)

/*
 * Some convenience macros.
 */
#define gk20a_dbg_fn(fmt, arg...)	gk20a_dbg(gpu_dbg_fn, fmt, ##arg)
#define gk20a_dbg_info(fmt, arg...)	gk20a_dbg(gpu_dbg_info, fmt, ##arg)

#endif /* NVGPU_LOG_H */
