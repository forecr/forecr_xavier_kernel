// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include "ptimer_gv11b.h"

#include <nvgpu/hw/gv11b/hw_timer_gv11b.h>

void gv11b_ptimer_get_timer_reg_offsets(u32 *timer0_offset, u32 *timer1_offset)
{
	if (timer0_offset != NULL) {
		*timer0_offset = timer_time_0_r();
	}
	if (timer1_offset != NULL) {
		*timer1_offset = timer_time_1_r();
	}
}
