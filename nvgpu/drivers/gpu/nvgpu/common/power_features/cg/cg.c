// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES.  All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/engines.h>
#include <nvgpu/device.h>
#include <nvgpu/enabled.h>
#include <nvgpu/falcon.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/pmu/pmu_pg.h>

void nvgpu_cg_load_enable(struct gk20a *g, struct nvgpu_falcon *flcn)
{

	if ((g->slcg_enabled) && (flcn->flcn_engine_dep_ops.slcg_load_enable != NULL)) {
		flcn->flcn_engine_dep_ops.slcg_load_enable(flcn->g, flcn->inst_id, true);
	}

	if ((g->blcg_enabled) && (flcn->flcn_engine_dep_ops.blcg_load_enable != NULL)) {
		flcn->flcn_engine_dep_ops.blcg_load_enable(flcn->g, flcn->inst_id, true);
	}
}

static void nvgpu_cg_set_mode(struct gk20a *g, u32 cgmode, u32 mode_config)
{
	u32 n;
	u32 engine_id = 0;
	const struct nvgpu_device *dev = NULL;
	struct nvgpu_fifo *f = &g->fifo;

	/*
	 * Add NULL check for ELCG and BLCG HALs based on cgmode
	 * to avoid coverity issue: CWE-476: NULL Pointer Dereference
	 */
	if (g->ops.pmu.is_cg_supported_by_pmu != NULL &&
		g->ops.pmu.is_cg_supported_by_pmu(g) == true) {
		if ((cgmode == BLCG_MODE && g->ops.pmu.pmu_init_blcg_mode == NULL)) {
			return;
		}
	} else {

		if ((cgmode == BLCG_MODE && g->ops.therm.init_blcg_mode == NULL) ||
				(cgmode == ELCG_MODE &&
				 g->ops.therm.init_elcg_mode == NULL)) {
			return;
		}
	}

	nvgpu_log_fn(g, " ");

	for (n = 0; n < f->num_engines; n++) {
		dev = f->active_engines[n];

#ifdef CONFIG_NVGPU_NON_FUSA
		/* gr_engine supports both BLCG and ELCG */
		if ((cgmode == BLCG_MODE) &&
			(dev->type == NVGPU_DEVTYPE_GRAPHICS)) {
			/*
			 * if clock gating registers are moved to PMU
			 * call BLCG HAL from PMU
			 */
			if (g->ops.pmu.is_cg_supported_by_pmu != NULL &&
				g->ops.pmu.is_cg_supported_by_pmu(g) == true) {
				g->ops.pmu.pmu_init_blcg_mode(g, (u32)mode_config,
								engine_id);
			} else {
				g->ops.therm.init_blcg_mode(g, (u32)mode_config,
								engine_id);
			}
			break;
		} else
#endif
		if (cgmode == ELCG_MODE) {
			/*
			 * if clock gating registers are moved to PMU
			 * call ELCG HAL from PMU
			 */
			if (g->ops.pmu.is_cg_supported_by_pmu != NULL &&
				g->ops.pmu.is_cg_supported_by_pmu(g) == true) {
#ifdef CONFIG_NVGPU_LS_PMU
				/**
				 * Checking if PG is supported, if yes, then taking the PMU RPC
				 * path for elcg enable/disable. Keeping this inside LS_PMU
				 * config flag check to make sure that can_elpg and pmu->pg is
				 * defined in the build. If PG is not supported then we take the
				 * legacy path for elcg enable/disable. The PMU path was
				 * originally made for non-auto SKUs but it was observed that
				 * auto SKUs were also taking that path and were failing because
				 * PG was not initialized.
				 */
				if (g->support_ls_pmu && g->can_elpg && g->pmu->pg != NULL) {
					/* send RPC to PMU to enable/disable ELCG */
					nvgpu_pmu_elcg_state_change(g, (u32)mode_config);
					break;
				} else
#endif
				{
					if (g->ops.pmu.pmu_init_elcg_mode != NULL) {
						g->ops.pmu.pmu_init_elcg_mode(g,
							(u32)mode_config, dev->engine_id);
					}
				}
			} else {
				g->ops.therm.init_elcg_mode(g, (u32)mode_config,
						dev->engine_id);
			}
		} else {
			nvgpu_err(g, "invalid cg mode %d, config %d for "
							"engine_id %d",
					cgmode, mode_config, engine_id);
		}
	}
}

void nvgpu_cg_elcg_enable_no_wait(struct gk20a *g)
{
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (g->elcg_enabled) {
		nvgpu_cg_set_mode(g, ELCG_MODE, ELCG_AUTO);
	}
	nvgpu_mutex_release(&g->cg_pg_lock);
}


void nvgpu_cg_elcg_disable_no_wait(struct gk20a *g)
{
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (g->elcg_enabled) {
		nvgpu_cg_set_mode(g, ELCG_MODE, ELCG_RUN);
	}
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_blcg_fb_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->blcg_enabled) {
		goto done;
	}
	if (g->ops.cg.blcg_fb_load_gating_prod != NULL) {
		g->ops.cg.blcg_fb_load_gating_prod(g, true);
	}
	if (g->ops.cg.blcg_fbhub_load_gating_prod != NULL) {
		g->ops.cg.blcg_fbhub_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_blcg_ltc_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->blcg_enabled) {
		goto done;
	}
	if (g->ops.cg.blcg_ltc_load_gating_prod != NULL) {
		g->ops.cg.blcg_ltc_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_blcg_fifo_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->blcg_enabled) {
		goto done;
	}
	if (g->ops.cg.blcg_fifo_load_gating_prod != NULL) {
		g->ops.cg.blcg_fifo_load_gating_prod(g, true);
	}
	if (g->ops.cg.blcg_runlist_load_gating_prod != NULL) {
		g->ops.cg.blcg_runlist_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_blcg_pmu_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->blcg_enabled) {
		goto done;
	}
	if (g->ops.cg.blcg_pmu_load_gating_prod != NULL) {
		g->ops.cg.blcg_pmu_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_blcg_ce_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->blcg_enabled) {
		goto done;
	}
	if (g->ops.cg.blcg_ce_load_gating_prod != NULL) {
		g->ops.cg.blcg_ce_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_blcg_ctrl_load_enable(struct gk20a *g, bool enable)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->blcg_enabled) {
		goto done;
	}

	if (g->ops.cg.blcg_ctrl_load_gating_prod != NULL) {
		g->ops.cg.blcg_ctrl_load_gating_prod(g, enable);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_blcg_gr_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->blcg_enabled) {
		goto done;
	}
	if (g->ops.cg.blcg_gr_load_gating_prod != NULL) {
		g->ops.cg.blcg_gr_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_fb_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}
	if (g->ops.cg.slcg_fb_load_gating_prod != NULL) {
		g->ops.cg.slcg_fb_load_gating_prod(g, true);
	}
	if (g->ops.cg.slcg_fbhub_load_gating_prod != NULL) {
		g->ops.cg.slcg_fbhub_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_ltc_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}
	if (g->ops.cg.slcg_ltc_load_gating_prod != NULL) {
		g->ops.cg.slcg_ltc_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

static void nvgpu_cg_slcg_priring_load_prod(struct gk20a *g, bool enable)
{

	if (g->ops.cg.slcg_priring_load_gating_prod != NULL) {
		g->ops.cg.slcg_priring_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_rs_ctrl_fbp_load_gating_prod != NULL) {
		g->ops.cg.slcg_rs_ctrl_fbp_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_rs_ctrl_gpc_load_gating_prod != NULL) {
		g->ops.cg.slcg_rs_ctrl_gpc_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_rs_ctrl_sys_load_gating_prod != NULL) {
		g->ops.cg.slcg_rs_ctrl_sys_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_rs_fbp_load_gating_prod != NULL) {
		g->ops.cg.slcg_rs_fbp_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_rs_gpc_load_gating_prod != NULL) {
		g->ops.cg.slcg_rs_gpc_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_rs_sys_load_gating_prod != NULL) {
		g->ops.cg.slcg_rs_sys_load_gating_prod(g, enable);
	}

}

void nvgpu_cg_slcg_priring_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}

	nvgpu_cg_slcg_priring_load_prod(g, true);
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_fifo_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}
	if (g->ops.cg.slcg_fifo_load_gating_prod != NULL) {
		g->ops.cg.slcg_fifo_load_gating_prod(g, true);
	}
	if (g->ops.cg.slcg_runlist_load_gating_prod != NULL) {
		g->ops.cg.slcg_runlist_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_pmu_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}
	if (g->ops.cg.slcg_pmu_load_gating_prod != NULL) {
		g->ops.cg.slcg_pmu_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_therm_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}
	if (g->ops.cg.slcg_therm_load_gating_prod != NULL) {
		g->ops.cg.slcg_therm_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_ce2_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}
	if (g->ops.cg.slcg_ce2_load_gating_prod != NULL) {
		g->ops.cg.slcg_ce2_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_timer_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}
	if (g->ops.cg.slcg_timer_load_gating_prod != NULL) {
		g->ops.cg.slcg_timer_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

#ifdef CONFIG_NVGPU_PROFILER
void nvgpu_cg_slcg_perf_load_enable(struct gk20a *g, bool enable)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}

	if (g->ops.cg.slcg_perf_load_gating_prod != NULL) {
		g->ops.cg.slcg_perf_load_gating_prod(g, enable);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}
#endif

void nvgpu_cg_slcg_gsp_load_enable(struct gk20a *g, bool enable)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}

	if (g->ops.cg.slcg_gsp_load_gating_prod != NULL) {
		g->ops.cg.slcg_gsp_load_gating_prod(g, enable);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_ctrl_load_enable(struct gk20a *g, bool enable)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}

	if (g->ops.cg.slcg_ctrl_load_gating_prod != NULL) {
		g->ops.cg.slcg_ctrl_load_gating_prod(g, enable);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

#ifdef CONFIG_NVGPU_NON_FUSA
static void cg_init_gr_flcg_load_gating_prod(struct gk20a *g)
{
	if (g->ops.cg.flcg_perf_load_gating_prod != NULL) {
		g->ops.cg.flcg_perf_load_gating_prod(g, true);
	}
}
#endif

static void cg_init_gr_slcg_load_gating_prod(struct gk20a *g)
{
	if (g->ops.cg.slcg_bus_load_gating_prod != NULL) {
		g->ops.cg.slcg_bus_load_gating_prod(g, true);
	}
	if (g->ops.cg.slcg_chiplet_load_gating_prod != NULL) {
		g->ops.cg.slcg_chiplet_load_gating_prod(g, true);
	}
	if (g->ops.cg.slcg_gr_load_gating_prod != NULL) {
		g->ops.cg.slcg_gr_load_gating_prod(g, true);
	}
	if (g->ops.cg.slcg_perf_load_gating_prod != NULL) {
		g->ops.cg.slcg_perf_load_gating_prod(g, true);
	}
	if (g->ops.cg.slcg_xbar_load_gating_prod != NULL) {
		g->ops.cg.slcg_xbar_load_gating_prod(g, true);
	}
	if (g->ops.cg.slcg_hshub_load_gating_prod != NULL) {
		g->ops.cg.slcg_hshub_load_gating_prod(g, true);
	}
}

static void cg_init_gr_blcg_load_gating_prod(struct gk20a *g)
{
	if (g->ops.cg.blcg_bus_load_gating_prod != NULL) {
		g->ops.cg.blcg_bus_load_gating_prod(g, true);
	}
	if (g->ops.cg.blcg_gr_load_gating_prod != NULL) {
		g->ops.cg.blcg_gr_load_gating_prod(g, true);
	}
	if (g->ops.cg.blcg_xbar_load_gating_prod != NULL) {
		g->ops.cg.blcg_xbar_load_gating_prod(g, true);
	}
	if (g->ops.cg.blcg_hshub_load_gating_prod != NULL) {
		g->ops.cg.blcg_hshub_load_gating_prod(g, true);
	}
}

void nvgpu_cg_init_gr_load_gating_prod(struct gk20a *g)
{
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	if (!g->flcg_enabled) {
		goto check_can_slcg;
	}

#ifdef CONFIG_NVGPU_NON_FUSA
	cg_init_gr_flcg_load_gating_prod(g);
#endif
check_can_slcg:
	if (!g->slcg_enabled) {
		goto check_can_blcg;
	}

	cg_init_gr_slcg_load_gating_prod(g);

check_can_blcg:
	if (!g->blcg_enabled) {
		goto exit;
	}

	cg_init_gr_blcg_load_gating_prod(g);

exit:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

#ifdef CONFIG_NVGPU_NON_FUSA
void nvgpu_cg_elcg_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (g->elcg_enabled) {
		nvgpu_cg_set_mode(g, ELCG_MODE, ELCG_AUTO);
	}
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_elcg_disable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (g->elcg_enabled) {
		nvgpu_cg_set_mode(g, ELCG_MODE, ELCG_RUN);
	}
	nvgpu_mutex_release(&g->cg_pg_lock);

}

void nvgpu_cg_blcg_mode_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (g->blcg_enabled) {
		nvgpu_cg_set_mode(g, BLCG_MODE, BLCG_AUTO);
	}
	nvgpu_mutex_release(&g->cg_pg_lock);

}

void nvgpu_cg_blcg_mode_disable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (g->blcg_enabled) {
		nvgpu_cg_set_mode(g, BLCG_MODE, BLCG_RUN);
	}
	nvgpu_mutex_release(&g->cg_pg_lock);


}

void nvgpu_cg_slcg_gr_perf_ltc_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}
	if (g->ops.cg.slcg_ltc_load_gating_prod != NULL) {
		g->ops.cg.slcg_ltc_load_gating_prod(g, true);
	}
	if (g->ops.cg.slcg_perf_load_gating_prod != NULL) {
		g->ops.cg.slcg_perf_load_gating_prod(g, true);
	}
	if (g->ops.cg.slcg_gr_load_gating_prod != NULL) {
		g->ops.cg.slcg_gr_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_gr_perf_ltc_load_disable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->slcg_enabled) {
		goto done;
	}
	if (g->ops.cg.slcg_gr_load_gating_prod != NULL) {
		g->ops.cg.slcg_gr_load_gating_prod(g, false);
	}
	if (g->ops.cg.slcg_perf_load_gating_prod != NULL) {
		g->ops.cg.slcg_perf_load_gating_prod(g, false);
	}
	if (g->ops.cg.slcg_ltc_load_gating_prod != NULL) {
		g->ops.cg.slcg_ltc_load_gating_prod(g, false);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_elcg_set_elcg_enabled(struct gk20a *g, bool enable)
{
	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (enable) {
		if (!g->elcg_enabled) {
			g->elcg_enabled = true;
			/*
			 * update elcg_engine_mask as ELCG is enabled
			 * for all engines
			 */
			g->elcg_engine_bitmask =
					CG_ELCG_ALL_ENGINE_ENABLED_MASK;
			nvgpu_cg_set_mode(g, ELCG_MODE, ELCG_AUTO);

			/* If FLCG is enabled, update FLCG related feature */
			if (nvgpu_is_enabled(g, NVGPU_GPU_CAN_FLCG)) {
				/*
				 * enable NVDCLK FLCG when ELCG gets
				 * enabled for all engines
				 */
				if (!g->nvdclk_flcg_enabled) {
					/*
					 * To-Do: here, we should send rpc
					 * to PMU to enable nvdclk flcg
					 * this runtime enable support is yet
					 * to be added on PMU
					 */
					g->nvdclk_flcg_enabled = true;
				}
			}
		}
	} else {
		if (g->elcg_enabled || g->elcg_engine_bitmask) {
			/*
			 * update elcg_engine_mask as ELCG is disabled
			 * for all engines
			 */
			g->elcg_engine_bitmask = 0U;
			nvgpu_cg_set_mode(g, ELCG_MODE, ELCG_RUN);
			g->elcg_enabled = false;

			/* If FLCG is enabled, update FLCG related feature */
			if (nvgpu_is_enabled(g, NVGPU_GPU_CAN_FLCG)) {
				/*
				 * disable NVDCLK FLCG when ELCG gets disabled
				 * for all engines
				 */
				if (g->nvdclk_flcg_enabled) {
					/*
					 * To-Do: here, we should send rpc
					 * to PMU to disable nvdclk flcg
					 * this runtime disable support is yet
					 * to be added on PMU
					 */
					g->nvdclk_flcg_enabled = false;
				}
			}
		}
	}
	if (g->ops.cg.elcg_ce_load_gating_prod != NULL) {
		g->ops.cg.elcg_ce_load_gating_prod(g, g->elcg_enabled);
	}
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_blcg_set_blcg_enabled(struct gk20a *g, bool enable)
{
	bool load = false;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (enable) {
		if (!g->blcg_enabled) {
			load = true;
			g->blcg_enabled = true;
		}
	} else {
		if (g->blcg_enabled) {
			load = true;
			g->blcg_enabled = false;
		}
	}
	if (!load ) {
		goto done;
	}

	if (g->ops.cg.blcg_bus_load_gating_prod != NULL) {
		g->ops.cg.blcg_bus_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_ce_load_gating_prod != NULL) {
		g->ops.cg.blcg_ce_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_fb_load_gating_prod != NULL) {
		g->ops.cg.blcg_fb_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_fbhub_load_gating_prod != NULL) {
		g->ops.cg.blcg_fbhub_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_fifo_load_gating_prod != NULL) {
		g->ops.cg.blcg_fifo_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_gr_load_gating_prod != NULL) {
		g->ops.cg.blcg_gr_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_runlist_load_gating_prod != NULL) {
		g->ops.cg.blcg_runlist_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_ltc_load_gating_prod != NULL) {
		g->ops.cg.blcg_ltc_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_pmu_load_gating_prod != NULL) {
		g->ops.cg.blcg_pmu_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_xbar_load_gating_prod != NULL) {
		g->ops.cg.blcg_xbar_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_hshub_load_gating_prod != NULL) {
		g->ops.cg.blcg_hshub_load_gating_prod(g, enable);
	}
	if (g->ops.cg.blcg_ctrl_load_gating_prod != NULL) {
		g->ops.cg.blcg_ctrl_load_gating_prod(g, enable);
	}

	if (g->ops.cg.blcg_nvenc_load_gating_prod != NULL) {
		g->ops.cg.blcg_nvenc_load_gating_prod(g, 0, enable);
		g->ops.cg.blcg_nvenc_load_gating_prod(g, 1, enable);
	}

	if (g->ops.cg.blcg_ofa_load_gating_prod != NULL) {
		g->ops.cg.blcg_ofa_load_gating_prod(g, 0, enable);
	}

	if (g->ops.cg.blcg_nvdec_load_gating_prod != NULL) {
		g->ops.cg.blcg_nvdec_load_gating_prod(g, 0, enable);
		g->ops.cg.blcg_nvdec_load_gating_prod(g, 1, enable);
	}

	if (g->ops.cg.blcg_nvjpg_load_gating_prod != NULL) {
		g->ops.cg.blcg_nvjpg_load_gating_prod(g, 0, enable);
		g->ops.cg.blcg_nvjpg_load_gating_prod(g, 1, enable);
	}

	if (g->ops.cg.blcg_xal_ep_load_gating_prod != NULL) {
		g->ops.cg.blcg_xal_ep_load_gating_prod(g, enable);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_slcg_set_slcg_enabled(struct gk20a *g, bool enable)
{
	bool load = false;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (enable) {
		if (!g->slcg_enabled) {
			load = true;
			g->slcg_enabled = true;
		}
	} else {
		if (g->slcg_enabled) {
			load = true;
			g->slcg_enabled = false;
		}
	}
	if (!load ) {
		goto done;
	}

	if (g->ops.cg.slcg_bus_load_gating_prod != NULL) {
		g->ops.cg.slcg_bus_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_ce2_load_gating_prod != NULL) {
		g->ops.cg.slcg_ce2_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_chiplet_load_gating_prod != NULL) {
		g->ops.cg.slcg_chiplet_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_fb_load_gating_prod != NULL) {
		g->ops.cg.slcg_fb_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_fbhub_load_gating_prod != NULL) {
		g->ops.cg.slcg_fbhub_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_fifo_load_gating_prod != NULL) {
		g->ops.cg.slcg_fifo_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_runlist_load_gating_prod != NULL) {
		g->ops.cg.slcg_runlist_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_timer_load_gating_prod != NULL) {
		g->ops.cg.slcg_timer_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_gr_load_gating_prod != NULL) {
		g->ops.cg.slcg_gr_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_ltc_load_gating_prod != NULL) {
		g->ops.cg.slcg_ltc_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_perf_load_gating_prod != NULL) {
		g->ops.cg.slcg_perf_load_gating_prod(g, enable);
	}

	nvgpu_cg_slcg_priring_load_prod(g, enable);

	if (g->ops.cg.slcg_pmu_load_gating_prod != NULL) {
		g->ops.cg.slcg_pmu_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_xbar_load_gating_prod != NULL) {
		g->ops.cg.slcg_xbar_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_hshub_load_gating_prod != NULL) {
		g->ops.cg.slcg_hshub_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_ctrl_load_gating_prod != NULL) {
		g->ops.cg.slcg_ctrl_load_gating_prod(g, enable);
	}
	if (g->ops.cg.slcg_gsp_load_gating_prod != NULL) {
		g->ops.cg.slcg_gsp_load_gating_prod(g, enable);
	}

	if (g->ops.cg.slcg_nvenc_load_gating_prod != NULL) {
		g->ops.cg.slcg_nvenc_load_gating_prod(g, 0, enable);
		g->ops.cg.slcg_nvenc_load_gating_prod(g, 1, enable);
	}

	if (g->ops.cg.slcg_ofa_load_gating_prod != NULL) {
		g->ops.cg.slcg_ofa_load_gating_prod(g, 0, enable);
	}

	if (g->ops.cg.slcg_nvdec_load_gating_prod != NULL) {
		g->ops.cg.slcg_nvdec_load_gating_prod(g, 0, enable);
		g->ops.cg.slcg_nvdec_load_gating_prod(g, 1, enable);
	}

	if (g->ops.cg.slcg_nvjpg_load_gating_prod != NULL) {
		g->ops.cg.slcg_nvjpg_load_gating_prod(g, 0, enable);
		g->ops.cg.slcg_nvjpg_load_gating_prod(g, 1, enable);
	}

	if (g->ops.cg.slcg_xal_ep_load_gating_prod != NULL) {
		g->ops.cg.slcg_xal_ep_load_gating_prod(g, enable);
	}

done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

void nvgpu_cg_flcg_set_flcg_enabled(struct gk20a *g, bool enable)
{
	bool load = false;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (enable) {
		if (!g->flcg_enabled) {
			load = true;
			g->flcg_enabled = true;
		}
	} else {
		if (g->flcg_enabled) {
			load = true;
			g->flcg_enabled = false;
		}
	}
	if (!load) {
		goto done;
	}

	if (g->ops.cg.flcg_perf_load_gating_prod != NULL) {
		g->ops.cg.flcg_perf_load_gating_prod(g, enable);
	}

done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

int nvgpu_cg_flcg_set_gpcclk_flcg_enabled(struct gk20a *g, bool enable)
{
	int err = 0;
	bool change_mode = false;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	/*
	 * GPCCLK FLCG depends upon GR ELCG so first
	 * check for all engines ELCG status
	 */
	if (!g->elcg_enabled) {
		nvgpu_err(g, "Global ELCG disabled, cannot update GPCCLK FLCG");
		goto done;
	}

	/* If FLCG is not supported, return */
	if (!nvgpu_is_enabled(g, NVGPU_GPU_CAN_FLCG)) {
		nvgpu_err(g, "FLCG not supported, cannot update GPCCLK FLCG");
		goto done;
	}

	/*
	 * GPCCLK FLCG should get engaged in ELPG window.
	 * if ELPG is not enabled, return
	 */
	if (!g->elpg_enabled) {
		nvgpu_err(g, "ELPG disabled, cannot update GPCCLK FLCG");
		goto done;
	}

	/*
	 * if GR-ELCG is enabled in elcg_engine_bitmask,
	 * we can update GPCCLK FLCG
	 */
	if (g->elcg_engine_bitmask &
			BIT32(CG_SUBFEATURE_ID_ELCG_CTRL_GR)) {
		if (enable) {
			if (!g->gpcclk_flcg_enabled) {
				change_mode = true;
				g->gpcclk_flcg_enabled = true;
			}
		} else {
			if (g->gpcclk_flcg_enabled) {
				change_mode = true;
				g->gpcclk_flcg_enabled = false;
			}
		}
	} else {
		nvgpu_log_fn(g,
			"GR elcg is disabled, cannot enable/disable GPCCLK FLCG");
	}

	if (!change_mode) {
		goto done;
	}
	err = nvgpu_pmu_gpcclk_flcg_enable_disable(g, enable);
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
	return err;
}

int nvgpu_cg_flcg_set_nvdclk_flcg_enabled(struct gk20a *g, bool enable)
{
	int err = 0;
	bool change_mode = false;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	/*
	 * NVDCLK FLCG depends upon Video ELCG so first
	 * check for all engines ELCG status
	 */
	if (!g->elcg_enabled) {
		nvgpu_err(g, "Global ELCG disabled, cannot update NVDCLK FLCG");
		goto done;
	}

	/* If FLCG is not supported, return */
	if (!nvgpu_is_enabled(g, NVGPU_GPU_CAN_FLCG)) {
		nvgpu_err(g, "FLCG not supported, cannot update NVDCLK FLCG");
		goto done;
	}

	/*
	 * if the elcg_engine_bitmask has atleast 1 video engine
	 * ELCG enabled, we can update/modify nvdclk flcg
	 */
	if (g->elcg_engine_bitmask & CG_ELCG_MM_ENGINE_ENABLED_MASK) {
		if (enable) {
			if (!g->nvdclk_flcg_enabled) {
				change_mode = true;
				g->nvdclk_flcg_enabled = true;
			}
		} else {
			if (g->nvdclk_flcg_enabled) {
				change_mode = true;
				g->nvdclk_flcg_enabled = false;
			}
		}
	} else {
		nvgpu_pmu_dbg(g,
			"video elcg disabled, cannot update NVDCLK FLCG");
	}

	if (!change_mode)
		goto done;

	err = nvgpu_pmu_nvdclk_flcg_enable_disable(g, enable);
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
	return err;
}

s32 nvgpu_cg_elcg_set_monitor_config(struct gk20a *g, u32 mode)
{
	s32 status = 0;

	g->ops.gr.init.wait_initialized(g);

	nvgpu_log_fn(g, " ");

	if (mode == ELCG_MON_ENTRY_COUNT_MODE) {
		/*
		 * skipping the operation if monitor mode is
		 * already set to entry count.
		 */
		if (g->elcg_ctrl->monitor_mode != ELCG_MON_ENTRY_COUNT_MODE) {
			status = nvgpu_pmu_elcg_monitor_config(g, mode);

			if (status != 0) {
				goto exit;
			}

			g->elcg_ctrl->monitor_mode = ELCG_MON_ENTRY_COUNT_MODE;
		}
	} else {
		/*
		 * skipping the operation if monitor mode is
		 * already set to residency.
		 */
		if (g->elcg_ctrl->monitor_mode != ELCG_MON_RESIDENCY_MODE) {
			status = nvgpu_pmu_elcg_monitor_config(g, mode);

			if (status != 0) {
				goto exit;
			}

			g->elcg_ctrl->monitor_mode = ELCG_MON_RESIDENCY_MODE;
		}
	}

	nvgpu_log_fn(g, "done");
exit:
	return status;
}

s32 nvgpu_cg_elcg_set_monitor_start(struct gk20a *g, u32 ctrl_id)
{
	s32 status = 0;
	u64 engine_bit = 1U;
	engine_bit = engine_bit << ctrl_id;

	g->ops.gr.init.wait_initialized(g);

	nvgpu_log_fn(g, " ");

	if (g->elcg_ctrl->monitor_mode == 0) {
		nvgpu_err(g, "Monitor mode is not set");
		goto exit;
	}

	if (!g->elcg_ctrl->monitor_started) {
		/**
		 * Here we are checking if ELCG is enabled for the passed
		 * ctrl_id. We check it by doing an AND operation between
		 * elcg_engine_bitmask and the mask for passed engine(ctrl_id).
		 */
		if ((g->elcg_engine_bitmask & engine_bit) != 0) {

			status = nvgpu_pmu_elcg_monitor_start(g, ctrl_id);
			if (status != 0) {
				goto exit;
			}

			g->elcg_ctrl->sub_feature_id = ctrl_id;
			g->elcg_ctrl->monitor_started = true;

		} else {
			nvgpu_err(g,
			"ELCG is not enabled for Sub Feature ID : %#X",
				ctrl_id);
			goto exit;
		}
	} else {
		nvgpu_err(g,
		"Monitor is already on for the Sub Feature ID : %#X",
			g->elcg_ctrl->sub_feature_id);
		goto exit;
	}

	nvgpu_log_fn(g, "done");
exit:
	return status;
}

s32 nvgpu_cg_elcg_set_monitor_stop(struct gk20a *g, u32 ctrl_id)
{
	s32 status = 0;

	g->ops.gr.init.wait_initialized(g);

	nvgpu_log_fn(g, " ");

	if (g->elcg_ctrl->monitor_started &&
		g->elcg_ctrl->sub_feature_id == ctrl_id) {

		status = nvgpu_pmu_elcg_monitor_stop(g, ctrl_id);
		if (status != 0) {
			goto exit;
		}

		g->elcg_ctrl->sub_feature_id = ctrl_id;
		g->elcg_ctrl->monitor_started = false;

	} else if (!g->elcg_ctrl->monitor_started) {
		nvgpu_err(g, "Monitor is not on for any engine.");
		goto exit;
	} else {
		nvgpu_err(g,
			"Monitor is currently on for the Sub Feature ID : %#X",
			g->elcg_ctrl->sub_feature_id);
		goto exit;
	}

	nvgpu_log_fn(g, "done");
exit:
	return status;
}

void nvgpu_cg_set_elcg_source(struct gk20a *g, u32 sub_feature_id)
{
	nvgpu_log_fn(g, " ");

	if (g->elcg_ctrl->sub_feature_id != sub_feature_id) {
		nvgpu_err(g,
		"ELCG Monitor engine doesn't match this sub_feature_id");
	} else {
		nvgpu_log_info(g, "ELCG source set successfully");
	}

	nvgpu_log_fn(g, "done");
}

u32 nvgpu_cg_get_elcg_residency(struct gk20a *g)
{
	u32 residency = 0;

	u64 engine_bit = 1;
	engine_bit = engine_bit << g->elcg_ctrl->sub_feature_id;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	/**
	 * This if statement checks if ELCG
	 * is on for the requested engine by
	 * checking the bit corresponding to the
	 * engine in the elcg_engine_bitmask.
	 */
	if (g->elcg_engine_bitmask & engine_bit) {

		/**
		 * This checks if the monitor mode
		 * is set to residency and also, if the
		 * monitor is started or not.
		 */
		if (g->elcg_ctrl->monitor_started &&
			g->elcg_ctrl->monitor_mode == ELCG_MON_RESIDENCY_MODE) {

			residency = nvgpu_pmu_elcg_residency(g);

		} else if (g->elcg_ctrl->monitor_mode !=
						ELCG_MON_RESIDENCY_MODE) {

			nvgpu_err(g, "Monitor mode is not set to residency");
			goto done;

		} else {
			nvgpu_err(g, "Monitor not started");
			goto done;
		}
	} else {
		nvgpu_err(g, "ELCG not enabled for %#X",
					g->elcg_ctrl->sub_feature_id);
		goto done;
	}

	nvgpu_log_fn(g, "done");
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
	return residency;
}

u32 nvgpu_cg_get_elcg_entry_count(struct gk20a *g)
{
	u32 entry_count = 0;

	u64 engine_bit = 1;
	engine_bit = engine_bit << g->elcg_ctrl->sub_feature_id;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	/**
	 * This if statement checks if ELCG
	 * is on for the requested engine by
	 * checking the bit corresponding to the
	 * engine in the elcg_engine_bitmask.
	 */
	if (g->elcg_engine_bitmask & engine_bit) {

		/**
		 * This checks if the monitor mode
		 * is set to entry_count and also, if the
		 * monitor is started or not.
		 */
		if (g->elcg_ctrl->monitor_started &&
			g->elcg_ctrl->monitor_mode ==
				ELCG_MON_ENTRY_COUNT_MODE){

			entry_count = nvgpu_pmu_elcg_entry_count(g);

		} else if (g->elcg_ctrl->monitor_mode !=
						ELCG_MON_ENTRY_COUNT_MODE) {

			nvgpu_err(g, "Monitor mode is not set to entry count");
			goto done;

		} else {
			nvgpu_err(g, "Monitor not started");
			goto done;
		}
	} else {
		nvgpu_err(g, "ELCG not enabled for %#X",
					g->elcg_ctrl->sub_feature_id);
		goto done;
	}

	nvgpu_log_fn(g, "done");
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
	return entry_count;
}

static int nvgpu_elcg_sub_feature_id_to_engine_id(struct gk20a *g,
							u32 sub_feature_id)
{
	int engine_id = -1;

	nvgpu_log_fn(g, " ");

	switch (sub_feature_id) {

	case CG_SUBFEATURE_ID_ELCG_CTRL_GR:
		engine_id = CG_ENGINE_ID_ELCG_GRAPHICS0;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_NVENC0:
		engine_id = CG_ENGINE_ID_ELCG_NVENC0;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_NVENC1:
		engine_id = CG_ENGINE_ID_ELCG_NVENC1;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_NVDEC0:
		engine_id = CG_ENGINE_ID_ELCG_NVDEC0;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_NVDEC1:
		engine_id = CG_ENGINE_ID_ELCG_NVDEC1;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_NVJPG0:
		engine_id = CG_ENGINE_ID_ELCG_NVJPG0;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_NVJPG1:
		engine_id = CG_ENGINE_ID_ELCG_NVJPG1;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_OFA0:
		engine_id = CG_ENGINE_ID_ELCG_OFA0;
		break;

	default:
		nvgpu_err(g, "Invalid Sub Feature ID %#X", sub_feature_id);
		break;
	}

	return engine_id;
}

static void nvgpu_cg_engine_elcg_set_mode(struct gk20a *g, u32 engine_id,
							u32 mode_config)
{
	struct nvgpu_pmu *pmu;
	u32 elcg_engine_status[CG_ELCG_CTRL_ENGINE_COUNT] = {0};
	u32 status = 0;

	if (!g->support_ls_pmu)
		return;

	pmu = g->pmu;

	nvgpu_log_fn(g, " ");

	if (pmu->pg->cg_elcg_state_change != NULL) {
		elcg_engine_status[engine_id] = mode_config;
		status = pmu->pg->cg_elcg_state_change(g, pmu,
				elcg_engine_status);
		if (status != 0) {
			nvgpu_err(g,
				"Failed to change elcg state=%d",
				status);
		}
	} else {

		/**
		 * if clock gating registers are moved to PMU
		 * call ELCG HAL from PMU
		 */
		if (g->ops.pmu.is_cg_supported_by_pmu != NULL &&
			g->ops.pmu.is_cg_supported_by_pmu(g) == true) {
			g->ops.pmu.pmu_init_elcg_mode(g, (u32)mode_config,
					engine_id);
		} else {
			g->ops.therm.init_elcg_mode(g, (u32)mode_config,
					engine_id);
		}
	}
	nvgpu_log_fn(g, "done");
}

void nvgpu_cg_elcg_set_engine_elcg_enable(struct gk20a *g,
			unsigned long new_engine_bitmask)
{
	unsigned int i;
	unsigned int engine_bit;
	unsigned int new_enable_status;
	int engine_id;

	/**
	 * This array holds the sub feature IDs of all
	 * the engines, this will cut the need to go through
	 * all the bits of the bitmask and we check bits which
	 * are actually mapped to an engine.
	 */
	u32 sub_feature_ids[] = {
		CG_SUBFEATURE_ID_ELCG_CTRL_GR,
		CG_SUBFEATURE_ID_ELCG_CTRL_NVDEC0,
		CG_SUBFEATURE_ID_ELCG_CTRL_NVDEC1,
		CG_SUBFEATURE_ID_ELCG_CTRL_NVENC0,
		CG_SUBFEATURE_ID_ELCG_CTRL_NVENC1,
		CG_SUBFEATURE_ID_ELCG_CTRL_NVJPG0,
		CG_SUBFEATURE_ID_ELCG_CTRL_NVJPG1,
		CG_SUBFEATURE_ID_ELCG_CTRL_OFA0
	};

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	if (nvgpu_is_enabled(g, NVGPU_GPU_CAN_ELCG)) {
		/**
		 * Disabling ELCG for all engines before
		 * enabling it for specific engines because some
		 * extra engines are present for which ELCG cannot
		 * be controlled specifically. So, we first disable
		 * it for all engines and then enable it for the
		 * requested ones.
		 */
		nvgpu_cg_elcg_set_elcg_enabled(g, false);

		nvgpu_mutex_acquire(&g->cg_pg_lock);

		// Traversing through each sub feature id one by one
		for (i = 0; i < ARRAY_SIZE(sub_feature_ids); i++) {

			/**
			 * This variable basically holds a mask
			 * with bit corresponding to current sub feature id
			 * set as 1 and rest of the bits set as 0.
			 */
			engine_bit = 1U;

			engine_bit = engine_bit << sub_feature_ids[i];

			new_enable_status = new_engine_bitmask & engine_bit;

			/**
			 * This function maps the Sub Feature IDs of
			 * the engines to their correspondig HW Engine
			 * IDs.
			 */
			engine_id =
				nvgpu_elcg_sub_feature_id_to_engine_id(
					g, sub_feature_ids[i]);

			if (engine_id == -1) {
				continue;
			}

			/**
			 * If new_enable_status is non zero then it means
			 * that the bit was set for current engine in the elcg
			 * engine bitmask and hence, we should enable elcg for
			 * it. If it is zero that it means that the elcg should
			 * be disabled for the current engine which is already
			 * done while disable elcg globally in this function
			 * earlier.
			 */
			if (new_enable_status != 0) {
				nvgpu_cg_engine_elcg_set_mode(g,
						(u32)engine_id, ELCG_AUTO);
			}
		}
		/**
		 * Doing this AND operation to clear any set bits
		 * which do not correspond to any engine.
		 */
		new_engine_bitmask = new_engine_bitmask &
				CG_ELCG_ALL_ENGINE_ENABLED_MASK;

		/* If FLCG is enabled, update nvdclk flcg as well */
		if (nvgpu_is_enabled(g, NVGPU_GPU_CAN_FLCG)) {
			/*
			 * if the new_engine_bitmask has atleast 1 video engine
			 * ELCG enabled, we can enable nvdclk flcg
			 */
			if (new_engine_bitmask & CG_ELCG_MM_ENGINE_ENABLED_MASK) {
				if (!g->nvdclk_flcg_enabled) {
					/*
					 * To-Do: here, we should send rpc
					 * to PMU to enable nvdclk flcg
					 * this runtime enable support is yet
					 * to be added on PMU
					 */
					g->nvdclk_flcg_enabled = true;
				}
			} else {
				/*
				 * keep nvdclk flcg disabled if new_engine_bitmask
				 * has no video engines support
				 */
				if (g->nvdclk_flcg_enabled) {
					/*
					 * To-Do: here, we should send rpc
					 * to PMU to disable nvdclk flcg
					 * this runtime disable support is yet
					 * to be added on PMU
					 */
					g->nvdclk_flcg_enabled = false;
				}
			}
		}

		g->elcg_engine_bitmask = new_engine_bitmask;

		/**
		 * Set the global ELCG flag to true after updating
		 * elcg_engine_bitmask
		 */
		if (g->elcg_engine_bitmask) {
			g->elcg_enabled = true;
		}

		nvgpu_mutex_release(&g->cg_pg_lock);

	} else {
		nvgpu_err(g, "ELCG is globally disabled");
		return;
	}

	nvgpu_log_fn(g, "done");
}

int nvgpu_cg_set_sysclk_slowdown_enabled(struct gk20a *g, bool enable)
{
	int err = 0;
	bool change_mode = false;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	if (enable) {
		if (!g->sysclk_slowdown_enabled) {
			change_mode = true;
			g->sysclk_slowdown_enabled = true;
		}
	} else {
		if (g->sysclk_slowdown_enabled) {
			change_mode = true;
			g->sysclk_slowdown_enabled = false;
		}
	}

	if (!change_mode) {
		goto done;
	}
	err = nvgpu_pmu_sysclk_slowdown_enable_disable(g, enable);
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
	return err;
}

u32 nvgpu_cg_get_sysclk_slowdown_entry_count(struct gk20a *g)
{
	u32 entry_count = 0;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	/**
	 * This if statement checks if SYSCLK Slowdown is enabled
	 */
	if (g->sysclk_slowdown_enabled) {

		entry_count = nvgpu_pmu_sysclk_slowdown_entry_count(g);

	} else {
		nvgpu_err(g, "SYSCLK Slowdown not enabled");
		goto done;
	}

	nvgpu_log_fn(g, "done");
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
	return entry_count;
}
#endif

void nvgpu_cg_elcg_ce_load_enable(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (!g->elcg_enabled) {
		goto done;
	}
	if (g->ops.cg.elcg_ce_load_gating_prod != NULL) {
		g->ops.cg.elcg_ce_load_gating_prod(g, true);
	}
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
}

int nvgpu_cg_elcg_ctrl_init(struct gk20a *g)
{
	int err = 0;
	struct elcg_ctrl_params *elcg_ctrl;

	nvgpu_log_fn(g, " ");

	if (g->elcg_ctrl != NULL) {
		// Skipping re-allocation of memory for the struct.
		goto exit;
	}

	elcg_ctrl = (struct elcg_ctrl_params *) nvgpu_kzalloc(g,
					sizeof(struct elcg_ctrl_params));

	if (elcg_ctrl == NULL) {
		err = -ENOMEM;
		goto exit;
	}

	g->elcg_ctrl = elcg_ctrl;

	/**
	 * Initializing all the member variables of
	 * the elcg_ctrl_params struct with default
	 * values of 0 and false.
	 */
	elcg_ctrl->sub_feature_id = 0;
	elcg_ctrl->monitor_mode = 0;
	elcg_ctrl->monitor_started = false;
	elcg_ctrl->entry_count = 0;
	elcg_ctrl->residency = 0;

exit:
	return err;
}

void nvgpu_cg_elcg_ctrl_deinit(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_kfree(g, g->elcg_ctrl);

	g->elcg_ctrl = NULL;

	nvgpu_log_fn(g, "done");
}
