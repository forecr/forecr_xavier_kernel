/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_PERF_CF_H
#define NVGPU_PMU_PERF_CF_H

#include <nvgpu/types.h>
#include <nvgpu/pmu/pmu_perfmon.h>
#include <nvgpu/boardobjgrp_e32.h>

struct gk20a;

#define NV_PMU_RPC_ID_PERF_CF_LOAD                                              0x2U

#define PERF_CF_TOPOLOGY_SET_RPC_ISSUED                                         0x00U
#define PERF_CF_TOPOLOGY_SET_RPC_COMPLETED                                      0x01U
#define PERF_CF_LOAD_RPC_ISSUED                                                 0x02U
#define PERF_CF_LOAD_RPC_COMPLETED                                              0x03U
#define PERF_CF_FIRST_GET_LOAD_COMPLETED                                        0x04U

/*
 * Defines the structure that holds data
 * used to execute LOAD RPC.
 */
struct nv_pmu_rpc_struct_perf_cf_load {
	/*[IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;
	u32  scratch[1];
};

struct perf_cf_topology_metadata {
	struct boardobjgrp_e32 topologies;
	struct boardobjgrp_iface_model_10 model10;
	u16 polling_period_ms;
	u8 hal_val;
	u8 perf_cf_top_state;
};

struct nvgpu_pmu_perf_cf_metadata {
	struct perf_cf_topology_metadata perf_cf_top_metadata;
};

struct nvgpu_pmu_perf_cf {
	struct nvgpu_pmu_perf_cf_metadata *perf_cf_metadata;
};

int nvgpu_pmu_perf_cf_sw_setup(struct gk20a *g);
int nvgpu_pmu_perf_cf_pmu_setup(struct gk20a *g);
void nvgpu_pmu_perf_cf_deinit(struct gk20a *g);
int nvgpu_pmu_perf_cf_init(struct gk20a *g);

int perf_cf_topology_get_load(struct gk20a *g, u32 *utilization,
	enum nvgpu_pmu_perfmon_class perfmon_class);

#endif /* NVGPU_PMU_PERF_CF_H */
