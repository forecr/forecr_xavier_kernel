/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_MSG_H
#define NVGPU_PMU_MSG_H

#include <nvgpu/pmu/pmuif/init.h>
#include <nvgpu/pmu/pmuif/pg.h>
#include <nvgpu/pmu/pmuif/perfmon.h>
#include <nvgpu/pmu/pmuif/acr.h>
#include <nvgpu/pmu/pmuif/rpc.h>
#include <nvgpu/pmu/pmuif/pmgr.h>

/* GPU ID */
#define PMU_SHA1_GID_SIGNATURE		0xA7C66AD2U
#define PMU_SHA1_GID_SIGNATURE_SIZE	4U

#define PMU_SHA1_GID_SIZE	16U

struct gk20a;
struct nvgpu_pmu;
struct nvgpu_allocator;
struct pmu_sequences;
struct nvgpu_mem;
struct nvgpu_falcon;

struct pmu_sha1_gid {
	bool valid;
	u8 gid[PMU_SHA1_GID_SIZE];
};

struct pmu_sha1_gid_data {
	union {
		u8 sign_bytes[PMU_SHA1_GID_SIGNATURE_SIZE];
		u32 signature;
	};
	u8 gid[PMU_SHA1_GID_SIZE];
};

struct pmu_msg {
	struct pmu_hdr hdr;
	union {
		union {
			struct pmu_init_msg init;
			struct pmu_perfmon_msg perfmon;
			struct pmu_pg_msg pg;
			struct pmu_rc_msg rc;
			struct pmu_acr_msg acr;
			struct nv_pmu_boardobj_msg obj;
			struct nv_pmu_pmgr_msg pmgr;
			struct nv_pmu_rpc_msg rpc;
		} msg;
		union {
			struct pmu_nvgpu_rpc_struct_cmdmgmt_init cmdmgmt_init;
			struct pmu_nvgpu_rpc_perfmon_init perfmon_init;
			struct pmu_nvgpu_rpc_perfmon_change perfmon_change;
		} event_rpc;
	};
};

int nvgpu_pmu_process_message(struct nvgpu_pmu *pmu);
void nvgpu_pmu_rpc_handler(struct gk20a *g, struct pmu_msg *msg,
		void *param, u32 status);
/* PMU wait*/
void pmu_wait_message_cond(struct nvgpu_pmu *pmu, u32 timeout_ms,
				void *var, u8 val);
u32 nvgpu_pmu_get_next_head(u32 head);
#endif /* NVGPU_PMU_MSG_H */
