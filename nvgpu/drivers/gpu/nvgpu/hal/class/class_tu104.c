// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/class.h>
#include <nvgpu/barrier.h>

#include "class_gv11b.h"
#include "class_tu104.h"

bool tu104_class_is_valid(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case TURING_CHANNEL_GPFIFO_A:
	case TURING_COMPUTE_A:
	case TURING_DMA_COPY_A:
	case TURING_NVENC_A:
#ifdef CONFIG_NVGPU_GRAPHICS
	case TURING_A:
#endif
		valid = true;
		break;
	default:
		valid = gv11b_class_is_valid(class_num);
		break;
	}
	return valid;
};

#ifdef CONFIG_NVGPU_GRAPHICS
bool tu104_class_is_valid_gfx(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case TURING_A:
		valid = true;
		break;
	default:
		valid = gv11b_class_is_valid_gfx(class_num);
		break;
	}
	return valid;
}
#endif

bool tu104_class_is_valid_compute(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case TURING_COMPUTE_A:
		valid = true;
		break;
	default:
		valid = gv11b_class_is_valid_compute(class_num);
		break;
	}
	return valid;
}

bool tu104_class_is_valid_nvenc(u32 class_num)
{
	nvgpu_speculation_barrier();

	if (class_num == TURING_NVENC_A) {
		return true;
	} else {
		return false;
	}
}

bool tu104_class_is_valid_multimedia(u32 class_num)
{
	return tu104_class_is_valid_nvenc(class_num);
}
