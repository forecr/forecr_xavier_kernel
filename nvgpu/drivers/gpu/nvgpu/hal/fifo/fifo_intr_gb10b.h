/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_INTR_GB10B_H
#define NVGPU_FIFO_INTR_GB10B_H

#include <nvgpu/types.h>

struct gk20a;

u32 gb10b_runlist_intr_ctrl(u32 intr_tree);
void gb10b_fifo_intr_top_enable(struct gk20a *g, bool enable);
void gb10b_fifo_runlist_intr_vectorid_init(struct gk20a *g);
void gb10b_fifo_intr_0_enable(struct gk20a *g, bool enable);

#endif /* NVGPU_FIFO_INTR_GB10B_H */
