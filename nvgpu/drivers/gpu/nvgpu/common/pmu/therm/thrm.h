/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_THERM_THRM_H
#define NVGPU_THERM_THRM_H

#include "therm_channel.h"
#include "therm_dev.h"

struct nvgpu_pmu_therm {
	struct therm_devices therm_deviceobjs;
	struct therm_channels *therm_channelobjs;
};

#endif /* NVGPU_THERM_THRM_H */
