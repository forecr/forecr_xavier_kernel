/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_MM_GM20B_H
#define HAL_MM_MM_GM20B_H

#include <nvgpu/types.h>

struct gk20a;

bool gm20b_mm_is_bar1_supported(struct gk20a *g);
void gm20b_mm_get_default_va_sizes(u64 *aperture_size,
			u64 *user_size, u64 *kernel_size);

#endif
