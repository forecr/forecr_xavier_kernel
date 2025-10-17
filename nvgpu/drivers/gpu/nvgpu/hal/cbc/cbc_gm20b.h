/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CBC_GM20B
#define NVGPU_CBC_GM20B

#ifdef CONFIG_NVGPU_COMPRESSION

#include <nvgpu/types.h>

struct gk20a;
struct gpu_ops;
struct nvgpu_cbc;
enum nvgpu_cbc_op;

int gm20b_cbc_alloc_comptags(struct gk20a *g, struct nvgpu_cbc *cbc);
void gm20b_cbc_init(struct gk20a *g, struct nvgpu_cbc *cbc, bool is_resume);
int gm20b_cbc_ctrl(struct gk20a *g, enum nvgpu_cbc_op op,
		       u32 min, u32 max);
u32 gm20b_cbc_fix_config(struct gk20a *g, u32 base);


#endif
#endif
