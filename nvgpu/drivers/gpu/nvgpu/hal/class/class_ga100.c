// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/class.h>
#include <nvgpu/barrier.h>

#include "hal/class/class_ga10b.h"

#include "class_ga100.h"

bool ga100_class_is_valid(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case AMPERE_COMPUTE_A:
	case AMPERE_DMA_COPY_A:
#ifdef CONFIG_NVGPU_GRAPHICS
	case AMPERE_A:
#endif
		valid = true;
		break;
	default:
		valid = ga10b_class_is_valid(class_num);
		break;
	}
	return valid;
};

#ifdef CONFIG_NVGPU_GRAPHICS
bool ga100_class_is_valid_gfx(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case AMPERE_A:
		valid = true;
		break;
	default:
		valid = false;
		break;
	}
	return valid;
}
#endif

bool ga100_class_is_valid_compute(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case AMPERE_COMPUTE_A:
		valid = true;
		break;
	default:
		valid = ga10b_class_is_valid_compute(class_num);
		break;
	}
	return valid;
}
