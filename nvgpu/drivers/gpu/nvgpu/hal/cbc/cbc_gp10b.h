/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CBC_GP10B_H
#define CBC_GP10B_H

#ifdef CONFIG_NVGPU_COMPRESSION

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_cbc;

int gp10b_cbc_alloc_comptags(struct gk20a *g, struct nvgpu_cbc *cbc);
int gp10b_cbc_ctrl(struct gk20a *g, enum nvgpu_cbc_op op, u32 min, u32 max);

#endif
#endif
