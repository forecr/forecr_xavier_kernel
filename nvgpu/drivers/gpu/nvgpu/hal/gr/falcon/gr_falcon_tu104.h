/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FALCON_TU104_H
#define NVGPU_GR_FALCON_TU104_H

#include <nvgpu/types.h>

struct gk20a;

int tu104_gr_falcon_ctrl_ctxsw(struct gk20a *g, u32 fecs_method,
		u32 data, u32 *ret_val);

#endif /* NVGPU_GR_FALCON_TU104_H */
