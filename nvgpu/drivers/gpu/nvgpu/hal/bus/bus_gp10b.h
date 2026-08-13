/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BUS_GP10B
#define NVGPU_BUS_GP10B

struct gk20a;
struct nvgpu_mem;

int gp10b_bus_bar2_bind(struct gk20a *g, struct nvgpu_mem *bar2_inst);

#endif
