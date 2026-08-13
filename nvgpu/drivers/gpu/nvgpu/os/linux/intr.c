// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/trace.h>

#include <nvgpu/gk20a.h>


void nvgpu_trace_intr_thread_stall_start(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_TRACE
	trace_mc_gk20a_intr_thread_stall(g->name);
#endif
}

void nvgpu_trace_intr_thread_stall_done(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_TRACE
	trace_mc_gk20a_intr_thread_stall_done(g->name);
#endif
}

void nvgpu_trace_intr_stall_start(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_TRACE
	trace_mc_gk20a_intr_stall(g->name);
#endif
}

void nvgpu_trace_intr_stall_done(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_TRACE
	trace_mc_gk20a_intr_stall_done(g->name);
#endif
}
