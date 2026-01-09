/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PERF_PSTATE_H
#define NVGPU_PERF_PSTATE_H

#define CTRL_PERF_PSTATE_TYPE_35	0x04U

struct pstate_clk_info_list {
	u32 num_info;
	struct nvgpu_pmu_perf_pstate_clk_info clksetinfo[CLK_SET_INFO_MAX_SIZE];
};

struct pstates {
	struct boardobjgrp_e32 super;
	u8 num_clk_domains;
};

struct pstate {
	struct pmu_board_obj super;
	u32 num;
	u8 lpwr_entry_idx;
	u32 flags;
	u8 pcie_idx;
	u8 nvlink_idx;
	struct pstate_clk_info_list clklist;
};

int perf_pstate_sw_setup(struct gk20a *g);
int perf_pstate_pmu_setup(struct gk20a *g);
int perf_pstate_get_table_entry_idx(struct gk20a *g, u32 num);

#endif /* NVGPU_PERF_PSTATE_H */
