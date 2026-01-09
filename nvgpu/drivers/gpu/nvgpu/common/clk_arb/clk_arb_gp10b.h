/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef CLK_ARB_GP10B_H
#define CLK_ARB_GP10B_H

struct nvgpu_clk_session;
struct nvgpu_clk_arb;

bool gp10b_check_clk_arb_support(struct gk20a *g);
u32 gp10b_get_arbiter_clk_domains(struct gk20a *g);
int gp10b_get_arbiter_f_points(struct gk20a *g,u32 api_domain,
				u32 *num_points, u16 *freqs_in_mhz);
int gp10b_get_arbiter_clk_range(struct gk20a *g, u32 api_domain,
		u16 *min_mhz, u16 *max_mhz);
int gp10b_get_arbiter_clk_default(struct gk20a *g, u32 api_domain,
		u16 *default_mhz);
int gp10b_init_clk_arbiter(struct gk20a *g);
void gp10b_clk_arb_run_arbiter_cb(struct nvgpu_clk_arb *arb);
void gp10b_clk_arb_cleanup(struct nvgpu_clk_arb *arb);

#endif /* CLK_ARB_GP106_H */
