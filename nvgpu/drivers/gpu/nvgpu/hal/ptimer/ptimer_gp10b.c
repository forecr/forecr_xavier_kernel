// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "ptimer_gp10b.h"

#include <nvgpu/hw/gp10b/hw_timer_gp10b.h>

int gp10b_ptimer_config_gr_tick_freq(struct gk20a *g)
{
	nvgpu_writel(g, timer_gr_tick_freq_r(),
		timer_gr_tick_freq_select_f(
			timer_gr_tick_freq_select_max_f()));

	return 0;
}
