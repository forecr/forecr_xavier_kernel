/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_GMMU_GB10B_H
#define HAL_MM_GMMU_GB10B_H

#include <nvgpu/types.h>

struct gk20a;
struct gk20a_mmu_level;
struct nvgpu_gmmu_attrs;

const struct gk20a_mmu_level *gb10b_mm_get_mmu_levels(struct gk20a *g,
						      u64 big_page_size);
u32 gb10b_get_max_page_table_levels(struct gk20a *g);
u32 gb10b_mm_default_huge_page_size(void);

#endif /* HAL_MM_GMMU_GB10B_H */
