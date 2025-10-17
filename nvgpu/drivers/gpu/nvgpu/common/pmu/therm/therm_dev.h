/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_THERM_THRMDEV_H
#define NVGPU_THERM_THRMDEV_H

#include <nvgpu/boardobjgrp_e32.h>
#include <common/pmu/boardobj/boardobj.h>

struct nvgpu_pmu_therm;

struct therm_devices {
	struct boardobjgrp_e32 super;
};

struct therm_device {
	struct pmu_board_obj super;
};

int therm_device_sw_setup(struct gk20a *g);
int therm_device_pmu_setup(struct gk20a *g);
bool therm_device_idx_is_valid(struct nvgpu_pmu_therm *therm_pmu, u8 idx);

#endif /* NVGPU_THERM_THRMDEV_H */
