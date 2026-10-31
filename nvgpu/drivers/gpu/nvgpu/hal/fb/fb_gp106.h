/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef FB_GP106_H
#define FB_GP106_H
struct gpu_ops;

void gp106_fb_init_fs_state(struct gk20a *g);
#ifdef CONFIG_NVGPU_DGPU
size_t gp106_fb_get_vidmem_size(struct gk20a *g);
#endif
#endif
