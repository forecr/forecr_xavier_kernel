// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/dma.h>
#include <nvgpu/gk20a.h>

#include "acr_wpr.h"
#include "acr_priv.h"
#include "acr_blob_alloc.h"

int nvgpu_acr_alloc_blob_space_sys(struct gk20a *g, size_t size,
	struct nvgpu_mem *mem)
{
	return nvgpu_dma_alloc_flags_sys(g, NVGPU_DMA_PHYSICALLY_ADDRESSED,
		size, mem);
}
#ifdef CONFIG_NVGPU_DGPU
int nvgpu_acr_alloc_blob_space_vid(struct gk20a *g, size_t size,
	struct nvgpu_mem *mem)
{
	struct wpr_carveout_info wpr_inf;
	int err;

	if (mem->size != 0ULL) {
		return 0;
	}

	g->acr->get_wpr_info(g, &wpr_inf);

	/*
	 * Even though this mem_desc wouldn't be used, the wpr region needs to
	 * be reserved in the allocator.
	 */
	err = nvgpu_dma_alloc_vid_at(g, wpr_inf.size,
		&g->acr->wpr_dummy, wpr_inf.wpr_base);
	if (err != 0) {
		return err;
	}

	return nvgpu_dma_alloc_vid_at(g, wpr_inf.size, mem,
		wpr_inf.nonwpr_base);
}
#endif
