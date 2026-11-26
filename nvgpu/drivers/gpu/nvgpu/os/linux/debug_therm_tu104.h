/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __DEBUG_THERM_GP106_H
#define __DEBUG_THERM_GP106_H

#ifdef CONFIG_DEBUG_FS
int tu104_therm_init_debugfs(struct gk20a *g);
#else
static inline int tu104_therm_init_debugfs(struct gk20a *g)
{
	return 0;
}
#endif

#endif
