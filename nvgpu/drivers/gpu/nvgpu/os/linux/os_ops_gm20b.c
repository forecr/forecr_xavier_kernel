// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "os_linux.h"
#include "os_ops_gm20b.h"

#include "cde_gm20b.h"
#include "debug_clk_gm20b.h"
#include "debug_fecs_trace.h"

static struct nvgpu_os_linux_ops gm20b_os_linux_ops = {
#ifdef CONFIG_NVGPU_SUPPORT_CDE
	.cde = {
		.get_program_numbers = gm20b_cde_get_program_numbers,
	},
#endif
	.clk = {
		.init_debugfs = gm20b_clk_init_debugfs,
	},

	.fecs_trace = {
		.init_debugfs = nvgpu_fecs_trace_init_debugfs,
	},
};

void nvgpu_gm20b_init_os_ops(struct nvgpu_os_linux *l)
{
#ifdef CONFIG_NVGPU_SUPPORT_CDE
	l->ops.cde = gm20b_os_linux_ops.cde;
#endif
	l->ops.clk = gm20b_os_linux_ops.clk;

	l->ops.fecs_trace = gm20b_os_linux_ops.fecs_trace;
}
