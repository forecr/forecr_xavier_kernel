/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMGR_PWRMONITOR_H
#define NVGPU_PMGR_PWRMONITOR_H

#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/pmu/pmuif/ctrlpmgr.h>
#include <common/pmu/boardobj/boardobj.h>

struct pwr_channel {
	struct pmu_board_obj super;
	u8 pwr_rail;
	u32 volt_fixed_uv;
	u32 pwr_corr_slope;
	s32 pwr_corr_offset_mw;
	u32 curr_corr_slope;
	s32 curr_corr_offset_ma;
	u32 dependent_ch_mask;
};

struct pwr_chrelationship {
	struct pmu_board_obj super;
	u8 chIdx;
};

struct pwr_channel_sensor {
	struct pwr_channel super;
	u8 pwr_dev_idx;
	u8 pwr_dev_prov_idx;
};

struct pmgr_pwr_monitor {
	bool b_is_topology_tbl_ver_1x;
	struct boardobjgrp_e32 pwr_channels;
	struct boardobjgrp_e32 pwr_ch_rels;
	u8 total_gpu_channel_idx;
	u32 physical_channel_mask;
	struct nv_pmu_pmgr_pwr_monitor_pack pmu_data;
};

#define PMGR_PWR_MONITOR_GET_PWR_CHANNEL(g, channel_idx)                    \
	((struct pwr_channel *)BOARDOBJGRP_OBJ_GET_BY_IDX(                                 \
		&(g->pmgr_pmu->pmgr_monitorobjs.pwr_channels.super), (channel_idx)))

int pmgr_monitor_sw_setup(struct gk20a *g);

#endif /* NVGPU_PMGR_PWRMONITOR_H */
