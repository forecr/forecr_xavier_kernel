// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/pmu/pmu_perfmon.h>
#include <nvgpu/log.h>

#include "pmu_perfmon_sw_ga10b.h"

void nvgpu_ga10b_perfmon_sw_init(struct gk20a *g,
		struct nvgpu_pmu_perfmon *perfmon)
{
	nvgpu_log_fn(g, " ");

	perfmon->init_perfmon = nvgpu_pmu_init_perfmon_rpc;
	perfmon->start_sampling = nvgpu_pmu_perfmon_start_sampling_rpc;
	perfmon->stop_sampling = nvgpu_pmu_perfmon_stop_sampling_rpc;
	perfmon->get_samples_rpc = nvgpu_pmu_perfmon_get_samples_rpc;
	perfmon->perfmon_event_handler = nvgpu_pmu_handle_perfmon_event_rpc;
}
