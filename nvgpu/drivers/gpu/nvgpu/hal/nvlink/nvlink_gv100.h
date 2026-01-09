/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVLINK_GV100_H
#define NVGPU_NVLINK_GV100_H

#define GV100_CONNECTED_LINK_MASK		0x8

struct gk20a;

u32 gv100_nvlink_get_link_reset_mask(struct gk20a *g);
int gv100_nvlink_discover_link(struct gk20a *g);
int gv100_nvlink_init(struct gk20a *g);
void gv100_nvlink_get_connected_link_mask(u32 *link_mask);
void gv100_nvlink_set_sw_errata(struct gk20a *g, u32 link_id);
int gv100_nvlink_configure_ac_coupling(struct gk20a *g,
				unsigned long mask, bool sync);
void gv100_nvlink_prog_alt_clk(struct gk20a *g);
void gv100_nvlink_clear_link_reset(struct gk20a *g, u32 link_id);
void gv100_nvlink_enable_link_an0(struct gk20a *g, u32 link_id);
#endif
