// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "bus_gv11b.h"

#include <nvgpu/hw/gv11b/hw_bus_gv11b.h>

void gv11b_bus_configure_debug_bus(struct gk20a *g)
{
#if !defined(CONFIG_NVGPU_DEBUGGER)
	nvgpu_writel(g, bus_debug_sel_0_r(), 0U);
	nvgpu_writel(g, bus_debug_sel_1_r(), 0U);
	nvgpu_writel(g, bus_debug_sel_2_r(), 0U);
	nvgpu_writel(g, bus_debug_sel_3_r(), 0U);
#else
	(void)g;
#endif
}
