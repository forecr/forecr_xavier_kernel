// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include "pramin_tu104.h"
#include <nvgpu/hw/tu104/hw_pram_tu104.h>

u32 tu104_pramin_data032_r(u32 i)
{
	return pram_data032_r(i);
}
