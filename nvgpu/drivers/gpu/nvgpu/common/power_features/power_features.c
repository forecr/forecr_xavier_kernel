// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/power_features/pg.h>
#include <nvgpu/power_features/power_features.h>

int nvgpu_cg_pg_disable(struct gk20a *g)
{
	int err = 0;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

#ifdef CONFIG_NVGPU_POWER_PG
	/* Disable elpg before clock gating.
	 * Disable aelpg also, if applicable.
	 */
	err = nvgpu_pg_elpg_disable(g);
	if (err != 0) {
		nvgpu_err(g, "failed to set disable elpg");
	}
#endif
	nvgpu_cg_slcg_gr_perf_ltc_load_disable(g);

	nvgpu_cg_blcg_mode_disable(g);

	/* Disables elcg and nvdclk_flcg */
	nvgpu_cg_elcg_set_elcg_enabled(g, false);

	return err;
}

int nvgpu_cg_pg_enable(struct gk20a *g)
{
	int err = 0;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_cg_elcg_set_elcg_enabled(g, true);

	nvgpu_cg_blcg_mode_enable(g);

	nvgpu_cg_slcg_gr_perf_ltc_load_enable(g);

#ifdef CONFIG_NVGPU_POWER_PG
	err = nvgpu_pg_elpg_enable(g);
	if (err != 0) {
		nvgpu_err(g, "failed to set enable elpg");
	}
#endif

	return err;
}
