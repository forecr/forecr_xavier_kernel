/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CTXSW_TIMEOUT_GA10B_H
#define NVGPU_CTXSW_TIMEOUT_GA10B_H

#include <nvgpu/types.h>

#define MS_TO_US	1000U

struct gk20a;

u32 ga10b_rleng_ctxsw_timeout_info_reg_off(u32 rleng_id);
u32 ga10b_ctxsw_timeout_config_size_1(void);
u32 ga10b_fifo_ctxsw_timeout_config(u32 rleng_id);
void ga10b_fifo_ctxsw_timeout_enable(struct gk20a *g, bool enable);
/**
 * @brief Interrupt Service Routine (ISR) for handling context switch (ctxsw) timeouts.
 *
 * This function is called when a ctxsw timeout interrupt occurs. It checks each runlist
 * engine (rleng) for pending ctxsw timeout interrupts, retrieves the timeout information,
 * and reports the error. If a valid TSG (Timeslice Group) is associated with the timeout,
 * it is retrieved for further handling.
 *
 * The steps performed by the function are as follows:
 * -# Iterate over all possible runlist engines based on the maximum number supported.
 * -# Read the runlist interrupt register to check if a ctxsw timeout is pending for the
 *    current runlist engine using nvgpu_runlist_readl().
 * -# If no ctxsw timeout is pending for the current runlist engine, continue to the next one.
 * -# Retrieve the device associated with the current runlist engine from the runlist's
 *    device list.
 * -# If the device is invalid (NULL), log an error and continue to the next runlist engine.
 * -# Read the ctxsw timeout configuration for the current runlist engine to obtain the
 *    timeout period using nvgpu_runlist_readl().
 * -# Call ga10b_fifo_ctxsw_timeout_info() to retrieve the TSG ID and status associated with
 *    the ctxsw timeout event for the current runlist engine.
 * -# Check if the TSG ID is valid by calling nvgpu_tsg_check_and_get_from_id(). If not,
 *    continue to the next runlist engine.
 * -# Report the ctxsw timeout error to the system diagnostic logger using
 *    nvgpu_report_err_to_sdl().
 * -# Log the ctxsw timeout error using nvgpu_err().
 *
 * @param [in] g        Pointer to the GPU structure.
 * @param [in] runlist  Pointer to the runlist structure.
 */
void ga10b_fifo_ctxsw_timeout_isr(struct gk20a *g,
			struct nvgpu_runlist *runlist);

#endif /* NVGPU_CTXSW_TIMEOUT_GA10B_H */
