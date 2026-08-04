/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_STATUS_GV100_H
#define NVGPU_ENGINE_STATUS_GV100_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_engine_status_info;
struct nvgpu_debug_context;

void gv100_read_engine_status_info(struct gk20a *g, u32 engine_id,
		struct nvgpu_engine_status_info *status);

void gv100_dump_engine_status(struct gk20a *g, struct nvgpu_debug_context *o);

#endif /* NVGPU_ENGINE_STATUS_GV100_H */

