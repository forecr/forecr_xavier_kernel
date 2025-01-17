// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "os_linux.h"

#include "os_ops_gv11b.h"
#include "debug_fecs_trace.h"

static struct nvgpu_os_linux_ops gv11b_os_linux_ops = {
	.fecs_trace = {
		.init_debugfs = nvgpu_fecs_trace_init_debugfs,
	},
};

void nvgpu_gv11b_init_os_ops(struct nvgpu_os_linux *l)
{
	l->ops.fecs_trace = gv11b_os_linux_ops.fecs_trace;
}
