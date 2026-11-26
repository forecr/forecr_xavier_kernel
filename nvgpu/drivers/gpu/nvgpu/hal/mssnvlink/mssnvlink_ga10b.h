/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MSSNVLINK_GA10B_H
#define NVGPU_MSSNVLINK_GA10B_H

struct gk20a;

u32 ga10b_mssnvlink_get_links(struct gk20a *g, u32 **links);
void ga10b_mssnvlink_init_soc_credits(struct gk20a *g);

#endif /* NVGPU_MSSNVLINK_GA10B_H */
