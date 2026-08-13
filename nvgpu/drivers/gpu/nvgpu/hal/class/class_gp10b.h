/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLASS_GP10B
#define NVGPU_CLASS_GP10B

#include <nvgpu/types.h>

bool gp10b_class_is_valid(u32 class_num);
bool gp10b_class_is_valid_compute(u32 class_num);

#ifdef CONFIG_NVGPU_GRAPHICS
bool gp10b_class_is_valid_gfx(u32 class_num);
#endif

#endif /* NVGPU_CLASS_GP10B */
