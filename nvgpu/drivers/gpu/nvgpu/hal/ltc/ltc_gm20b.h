/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_GM20B
#define NVGPU_LTC_GM20B

#include <nvgpu/types.h>

struct gk20a;

void gm20b_flush_ltc(struct gk20a *g);
#ifdef CONFIG_NVGPU_FALCON_NON_FUSA
u64 gm20b_determine_L2_size_bytes(struct gk20a *g);
void gm20b_ltc_set_enabled(struct gk20a *g, bool enabled);
void gm20b_ltc_init_fs_state(struct gk20a *g);
#endif
#ifdef CONFIG_NVGPU_GRAPHICS
void gm20b_ltc_set_zbc_color_entry(struct gk20a *g,
					  u32 *color_l2,
					  u32 index);
void gm20b_ltc_set_zbc_depth_entry(struct gk20a *g,
					  u32 depth_val,
					  u32 index);
#endif /* CONFIG_NVGPU_GRAPHICS */
#ifdef CONFIG_NVGPU_DEBUGGER
bool gm20b_ltc_pri_is_ltc_addr(struct gk20a *g, u32 addr);
bool gm20b_ltc_is_pltcg_ltcs_addr(struct gk20a *g, u32 addr);
bool gm20b_ltc_is_ltcs_ltss_addr(struct gk20a *g, u32 addr);
bool gm20b_ltc_is_ltcn_ltss_addr(struct gk20a *g, u32 addr);
void gm20b_ltc_split_lts_broadcast_addr(struct gk20a *g, u32 addr,
					u32 *priv_addr_table,
					u32 *priv_addr_table_index);
void gm20b_ltc_split_ltc_broadcast_addr(struct gk20a *g, u32 addr,
					u32 *priv_addr_table,
					u32 *priv_addr_table_index);
#endif /* CONFIG_NVGPU_DEBUGGER */

#endif
