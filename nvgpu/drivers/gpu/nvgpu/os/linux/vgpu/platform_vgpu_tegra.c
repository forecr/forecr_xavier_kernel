// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

/*
 * Tegra Virtualized GPU Platform Interface
 */

#include <nvgpu/gk20a.h>

#include "os/linux/platform_gk20a.h"
#include "os/linux/scale.h"
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

/**
 * vgpu_plat_get_clk_freqs_opp() - Get GPU clock frequencies with OPP support
 * @dev: Device pointer
 * @freqs: Pointer to store allocated frequency array
 * @num_freqs: Pointer to store number of frequencies
 *
 * This function provides OPP-aware frequency retrieval for vGPU:
 * 1. First tries device tree OPP table (enables thermal throttling)
 * 2. Falls back to GPU server frequencies if no OPP table exists
 *
 * Use this for vGPU platforms that support thermal throttling via OPP.
 * For legacy behavior without OPP, use vgpu_plat_clk_get_freqs().
 *
 * Returns: 0 on success, error code on failure
 */
int vgpu_plat_get_clk_freqs_opp(struct device *dev, unsigned long **freqs,
				 int *num_freqs)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	struct gk20a *g = platform->g;
	int err;

	/* Try to get frequencies from device tree OPP table first */
	err = gk20a_tegra_get_clk_freqs_from_opp(dev, freqs, num_freqs);
	if (err == 0) {
		/*
		 * Success! Using OPP table from device tree.
		 * Thermal throttling via devfreq cooling will work.
		 */
		nvgpu_info(g, "Using %d OPP frequencies from device tree",
			*num_freqs);
		return 0;
	}

	/*
	 * If OPP table not found (-ENODEV), fall back to GPU server
	 * frequencies. Other errors are propagated as failures.
	 */
	if (err != -ENODEV) {
		nvgpu_err(g, "Failed to get OPP frequencies: %d", err);
		return err;
	}

	/*
	 * No OPP table in device tree, fall back to vGPU server/vf table frequencies.
	 */
	nvgpu_info(g,
		"No OPP table, falling back to GPU server frequencies");
	err = vgpu_clk_get_freqs(g, freqs, num_freqs);
	if (err) {
		nvgpu_err(g, "Failed to get GPU server frequencies: %d", err);
		return err;
	}

	nvgpu_info(g,
		"Using %d frequencies from GPU server (no thermal support)",
		*num_freqs);
	return 0;
}

int vgpu_plat_clk_cap_rate(struct device *dev, unsigned long rate)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	struct gk20a *g = platform->g;

	return vgpu_clk_cap_rate(g, rate);
}
