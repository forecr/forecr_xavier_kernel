// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/types.h>
#include <nvgpu/dma.h>

#include "acr_wpr.h"

/* Both size and address of WPR need to be 128K-aligned */
#define DGPU_WPR_SIZE 0x200000U

void nvgpu_acr_wpr_info_sys(struct gk20a *g, struct wpr_carveout_info *inf)
{
	g->ops.fb.read_wpr_info(g, &inf->wpr_base, &inf->size);
}
#ifdef CONFIG_NVGPU_DGPU
void nvgpu_acr_wpr_info_vid(struct gk20a *g, struct wpr_carveout_info *inf)
{
	inf->wpr_base = g->mm.vidmem.bootstrap_base;
	inf->nonwpr_base = inf->wpr_base + DGPU_WPR_SIZE;
	inf->size = DGPU_WPR_SIZE;
}
#endif
