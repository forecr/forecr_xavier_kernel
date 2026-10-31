/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_GA100_H
#define NVGPU_FB_GA100_H

struct gk20a;

void ga100_fb_init_fs_state(struct gk20a *g);

#ifdef CONFIG_NVGPU_COMPRESSION
u64 ga100_fb_compression_page_size(struct gk20a *g);
bool ga100_fb_is_comptagline_mode_enabled(struct gk20a *g);
#endif

#endif /* NVGPU_FB_GA100_H */
