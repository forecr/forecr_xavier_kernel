// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2019-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/debugfs.h>
#include "os_linux.h"
#include "debug_volt.h"

#include <nvgpu/pmu/volt.h>

static int get_curr_voltage(void *data, u64 *val)
{
	struct gk20a *g = (struct gk20a *)data;
	u32 readval;
	int err;

	err = nvgpu_pmu_volt_get_curr_volt_ps35(g, &readval);
	if (!err)
		*val = readval;

	return err;
}
DEFINE_SIMPLE_ATTRIBUTE(curr_volt_ctrl_fops, get_curr_voltage, NULL, "%llu\n");

static int get_min_voltage(void *data, u64 *val)
{
	struct gk20a *g = (struct gk20a *)data;
	u32 minval, maxval;
	int err;

	err = nvgpu_pmu_volt_get_vmin_vmax_ps35(g, &minval, &maxval);
	if (!err)
		*val = minval;

	return err;
}
DEFINE_SIMPLE_ATTRIBUTE(min_volt_ctrl_fops, get_min_voltage, NULL, "%llu\n");

static int get_max_voltage(void *data, u64 *val)
{
	struct gk20a *g = (struct gk20a *)data;
	u32 minval, maxval;
	int err;

	err = nvgpu_pmu_volt_get_vmin_vmax_ps35(g, &minval, &maxval);
	if (!err)
		*val = maxval;

	return err;
}
DEFINE_SIMPLE_ATTRIBUTE(max_volt_ctrl_fops, get_max_voltage, NULL, "%llu\n");

int nvgpu_volt_init_debugfs(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct dentry *dbgentry, *volt_root;

	volt_root = debugfs_create_dir("volt", l->debugfs);

	dbgentry = debugfs_create_file(
		"current_voltage", S_IRUGO, volt_root, g, &curr_volt_ctrl_fops);
	if (!dbgentry) {
		pr_err("%s: Failed to make debugfs node\n", __func__);
		return -ENOMEM;
	}

	dbgentry = debugfs_create_file("minimum_voltage",
			S_IRUGO, volt_root, g, &min_volt_ctrl_fops);
	if (!dbgentry) {
		pr_err("%s: Failed to make debugfs node\n", __func__);
		return -ENOMEM;
	}

	dbgentry = debugfs_create_file("maximum_voltage",
			S_IRUGO, volt_root, g, &max_volt_ctrl_fops);
	if (!dbgentry) {
		pr_err("%s: Failed to make debugfs node\n", __func__);
		return -ENOMEM;
	}

	return 0;
}
