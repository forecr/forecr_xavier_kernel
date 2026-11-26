// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "top_gv100.h"

#include <nvgpu/hw/gv100/hw_top_gv100.h>

u32 gv100_top_get_nvhsclk_ctrl_e_clk_nvl(struct gk20a *g)
{
	u32 reg;

	reg = nvgpu_readl(g, top_nvhsclk_ctrl_r());
	return top_nvhsclk_ctrl_e_clk_nvl_v(reg);
}

void gv100_top_set_nvhsclk_ctrl_e_clk_nvl(struct gk20a *g, u32 val)
{
	u32 reg;

	reg = nvgpu_readl(g, top_nvhsclk_ctrl_r());
	reg = set_field(reg, top_nvhsclk_ctrl_e_clk_nvl_m(),
				top_nvhsclk_ctrl_e_clk_nvl_f(val));
	nvgpu_writel(g, top_nvhsclk_ctrl_r(), reg);
}

u32 gv100_top_get_nvhsclk_ctrl_swap_clk_nvl(struct gk20a *g)
{
	u32 reg;

	reg = nvgpu_readl(g, top_nvhsclk_ctrl_r());
	return top_nvhsclk_ctrl_swap_clk_nvl_v(reg);
}

void gv100_top_set_nvhsclk_ctrl_swap_clk_nvl(struct gk20a *g, u32 val)
{
	u32 reg;

	reg = nvgpu_readl(g, top_nvhsclk_ctrl_r());
	reg = set_field(reg, top_nvhsclk_ctrl_swap_clk_nvl_m(),
				top_nvhsclk_ctrl_swap_clk_nvl_f(val));
	nvgpu_writel(g, top_nvhsclk_ctrl_r(), reg);
}

u32 gv100_top_get_max_fbpas_count(struct gk20a *g)
{
	u32 tmp;

	tmp = nvgpu_readl(g, top_num_fbpas_r());
	return top_num_fbpas_value_v(tmp);
}
