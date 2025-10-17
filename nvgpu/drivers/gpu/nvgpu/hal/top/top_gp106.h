/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef TOP_GP106_H
#define TOP_GP106_H

#include <nvgpu/types.h>

struct gk20a;

u32 gp106_top_read_scratch1_reg(struct gk20a *g);
u32 gp106_top_scratch1_devinit_completed(struct gk20a *g, u32 value);

#endif /*TOP_GP106_H */
