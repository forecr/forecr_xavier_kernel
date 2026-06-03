// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/class.h>
#include <nvgpu/barrier.h>

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
#include "class_gp10b.h"
#endif
#include "class_gv11b.h"

bool gv11b_class_is_valid(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case VOLTA_COMPUTE_A:
	case VOLTA_DMA_COPY_A:
	case VOLTA_CHANNEL_GPFIFO_A:
		valid = true;
		break;
#ifdef CONFIG_NVGPU_GRAPHICS
	case VOLTA_A:
		valid = true;
		break;
#endif
	default:
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
		valid = gp10b_class_is_valid(class_num);
#else
		valid = false;
#endif
		break;
	}
	return valid;
}

#ifdef CONFIG_NVGPU_GRAPHICS
bool gv11b_class_is_valid_gfx(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case VOLTA_A:
		valid = true;
		break;
	default:
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
		valid = gp10b_class_is_valid_gfx(class_num);
#else
		valid = false;
#endif
		break;
	}
	return valid;
}
#endif

bool gv11b_class_is_valid_compute(u32 class_num)
{
	if (class_num == VOLTA_COMPUTE_A) {
		return true;
	} else {
		return false;
	}
}
