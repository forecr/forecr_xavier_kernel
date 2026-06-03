/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_THERM_TU104_H
#define NVGPU_THERM_TU104_H

#include <nvgpu/types.h>

struct gk20a;

void tu104_get_internal_sensor_curr_temp(struct gk20a *g, u32 *temp_f24_8);
void tu104_get_internal_sensor_limits(s32 *max_24_8, s32 *min_24_8);

#endif
