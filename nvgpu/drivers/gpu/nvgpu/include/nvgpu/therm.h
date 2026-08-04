/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_THERM_H
#define NVGPU_THERM_H

/**
 * @page therm therm
 *
 * Overview
 * ========
 *
 * The |nvgpu-rm| therm unit provides a set of HAL interfaces to init and
 * manage thermal, ELCG and BLCG settings. The therm unit is initialized
 * during |nvgpu-rm| initialization.
 *
 * The therm initialization enables external thermal alert 0, 1 and 2
 * events. The events are setup with slowdown factors of 0x2, 0x6 and 0xE for
 * external thermal alert 0, 1 and 2 respectively.
 *
 * The gradual slowdown and stepping feature for clocks is enabled. This uses
 * stepping tables NV_THERM_GRAD_STEPPING_TABLE(0) with steps of div_by_1,
 * div_by_1p5, div_by_2, div_by_4 and div_by_8 and NV_THERM_GRAD_STEPPING_TABLE(1)
 * with steps of div_by_16 and div_by_32. It also configures PDIV duration with 0xBF4.
 * The idle clock slowdown feature is disabled.
 *
 * Once the therm unit is successfully initialized, it allows managing ELCG
 * and BLCG mode settings though HAL APIs.
 *
 * Static Design
 * =============
 *
 * External APIs
 * -------------
 *   + gops_therm.elcg_init_idle_filters
 *   + gops_therm.init_elcg_mode
 *   + gops_therm.init_therm_setup_hw
 *   + gops_therm.init_therm_support
 */

struct gk20a;

int nvgpu_init_therm_support(struct gk20a *g);

#endif
