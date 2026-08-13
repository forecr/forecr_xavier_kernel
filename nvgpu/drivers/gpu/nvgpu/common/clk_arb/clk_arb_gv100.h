/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef CLK_ARB_GV100_H
#define CLK_ARB_GV100_H

struct nvgpu_clk_session;
struct nvgpu_clk_arb;

#define DVCO_MIN_DEFAULT_MHZ	405

bool gv100_check_clk_arb_support(struct gk20a *g);
u32 gv100_get_arbiter_clk_domains(struct gk20a *g);
int gv100_get_arbiter_f_points(struct gk20a *g,u32 api_domain,
				u32 *num_points, u16 *freqs_in_mhz);
int gv100_get_arbiter_clk_range(struct gk20a *g, u32 api_domain,
		u16 *min_mhz, u16 *max_mhz);
int gv100_get_arbiter_clk_default(struct gk20a *g, u32 api_domain,
		u16 *default_mhz);
int gv100_init_clk_arbiter(struct gk20a *g);
void gv100_clk_arb_run_arbiter_cb(struct nvgpu_clk_arb *arb);
void gv100_clk_arb_cleanup(struct nvgpu_clk_arb *arb);
void gv100_stop_clk_arb_threads(struct gk20a *g);
#endif /* CLK_ARB_GV100_H */
