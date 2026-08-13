/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_MSSNVLINK_H
#define NVGPU_GOPS_MSSNVLINK_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * MSSNVLINK unit HAL interface
 *
 */
struct gk20a;

/**
 * MSSNVLINK unit HAL operations
 *
 * @see gpu_ops
 */
struct gops_mssnvlink {
	u32 (*get_links)(struct gk20a *g, u32 **links);
	void (*init_soc_credits)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_MSSNVLINK_H */
