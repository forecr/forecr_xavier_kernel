/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_GMMU_GA10B_H
#define HAL_MM_GMMU_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct gk20a_mmu_level;
struct nvgpu_gmmu_attrs;

u32 ga10b_mm_get_iommu_bit(struct gk20a *g);
const struct gk20a_mmu_level *ga10b_mm_get_mmu_levels(struct gk20a *g,
			u64 big_page_size);
u32 ga10b_get_max_page_table_levels(struct gk20a *g);

#endif /* HAL_MM_GMMU_GA10B_H */
