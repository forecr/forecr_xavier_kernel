/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_STATUS_GA10B_H
#define NVGPU_ENGINE_STATUS_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_engine_status_info;

u32 ga10b_rleng_status0_reg_off(u32 rleng_id);
u32 ga10b_rleng_status1_reg_off(u32 rleng_id);
void ga10b_read_engine_status_info(struct gk20a *g, u32 engine_id,
		struct nvgpu_engine_status_info *status);

#endif /* NVGPU_ENGINE_STATUS_GA10B_H */
