// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "hal/func/func_ga10b.h"

#include <nvgpu/hw/ga10b/hw_func_ga10b.h>

u32 ga10b_func_get_full_phys_offset(struct gk20a *g)
{
	(void)g;
	return func_full_phys_offset_v();
}

#ifdef CONFIG_NVGPU_IOCTL_NON_FUSA
int ga10b_func_read_ptimer(struct gk20a *g, u64 *value)
{
	const unsigned int max_iterations = 3;
	unsigned int i = 0;
	u32 gpu_timestamp_hi_prev = 0;

	if (value == NULL) {
		return -EINVAL;
	}

	/* Note. The GPU nanosecond timer consists of two 32-bit
	 * registers (high & low). To detect a possible low register
	 * wrap-around between the reads, we need to read the high
	 * register before and after low. The wraparound happens
	 * approximately once per 4 secs. */

	/* get initial gpu_timestamp_hi value */
	gpu_timestamp_hi_prev = nvgpu_func_readl(g, func_time_1_r());

	for (i = 0; i < max_iterations; ++i) {
		u32 gpu_timestamp_hi = 0;
		u32 gpu_timestamp_lo = 0;

		gpu_timestamp_lo = nvgpu_func_readl(g, func_time_0_r());
		gpu_timestamp_hi = nvgpu_func_readl(g, func_time_1_r());

		if (gpu_timestamp_hi == gpu_timestamp_hi_prev) {
			*value = (((u64)gpu_timestamp_hi) << 32) |
				gpu_timestamp_lo;
			return 0;
		}

		/* wrap-around detected, retry */
		gpu_timestamp_hi_prev = gpu_timestamp_hi;
	}

	/* too many iterations, bail out */
	nvgpu_err(g, "failed to read ptimer");
	return -EBUSY;
}
#endif
