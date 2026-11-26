// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/class.h>

#include "class_gm20b.h"

bool gm20b_class_is_valid_gfx(u32 class_num)
{
	if (class_num == MAXWELL_B) {
		return true;
	} else {
		return false;
	}
}

bool gm20b_class_is_valid_compute(u32 class_num)
{
	if (class_num == MAXWELL_COMPUTE_B) {
		return true;
	} else {
		return false;
	}
}

bool gm20b_class_is_valid(u32 class_num)
{
	bool valid;

	switch (class_num) {
	case KEPLER_DMA_COPY_A:
	case KEPLER_INLINE_TO_MEMORY_B:
	case MAXWELL_DMA_COPY_A:
	case MAXWELL_CHANNEL_GPFIFO_A:
		valid = true;
		break;

	case MAXWELL_COMPUTE_B:
		valid = true;
		break;
#ifdef CONFIG_NVGPU_GRAPHICS
	case MAXWELL_B:
	case FERMI_TWOD_A:
		valid = true;
		break;
#endif
	default:
		valid = false;
		break;
	}

	return valid;
}
