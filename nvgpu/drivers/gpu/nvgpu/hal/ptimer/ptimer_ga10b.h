/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef PTIMER_GA10B_H
#define PTIMER_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

void ga10b_ptimer_isr(struct gk20a *g);

#endif /* PTIMER_GA10B_H */
