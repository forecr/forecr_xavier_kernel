/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_MM_GA10B_H
#define HAL_MM_MM_GA10B_H

struct gk20a;

int ga10b_mm_bar2_vm_size(struct gk20a *g, u32 *buffer_size);

#endif /* HAL_MM_MM_GA10B_H */
