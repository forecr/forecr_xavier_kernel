/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_BUS_GA10B_H
#define NVGPU_BUS_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

int ga10b_bus_init_hw(struct gk20a *g);
void ga10b_bus_isr(struct gk20a *g);

#endif /* NVGPU_BUS_GA10B_H */
