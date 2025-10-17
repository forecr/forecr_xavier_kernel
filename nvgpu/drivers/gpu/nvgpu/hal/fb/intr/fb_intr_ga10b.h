/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_INTR_GA10B_H
#define NVGPU_FB_INTR_GA10B_H

struct gk20a;

void ga10b_fb_intr_vectorid_init(struct gk20a *g);
void ga10b_fb_intr_enable(struct gk20a *g);
void ga10b_fb_intr_disable(struct gk20a *g);
void ga10b_fb_intr_isr(struct gk20a *g, u32 intr_unit_bitmask);

#endif /* NVGPU_FB_INTR_GA10B_H */
