// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/gmmu.h>

#include "gmmu_gm20b.h"

u32 gm20b_mm_get_big_page_sizes(void)
{
	return nvgpu_safe_cast_u64_to_u32(SZ_64K | SZ_128K);
}

