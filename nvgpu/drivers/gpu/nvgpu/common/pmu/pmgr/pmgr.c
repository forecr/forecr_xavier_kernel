// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pmu/pmgr.h>

#include "pmgr.h"
#include "pwrdev.h"
#include "pmgrpmu.h"

int pmgr_pwr_devices_get_power(struct gk20a *g, u32 *val)
{
	struct nv_pmu_pmgr_pwr_devices_query_payload payload;
	int status;

	status = pmgr_pmu_pwr_devices_query_blocking(g, 1, &payload);
	if (status != 0) {
		nvgpu_err(g, "pmgr_pwr_devices_get_current_power failed %x",
			status);
	}

	*val = payload.devices[0].powerm_w;

	return status;
}

int pmgr_pwr_devices_get_current(struct gk20a *g, u32 *val)
{
	struct nv_pmu_pmgr_pwr_devices_query_payload payload;
	int status;

	status = pmgr_pmu_pwr_devices_query_blocking(g, 1, &payload);
	if (status != 0) {
		nvgpu_err(g, "pmgr_pwr_devices_get_current failed %x",
			status);
	}

	*val = payload.devices[0].currentm_a;

	return status;
}

int pmgr_pwr_devices_get_voltage(struct gk20a *g, u32 *val)
{
	struct nv_pmu_pmgr_pwr_devices_query_payload payload;
	int status;

	status = pmgr_pmu_pwr_devices_query_blocking(g, 1, &payload);
	if (status != 0) {
		nvgpu_err(g, "pmgr_pwr_devices_get_current_voltage failed %x",
			status);
	}

	*val = payload.devices[0].voltageu_v;

	return status;
}

int pmgr_domain_sw_setup(struct gk20a *g)
{
	int status;

	status = pmgr_device_sw_setup(g);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for pmgr devices, status - 0x%x",
			status);
		goto exit;
	}

	status = pmgr_monitor_sw_setup(g);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for pmgr monitor, status - 0x%x",
			status);
		goto exit;
	}

	status = pmgr_policy_sw_setup(g);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for pmgr policy, status - 0x%x",
			status);
		goto exit;
	}

exit:
	return status;
}

int pmgr_domain_pmu_setup(struct gk20a *g)
{
	return pmgr_send_pmgr_tables_to_pmu(g);
}

int pmgr_pmu_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmgr_pmu != NULL) {
		return 0;
	}

	g->pmgr_pmu = nvgpu_kzalloc(g, sizeof(*g->pmgr_pmu));
	if (g->pmgr_pmu == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void pmgr_pmu_free_pmupstate(struct gk20a *g)
{
	nvgpu_kfree(g, g->pmgr_pmu);
	g->pmgr_pmu = NULL;
}
