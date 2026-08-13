// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/io_usermode.h>
#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>

#include "os_linux.h"

void nvgpu_usermode_writel(struct gk20a *g, u32 r, u32 v)
{
	uintptr_t reg = g->usermode_regs + (r - g->ops.usermode.base(g));

	nvgpu_os_writel_relaxed(v, reg);
	nvgpu_log(g, gpu_dbg_reg, "usermode r=0x%x v=0x%x", r, v);
}
