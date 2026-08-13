/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_USERMODE_GV11B_H
#define NVGPU_USERMODE_GV11B_H

#include <nvgpu/types.h>

struct nvgpu_channel;

u64 gv11b_usermode_base(struct gk20a *g);
u64 gv11b_usermode_bus_base(struct gk20a *g);
u32 gv11b_usermode_doorbell_token(struct gk20a *g, u32 runlist_id, u32 chid);
void gv11b_usermode_ring_doorbell(struct gk20a *g, u32 runlist_id, u32 chid);

#endif /* NVGPU_USERMODE_GV11B_H */
