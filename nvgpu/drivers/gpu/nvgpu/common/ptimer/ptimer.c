// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/ptimer.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/timers.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/ptimer.h>

static u32 ptimer_scalingfactor10x(u32 ptimer_src_freq)
{
	return nvgpu_safe_cast_u64_to_u32((U64(PTIMER_REF_FREQ_HZ) * U64(10))
						/ U64(ptimer_src_freq));
}

int nvgpu_ptimer_scale(struct gk20a *g, u32 timeout, u32 *scaled_timeout)
{
	u32 scale10x;

	/* Validate the input parameters */
	if (scaled_timeout == NULL) {
		return -EINVAL;
	}

	if (timeout > U32_MAX / 10U) {
		return -EINVAL;
	}

	/* Calculate the scaling factor */
	if (g->ptimer_src_freq == 0U) {
		return -EINVAL;
	}
	scale10x = ptimer_scalingfactor10x(g->ptimer_src_freq);
	if (scale10x == 0U) {
		return -EINVAL;
	}

	/* Scale the timeout value */
	if (((timeout * 10U) % scale10x) >= (scale10x / 2U)) {
		*scaled_timeout = ((timeout * 10U) / scale10x) + 1U;
	} else {
		*scaled_timeout = (timeout * 10U) / scale10x;
	}

	return 0;
}

int nvgpu_ptimer_init(struct gk20a *g)
{
	nvgpu_cg_slcg_timer_load_enable(g);

	return 0;
}

#ifdef CONFIG_NVGPU_IOCTL_NON_FUSA
int nvgpu_get_timestamps_zipper(struct gk20a *g,
		enum nvgpu_cpu_timestamp_source cpu_timestamp_source,
		u32 count,
		struct nvgpu_cpu_time_correlation_sample *samples)
{
	int err = 0;
	unsigned int i = 0;

	if (gk20a_busy(g) != 0) {
		nvgpu_err(g, "GPU not powered on\n");
		err = -EINVAL;
		return err;
	}

	for (i = 0; i < count; i++) {
		err = g->ops.ptimer.read_ptimer(g, &samples[i].gpu_timestamp);
		if (err != 0) {
			goto idle;
		}

		/* The cpu timestamp is already validated when handling ioctl/devctls */
		if (cpu_timestamp_source == NVGPU_CPU_TIMESTAMP_SOURCE_TSC) {
			samples[i].cpu_timestamp = nvgpu_hr_timestamp();
		}
#ifndef NVGPU_HVRTOS
		/*
		 * The HV library ecosystem does not implement a function to collect
		 * the CPU time samples and thus this query cannot be implemented
		 * without refactoring the GPU KMD - GPU server HALs. Thus, the
		 * code needs to be ifdef-out for HV because nvgpu_current_unix_time_us
		 * is not implemented.
		 *
		 * Under HV, this call can always be handled by userspace for
		 * two reasons:
		 * - all HV configurations have the usermode region mapped to
		 *   userspace.
		 * - RG is disabled under HV, and thus it is safe for usermode
		 *   to read from the region.
		 *
		 * Thus, this path does not need to be handled under HV.
		 */
		else if (cpu_timestamp_source == NVGPU_CPU_TIMESTAMP_SOURCE_UNIX) {
			err = nvgpu_current_unix_time_us(&samples[i].cpu_timestamp);
			if (err < 0) {
				goto idle;
			}
		}
#endif
		else {
			err = -EINVAL;
			goto idle;
		}
	}

idle:
	gk20a_idle(g);
	return err;
}
#endif
