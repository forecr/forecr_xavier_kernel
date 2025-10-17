/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GA100_BUS
#define NVGPU_GA100_BUS

#include <nvgpu/types.h>

struct gk20a;

u32 ga100_bus_read_sw_scratch(struct gk20a *g, u32 index);
void ga100_bus_write_sw_scratch(struct gk20a *g, u32 index, u32 val);

#endif
