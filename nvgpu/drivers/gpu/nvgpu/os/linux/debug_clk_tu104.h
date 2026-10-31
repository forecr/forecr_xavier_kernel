/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __DEBUG_CLK_TU104_H
#define __DEBUG_CLK_TU104_H

#ifdef CONFIG_DEBUG_FS
int tu104_clk_init_debugfs(struct gk20a *g);
#else
static inline int tu104_clk_init_debugfs(struct gk20a *g)
{
	return 0;
}
#endif

#endif
