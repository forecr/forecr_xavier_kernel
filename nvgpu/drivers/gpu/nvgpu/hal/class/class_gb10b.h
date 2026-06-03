/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLASS_GB10B
#define NVGPU_CLASS_GB10B

#include <nvgpu/types.h>

bool gb10b_class_is_valid(u32 class_num);
bool gb10b_class_is_valid_compute(u32 class_num);

#ifdef CONFIG_NVGPU_GRAPHICS
bool gb10b_class_is_valid_gfx(u32 class_num);
#endif

bool gb10b_class_is_valid_nvenc(u32 class_num);
bool gb10b_class_is_valid_ofa(u32 class_num);
bool gb10b_class_is_valid_nvdec(u32 class_num);
bool gb10b_class_is_valid_nvjpg(u32 class_num);
bool gb10b_class_is_valid_multimedia(u32 class_num);

#endif /* NVGPU_CLASS_GB10B */
