/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLASS_GA10B
#define NVGPU_CLASS_GA10B

#include <nvgpu/types.h>

bool ga10b_class_is_valid(u32 class_num);
bool ga10b_class_is_valid_compute(u32 class_num);

#ifdef CONFIG_NVGPU_GRAPHICS
bool ga10b_class_is_valid_gfx(u32 class_num);
#endif

#endif /* NVGPU_CLASS_GA10B */
