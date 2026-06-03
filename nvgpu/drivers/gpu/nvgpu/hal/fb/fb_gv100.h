/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_GV100_H
#define NVGPU_FB_GV100_H

struct gk20a;

void gv100_fb_reset(struct gk20a *g);
int gv100_fb_memory_unlock(struct gk20a *g);
int gv100_fb_init_nvlink(struct gk20a *g);
int gv100_fb_enable_nvlink(struct gk20a *g);
int gv100_fb_set_atomic_mode(struct gk20a *g);
#ifdef CONFIG_NVGPU_DGPU
size_t gv100_fb_get_vidmem_size(struct gk20a *g);
#endif
void gv100_fb_set_mmu_debug_mode(struct gk20a *g, bool enable);

#endif /* NVGPU_FB_GV100_H */
