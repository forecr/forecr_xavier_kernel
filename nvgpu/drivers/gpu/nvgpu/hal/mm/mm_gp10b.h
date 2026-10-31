/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_MM_GP10B_H
#define HAL_MM_MM_GP10B_H

struct gk20a;

int gp10b_mm_init_bar2_vm(struct gk20a *g);
void gp10b_mm_remove_bar2_vm(struct gk20a *g);
void gp10b_mm_get_default_va_sizes(u64 *aperture_size,
			u64 *user_size, u64 *kernel_size);

#endif
