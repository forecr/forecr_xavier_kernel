// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.


#include <nvgpu/pmu/pmu_perfmon.h>
#include <nvgpu/log.h>
#include "pmu_perfmon_sw_gm20b.h"

void nvgpu_gm20b_perfmon_sw_init(struct gk20a *g,
		struct nvgpu_pmu_perfmon *perfmon)
{
	nvgpu_log_fn(g, " ");

	perfmon->init_perfmon = nvgpu_pmu_init_perfmon;
	perfmon->start_sampling =
		nvgpu_pmu_perfmon_start_sampling;
	perfmon->stop_sampling =
		nvgpu_pmu_perfmon_stop_sampling;
	perfmon->get_samples_rpc = NULL;
	perfmon->perfmon_event_handler =
		nvgpu_pmu_handle_perfmon_event;
}

