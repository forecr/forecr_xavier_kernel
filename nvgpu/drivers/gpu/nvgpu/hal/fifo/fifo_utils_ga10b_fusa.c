// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/io.h>

#include <nvgpu/gk20a.h>
#include <nvgpu/runlist.h>

#include "fifo_utils_ga10b.h"

u32 nvgpu_runlist_readl(struct gk20a *g, struct nvgpu_runlist *runlist,
			u32 r)
{
	u32 runlist_pri_base = 0U;

	nvgpu_assert(runlist != NULL);
	runlist_pri_base = runlist->runlist_pri_base;
	nvgpu_assert(runlist_pri_base != 0U);

	return nvgpu_readl(g, nvgpu_safe_add_u32(runlist_pri_base, r));
}

void nvgpu_runlist_writel(struct gk20a *g, struct nvgpu_runlist *runlist,
			u32 r, u32 v)
{
	u32 runlist_pri_base = 0U;

	nvgpu_assert(runlist != NULL);
	runlist_pri_base = runlist->runlist_pri_base;
	nvgpu_assert(runlist_pri_base != 0U);

	nvgpu_writel(g, nvgpu_safe_add_u32(runlist_pri_base, r), v);
}

u32 nvgpu_chram_bar0_readl(struct gk20a *g, struct nvgpu_runlist *runlist,
			u32 r)
{
	u32 chram_bar0_offset = 0U;

	nvgpu_assert(runlist != NULL);
	chram_bar0_offset = runlist->chram_bar0_offset;
	nvgpu_assert(chram_bar0_offset != 0U);

	return nvgpu_readl(g, nvgpu_safe_add_u32(chram_bar0_offset, r));
}

void nvgpu_chram_bar0_writel(struct gk20a *g,
			struct nvgpu_runlist *runlist, u32 r, u32 v)
{
	u32 chram_bar0_offset = 0U;

	nvgpu_assert(runlist != NULL);
	chram_bar0_offset = runlist->chram_bar0_offset;
	nvgpu_assert(chram_bar0_offset != 0U);

	nvgpu_writel(g, nvgpu_safe_add_u32(chram_bar0_offset, r), v);
}
