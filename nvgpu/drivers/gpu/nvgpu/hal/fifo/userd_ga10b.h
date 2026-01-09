/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef USERD_GA10B_H
#define USERD_GA10B_H

#ifdef CONFIG_NVGPU_USERD
struct gk20a;
struct nvgpu_channel;

void ga10b_userd_init_mem(struct gk20a *g, struct nvgpu_channel *c);
#endif

#endif /* USERD_GA10B_H */
