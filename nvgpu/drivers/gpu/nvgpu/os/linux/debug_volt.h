/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __DEBUG_VOLT_H
#define __DEBUG_VOLT_H

#ifdef CONFIG_DEBUG_FS
int nvgpu_volt_init_debugfs(struct gk20a *g);
#else
static inline int nvgpu_volt_init_debugfs(struct gk20a *g)
{
	return 0;
}
#endif

#endif
