// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/power_features/pg.h>

#include "fecs_trace_gb10b.h"

#include <nvgpu/hw/gb10b/hw_func_gb10b.h>
#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>

int gb10b_fecs_trace_get_read_index(struct gk20a *g)
{
	return nvgpu_pg_elpg_protected_call(g,
			(int)nvgpu_func_readl(g, func_priv_mailbox_scratch_r(1)));
}

int gb10b_fecs_trace_get_write_index(struct gk20a *g)
{
	return nvgpu_pg_elpg_protected_call(g,
			(int)nvgpu_func_readl(g, func_priv_mailbox_scratch_r(0)));
}

int gb10b_fecs_trace_set_read_index(struct gk20a *g, int index)
{
	nvgpu_log(g, gpu_dbg_ctxsw, "set read=%d", index);
	return nvgpu_pg_elpg_protected_call(g,
			(nvgpu_func_writel(g, func_priv_mailbox_scratch_r(1), (u32)index), 0));
}
