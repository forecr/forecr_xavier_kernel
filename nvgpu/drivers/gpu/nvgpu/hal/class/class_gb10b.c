// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvgpu_gb10b_class.h>
#include <nvgpu/barrier.h>

#include "hal/class/class_ga10b.h"
#include "hal/class/class_tu104.h"

#include "class_gb10b.h"

bool gb10b_class_is_valid(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case BLACKWELL_CHANNEL_GPFIFO_A:
	case BLACKWELL_DMA_COPY_A:
#ifdef CONFIG_NVGPU_GRAPHICS
	case BLACKWELL_A:
	case BLACKWELL_B:
#endif
	case BLACKWELL_COMPUTE_A:
	case BLACKWELL_COMPUTE_B:
	case BLACKWELL_INLINE_TO_MEMORY_A:
	case NVCEB7_VIDEO_ENCODER:
	case NVCEFA_VIDEO_OFA:
	case NVCEB0_VIDEO_DECODER:
	case NVCED0_VIDEO_NVJPG:
		valid = true;
		break;
	default:
		valid = ga10b_class_is_valid(class_num);
		break;
	}
	return valid;
};

#ifdef CONFIG_NVGPU_GRAPHICS
bool gb10b_class_is_valid_gfx(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case BLACKWELL_A:
	case BLACKWELL_B:
		valid = true;
		break;
	default:
		valid = ga10b_class_is_valid_gfx(class_num);
		break;
	}
	return valid;
}
#endif

bool gb10b_class_is_valid_compute(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case BLACKWELL_COMPUTE_A:
	case BLACKWELL_COMPUTE_B:
		valid = true;
		break;
	default:
		valid = ga10b_class_is_valid_compute(class_num);
		break;
	}
	return valid;
}

bool gb10b_class_is_valid_nvenc(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case NVCEB7_VIDEO_ENCODER:
		valid = true;
		break;
	default:
		valid = tu104_class_is_valid_nvenc(class_num);
		break;
	}
	return valid;
}

bool gb10b_class_is_valid_ofa(u32 class_num)
{
	nvgpu_speculation_barrier();

	if (class_num == NVCEFA_VIDEO_OFA) {
		return true;
	} else {
		return false;
	}
}

bool gb10b_class_is_valid_nvdec(u32 class_num)
{
	nvgpu_speculation_barrier();

	if (class_num == NVCEB0_VIDEO_DECODER) {
		return true;
	} else {
		return false;
	}
}

bool gb10b_class_is_valid_nvjpg(u32 class_num)
{
	nvgpu_speculation_barrier();

	if (class_num == NVCED0_VIDEO_NVJPG) {
		return true;
	} else {
		return false;
	}
}

bool gb10b_class_is_valid_multimedia(u32 class_num)
{
	return gb10b_class_is_valid_nvenc(class_num) ||
		gb10b_class_is_valid_ofa(class_num) ||
		gb10b_class_is_valid_nvdec(class_num) ||
		gb10b_class_is_valid_nvjpg(class_num);
}
