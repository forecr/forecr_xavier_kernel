// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>

#include "func_tu104.h"

#include <nvgpu/hw/tu104/hw_func_tu104.h>

u32 tu104_func_get_full_phys_offset(struct gk20a *g)
{
	return func_full_phys_offset_v();
}
