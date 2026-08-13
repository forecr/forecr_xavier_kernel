/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_VGPU_H
#define NVGPU_CLK_VGPU_H

struct pto_counter_regs;

void vgpu_init_clk_support(struct gk20a *g);
int vgpu_clk_get_freqs(struct gk20a *g, unsigned long **freqs, int *num_freqs);
int vgpu_clk_cap_rate(struct gk20a *g, unsigned long rate);
int vgpu_pto_counter_get_freq(struct gk20a *g, const struct pto_counter_regs *regs, u64 *freq_hz);
#endif /* NVGPU_CLK_VGPU_H */
