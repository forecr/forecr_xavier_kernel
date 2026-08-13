// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/hw/ga100/hw_trim_ga100.h>

#include "clk_ga100.h"

u32 ga100_clk_get_cntr_sysclk_source(struct gk20a *g)
{
	return trim_sys_fr_clk_cntr_sysclk_cfg_source_sys_noeg_f();
}

u32 ga100_clk_get_cntr_xbarclk_source(struct gk20a *g)
{
	return trim_sys_fll_fr_clk_cntr_xbarclk_cfg_source_xbar_nobg_duplicate_f();
}
