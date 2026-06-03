/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef FUNC_GA10B_H
#define FUNC_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

u32 ga10b_func_get_full_phys_offset(struct gk20a *g);

#ifdef CONFIG_NVGPU_IOCTL_NON_FUSA
int ga10b_func_read_ptimer(struct gk20a *g, u64 *value);
#endif

#endif /* FUNC_GA10B_H */
