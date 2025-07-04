// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "os/linux/os_linux.h"
#include "os/linux/os_ops_tu104.h"

#include "os/linux/debug_therm_tu104.h"
#include "os/linux/debug_clk_tu104.h"
#include "os/linux/debug_volt.h"
#include "os/linux/debug_s_param.h"

static struct nvgpu_os_linux_ops tu104_os_linux_ops = {
	.therm = {
		.init_debugfs = tu104_therm_init_debugfs,
	},
	.clk = {
		.init_debugfs = tu104_clk_init_debugfs,
	},
	.volt = {
		.init_debugfs = nvgpu_volt_init_debugfs,
	},
	.s_param = {
		.init_debugfs = nvgpu_s_param_init_debugfs,
	},
};

void nvgpu_tu104_init_os_ops(struct nvgpu_os_linux *l)
{
	l->ops.therm = tu104_os_linux_ops.therm;
	l->ops.clk = tu104_os_linux_ops.clk;
	l->ops.volt = tu104_os_linux_ops.volt;
	l->ops.s_param = tu104_os_linux_ops.s_param;
}
