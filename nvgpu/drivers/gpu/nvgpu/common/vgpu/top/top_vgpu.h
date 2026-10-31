/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef COMMON_VGPU_TOP_VGPU_H
#define COMMON_VGPU_TOP_VGPU_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_device;

struct nvgpu_device *vgpu_top_parse_next_dev(struct gk20a *g, u32 *token);
u32 vgpu_top_get_max_rop_per_gpc(struct gk20a *g);

#endif
