// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pmu/therm.h>
#include <nvgpu/boardobjgrp.h>

#include "thrm.h"

static void therm_unit_rpc_handler(struct gk20a *g, struct nvgpu_pmu *pmu,
		struct nv_pmu_rpc_header *rpc)
{
	(void)pmu;
	switch (rpc->function) {
	case NV_PMU_RPC_ID_THERM_BOARD_OBJ_GRP_CMD:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_THERM_BOARD_OBJ_GRP_CMD");
		break;
	default:
		nvgpu_pmu_dbg(g, "reply PMU_UNIT_THERM");
		break;
	}
}

int nvgpu_pmu_therm_sw_setup(struct gk20a *g, struct nvgpu_pmu *pmu)
{
	int status;

	status = therm_device_sw_setup(g);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for therm devices, status - 0x%x",
			status);
		goto exit;
	}

	status = therm_channel_sw_setup(g);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for therm channel, status - 0x%x",
			status);
		goto exit;
	}

	pmu->therm_rpc_handler = therm_unit_rpc_handler;

exit:
	return status;
}

int nvgpu_pmu_therm_pmu_setup(struct gk20a *g, struct nvgpu_pmu *pmu)
{
	int status;

	(void)pmu;

	status = therm_device_pmu_setup(g);
	if (status != 0) {
		nvgpu_err(g, "Therm device pmu setup failed - 0x%x", status);
		goto exit;
	}

	status = therm_channel_pmu_setup(g);
	if (status != 0) {
		nvgpu_err(g,"Therm channel pmu setup failed - 0x%x", status);
		goto exit;
	}

exit:
	return status;
}

int nvgpu_pmu_therm_init(struct gk20a *g, struct nvgpu_pmu *pmu)
{
	/* If already allocated, do not re-allocate */
	if (pmu->therm_pmu != NULL) {
		return 0;
	}

	pmu->therm_pmu = nvgpu_kzalloc_impl(g, sizeof(*(pmu->therm_pmu)), NVGPU_GET_IP);
	if (pmu->therm_pmu == NULL) {
		return -ENOMEM;
	}

	if (pmu->therm_pmu->therm_channelobjs == NULL) {
		pmu->therm_pmu->therm_channelobjs = nvgpu_kzalloc_impl(g, sizeof(*(pmu->therm_pmu->therm_channelobjs)), NVGPU_GET_IP);
		if (pmu->therm_pmu->therm_channelobjs == NULL) {
			nvgpu_err(g, "Failed to allocate therm_channelobjs");
			return -ENOMEM;
		}
	}

	return 0;
}

void nvgpu_pmu_therm_deinit(struct gk20a *g, struct nvgpu_pmu *pmu)
{

	/* Free allocated therm_channelobjs if it exists */
	if (pmu->therm_pmu->therm_channelobjs != NULL) {
		nvgpu_kfree_impl(g, pmu->therm_pmu->therm_channelobjs);
		pmu->therm_pmu->therm_channelobjs = NULL;
	}

	nvgpu_kfree_impl(g, pmu->therm_pmu);
	pmu->therm_pmu = NULL;
}
