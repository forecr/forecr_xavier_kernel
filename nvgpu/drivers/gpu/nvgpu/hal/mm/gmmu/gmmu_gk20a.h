/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_GMMU_GMMU_GK20A_H
#define HAL_MM_GMMU_GMMU_GK20A_H

#include <nvgpu/types.h>
#include <nvgpu/gmmu.h>

struct gk20a;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA

const struct gk20a_mmu_level *gk20a_mm_get_mmu_levels(struct gk20a *g,
						      u64 big_page_size);
u32 gk20a_get_max_page_table_levels(struct gk20a *g);
u32 gk20a_mm_get_iommu_bit(struct gk20a *g);
#endif

u32 gk20a_get_pde_pgsz(struct gk20a *g, const struct gk20a_mmu_level *l,
				struct nvgpu_gmmu_pd *pd, u32 pd_idx);
u32 gk20a_get_pte_pgsz(struct gk20a *g, const struct gk20a_mmu_level *l,
				struct nvgpu_gmmu_pd *pd, u32 pd_idx);

#endif
