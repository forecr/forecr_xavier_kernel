/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK3_FREQ_DOMAIN_H
#define NVGPU_CLK3_FREQ_DOMAIN_H

/*
 * Set of FREQ_DOMAINs. Implements BOARDOBJGRP_E32.
 */
struct freq_domain {
	struct pmu_board_obj super;
	u32 clkDomain;
};

/*
 * Set of FREQ_DOMAINs. Implements BOARDOBJGRP_E32.
 */
struct nvgpu_freq_domain_grp {
	struct boardobjgrp_e32 super;
	u32 initFlags;
	u16 mclkFreqMHzBootPstate;
};

s32 clk3_freq_domain_init_pmupstate(struct gk20a *g);
void clk3_freq_domain_free_pmupstate(struct gk20a *g);
s32 clk3_freq_domain_sw_setup(struct gk20a *g);
s32 clk3_freq_domain_pmu_setup(struct gk20a *g);

#endif /* NVGPU_CLK3_FREQ_DOMAIN_H */
