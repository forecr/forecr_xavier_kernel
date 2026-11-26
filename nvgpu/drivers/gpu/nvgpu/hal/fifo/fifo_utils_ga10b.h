/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef FIFO_UTILS_GA10B_H
#define FIFO_UTILS_GA10B_H

struct gk20a;
struct nvgpu_runlist;

u32 nvgpu_runlist_readl(struct gk20a *g, struct nvgpu_runlist *runlist,
			u32 r);
void nvgpu_runlist_writel(struct gk20a *g, struct nvgpu_runlist *runlist,
			u32 r, u32 v);
u32 nvgpu_chram_bar0_readl(struct gk20a *g, struct nvgpu_runlist *runlist,
			u32 r);
void nvgpu_chram_bar0_writel(struct gk20a *g,
			struct nvgpu_runlist *runlist, u32 r, u32 v);

#endif /* FIFO_UTILS_GA10B_H */
