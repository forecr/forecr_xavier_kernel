/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_GP10B_H
#define NVGPU_FB_GP10B_H
struct gk20a;

#ifdef CONFIG_NVGPU_COMPRESSION
u64 gp10b_fb_compression_page_size(struct gk20a *g);
unsigned int gp10b_fb_compressible_page_size(struct gk20a *g);
#endif

#endif /* NVGPU_FB_GP10B_H */
