/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_GV11B_H
#define NVGPU_FB_GV11B_H

struct gk20a;

void gv11b_fb_init_hw(struct gk20a *g);
void gv11b_fb_init_fs_state(struct gk20a *g);
int gv11b_fb_set_atomic_mode(struct gk20a *g);

#ifdef CONFIG_NVGPU_COMPRESSION
struct nvgpu_cbc;
void gv11b_fb_cbc_configure(struct gk20a *g, struct nvgpu_cbc *cbc);
#endif

#endif /* NVGPU_FB_GV11B_H */
