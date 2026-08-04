/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_GV11B_H
#define NVGPU_ENGINE_GV11B_H

#include <nvgpu/types.h>

struct gk20a;

bool gv11b_is_fault_engine_subid_gpc(struct gk20a *g, u32 engine_subid);

#endif /* NVGPU_ENGINE_GV11B_H */
