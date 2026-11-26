/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLASS_GV11B
#define NVGPU_CLASS_GV11B

#include <nvgpu/types.h>

bool gv11b_class_is_valid(u32 class_num);
bool gv11b_class_is_valid_compute(u32 class_num);

#ifdef CONFIG_NVGPU_GRAPHICS
bool gv11b_class_is_valid_gfx(u32 class_num);
#endif

#endif /* NVGPU_CLASS_GV11B */
