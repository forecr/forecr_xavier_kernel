/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_PROP_TOP_REL_H
#define NVGPU_CLK_PROP_TOP_REL_H

#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <common/pmu/boardobj/boardobj.h>

#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TABLE_REL_TUPLE_MAX 32
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_CPMU_REL_TUPLE_MAX  32

struct ctrl_clk_prop_top_rel_table_rel_tuple {
	u16 freqMHzSrc;
	u16 freqMHzDst;
};

struct ctrl_clk_prop_top_rel_cpmu_rel_tuple {
	u16 clkDomainIdxSrc;
	u16 clkDomainIdxDst;
};

struct clk_prop_top_rel {
	struct pmu_board_obj super;
	u8 clkDomainIdxSrc;
	u8 clkDomainIdxDst;
	bool bBiDirectional;
};

struct nvgpu_clk_prop_top_rels {
	struct boardobjgrp_e255 super;
	u8 tableRelTupleCount;
	struct ctrl_clk_prop_top_rel_table_rel_tuple
		tableRelTuple[NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TABLE_REL_TUPLE_MAX];
	u8 cpmuRelTupleCount;
	struct ctrl_clk_prop_top_rel_cpmu_rel_tuple
		cpmuRelTuple[NV2080_CTRL_CLK_CLK_PROP_TOP_REL_CPMU_REL_TUPLE_MAX];
};

struct clk_prop_top_rel_model_10 {
	struct clk_prop_top_rel super;
	// Todo: Do we need full model 10 struct: BOARDOBJ_IFACE_MODEL_10
	// BOARDOBJ_IFACE_MODEL_10 model10;
	s16 *offset;	// Todo: TBD update to model 10 IFACE with interface_virtual_table struct?
};

struct clk_prop_top_rel_1x {
	struct clk_prop_top_rel_model_10 super;
};

struct clk_prop_top_rel_1x_ratio {
	struct clk_prop_top_rel_1x super;
	u32 ratio;        // fixed point 16.16 format.
	u32 ratioInverse; // fixed point 16.16 format.
};

struct clk_prop_top_rel_1x_volt {
	struct clk_prop_top_rel_1x super;
	u8 voltRailIdx;
};

s32 clk_prop_top_rel_init_pmupstate(struct gk20a *g);
void clk_prop_top_rel_free_pmupstate(struct gk20a *g);
s32 clk_prop_top_rel_sw_setup(struct gk20a *g);
s32 clk_prop_top_rel_pmu_setup(struct gk20a *g);

#endif /* NVGPU_CLK_PROP_TOP_REL_H */
