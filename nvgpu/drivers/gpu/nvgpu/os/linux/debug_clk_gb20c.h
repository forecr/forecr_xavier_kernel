// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#ifndef __DEBUG_CLK_GB20C_H__
#define __DEBUG_CLK_GB20C_H__

#include <nvgpu/types.h>

struct gk20a;

/**
 * PTO (Performance Tracking Operations) counter register layout.
 * Used for frequency measurement in both virtualized and non-virtualized environments.
 */
struct pto_counter_regs {
	u32 cfg;
	u32 cnt0;
	u32 cnt1;
};

/* Main initialization function for clock and regulator debugging */
void gk20a_debug_clk_init(struct gk20a *g);

#endif /* __DEBUG_CLK_GB20C_H__ */
