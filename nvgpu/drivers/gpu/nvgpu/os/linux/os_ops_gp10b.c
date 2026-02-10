// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "os_linux.h"
#include "os_ops_gp10b.h"

#include "cde_gp10b.h"
#include "debug_fecs_trace.h"

static struct nvgpu_os_linux_ops gp10b_os_linux_ops = {
#ifdef CONFIG_NVGPU_SUPPORT_CDE
	.cde = {
		.get_program_numbers = gp10b_cde_get_program_numbers,
		.need_scatter_buffer = gp10b_need_scatter_buffer,
		.populate_scatter_buffer = gp10b_populate_scatter_buffer,
	},
#endif
	.fecs_trace = {
		.init_debugfs = nvgpu_fecs_trace_init_debugfs,
	},
};

void nvgpu_gp10b_init_os_ops(struct nvgpu_os_linux *l)
{
#ifdef CONFIG_NVGPU_SUPPORT_CDE
	l->ops.cde = gp10b_os_linux_ops.cde;
#endif
	l->ops.fecs_trace = gp10b_os_linux_ops.fecs_trace;
}
