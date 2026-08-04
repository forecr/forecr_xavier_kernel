/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_TU104_H
#define NVGPU_FB_TU104_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

#ifdef CONFIG_NVGPU_COMPRESSION
struct nvgpu_cbc;

void tu104_fb_cbc_get_alignment(struct gk20a *g,
		u64 *base_divisor, u64 *top_divisor);
void tu104_fb_cbc_configure(struct gk20a *g, struct nvgpu_cbc *cbc);
#endif

#ifdef CONFIG_NVGPU_DGPU
size_t tu104_fb_get_vidmem_size(struct gk20a *g);
#endif
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
int  fb_tu104_tlb_invalidate(struct gk20a *g, struct nvgpu_mem *pdb);
int  tu104_fb_enable_nvlink(struct gk20a *g);
int tu104_fb_set_atomic_mode(struct gk20a *g);
int  tu104_fb_apply_pdb_cache_errata(struct gk20a *g);
#endif /* CONFIG_NVGPU_HAL_NON_FUSA */
#endif /* NVGPU_FB_TU104_H */
