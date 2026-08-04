/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LRC_GB10B_H
#define NVGPU_LRC_GB10B_H

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_DEBUGGER
bool gb10b_lrc_pri_is_lrcc_addr(struct gk20a *g, u32 addr);
bool gb10b_lrc_pri_is_lrcs_addr(struct gk20a *g, u32 addr);
bool gb10b_lrc_pri_is_lrcs_lrccs_addr(struct gk20a *g, u32 addr);
void gb10b_lrc_split_lrc_broadcast_addr(struct gk20a *g, u32 addr,
					u32 *priv_addr_table,
					u32 *priv_addr_table_index);
void gb10b_lrc_split_lrcc_broadcast_addr(struct gk20a *g, u32 addr,
					u32 *priv_addr_table,
					u32 *priv_addr_table_index);
#endif
int gb10b_lrc_init_hw(struct gk20a *g);
#endif /* NVGPU_LRC_GB10B_H */
