/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef CLK_GA100_H
#define CLK_GA100_H

u32 ga100_clk_get_cntr_xbarclk_source(struct gk20a *g);
u32 ga100_clk_get_cntr_sysclk_source(struct gk20a *g);

#endif /* CLK_GA100_H */
