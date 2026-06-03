/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_GMMU_GMMU_GM20B_H
#define HAL_MM_GMMU_GMMU_GM20B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gmmu_attrs;

u32 gm20b_mm_get_big_page_sizes(void);
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
u64 gm20b_gpu_phys_addr(struct gk20a *g,
			struct nvgpu_gmmu_attrs *attrs, u64 phys);
#endif

#endif
