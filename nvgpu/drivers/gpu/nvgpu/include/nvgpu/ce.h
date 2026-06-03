/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_CE_H
#define NVGPU_CE_H

/**
 * @file
 * @page common.ce common.ce
 *
 * Overview
 * ========
 *
 * The CE unit is responsible for initializing the copy engines. The GPU has two
 * types of copy engines, GRCE and LCE.
 *
 * Data Structures
 * ===============
 * NA
 *
 * Static Design
 * =============
 *
 * CE Initialization
 * -----------------
 * The CE unit resets the copy engines at Master Control (MC) level and programs
 * the production clock gating and configuration options for copy engines.
 *
 * External APIs
 * -------------
 *   + nvgpu_ce_init_support()
 *
 * Dynamic Design
 * ==============
 *
 * At runtime, the CE stalling and non-stalling interrupts are handled through
 * CE unit hal interfaces. TSG initialization calls CE unit hal interface to
 * get the number of physical CEs.
 *
 * External APIs
 * -------------
 * Dynamic interfaces are HAL functions. They are documented here:
 *   + include/nvgpu/gops/ce.h
 */

struct gk20a;
struct nvgpu_device;

/**
 * @brief Initialize the CE support.
 *
 * @param g [in] The GPU driver struct.
 *
 * This function is invoked during #nvgpu_finalize_poweron to initialize the
 * copy engines.
 *
 * Steps:
 * - Get the reset mask for all copy engines.
 * - Reset the engines at master control level through mc_enable_r.
 * - Load Second Level Clock Gating (SLCG) configuration for copy engine.
 * - Load Block Level Clock Gating (BLCG) configuration for copy engine.
 * - Set FORCE_BARRIERS_NPL configuration option for LCEs.
 * - Enable CE engines' stalling and non-stalling interrupts at MC level.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_ce_init_support(struct gk20a *g);
int nvgpu_ce_early_init_support(struct gk20a *g);
void nvgpu_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base);
s32 nvgpu_ce_reset(struct gk20a *g, const struct nvgpu_device *dev);
#endif /*NVGPU_CE_H*/
