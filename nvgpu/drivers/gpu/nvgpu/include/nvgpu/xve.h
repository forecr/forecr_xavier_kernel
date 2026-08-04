/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_XVE_H
#define NVGPU_XVE_H

#include <nvgpu/types.h>
#include <nvgpu/log2.h>
#include <nvgpu/bitops.h>

/*
 * For the available speeds bitmap.
 */
#define GPU_XVE_SPEED_2P5	BIT32(0)
#define GPU_XVE_SPEED_5P0	BIT32(1)
#define GPU_XVE_SPEED_8P0	BIT32(2)
#define GPU_XVE_NR_SPEEDS	3U

#define GPU_XVE_SPEED_MASK	(GPU_XVE_SPEED_2P5 |	\
				 GPU_XVE_SPEED_5P0 |	\
				 GPU_XVE_SPEED_8P0)

/*
 * The HW uses a 2 bit field where speed is defined by a number:
 *
 *   NV_XVE_LINK_CONTROL_STATUS_LINK_SPEED_2P5 = 1
 *   NV_XVE_LINK_CONTROL_STATUS_LINK_SPEED_5P0 = 2
 *   NV_XVE_LINK_CONTROL_STATUS_LINK_SPEED_8P0 = 3
 *
 * This isn't ideal for a bitmap with available speeds. So the external
 * APIs think about speeds as a bit in a bitmap and this function converts
 * from those bits to the actual HW speed setting.
 *
 * @speed_bit must have only 1 bit set and must be one of the 3 available
 * HW speeds. Not all chips support all speeds so use available_speeds() to
 * determine what a given chip supports.
 */
static inline const char *xve_speed_to_str(u32 speed)
{
	if ((speed == 0U) || !is_power_of_2(speed) ||
	    (speed & GPU_XVE_SPEED_MASK) == 0U) {
		return "Unknown ???";
	}

	return (speed & GPU_XVE_SPEED_2P5) != 0U ? "Gen1" :
	       (speed & GPU_XVE_SPEED_5P0) != 0U ? "Gen2" :
	       (speed & GPU_XVE_SPEED_8P0) != 0U ? "Gen3" :
	       "Unknown ???";
}

#endif /* NVGPU_XVE_H */
