/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TPC_GV11B_H
#define NVGPU_TPC_GV11B_H

#ifdef CONFIG_NVGPU_STATIC_POWERGATE
struct gk20a;

int gv11b_tpc_pg(struct gk20a *g, bool *can_tpc_pg);
void gv11b_gr_pg_tpc(struct gk20a *g);
#endif

#endif /* NVGPU_TPC_GV11B_H */

