// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/xve.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/timers.h>
#include <nvgpu/gk20a.h>

#include "xve_tu104.h"

#include <nvgpu/hw/tu104/hw_xve_tu104.h>
#include <nvgpu/hw/tu104/hw_xp_tu104.h>

#define DL_TIMER_LIMIT 0x58EU

void tu104_devinit_deferred_settings(struct gk20a *g)
{
	u32 data;
	g->ops.xve.xve_writel(g, xve_pcie_capability_r(),
			xve_pcie_capability_gen2_capable_enable_f() |
			xve_pcie_capability_gen3_capable_enable_f());
	nvgpu_writel(g, xp_dl_mgr_timing_r(0), DL_TIMER_LIMIT);
	data = xve_high_latency_snoop_latency_value_init_f() |
			xve_high_latency_snoop_latency_scale_init_f() |
			xve_high_latency_no_snoop_latency_value_init_f() |
			xve_high_latency_no_snoop_latency_scale_init_f();
	g->ops.xve.xve_writel(g, xve_high_latency_r(), data);
	g->ops.xve.xve_writel(g, xve_ltr_msg_ctrl_r(),
			xve_ltr_msg_ctrl_trigger_not_pending_f());
}
