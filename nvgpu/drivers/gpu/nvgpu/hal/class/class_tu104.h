/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLASS_TU104
#define NVGPU_CLASS_TU104

#include <nvgpu/types.h>

bool tu104_class_is_valid(u32 class_num);
bool tu104_class_is_valid_compute(u32 class_num);
bool tu104_class_is_valid_multimedia(u32 class_num);
bool tu104_class_is_valid_nvenc(u32 class_num);

#ifdef CONFIG_NVGPU_GRAPHICS
bool tu104_class_is_valid_gfx(u32 class_num);
#endif

#endif /* NVGPU_CLASS_TU104 */
