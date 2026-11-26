/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_GMMU_GMMU_GV11B_H
#define HAL_MM_GMMU_GMMU_GV11B_H

#include <nvgpu/types.h>

struct gk20a;
struct gk20a_mmu_level;

u64 gv11b_gpu_phys_addr(struct gk20a *g,
			struct nvgpu_gmmu_attrs *attrs, u64 phys);

#endif
