/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_ENUM_H
#define NVGPU_CLK_ENUM_H

#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <common/pmu/boardobj/boardobj.h>

/*!
 * Clock Enumeration entry.  Defines the enumeration of stable frequency points
 * on a clock domain per the VBIOS specification.
 */
struct clk_enum {
	struct pmu_board_obj super;
	bool b_ocov_enabled;
	u16 freq_min_mhz;
	u16 freq_max_mhz;
};

/*!
 * Clock enum group info. Specifies all clock enum groups that we will
 * control per the VBIOS specification.
 */
struct nvgpu_clk_enums {
	struct boardobjgrp_e255 super;
};

s32 clk_enum_init_pmupstate(struct gk20a *g);
void clk_enum_free_pmupstate(struct gk20a *g);
s32 clk_enum_sw_setup(struct gk20a *g);

#endif	/* #ifndef NVGPU_CLK_ENUM_H */
