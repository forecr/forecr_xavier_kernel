/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef FIFO_CHANNEL_GM20B_H
#define FIFO_CHANNEL_GM20B_H

#include <nvgpu/types.h>

struct nvgpu_channel;
struct gk20a;

void gm20b_channel_bind(struct nvgpu_channel *c);
void gm20b_channel_force_ctx_reload(struct gk20a *g, u32 runlist_id, u32 chid);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
u32 gm20b_channel_count(struct gk20a *g);
#endif

#endif /* FIFO_CHANNEL_GM20B_H */
