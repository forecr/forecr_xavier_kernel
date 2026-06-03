// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu.h>

#include "pmu_sw_ga10b.h"

void nvgpu_ga10b_pmu_sw_init(struct gk20a *g,
		struct nvgpu_pmu *pmu)
{
	nvgpu_log_fn(g, " ");

	pmu->is_fbq_ptcb_enabled = true;
}
