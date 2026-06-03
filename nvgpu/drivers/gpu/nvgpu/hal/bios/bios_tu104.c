// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "nvgpu/hw/tu104/hw_gc6_tu104.h"

#include "bios_tu104.h"

u32 tu104_get_aon_secure_scratch_reg(struct gk20a *g, u32 i)
{
	return gc6_aon_secure_scratch_group_05_r(i);
}

