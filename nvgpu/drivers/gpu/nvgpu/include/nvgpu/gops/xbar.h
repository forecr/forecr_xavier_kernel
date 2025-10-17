/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_XBAR_H
#define NVGPU_GOPS_XBAR_H

#include <nvgpu/types.h>

struct gk20a;

/**
 * This structure stores common.xbar unit hal pointers.
 */
struct gops_xbar {
	void (*handle_xbar_read_ecc_err)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_XBAR_H */
