// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2019-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/debugfs.h>
#include <nvgpu/pmu/therm.h>

#include "os_linux.h"
#include "debug_therm_tu104.h"

static int therm_get_internal_sensor_curr_temp(void *data, u64 *val)
{
	struct gk20a *g = (struct gk20a *)data;
	u32 readval;
	int err = 0;

	/*
	 * If PSTATE is enabled, temp value is taken from THERM_GET_STATUS.
	 * If PSTATE is disable, temp value is read from NV_THERM_I2CS_SENSOR_00
	 * register value.
	 */
	if (nvgpu_is_enabled(g, NVGPU_PMU_PSTATE)) {
		err = nvgpu_pmu_therm_channel_get_curr_temp(g, &readval);
		if (!err) {
			*val = readval;
		}
	} else {
		if (!g->ops.therm.get_internal_sensor_curr_temp) {
			nvgpu_err(g, "reading NV_THERM_I2CS_SENSOR_00 not enabled");
			return -EINVAL;
		}

		g->ops.therm.get_internal_sensor_curr_temp(g, &readval);
		*val = readval;
	}

	return err;
}
DEFINE_SIMPLE_ATTRIBUTE(therm_ctrl_fops, therm_get_internal_sensor_curr_temp, NULL, "%llu\n");

int tu104_therm_init_debugfs(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct dentry *dbgentry;

	dbgentry = debugfs_create_file(
		"temp", S_IRUGO, l->debugfs, g, &therm_ctrl_fops);
	if (!dbgentry)
		nvgpu_err(g, "debugfs entry create failed for therm_curr_temp");

	return 0;
}
