// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu.h>
#include <nvgpu/pmu/super_surface.h>

#include "pmu_sw_gb20c.h"

void nvgpu_gb20c_pmu_sw_init(struct gk20a *g,
		struct nvgpu_pmu *pmu)
{
	nvgpu_log_fn(g, " ");

	pmu->ss_unmapped_members_size_add = SUPER_SURFACE_SIZE_20 - SUPER_SURFACE_SIZE;
	pmu->is_fbq_ptcb_enabled = true;
	pmu->is_pmu_ucode_model_20 = true;
}
