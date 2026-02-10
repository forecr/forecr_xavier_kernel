// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2019-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/debugfs.h>
#include "os_linux.h"
#include "include/nvgpu/bios.h"
#include "debug_s_param.h"

#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/pmu/perf.h>

static int get_s_param_info(void *data, u64 *val)
{
	struct gk20a *g = (struct gk20a *)data;
	int status = 0;

	status = nvgpu_pmu_perf_vfe_get_s_param(g, val);
	if(status != 0) {
		nvgpu_err(g, "Vfe_var get s_param failed");
		return status;
	}
	return status;
}
DEFINE_SIMPLE_ATTRIBUTE(s_param_fops, get_s_param_info , NULL, "%llu\n");

int nvgpu_s_param_init_debugfs(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct dentry *dbgentry;

	dbgentry = debugfs_create_file(
		"s_param", S_IRUGO, l->debugfs, g, &s_param_fops);
	if (!dbgentry) {
		pr_err("%s: Failed to make debugfs node\n", __func__);
		return -ENOMEM;
	}

	return 0;
}
