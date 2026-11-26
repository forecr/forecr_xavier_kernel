/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_MM_TU104_H
#define HAL_MM_MM_TU104_H

struct gk20a;
enum nvgpu_flush_op;

u32 tu104_mm_get_flush_retries(struct gk20a *g, enum nvgpu_flush_op op);

#endif
