// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include "pramin_gp10b.h"
#include <nvgpu/hw/gp10b/hw_pram_gp10b.h>

u32 gp10b_pramin_data032_r(u32 i)
{
	return pram_data032_r(i);
}
