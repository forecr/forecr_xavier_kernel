/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PMU_PERF_H_
#define PMU_PERF_H_

#include "vfe_equ.h"
#include "vfe_var.h"
#include "change_seq.h"
#include "pstate.h"

/* PERF RPC ID Definitions */
#define NV_PMU_RPC_ID_PERF_DOMAIN_GROUP_LIMIT                               0x00
#define NV_PMU_RPC_ID_PERF_VFE_CALLBACK                                     0x01
#define NV_PMU_RPC_ID_PERF_VFE_CALLBACK_WITH_EQU_MONITOR_PAYLOAD            0x02
#define NV_PMU_RPC_ID_PERF_SEQ_COMPLETION                                   0x03
#define NV_PMU_RPC_ID_PERF_PSTATES_INVALIDATE                               0x04
#define NV_PMU_RPC_ID_PERF_PSTATES_INVALIDATE_WITH_PSTATE_DATA_PAYLOAD      0x05
#define NV_PMU_RPC_ID_PERF_PERF_LIMITS_GPU_BOOST_SYNC_CHANGE                0x06
#define NV_PMU_RPC_ID_PERF_SCRIPT_EXECUTE                                   0x07
#define NV_PMU_RPC_ID_PERF_PERF_MODE_COMPLETION                             0x08
#define NV_PMU_RPC_ID_PERF_PERF_MODE_CONFIGS_EXECUTE                        0x09

/*
 * Defines the structure that holds data
 * used to execute LOAD RPC.
 */
struct nv_pmu_rpc_struct_perf_load {
	/* [IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;
	bool b_load;
	u32 scratch[1];
};

struct nv_pmu_rpc_struct_perf_post_clk_domain_and_volt_rail_init {
	/* [IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;
	u32 scratch[1];
};

struct nv_pmu_rpc_struct_perf_vfe_invalidate {
	/* [IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;
	/* [NONE] Must be last field in RPC structure */
	u32 scratch[1];
};

/*
 * Simply a union of all specific PERF messages. Forms the general packet
 * exchanged between the Kernel and PMU when sending and receiving PERF messages
 * (respectively).
 */

struct pmu_nvgpu_rpc_perf_event {
	struct pmu_hdr msg_hdr;
	struct pmu_nvgpu_rpc_header rpc_hdr;
};

struct perf_vfe_invalidate {
	bool state_change;
	struct nvgpu_cond wq;
	struct nvgpu_thread state_task;
};

struct nvgpu_pmu_perf {
	struct vfe_vars vfe_varobjs;
	struct vfe_equs vfe_equobjs;
	struct ctrl_perf_vfe_param *vfe_paramsobjs;
	u16 param_count;
	struct pstates pstatesobjs;
	struct perf_vfe_invalidate vfe_init;
	struct change_seq_pmu changeseq_pmu;
	u8 polling_period_ms;
};

#endif /* PMU_PERF_H_ */
