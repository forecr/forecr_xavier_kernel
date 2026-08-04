// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pmu/therm.h>

#include "therm_tu104.h"

#include <nvgpu/hw/tu104/hw_therm_tu104.h>

#include <nvgpu/utils.h>

void tu104_get_internal_sensor_limits(s32 *max_24_8, s32 *min_24_8)
{
	*max_24_8 = (0x87 << 8);
	*min_24_8 = (s32)(((u32)-216) << 8);
}

void tu104_get_internal_sensor_curr_temp(struct gk20a *g, u32 *temp_f24_8)
{
	u32 read_val;

	read_val = nvgpu_readl(g, therm_i2cs_sensor_00_r());

	/* Convert from celsius to f24_8 format*/
	*temp_f24_8 = (read_val << 8);
}
