/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_USERMODE_TU104_H
#define NVGPU_USERMODE_TU104_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;

u64 tu104_usermode_base(struct gk20a *g);
u64 tu104_usermode_bus_base(struct gk20a *g);
void tu104_usermode_setup_hw(struct gk20a *g);
u32 tu104_usermode_doorbell_token(struct gk20a *g, u32 runlist_id, u32 chid);
void tu104_usermode_ring_doorbell(struct gk20a *g, u32 runlist_id, u32 chid);

#endif /* NVGPU_USERMODE_TU104_H */
