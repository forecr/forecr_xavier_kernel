// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/timers.h>
#include <nvgpu/ptimer.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "fifo_intr_gv100.h"

#include <nvgpu/hw/gv100/hw_fifo_gv100.h>


void gv100_fifo_intr_set_recover_mask(struct gk20a *g)
{
	u32 val;

	val = gk20a_readl(g, fifo_intr_en_0_r());
	val &= ~(fifo_intr_en_0_sched_error_m());
	gk20a_writel(g, fifo_intr_en_0_r(), val);
	gk20a_writel(g, fifo_intr_0_r(), fifo_intr_0_sched_error_reset_f());
}

void gv100_fifo_intr_unset_recover_mask(struct gk20a *g)
{
	u32 val;

	val = gk20a_readl(g, fifo_intr_en_0_r());
	val |= fifo_intr_en_0_sched_error_f(1);
	gk20a_writel(g, fifo_intr_en_0_r(), val);
}
