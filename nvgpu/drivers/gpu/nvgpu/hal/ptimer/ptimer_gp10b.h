/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef PTIMER_GP10B_H
#define PTIMER_GP10B_H

#ifdef CONFIG_NVGPU_DEBUGGER

struct gk20a;

int gp10b_ptimer_config_gr_tick_freq(struct gk20a *g);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* PTIMER_GP10B_H */
