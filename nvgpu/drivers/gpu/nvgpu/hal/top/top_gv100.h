/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef TOP_GV100_H
#define TOP_GV100_H

#include <nvgpu/types.h>

struct gk20a;

u32 gv100_top_get_nvhsclk_ctrl_e_clk_nvl(struct gk20a *g);
void gv100_top_set_nvhsclk_ctrl_e_clk_nvl(struct gk20a *g, u32 val);
u32 gv100_top_get_nvhsclk_ctrl_swap_clk_nvl(struct gk20a *g);
void gv100_top_set_nvhsclk_ctrl_swap_clk_nvl(struct gk20a *g, u32 val);

u32 gv100_top_get_max_fbpas_count(struct gk20a *g);

#endif
