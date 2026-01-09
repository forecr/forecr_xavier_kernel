/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLASS_GB20C
#define NVGPU_CLASS_GB20C

#include <nvgpu/types.h>

bool gb20c_class_is_valid(u32 class_num);
bool gb20c_class_is_valid_nvenc(u32 class_num);
bool gb20c_class_is_valid_ofa(u32 class_num);
bool gb20c_class_is_valid_nvdec(u32 class_num);
bool gb20c_class_is_valid_multimedia(u32 class_num);

#endif /* NVGPU_CLASS_GB20C */
