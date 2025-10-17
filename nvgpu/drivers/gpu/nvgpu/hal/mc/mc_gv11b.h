/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MC_GV11B_H
#define NVGPU_MC_GV11B_H

#include <nvgpu/types.h>

struct gk20a;

bool gv11b_mc_is_stall_and_eng_intr_pending(struct gk20a *g, u32 engine_id,
			u32 *eng_intr_pending);
bool gv11b_mc_is_mmu_fault_pending(struct gk20a *g);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
bool gv11b_mc_is_intr_hub_pending(struct gk20a *g, u32 mc_intr_0);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

#endif /* NVGPU_MC_GV11B_H */
