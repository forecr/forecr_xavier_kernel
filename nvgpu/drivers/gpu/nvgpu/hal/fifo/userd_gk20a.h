/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef USERD_GK20A_H
#define USERD_GK20A_H

struct gk20a;
#ifdef CONFIG_NVGPU_USERD
struct nvgpu_channel;

void gk20a_userd_init_mem(struct gk20a *g, struct nvgpu_channel *c);
#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
u32 gk20a_userd_gp_get(struct gk20a *g, struct nvgpu_channel *c);
u64 gk20a_userd_pb_get(struct gk20a *g, struct nvgpu_channel *c);
void gk20a_userd_gp_put(struct gk20a *g, struct nvgpu_channel *c);
#endif
#endif /* CONFIG_NVGPU_USERD */
u32 gk20a_userd_entry_size(struct gk20a *g);

#endif /* USERD_GK20A_H */
