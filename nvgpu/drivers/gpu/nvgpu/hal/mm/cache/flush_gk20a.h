/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_FLUSH_FLUSH_GK20A_H
#define HAL_MM_FLUSH_FLUSH_GK20A_H

#include <nvgpu/types.h>

struct gk20a;

int gk20a_mm_fb_flush(struct gk20a *g);
int gk20a_mm_l2_flush(struct gk20a *g, bool invalidate);
#ifdef CONFIG_NVGPU_COMPRESSION
void gk20a_mm_cbc_clean(struct gk20a *g);
#endif
void gk20a_mm_l2_invalidate(struct gk20a *g);

#endif
