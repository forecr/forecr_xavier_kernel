/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_BUS_GK20A_H
#define NVGPU_BUS_GK20A_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;
struct nvgpu_sgt;

void gk20a_bus_isr(struct gk20a *g);
int gk20a_bus_init_hw(struct gk20a *g);
#ifdef CONFIG_NVGPU_DGPU
u32 gk20a_bus_set_bar0_window(struct gk20a *g, struct nvgpu_mem *mem,
			struct nvgpu_sgt *sgt, void *sgl, u32 w);
#endif

#endif /* NVGPU_BUS_GK20A_H */
