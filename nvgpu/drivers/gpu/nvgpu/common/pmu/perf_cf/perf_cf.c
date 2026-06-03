// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/pmu/perf_cf.h>

#include "perf_cf_topology.h"

/*
 * If there are more Perf-CF tables that need to be parsed by nvgpu
 * then add corresponding perf_cf_*_sw_setup() and call them here.
 */
int nvgpu_pmu_perf_cf_sw_setup(struct gk20a *g)
{
	int err;

	nvgpu_log_fn(g, " ");

	err = perf_cf_topology_sw_setup(g);
	if (err != 0) {
		return err;
	}

	return 0;
}

int nvgpu_pmu_perf_cf_init(struct gk20a *g)
{
	int err = 0;

	nvgpu_log_fn(g, " ");

	/* If already allocated, do not re-allocate */
	if (g->pmu->perf_cf != NULL) {
		return 0;
	}

	g->pmu->perf_cf = (struct nvgpu_pmu_perf_cf *) nvgpu_kzalloc(g,
			sizeof(struct nvgpu_pmu_perf_cf));
	if (g->pmu->perf_cf == NULL) {
		err = -ENOMEM;
		return err;
	}

	g->pmu->perf_cf->perf_cf_metadata = (struct nvgpu_pmu_perf_cf_metadata *)
				nvgpu_kzalloc(g, sizeof(struct nvgpu_pmu_perf_cf_metadata));
	if (g->pmu->perf_cf->perf_cf_metadata == NULL) {
		err = -ENOMEM;
		nvgpu_kfree(g, g->pmu->perf_cf);
		return err;
	}

	return err;
}

void nvgpu_pmu_perf_cf_deinit(struct gk20a *g)
{
	if (g->pmu == NULL) {
		return;
	}

	if ((g->pmu->perf_cf != NULL) && (g->pmu->perf_cf->perf_cf_metadata != NULL)) {
		nvgpu_kfree(g, g->pmu->perf_cf->perf_cf_metadata);
		nvgpu_kfree(g, g->pmu->perf_cf);
		g->pmu->perf_cf = NULL;
	}
}

static int nvgpu_pmu_perf_cf_load(struct gk20a *g)
{
	struct nvgpu_pmu *pmu = g->pmu;
	struct nv_pmu_rpc_struct_perf_cf_load rpc;
	int status = 0;

	(void) memset(&rpc, 0, sizeof(struct nv_pmu_rpc_struct_perf_cf_load));
	PMU_RPC_EXECUTE(status, pmu, PERF_CF, LOAD, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute PERF_CF Load RPC, status=0x%x", status);
	}

	return status;
}

int nvgpu_pmu_perf_cf_pmu_setup(struct gk20a *g)
{
	int err;

	err = perf_cf_topology_pmu_setup(g);
	if (err != 0) {
		return err;
	}

	err = nvgpu_pmu_perf_cf_load(g);
	if (err != 0) {
		return err;
	}

	return 0;
}
