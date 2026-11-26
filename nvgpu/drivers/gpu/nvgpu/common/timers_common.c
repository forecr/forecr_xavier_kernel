// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/timers.h>

void nvgpu_timeout_init_cpu_timer(struct gk20a *g, struct nvgpu_timeout *timeout,
		       u32 duration_ms)
{
	int err = nvgpu_timeout_init_flags(g, timeout, duration_ms,
					   NVGPU_TIMER_CPU_TIMER);

	nvgpu_assert(err == 0);
}

void nvgpu_timeout_init_cpu_timer_sw(struct gk20a *g, struct nvgpu_timeout *timeout,
		       u32 duration_ms)
{
	int err = nvgpu_timeout_init_flags(g, timeout, duration_ms,
					   NVGPU_TIMER_CPU_TIMER |
					   NVGPU_TIMER_NO_PRE_SI);

	nvgpu_assert(err == 0);
}

void nvgpu_timeout_init_retry(struct gk20a *g, struct nvgpu_timeout *timeout,
		       u32 duration_count)
{
	int err = nvgpu_timeout_init_flags(g, timeout, duration_count,
					   NVGPU_TIMER_RETRY_TIMER);

	nvgpu_assert(err == 0);
}

