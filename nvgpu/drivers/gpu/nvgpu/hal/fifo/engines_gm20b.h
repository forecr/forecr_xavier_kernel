/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_GM20B_H
#define NVGPU_ENGINE_GM20B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_fifo;

bool gm20b_is_fault_engine_subid_gpc(struct gk20a *g, u32 engine_subid);
int  gm20b_engine_init_ce_info(struct nvgpu_fifo *f);

#endif /* NVGPU_ENGINE_GM20B_H */
