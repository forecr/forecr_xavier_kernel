// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/gmmu.h>

#include "gmmu_gk20a.h"

u32 gk20a_get_pde_pgsz(struct gk20a *g, const struct gk20a_mmu_level *l,
				struct nvgpu_gmmu_pd *pd, u32 pd_idx)
{
	(void)g;
	(void)l;
	(void)pd;
	(void)pd_idx;
	/*
	 * big and small page sizes are the same
	 */
	return GMMU_PAGE_SIZE_SMALL;
}

u32 gk20a_get_pte_pgsz(struct gk20a *g, const struct gk20a_mmu_level *l,
				struct nvgpu_gmmu_pd *pd, u32 pd_idx)
{
	(void)g;
	(void)l;
	(void)pd;
	(void)pd_idx;
	/*
	 * return invalid
	 */
	return GMMU_NR_PAGE_SIZES;
}
