/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLASS_GA100
#define NVGPU_CLASS_GA100

#include <nvgpu/types.h>

bool ga100_class_is_valid(u32 class_num);
bool ga100_class_is_valid_compute(u32 class_num);

#ifdef CONFIG_NVGPU_GRAPHICS
bool ga100_class_is_valid_gfx(u32 class_num);
#endif

#endif /* NVGPU_CLASS_GA100 */
