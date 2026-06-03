/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_FLUSH_FLUSH_GB10B_H
#define HAL_MM_FLUSH_FLUSH_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/timers.h>

struct gk20a;

/* TODO: add to dev_nv_xal_addendum.h */
#define NV_XAL_EP_MEMOP_TOKEN_MAX_ISSUED	(4U)

void gb10b_mm_fb_flush_internal(struct gk20a *g, struct nvgpu_timeout *timeout);
int gb10b_mm_l2_flush_internal(struct gk20a *g, struct nvgpu_timeout *timeout);
#ifdef CONFIG_NVGPU_COMPRESSION
void gb10b_mm_l2_clean_comptags(struct gk20a *g, struct nvgpu_timeout *timeout);
#endif

#endif
