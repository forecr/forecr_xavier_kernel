/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef CLK_TU104_H
#define CLK_TU104_H

#include <nvgpu/lock.h>
#include <nvgpu/gk20a.h>

#define CLK_NAME_MAX			24
#define CLK_MAX_CNTRL_REGISTERS		2

struct namemap_cfg {
	u32 namemap;
	u32 is_enable;
	u32 is_counter;
	struct gk20a *g;
	struct {
		u32 reg_ctrl_addr;
		u32 reg_ctrl_idx;
		u32 reg_cntr_addr[CLK_MAX_CNTRL_REGISTERS];
	} cntr;
	u32 scale;
	char name[CLK_NAME_MAX];
};

u32 tu104_get_rate_cntr(struct gk20a *g, struct namemap_cfg *c);
int tu104_init_clk_support(struct gk20a *g);
u32 tu104_crystal_clk_hz(struct gk20a *g);
u32 tu104_clk_get_cntr_xbarclk_source(struct gk20a *g);
u32 tu104_clk_get_cntr_sysclk_source(struct gk20a *g);
unsigned long tu104_clk_measure_freq(struct gk20a *g, u32 api_domain);
void tu104_suspend_clk_support(struct gk20a *g);
int tu104_clk_domain_get_f_points(
	struct gk20a *g,
	u32 clkapidomain,
	u32 *pfpointscount,
	u16 *pfreqpointsinmhz);
unsigned long tu104_clk_maxrate(struct gk20a *g, u32 api_domain);
void tu104_get_change_seq_time(struct gk20a *g, s64 *change_time);
void tu104_change_host_clk_source(struct gk20a *g);

#endif /* CLK_TU104_H */
