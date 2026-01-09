// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>

#include "hal/func/func_ga10b.h"

#include <nvgpu/hw/ga10b/hw_func_ga10b.h>

u32 ga10b_func_get_full_phys_offset(struct gk20a *g)
{
	(void)g;
	return func_full_phys_offset_v();
}
