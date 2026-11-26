// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvgpu_gb20c_class.h>
#include <nvgpu/barrier.h>

#include "hal/class/class_gb10b.h"

#include "class_gb20c.h"

bool gb20c_class_is_valid(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case BLACKWELL_DMA_COPY_B:
	case NVD1B7_VIDEO_ENCODER:
	case NVD1FA_VIDEO_OFA:
	case NVD1B0_VIDEO_DECODER:
		valid = true;
		break;
	default:
		valid = gb10b_class_is_valid(class_num);
		break;
	}
	return valid;
};

bool gb20c_class_is_valid_nvenc(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case NVD1B7_VIDEO_ENCODER:
		valid = true;
		break;
	default:
		valid = gb10b_class_is_valid_nvenc(class_num);
		break;
	}
	return valid;
}

bool gb20c_class_is_valid_ofa(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case NVD1FA_VIDEO_OFA:
		valid = true;
		break;
	default:
		valid = gb10b_class_is_valid_ofa(class_num);
		break;
	}
	return valid;
}

bool gb20c_class_is_valid_nvdec(u32 class_num)
{
	bool valid;

	nvgpu_speculation_barrier();

	switch (class_num) {
	case NVD1B0_VIDEO_DECODER:
		valid = true;
		break;
	default:
		valid = gb10b_class_is_valid_nvdec(class_num);
		break;
	}
	return valid;
}

bool gb20c_class_is_valid_multimedia(u32 class_num)
{
	return gb20c_class_is_valid_nvenc(class_num) ||
		gb20c_class_is_valid_ofa(class_num) ||
		gb20c_class_is_valid_nvdec(class_num);
}
