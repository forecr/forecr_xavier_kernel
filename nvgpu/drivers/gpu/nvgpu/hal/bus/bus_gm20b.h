/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GM20B_BUS
#define NVGPU_GM20B_BUS

struct gk20a;
struct nvgpu_mem;

int gm20b_bus_bar1_bind(struct gk20a *g, struct nvgpu_mem *bar1_inst);

#endif
