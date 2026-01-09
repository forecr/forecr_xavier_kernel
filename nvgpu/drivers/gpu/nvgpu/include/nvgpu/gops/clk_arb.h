/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_CLK_ARB_H
#define NVGPU_GOPS_CLK_ARB_H

#ifdef CONFIG_NVGPU_CLK_ARB
struct gops_clk_arb {
	int (*clk_arb_init_arbiter)(struct gk20a *g);
	int (*arbiter_clk_init)(struct gk20a *g);
	bool (*check_clk_arb_support)(struct gk20a *g);
	u32 (*get_arbiter_clk_domains)(struct gk20a *g);
	int (*get_arbiter_f_points)(struct gk20a *g, u32 api_domain,
			u32 *num_points, u16 *freqs_in_mhz);
	int (*get_arbiter_clk_range)(struct gk20a *g, u32 api_domain,
			u16 *min_mhz, u16 *max_mhz);
	int (*get_arbiter_clk_default)(struct gk20a *g, u32 api_domain,
			u16 *default_mhz);
	void (*clk_arb_run_arbiter_cb)(struct nvgpu_clk_arb *arb);
	/* This function is inherently unsafe to call while
	 *  arbiter is running arbiter must be blocked
	 *  before calling this function
	 */
	u32 (*get_current_pstate)(struct gk20a *g);
	void (*clk_arb_cleanup)(struct nvgpu_clk_arb *arb);
	void (*stop_clk_arb_threads)(struct gk20a *g);
};
#endif

#endif /* NVGPU_GOPS_CLK_ARB_H */
