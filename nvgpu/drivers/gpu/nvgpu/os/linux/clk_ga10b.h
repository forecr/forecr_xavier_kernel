/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_COMMON_LINUX_CLK_GA10B_H

struct gk20a;

unsigned long nvgpu_ga10b_linux_clk_get_rate(
					struct gk20a *g, u32 api_domain);
int nvgpu_ga10b_linux_clk_set_rate(struct gk20a *g,
					u32 api_domain, unsigned long rate);

#endif
