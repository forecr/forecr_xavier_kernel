// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>

#include <nvgpu/hw/gp106/hw_top_gp106.h>

#include "top_gp106.h"

u32 gp106_top_read_scratch1_reg(struct gk20a *g)
{
	return gk20a_readl(g, top_scratch1_r());
}

u32 gp106_top_scratch1_devinit_completed(struct gk20a *g, u32 value)
{
	(void)g;
	return top_scratch1_devinit_completed_v(value);
}
