/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TRACE_H
#define NVGPU_TRACE_H

struct gk20a;

#ifdef __KERNEL__

#ifdef CONFIG_NVGPU_TRACE
#include <trace/events/gk20a.h>
#endif
void nvgpu_trace_intr_stall_start(struct gk20a *g);
void nvgpu_trace_intr_stall_done(struct gk20a *g);
void nvgpu_trace_intr_thread_stall_start(struct gk20a *g);
void nvgpu_trace_intr_thread_stall_done(struct gk20a *g);

#elif defined(__NVGPU_POSIX__)

#ifdef CONFIG_NVGPU_TRACE
#include <nvgpu/posix/trace_gk20a.h>
#endif /* CONFIG_NVGPU_TRACE */
static inline void nvgpu_trace_intr_stall_start(struct gk20a *g)
{
	(void)g;
}
static inline void nvgpu_trace_intr_stall_done(struct gk20a *g)
{
	(void)g;
}
static inline void nvgpu_trace_intr_thread_stall_start(struct gk20a *g)
{
	(void)g;
}
static inline void nvgpu_trace_intr_thread_stall_done(struct gk20a *g)
{
	(void)g;
}

#else

#ifdef CONFIG_NVGPU_TRACE
#include <nvgpu/posix/trace_gk20a.h>
#endif
static inline void nvgpu_trace_intr_stall_start(struct gk20a *g)
{
	(void)g;
}
static inline void nvgpu_trace_intr_stall_done(struct gk20a *g)
{
	(void)g;
}
void nvgpu_trace_intr_thread_stall_start(struct gk20a *g);
void nvgpu_trace_intr_thread_stall_done(struct gk20a *g);

#endif /* __KERNEL__ */

#endif /* NVGPU_TRACE_H */
