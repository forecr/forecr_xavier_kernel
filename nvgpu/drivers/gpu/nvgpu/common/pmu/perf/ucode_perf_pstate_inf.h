/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMUIF_PERFPSTATE_H_
#define NVGPU_PMUIF_PERFPSTATE_H_

#define PMU_PERF_CLK_DOMAINS_IDX_MAX	(16U)

struct nv_pmu_perf_pstate_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	u8 numClkDomains;
	u8 boot_pstate_idx;
};

struct nv_pmu_perf_pstate {
	struct nv_pmu_boardobj super;
	u8 lpwrEntryIdx;
	u32 flags;
};

struct nv_pmu_perf_pstate_3x {
	struct nv_pmu_perf_pstate super;
};

struct nv_ctrl_perf_pstate_clk_freq_35 {
	u32 freqKz;
	u32 freqVfMaxKhz;
	u32 baseFreqKhz;
	u32 origFreqKhz;
	u32 porFreqKhz;
};

struct ctrl_perf_pstate_clk_entry_35 {
	struct nv_ctrl_perf_pstate_clk_freq_35 min;
	struct nv_ctrl_perf_pstate_clk_freq_35 max;
	struct nv_ctrl_perf_pstate_clk_freq_35 nom;
};

struct ctrl_perf_pstate_clk_entry_30 {
	u32 targetFreqKhz;
	u32 freqRangeMinKhz;
	u32 freqRangeMaxKhz;
};

struct nv_pmu_perf_pstate_30 {
	struct nv_pmu_perf_pstate_3x super;
	struct ctrl_perf_pstate_clk_entry_30
		clkEntries[PMU_PERF_CLK_DOMAINS_IDX_MAX];
};

struct nv_pmu_perf_pstate_35 {
	struct nv_pmu_perf_pstate_3x super;
	u8 pcieIdx;
	u8 nvlinkIdx;
	struct ctrl_perf_pstate_clk_entry_35
	clkEntries[PMU_PERF_CLK_DOMAINS_IDX_MAX];
};

union nv_pmu_perf_pstate_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_perf_pstate super;
	struct nv_pmu_perf_pstate_3x v3x;
	struct nv_pmu_perf_pstate_30 v30;
	struct nv_pmu_perf_pstate_35 v35;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(perf, pstate);

#endif /* NVGPU_PMUIF_PERFPSTATE_H_ */
