// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include "pramin_gv100.h"
#include <nvgpu/hw/gv100/hw_pram_gv100.h>

u32 gv100_pramin_data032_r(u32 i)
{
	return pram_data032_r(i);
}
