/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PRIV_RING_GV11B_H
#define NVGPU_PRIV_RING_GV11B_H

#ifdef CONFIG_NVGPU_PROFILER
struct gk20a;
void gv11b_priv_ring_read_pri_fence(struct gk20a *g);
#endif

#endif /* NVGPU_PRIV_RING_GV11B_H */
