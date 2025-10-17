/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_XTL_H
#define NVGPU_GOPS_XTL_H

#include <nvgpu/types.h>

struct gk20a;

/**
 * This structure stores common.xtl unit hal pointers.
 */
struct gops_xtl {
	int (*init_hw)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_XTL_H */

