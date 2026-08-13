/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_INTR_GB10B_H
#define NVGPU_FB_INTR_GB10B_H

struct gk20a;

void gb10b_fb_intr_vectorid_init(struct gk20a *g);
void gb10b_fb_intr_enable(struct gk20a *g);
void gb10b_fb_intr_disable(struct gk20a *g);

#endif /* NVGPU_FB_INTR_GB10B_H */
