/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef PTIMER_GV11B_H
#define PTIMER_GV11B_H

#ifdef CONFIG_NVGPU_PROFILER

#include <nvgpu/types.h>

void gv11b_ptimer_get_timer_reg_offsets(u32 *timer0_offset, u32 *timer1_offset);

#endif /* CONFIG_NVGPU_PROFILER */
#endif /* PTIMER_GV11B_H */
