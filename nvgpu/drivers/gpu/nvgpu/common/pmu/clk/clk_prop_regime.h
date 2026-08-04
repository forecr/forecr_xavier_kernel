/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_PROP_REGIME_H
#define NVGPU_CLK_PROP_REGIME_H

#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <common/pmu/boardobj/boardobj.h>

struct nvgpu_clk_prop_regimes {
	struct boardobjgrp_e32 super;
	u8 regimeHal;
	/*
	 * Regime ID to board object index map.
	 */
	u16 regimeIdToIdxMap[NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MAX];
};

struct clk_prop_regime {
	struct pmu_board_obj super;
	u8 regimeId;
	/*!
	 * Mask of clock domains that must be programmed
	 * based on their clock propagation relationship.
	 */
	struct boardobjgrpmask_e32 clkDomainMask;
};

s32 clk_prop_regime_init_pmupstate(struct gk20a *g);
void clk_prop_regime_free_pmupstate(struct gk20a *g);
s32 clk_prop_regime_sw_setup(struct gk20a *g);
s32 clk_prop_regime_pmu_setup(struct gk20a *g);

#endif /* NVGPU_CLK_PROP_REGIME_H */
