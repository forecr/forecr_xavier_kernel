// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/class.h>
#include <nvgpu/barrier.h>

#include "hal/class/class_tu104.h"

#include "class_ga10b.h"

bool ga10b_class_is_valid(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case AMPERE_SMC_PARTITION_REF:
	case AMPERE_COMPUTE_B:
	case AMPERE_DMA_COPY_B:
	case AMPERE_CHANNEL_GPFIFO_B:
#ifdef CONFIG_NVGPU_GRAPHICS
	case AMPERE_B:
#endif
		valid = true;
		break;
	default:
		valid = tu104_class_is_valid(class_num);
		break;
	}
	return valid;
};

#ifdef CONFIG_NVGPU_GRAPHICS
bool ga10b_class_is_valid_gfx(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case AMPERE_B:
		valid = true;
		break;
	default:
		valid = tu104_class_is_valid_gfx(class_num);
		break;
	}
	return valid;
}
#endif

bool ga10b_class_is_valid_compute(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case AMPERE_COMPUTE_B:
		valid = true;
		break;
	default:
		valid = tu104_class_is_valid_compute(class_num);
		break;
	}
	return valid;
}
