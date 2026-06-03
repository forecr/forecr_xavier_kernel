/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MC_GV100_H
#define NVGPU_MC_GV100_H

#include <nvgpu/types.h>

struct gk20a;

bool gv100_mc_is_intr_nvlink_pending(struct gk20a *g, u32 mc_intr_0);
bool gv100_mc_is_stall_and_eng_intr_pending(struct gk20a *g, u32 engine_id,
			u32 *eng_intr_pending);
int gv100_mc_enable_units(struct gk20a *g, u32 units, bool enable);

#endif /* NVGPU_MC_GV100_H */
