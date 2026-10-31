/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_FLUSH_FLUSH_GV11B_H
#define HAL_MM_FLUSH_FLUSH_GV11B_H

#include <nvgpu/types.h>

struct gk20a;

int gv11b_mm_l2_flush(struct gk20a *g, bool invalidate);

#endif
