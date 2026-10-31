/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef USERD_GB10B_H
#define USERD_GB10B_H

#ifdef CONFIG_NVGPU_USERD
struct gk20a;
struct nvgpu_channel;

void gb10b_userd_init_mem(struct gk20a *g, struct nvgpu_channel *c);
u32 gb10b_sema_based_gp_get(struct gk20a *g, struct nvgpu_channel *ch);
#endif

#endif /* USERD_GB10B_H */
