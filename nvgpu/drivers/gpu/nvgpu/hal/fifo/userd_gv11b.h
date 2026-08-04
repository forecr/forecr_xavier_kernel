/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef USERD_GV11B_H
#define USERD_GV11B_H

struct gk20a;
struct nvgpu_channel;

u32 gv11b_userd_gp_get(struct gk20a *g, struct nvgpu_channel *ch);
u64 gv11b_userd_pb_get(struct gk20a *g, struct nvgpu_channel *ch);
void gv11b_userd_gp_put(struct gk20a *g, struct nvgpu_channel *ch);

#endif /* USERD_GV11B_H */
