/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GA10B_GATING_REGLIST_H
#define GA10B_GATING_REGLIST_H

#include <nvgpu/types.h>

struct gating_desc;
struct gk20a;

void ga10b_slcg_bus_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_bus_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_bus_get_gating_prod(void);

void ga10b_slcg_ce2_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_ce2_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_ce2_get_gating_prod(void);

void ga10b_slcg_chiplet_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_chiplet_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_chiplet_get_gating_prod(void);

void ga10b_slcg_fb_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_fb_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_fb_get_gating_prod(void);

void ga10b_slcg_runlist_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_runlist_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_runlist_get_gating_prod(void);

void ga10b_slcg_gr_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_gr_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_gr_get_gating_prod(void);

void ga10b_slcg_ltc_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_ltc_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_ltc_get_gating_prod(void);

void ga10b_slcg_perf_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_perf_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_perf_get_gating_prod(void);

void ga10b_slcg_priring_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_priring_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_priring_get_gating_prod(void);

void ga10b_slcg_rs_ctrl_fbp_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_rs_ctrl_fbp_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_rs_ctrl_fbp_get_gating_prod(void);

void ga10b_slcg_rs_ctrl_gpc_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_rs_ctrl_gpc_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_rs_ctrl_gpc_get_gating_prod(void);

void ga10b_slcg_rs_ctrl_sys_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_rs_ctrl_sys_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_rs_ctrl_sys_get_gating_prod(void);

void ga10b_slcg_rs_fbp_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_rs_fbp_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_rs_fbp_get_gating_prod(void);

void ga10b_slcg_rs_gpc_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_rs_gpc_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_rs_gpc_get_gating_prod(void);

void ga10b_slcg_rs_sys_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_rs_sys_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_rs_sys_get_gating_prod(void);

void ga10b_slcg_timer_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_timer_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_timer_get_gating_prod(void);

void ga10b_slcg_pmu_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_pmu_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_pmu_get_gating_prod(void);

void ga10b_slcg_therm_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_therm_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_therm_get_gating_prod(void);

void ga10b_slcg_xbar_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_xbar_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_xbar_get_gating_prod(void);

void ga10b_slcg_hshub_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_hshub_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_hshub_get_gating_prod(void);

void ga10b_slcg_ctrl_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_ctrl_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_ctrl_get_gating_prod(void);

void ga10b_slcg_gsp_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_slcg_gsp_gating_prod_size(void);
const struct gating_desc *ga10b_slcg_gsp_get_gating_prod(void);

void ga10b_blcg_bus_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_blcg_bus_gating_prod_size(void);
const struct gating_desc *ga10b_blcg_bus_get_gating_prod(void);

void ga10b_blcg_ce_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_blcg_ce_gating_prod_size(void);
const struct gating_desc *ga10b_blcg_ce_get_gating_prod(void);

void ga10b_blcg_fb_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_blcg_fb_gating_prod_size(void);
const struct gating_desc *ga10b_blcg_fb_get_gating_prod(void);

void ga10b_blcg_runlist_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_blcg_runlist_gating_prod_size(void);
const struct gating_desc *ga10b_blcg_runlist_get_gating_prod(void);

void ga10b_blcg_gr_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_blcg_gr_gating_prod_size(void);
const struct gating_desc *ga10b_blcg_gr_get_gating_prod(void);

void ga10b_blcg_ltc_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_blcg_ltc_gating_prod_size(void);
const struct gating_desc *ga10b_blcg_ltc_get_gating_prod(void);

void ga10b_blcg_pmu_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_blcg_pmu_gating_prod_size(void);
const struct gating_desc *ga10b_blcg_pmu_get_gating_prod(void);

void ga10b_blcg_xbar_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_blcg_xbar_gating_prod_size(void);
const struct gating_desc *ga10b_blcg_xbar_get_gating_prod(void);

void ga10b_blcg_hshub_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_blcg_hshub_gating_prod_size(void);
const struct gating_desc *ga10b_blcg_hshub_get_gating_prod(void);

void ga10b_elcg_ce_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga10b_elcg_ce_gating_prod_size(void);
const struct gating_desc *ga10b_elcg_ce_get_gating_prod(void);

#endif /* GA10B_GATING_REGLIST_H */
