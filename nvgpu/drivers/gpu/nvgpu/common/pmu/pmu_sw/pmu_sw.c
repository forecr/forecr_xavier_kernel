// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pmu.h>
#include <nvgpu/string.h>
#include <nvgpu/log.h>
#include <nvgpu/pmu/pmu_sw.h>

#if defined(CONFIG_NVGPU_NON_FUSA)
#include "pmu_sw_ga10b.h"
#endif /* CONFIG_NVGPU_NON_FUSA */

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_pmu_pmu_sw.h>
#endif

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
#include "pmu_sw_gb20c.h"
#endif

void nvgpu_pmu_sw_init(struct gk20a *g, struct nvgpu_pmu *pmu)
{
	u32 ver = g->params.gpu_arch + g->params.gpu_impl;

	(void)pmu;

	if (!g->support_ls_pmu) {
		return;
	}

	switch (ver) {
	case GK20A_GPUID_GM20B:
	case GK20A_GPUID_GM20B_B:
	case NVGPU_GPUID_GP10B:
	case NVGPU_GPUID_GV11B:
		break;
	case NVGPU_GPUID_GA10B:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
#endif
		nvgpu_ga10b_pmu_sw_init(g, pmu);
		break;
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	case NVGPU_GPUID_GB20C:
		nvgpu_gb20c_pmu_sw_init(g, pmu);
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		nvgpu_next_pmu_sw_init(g, pmu);
#endif
		break;
	}
}
