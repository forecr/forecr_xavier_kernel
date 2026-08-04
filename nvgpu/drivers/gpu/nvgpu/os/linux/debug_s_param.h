/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __DEBUG_S_PARAM_TU104_H
#define __DEBUG_S_PARAM_TU104_H

#include <nvgpu/gk20a.h>

#ifdef CONFIG_DEBUG_FS
int nvgpu_s_param_init_debugfs(struct gk20a *g);
#else
static inline int nvgpu_s_param_init_debugfs(struct gk20a *g)
{
	return 0;
}
#endif

#endif
