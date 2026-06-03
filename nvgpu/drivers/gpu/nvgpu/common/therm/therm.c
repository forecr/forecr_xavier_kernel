// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/log.h>
#include <nvgpu/therm.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/power_features/cg.h>

int nvgpu_init_therm_support(struct gk20a *g)
{
	int err = 0;

	nvgpu_log_fn(g, " ");

	if (g->ops.therm.init_therm_setup_hw != NULL) {
		err = g->ops.therm.init_therm_setup_hw(g);
	}
	if (err != 0) {
		return err;
	}

	nvgpu_cg_slcg_therm_load_enable(g);

#ifdef CONFIG_DEBUG_FS
	if (g->ops.therm.therm_debugfs_init)
	    g->ops.therm.therm_debugfs_init(g);
#endif

	return err;
}
