// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "os_linux.h"
#include "os_ops_gv100.h"

#include "debug_clk_tu104.h"
#include "debug_therm_tu104.h"
#include "debug_fecs_trace.h"

static struct nvgpu_os_linux_ops gv100_os_linux_ops = {
	.clk = {
		.init_debugfs = tu104_clk_init_debugfs,
	},
	.therm = {
		.init_debugfs = tu104_therm_init_debugfs,
	},
	.fecs_trace = {
		.init_debugfs = nvgpu_fecs_trace_init_debugfs,
	},
};

void nvgpu_gv100_init_os_ops(struct nvgpu_os_linux *l)
{
	l->ops.clk = gv100_os_linux_ops.clk;
	l->ops.therm = gv100_os_linux_ops.therm;
	l->ops.fecs_trace = gv100_os_linux_ops.fecs_trace;
}
