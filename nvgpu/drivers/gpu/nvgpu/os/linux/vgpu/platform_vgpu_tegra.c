// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2014-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

/*
 * Tegra Virtualized GPU Platform Interface
 */

#include <nvgpu/gk20a.h>

#include "os/linux/platform_gk20a.h"
#include "common/vgpu/clk_vgpu.h"
#include "vgpu_linux.h"
#include "platform_vgpu_tegra.h"

long vgpu_plat_clk_round_rate(struct device *dev, unsigned long rate)
{
	/* server will handle frequency rounding */
	return rate;
}

int vgpu_plat_clk_get_freqs(struct device *dev, unsigned long **freqs,
			int *num_freqs)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	struct gk20a *g = platform->g;

	return vgpu_clk_get_freqs(g, freqs, num_freqs);
}

int vgpu_plat_clk_cap_rate(struct device *dev, unsigned long rate)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	struct gk20a *g = platform->g;

	return vgpu_clk_cap_rate(g, rate);
}
