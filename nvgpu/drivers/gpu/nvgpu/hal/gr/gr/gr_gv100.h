/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_GV100_H
#define NVGPU_GR_GV100_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_TEGRA_FUSE
void gr_gv100_set_gpc_tpc_mask(struct gk20a *g, u32 gpc_index);
#endif
void gr_gv100_split_fbpa_broadcast_addr(struct gk20a *g, u32 addr,
	u32 num_fbpas,
	u32 *priv_addr_table, u32 *t);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_GV100_H */
