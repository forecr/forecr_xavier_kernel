// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/timers.h>
#include <nvgpu/mm.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "bus_ga100.h"

#include <nvgpu/hw/ga100/hw_bus_ga100.h>

u32 ga100_bus_read_sw_scratch(struct gk20a *g, u32 index)
{
	return gk20a_readl(g, bus_sw_scratch_r(index));
}

void ga100_bus_write_sw_scratch(struct gk20a *g, u32 index, u32 val)
{
	gk20a_writel(g, bus_sw_scratch_r(index), val);
}
