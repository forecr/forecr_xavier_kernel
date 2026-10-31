/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_REGOPS_ALLOWLIST_H
#define NVGPU_REGOPS_ALLOWLIST_H

#include <nvgpu/types.h>

struct nvgpu_pm_resource_register_range {
	u32 start;
	u32 end;
};

enum nvgpu_pm_resource_hwpm_register_type {
	NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMON,
	NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER,
	NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER,
	NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX,
	NVGPU_HWPM_REGISTER_TYPE_SMPC,
	NVGPU_HWPM_REGISTER_TYPE_CAU,
	NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_CHANNEL,
	NVGPU_HWPM_REGISTER_TYPE_PC_SAMPLER,
	NVGPU_HWPM_REGISTER_TYPE_HES,
	NVGPU_HWPM_REGISTER_TYPE_HWPM_MM,
	NVGPU_HWPM_REGISTER_TYPE_TEST,
	NVGPU_HWPM_REGISTER_TYPE_COUNT,
};

struct nvgpu_pm_resource_register_range_map {
	u32 start;
	u32 end;
	enum nvgpu_pm_resource_hwpm_register_type type;
};

#endif /* NVGPU_REGOPS_ALLOWLIST_H */
