/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMUIF_PERF_CF_H
#define NVGPU_PMUIF_PERF_CF_H

#include <nvgpu/flcnif_cmn.h>

struct nv_pmu_perf_cf_topology_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	u16 polling_period_ms;
};

struct nv_pmu_perf_cf_topology_boardobj_set {
	struct nv_pmu_boardobj super;
	struct nv_pmu_boardobj_iface_model_10 model10;
	u8 gpumon_tag;
	bool not_available;
};

struct nv_pmu_perf_cf_topology_sensed_base_boardobj_set {
	struct nv_pmu_perf_cf_topology_boardobj_set super;
	u8 sensor_idx;
	u8 base_sensor_idx;
	// Added this to make size of struct 16 bytes in total
	u32 rsvd;
};

struct nv_pmu_perf_cf_topology_min_max_boardobj_set {
	struct nv_pmu_perf_cf_topology_boardobj_set super;
	u8 topology_idx1;
	u8 topology_idx2;
	bool is_max;
};

struct nv_pmu_perf_cf_topology_sensed_boardobj_set {
	struct nv_pmu_perf_cf_topology_boardobj_set super;
	u16 sensor_idx;
};

union nv_pmu_perf_cf_topology_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_perf_cf_topology_boardobj_set super;
	struct nv_pmu_perf_cf_topology_sensed_base_boardobj_set sensed_base;
	struct nv_pmu_perf_cf_topology_min_max_boardobj_set min_max;
	struct nv_pmu_perf_cf_topology_sensed_boardobj_set sensed;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(perf_cf, topology);

/* ---------------- PERF_CF TOPOLOGY GRP_GET_STATUS defines and structures --------- */
struct nv_pmu_perf_cf_topology_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_perf_cf_topology_get_status {
	struct nv_pmu_boardobj_status super;
	u64 reading;
	u64 last_polled_reading;
} __attribute__((packed));

struct nv_pmu_perf_cf_topology_sensed_base_get_status {
	struct nv_pmu_perf_cf_topology_get_status super;
	u64 last_sensor_reading;
	u64 last_base_sensor_reading;
} __attribute__((packed));

struct nv_pmu_perf_cf_topology_min_max_get_status {
	struct nv_pmu_perf_cf_topology_get_status super;
} __attribute__((packed));

struct nv_pmu_perf_cf_topology_sensed_get_status {
	struct nv_pmu_perf_cf_topology_get_status super;
	u64 last_sensor_reading;
} __attribute__((packed));

union nv_pmu_perf_cf_topology_boardobj_get_status_union {
	struct nv_pmu_boardobj_status obj;
	struct nv_pmu_perf_cf_topology_get_status super;
	struct nv_pmu_perf_cf_topology_sensed_base_get_status sensed_base;
	struct nv_pmu_perf_cf_topology_min_max_get_status min_max;
	struct nv_pmu_perf_cf_topology_sensed_get_status sensed;
};

NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_E32(perf_cf, topology);

#endif /* NVGPU_PMUIF_PERF_CF_H */
