/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_INTR_GB10B_VF_H
#define NVGPU_FB_INTR_GB10B_VF_H

struct gk20a;

void vf_gb10b_fb_intr_vectorid_init(struct gk20a *g);
void vf_gb10b_fb_intr_enable(struct gk20a *g);
void vf_gb10b_fb_intr_disable(struct gk20a *g);

#endif /* NVGPU_FB_INTR_GB10B_VF_H */
