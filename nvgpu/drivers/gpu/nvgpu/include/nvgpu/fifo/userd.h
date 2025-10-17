/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_USERD_H
#define NVGPU_FIFO_USERD_H

#ifdef CONFIG_NVGPU_USERD
#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;

int nvgpu_userd_setup_sw(struct gk20a *g);
void nvgpu_userd_cleanup_sw(struct gk20a *g);
int nvgpu_userd_init_channel(struct gk20a *g, struct nvgpu_channel *c);

int nvgpu_userd_init_slabs(struct gk20a *g);
void nvgpu_userd_free_slabs(struct gk20a *g);
#endif

#endif /* NVGPU_FIFO_USERD_H */
