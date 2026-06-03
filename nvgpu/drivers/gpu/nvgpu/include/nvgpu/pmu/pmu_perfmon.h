/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_PERFMON_H
#define NVGPU_PMU_PERFMON_H

#include <nvgpu/lock.h>
//#include <nvgpu/enabled.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/pmu/pmuif/perfmon.h>

struct gk20a;
struct nvgpu_pmu;
struct rpc_handler_payload;
struct nv_pmu_rpc_header;
struct pmu_msg;

/* pmu load const defines */
#define PMU_BUSY_CYCLES_NORM_MAX			(1000U)

#define PMU_PERFMON_SAMPLE_PERIOD_US_SIM	(4175U)

#define PMU_PERFMON_SAMPLE_PERIOD_US		(16700U)

#define PMU_PERFMON_MOVING_AVG_SAMPLE_CNT	(17U)

enum nvgpu_pmu_perfmon_class {
	NVGPU_PMU_PERFMON_CLASS_GR     =  0,
	NVGPU_PMU_PERFMON_CLASS_NVD    =  1,
	NVGPU_PMU_PERFMON_CLASS_NVENC0 =  2,
	NVGPU_PMU_PERFMON_CLASS_NVENC1 =  3,
	NVGPU_PMU_PERFMON_CLASS_NVDEC0 =  4,
	NVGPU_PMU_PERFMON_CLASS_NVDEC1 =  5,
	NVGPU_PMU_PERFMON_CLASS_OFA    =  6,
	NVGPU_PMU_PERFMON_CLASS_NVJPG0 =  7,
	NVGPU_PMU_PERFMON_CLASS_NUM
};

struct nvgpu_pmu_perfmon {
	struct pmu_perfmon_counter_v2 perfmon_counter_v2;
	u64 perfmon_events_cnt;
	u32 perfmon_query;
	struct nvgpu_mutex query_mutex;
	u8 perfmon_state_id[PMU_DOMAIN_GROUP_NUM];
	u32 sample_buffer;
	u32 load_shadow[NVGPU_PMU_PERFMON_CLASS_NUM];
	u32 load_avg[NVGPU_PMU_PERFMON_CLASS_NUM];
	u32 load[NVGPU_PMU_PERFMON_CLASS_NUM];
	u32 min_load_refresh_interv_us;
	s64 last_load_refresh_time_us;
	bool query_mutex_initialized;
	bool perfmon_ready;
	bool perfmon_sampling_enabled;
	int (*init_perfmon)(struct nvgpu_pmu *pmu);
	int (*start_sampling)(struct nvgpu_pmu *pmu);
	int (*stop_sampling)(struct nvgpu_pmu *pmu);
	int (*get_samples_rpc)(struct nvgpu_pmu *pmu);
	int (*perfmon_event_handler)(struct gk20a *g,
		struct nvgpu_pmu *pmu, void *msg);
};

/* perfmon */
void nvgpu_pmu_perfmon_rpc_handler(struct gk20a *g, struct nvgpu_pmu *pmu,
		struct nv_pmu_rpc_header *rpc,
		struct rpc_handler_payload *rpc_payload);
int nvgpu_pmu_initialize_perfmon(struct gk20a *g, struct nvgpu_pmu *pmu,
		struct nvgpu_pmu_perfmon **perfmon_ptr);
void nvgpu_pmu_deinitialize_perfmon(struct gk20a *g, struct nvgpu_pmu *pmu);
int nvgpu_pmu_init_perfmon(struct nvgpu_pmu *pmu);
int nvgpu_pmu_perfmon_start_sampling(struct nvgpu_pmu *pmu);
int nvgpu_pmu_perfmon_stop_sampling(struct nvgpu_pmu *pmu);
int nvgpu_pmu_perfmon_start_sampling_rpc(struct nvgpu_pmu *pmu);
int nvgpu_pmu_perfmon_stop_sampling_rpc(struct nvgpu_pmu *pmu);
int nvgpu_pmu_perfmon_get_samples_rpc(struct nvgpu_pmu *pmu);
int nvgpu_pmu_handle_perfmon_event(struct gk20a *g,
		struct nvgpu_pmu *pmu, void *pmsg);
int nvgpu_pmu_handle_perfmon_event_rpc(struct gk20a *g,
		struct nvgpu_pmu *pmu, void *msg);
int nvgpu_pmu_init_perfmon_rpc(struct nvgpu_pmu *pmu);
int nvgpu_pmu_sysfs_load_refresh(struct gk20a *g, u32 *query,
		enum nvgpu_pmu_perfmon_class perfmon_class);
int nvgpu_pmu_load_query(struct gk20a *g, u32 *query,
		enum nvgpu_pmu_perfmon_class perfmon_class);
void nvgpu_pmu_load_read(struct gk20a *g, u32 *value,
		enum nvgpu_pmu_perfmon_class perfmon_class);
void nvgpu_pmu_reset_load_counters(struct gk20a *g);
void nvgpu_pmu_get_load_counters(struct gk20a *g, u32 *busy_cycles,
		u32 *total_cycles);
int nvgpu_pmu_perfmon_get_sampling_enable_status(struct nvgpu_pmu *pmu);
void nvgpu_pmu_perfmon_set_sampling_enable_status(struct nvgpu_pmu *pmu,
		bool status);
u64 nvgpu_pmu_perfmon_get_events_count(struct nvgpu_pmu *pmu);
u32 nvgpu_pmu_perfmon_get_load_avg(struct nvgpu_pmu *pmu);

/* perfmon SW Ops */
int nvgpu_pmu_perfmon_initialization(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_perfmon *perfmon);
int nvgpu_pmu_perfmon_start_sample(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_perfmon *perfmon);
int nvgpu_pmu_perfmon_stop_sample(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_perfmon *perfmon);
int nvgpu_pmu_perfmon_get_sample(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_perfmon *perfmon);
int nvgpu_pmu_perfmon_event_handler(struct gk20a *g,
	struct nvgpu_pmu *pmu, void *msg);

#endif /* NVGPU_PMU_PERFMON_H */
