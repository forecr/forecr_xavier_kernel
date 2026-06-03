/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef LINUX_DEBUG_FECS_TRACE_H
#define LINUX_DEBUG_FECS_TRACE_H

struct gk20a;

#if defined(CONFIG_DEBUG_FS) && defined(CONFIG_NVGPU_FECS_TRACE)
int nvgpu_fecs_trace_init_debugfs(struct gk20a *g);
#else
static int nvgpu_fecs_trace_init_debugfs(struct gk20a *g)
{
	return 0;
}
#endif
#endif
