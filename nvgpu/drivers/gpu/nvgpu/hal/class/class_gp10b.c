// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/class.h>
#include <nvgpu/barrier.h>

#include "class_gm20b.h"
#include "class_gp10b.h"

bool gp10b_class_is_valid(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();
	switch (class_num) {
	case PASCAL_DMA_COPY_A:
	case PASCAL_CHANNEL_GPFIFO_A:
		valid = true;
		break;
#ifdef CONFIG_NVGPU_GRAPHICS
	case PASCAL_A:
		valid = true;
		break;
#endif
	case PASCAL_COMPUTE_A:
		valid = true;
		break;
	default:
		valid = gm20b_class_is_valid(class_num);
		break;
	}
	return valid;
}

#ifdef CONFIG_NVGPU_GRAPHICS
bool gp10b_class_is_valid_gfx(u32 class_num)
{
	if (class_num == PASCAL_A ||  class_num == MAXWELL_B) {
		return true;
	} else {
		return false;
	}
}
#endif

bool gp10b_class_is_valid_compute(u32 class_num)
{
	if (class_num == PASCAL_COMPUTE_A ||  class_num == MAXWELL_COMPUTE_B) {
		return true;
	} else {
		return false;
	}
}
