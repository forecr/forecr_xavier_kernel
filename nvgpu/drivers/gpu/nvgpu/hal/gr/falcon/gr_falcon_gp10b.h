/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FALCON_GP10B_H
#define NVGPU_GR_FALCON_GP10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_falcon_query_sizes;

int gp10b_gr_falcon_ctrl_ctxsw(struct gk20a *g, u32 fecs_method,
						u32 data, u32 *ret_val);
int gp10b_gr_falcon_init_ctx_state(struct gk20a *g,
		struct nvgpu_gr_falcon_query_sizes *sizes);
#endif /* NVGPU_GR_FALCON_GP10B_H */
