/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_FUNC_H
#define NVGPU_GOPS_FUNC_H

struct gops_func {
	u32 (*get_full_phys_offset)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_FUNC_H */
