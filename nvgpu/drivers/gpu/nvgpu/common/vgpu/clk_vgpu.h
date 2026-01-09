/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_VGPU_H
#define NVGPU_CLK_VGPU_H

void vgpu_init_clk_support(struct gk20a *g);
int vgpu_clk_get_freqs(struct gk20a *g, unsigned long **freqs, int *num_freqs);
int vgpu_clk_cap_rate(struct gk20a *g, unsigned long rate);
#endif /* NVGPU_CLK_VGPU_H */
