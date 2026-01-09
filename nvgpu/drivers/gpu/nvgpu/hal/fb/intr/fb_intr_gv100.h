/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_INTR_GV100_H
#define NVGPU_FB_INTR_GV100_H

struct gk20a;

void gv100_fb_intr_enable(struct gk20a *g);
void gv100_fb_intr_disable(struct gk20a *g);

#endif /* NVGPU_FB_INTR_GV100_H */
