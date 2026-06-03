/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_PERF_H
#define NVGPU_PMU_PERF_H

#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/boardobjgrp_e255.h>
#include <nvgpu/boardobjgrp_e2048.h>
#include <nvgpu/boardobjgrpmask.h>
#include <nvgpu/pmu/clk/clk.h>

struct nvgpu_clk_slave_freq;

#define CTRL_PERF_PSTATE_P0		0U
#define CTRL_PERF_PSTATE_P5		5U
#define CTRL_PERF_PSTATE_P8		8U
#define CLK_SET_INFO_MAX_SIZE		(32U)

#define NV_PMU_PERF_CMD_ID_RPC                                   (0x00000002U)
#define NV_PMU_PERF_CMD_ID_BOARDOBJ_GRP_SET                      (0x00000003U)
#define NV_PMU_PERF_CMD_ID_BOARDOBJ_GRP_GET_STATUS               (0x00000004U)

/*!
 * RPC calls serviced by PERF unit.
 */
#define NV_PMU_RPC_ID_PERF_BOARD_OBJ_GRP_CMD				0x00U
#define NV_PMU_RPC_ID_PERF_BOARD_OBJ_GRP_IFACE_MODEL_PMU_INIT		0x01U
#define NV_PMU_RPC_ID_PERF_LOAD						0x02U
#define NV_PMU_RPC_ID_PERF_CHANGE_SEQ_INFO_GET				0x03U
#define NV_PMU_RPC_ID_PERF_CHANGE_SEQ_INFO_SET				0x04U
#define NV_PMU_RPC_ID_PERF_CHANGE_SEQ_SET_CONTROL			0x05U
#define NV_PMU_RPC_ID_PERF_CHANGE_SEQ_QUEUE_CHANGE			0x06U
#define NV_PMU_RPC_ID_PERF_CHANGE_SEQ_LOCK				0x07U
#define NV_PMU_RPC_ID_PERF_CHANGE_SEQ_QUERY				0x08U
#define NV_PMU_RPC_ID_PERF_PERF_LIMITS_INVALIDATE			0x09U
#define NV_PMU_RPC_ID_PERF_VFE_EQU_EVAL					0x0AU
#define NV_PMU_RPC_ID_PERF_POST_CLK_DOMAIN_AND_VOLT_RAIL_INIT		0x0BU
#define NV_PMU_RPC_ID_PERF_VFE_INVALIDATE				0x0CU
#define NV_PMU_RPC_ID_PERF_VFE_EQU_MONITOR_SET				0x0DU
#define NV_PMU_RPC_ID_PERF_VFE_EQU_MONITOR_GET				0x0EU
#define NV_PMU_RPC_ID_PERF_PERF_MODES_CONTROL				0x0FU
#define NV_PMU_RPC_ID_PERF_PERF_MODE_CONFIGS_EXECUTE_COMPLETION	0x10U
#define NV_PMU_RPC_ID_PERF_PERF_PUNISH_HW_DVFS_GET_INFO		0x11U
#define NV_PMU_RPC_ID_PERF_PERF_PUNISH_HW_DVFS_SET_CONTROL		0x12U
#define NV_PMU_RPC_ID_PERF__COUNT					0x13U

/* PERF Message-type Definitions */
#define NV_PMU_PERF_MSG_ID_RPC                                   (0x00000003U)
#define NV_PMU_PERF_MSG_ID_BOARDOBJ_GRP_SET                      (0x00000004U)
#define NV_PMU_PERF_MSG_ID_BOARDOBJ_GRP_GET_STATUS               (0x00000006U)

struct nvgpu_pmu_perf_pstate_clk_info {
	u32 clkwhich;
	u32 nominal_mhz;
	u16 min_mhz;
	u16 max_mhz;
};

struct perf_chage_seq_input_clk {
	u32 clk_freq_khz;
};

struct nvgpu_pmu_perf_change_input_clk_info {
	struct ctrl_boardobjgrp_mask_e32 clk_domains_mask;
	struct perf_chage_seq_input_clk
		clk[CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS];
};

int nvgpu_pmu_perf_init(struct gk20a *g);
void nvgpu_pmu_perf_deinit(struct gk20a *g);
int nvgpu_pmu_perf_sw_setup(struct gk20a *g);
int nvgpu_pmu_perf_pmu_setup(struct gk20a *g);

int nvgpu_pmu_perf_load(struct gk20a *g);

int nvgpu_pmu_perf_post_clk_domain_and_volt_rail_init(struct gk20a *g);

int nvgpu_pmu_perf_vfe_invalidate(struct gk20a *g);

int nvgpu_pmu_perf_vfe_get_s_param(struct gk20a *g, u64 *s_param);

int nvgpu_pmu_perf_vfe_get_volt_margin(struct gk20a *g, u32 *vmargin_uv);
int nvgpu_pmu_perf_vfe_get_freq_margin(struct gk20a *g, u32 *fmargin_mhz);

int nvgpu_pmu_perf_changeseq_set_voltage_only(struct gk20a *g,
	u8 rail_idx,
	u32 voltage_uv);

int nvgpu_pmu_perf_changeseq_set_clks(struct gk20a *g,
	struct nvgpu_clk_slave_freq *vf_point);

int nvgpu_pmu_perf_changeseq_set_clks_50(struct gk20a *g,
	struct nvgpu_clk_vf_point_5x *vf_point);

struct nvgpu_pmu_perf_pstate_clk_info *nvgpu_pmu_perf_pstate_get_clk_set_info(
		struct gk20a *g, u32 pstate_num, u32 clkwhich);

void nvgpu_perf_change_seq_execute_time(struct gk20a *g, s64 *change_time);

#endif /* NVGPU_PMU_PERF_H */
