/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BUS_TU104_H
#define NVGPU_BUS_TU104_H

struct gk20a;
struct nvgpu_mem;

int tu104_bus_init_hw(struct gk20a *g);
int bus_tu104_bar2_bind(struct gk20a *g, struct nvgpu_mem *bar2_inst);

#endif /* NVGPU_BUS_TU104_H */
