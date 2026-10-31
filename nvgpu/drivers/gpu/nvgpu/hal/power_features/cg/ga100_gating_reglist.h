/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GA100_GATING_REGLIST_H
#define GA100_GATING_REGLIST_H

#include <nvgpu/types.h>

struct gating_desc;
struct gk20a;

void ga100_slcg_bus_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_bus_gating_prod_size(void);
const struct gating_desc *ga100_slcg_bus_get_gating_prod(void);

void ga100_slcg_ce2_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_ce2_gating_prod_size(void);
const struct gating_desc *ga100_slcg_ce2_get_gating_prod(void);

void ga100_slcg_chiplet_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_chiplet_gating_prod_size(void);
const struct gating_desc *ga100_slcg_chiplet_get_gating_prod(void);

void ga100_slcg_fb_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_fb_gating_prod_size(void);
const struct gating_desc *ga100_slcg_fb_get_gating_prod(void);

void ga100_slcg_runlist_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_runlist_gating_prod_size(void);
const struct gating_desc *ga100_slcg_runlist_get_gating_prod(void);

void ga100_slcg_gr_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_gr_gating_prod_size(void);
const struct gating_desc *ga100_slcg_gr_get_gating_prod(void);

void ga100_slcg_ltc_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_ltc_gating_prod_size(void);
const struct gating_desc *ga100_slcg_ltc_get_gating_prod(void);

void ga100_slcg_perf_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_perf_gating_prod_size(void);
const struct gating_desc *ga100_slcg_perf_get_gating_prod(void);

void ga100_slcg_priring_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_priring_gating_prod_size(void);
const struct gating_desc *ga100_slcg_priring_get_gating_prod(void);

void ga100_slcg_timer_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_timer_gating_prod_size(void);
const struct gating_desc *ga100_slcg_timer_get_gating_prod(void);

void ga100_slcg_pmu_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_pmu_gating_prod_size(void);
const struct gating_desc *ga100_slcg_pmu_get_gating_prod(void);

void ga100_slcg_therm_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_therm_gating_prod_size(void);
const struct gating_desc *ga100_slcg_therm_get_gating_prod(void);

void ga100_slcg_xbar_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_xbar_gating_prod_size(void);
const struct gating_desc *ga100_slcg_xbar_get_gating_prod(void);

void ga100_slcg_hshub_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_slcg_hshub_gating_prod_size(void);
const struct gating_desc *ga100_slcg_hshub_get_gating_prod(void);

void ga100_blcg_bus_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_blcg_bus_gating_prod_size(void);
const struct gating_desc *ga100_blcg_bus_get_gating_prod(void);

void ga100_blcg_ce_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_blcg_ce_gating_prod_size(void);
const struct gating_desc *ga100_blcg_ce_get_gating_prod(void);

void ga100_blcg_fb_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_blcg_fb_gating_prod_size(void);
const struct gating_desc *ga100_blcg_fb_get_gating_prod(void);

void ga100_blcg_runlist_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_blcg_runlist_gating_prod_size(void);
const struct gating_desc *ga100_blcg_runlist_get_gating_prod(void);

void ga100_blcg_gr_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_blcg_gr_gating_prod_size(void);
const struct gating_desc *ga100_blcg_gr_get_gating_prod(void);

void ga100_blcg_ltc_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_blcg_ltc_gating_prod_size(void);
const struct gating_desc *ga100_blcg_ltc_get_gating_prod(void);

void ga100_blcg_pmu_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_blcg_pmu_gating_prod_size(void);
const struct gating_desc *ga100_blcg_pmu_get_gating_prod(void);

void ga100_blcg_xbar_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_blcg_xbar_gating_prod_size(void);
const struct gating_desc *ga100_blcg_xbar_get_gating_prod(void);

void ga100_blcg_hshub_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_blcg_hshub_gating_prod_size(void);
const struct gating_desc *ga100_blcg_hshub_get_gating_prod(void);

void ga100_elcg_ce_load_gating_prod(struct gk20a *g,
	bool prod);
u32 ga100_elcg_ce_gating_prod_size(void);
const struct gating_desc *ga100_elcg_ce_get_gating_prod(void);

#endif /* GA100_GATING_REGLIST_H */
