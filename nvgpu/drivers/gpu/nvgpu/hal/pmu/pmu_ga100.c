// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "pmu_ga100.h"

#include <nvgpu/hw/ga100/hw_pwr_ga100.h>

bool ga100_is_pmu_supported(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_SIM
	/* PMU not supported in dGpu Simulation */
	if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)) {
		return false;
	} else
#endif
	{
		/* PMU support not required until PSTATE support enabled */
		return false;
	}
}
