// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#ifdef CONFIG_NVGPU_LS_PMU
#include <nvgpu/pmu.h>
#include <nvgpu/pmu/pmu_pg.h>
#endif
#include <nvgpu/power_features/pg.h>
#include <nvgpu/pmu/pmuif/pg.h>

bool nvgpu_pg_elpg_is_enabled(struct gk20a *g)
{
	bool elpg_enabled;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	elpg_enabled = g->elpg_enabled;
	nvgpu_mutex_release(&g->cg_pg_lock);
	return elpg_enabled;
}

int nvgpu_pg_elpg_enable(struct gk20a *g)
{
	int err = 0;
#ifdef CONFIG_NVGPU_LS_PMU

	if (!g->can_elpg) {
		return 0;
	}

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (g->elpg_enabled) {
		err = nvgpu_pmu_pg_global_enable(g, true);
	}
	nvgpu_mutex_release(&g->cg_pg_lock);
#endif
	return err;
}

int nvgpu_pg_elpg_disable(struct gk20a *g)
{
	int err = 0;
#ifdef CONFIG_NVGPU_LS_PMU

	if (!g->can_elpg) {
		return 0;
	}

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (g->elpg_enabled) {
		err = nvgpu_pmu_pg_global_enable(g, false);
	}
	nvgpu_mutex_release(&g->cg_pg_lock);
#endif
	return err;
}

int nvgpu_pg_elpg_set_elpg_enabled(struct gk20a *g, bool enable)
{
	int err = 0;
	bool change_mode = false;

	nvgpu_log_fn(g, " ");

	if (!g->can_elpg) {
		return 0;
	}

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	if (enable) {
		if (!g->elpg_enabled) {
			change_mode = true;
			g->elpg_enabled = true;
		}
	} else {
		if (g->elpg_enabled) {
			change_mode = true;
			g->elpg_enabled = false;
		}
	}
	if (!change_mode) {
		goto done;
	}
#ifdef CONFIG_NVGPU_LS_PMU
	err = nvgpu_pmu_pg_global_enable(g, enable);
#endif
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
	return err;
}

bool nvgpu_pg_elpg_ms_is_enabled(struct gk20a *g)
{
	bool elpg_ms_enabled;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);
	elpg_ms_enabled = g->elpg_ms_enabled;
	nvgpu_mutex_release(&g->cg_pg_lock);
	return elpg_ms_enabled;
}

int nvgpu_pg_elpg_ms_enable(struct gk20a *g)
{
	int err = 0;
#ifdef CONFIG_NVGPU_LS_PMU
	nvgpu_log_fn(g, " ");

	if (!g->can_elpg && !g->can_elpg_ms) {
		return 0;
	}

	/*
	 * If elpg is enabled and elpg_ms is NOT enabled
	 * and pg is initialized then only we wait
	 * for gr init and then enable ELPG_MS feature.
	 * In rmmod path, the gr struct could be freed earlier.
	 * In order to avoid NULL access for gr, we check for these
	 * conditions then proceed further.
	 */
	if ((g->elpg_enabled) && (!g->elpg_ms_enabled) &&
		(g->pmu->pg->initialized)) {
		g->ops.gr.init.wait_initialized(g);
		nvgpu_mutex_acquire(&g->cg_pg_lock);
		g->elpg_ms_enabled = true;
		err = nvgpu_pmu_enable_elpg_ms(g);
		nvgpu_mutex_release(&g->cg_pg_lock);
	}
#endif
	return err;
}

int nvgpu_pg_elpg_ms_disable(struct gk20a *g)
{
	int err = 0;
#ifdef CONFIG_NVGPU_LS_PMU
	nvgpu_log_fn(g, " ");

	if (!g->can_elpg && !g->can_elpg_ms) {
		return 0;
	}

	/*
	 * If elpg and elpg_ms flags are set to true
	 * then only we check further conditions.
	 */
	if ((g->elpg_enabled) && (g->elpg_ms_enabled) &&
		(g->pmu->pg->initialized)) {
		g->ops.gr.init.wait_initialized(g);
		nvgpu_mutex_acquire(&g->cg_pg_lock);
		g->elpg_ms_enabled = false;
		err = nvgpu_pmu_disable_elpg_ms(g);
		nvgpu_mutex_release(&g->cg_pg_lock);
	}
#endif
	return err;
}

int nvgpu_pg_fgpg_set_fgpg_enabled(struct gk20a *g, bool enable)
{
	int err = 0;
	bool change_mode = false;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	/**
	 * ELCG is a pre-requisite of FG-RPPG,
	 * FG-RPPG can only be engaged if ELCG
	 * is engaged.
	 */
	if (g->elcg_enabled) {
		if (enable) {
			if (!g->fgpg_enabled) {
				change_mode = true;
				g->fgpg_enabled = true;
			}
		} else {
			if (g->fgpg_enabled) {
				change_mode = true;
				g->fgpg_enabled = false;
			}
		}
	} else {
		nvgpu_log_fn(g,
			"elcg is disabled, cannot enable/disable FG-RPPG");
	}

	if (!change_mode)
		goto done;

	err = nvgpu_pmu_fgpg_enable_disable(g, enable);
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
	return err;
}

int nvgpu_pg_fgpg_set_engine_fgpg_enable(struct gk20a *g,
			unsigned long new_engine_bitmask)
{
	int err = 0;

	nvgpu_log_fn(g, " ");

	g->ops.gr.init.wait_initialized(g);

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	/**
	 * ELCG is a pre-requisite of FG-RPPG,
	 * FG-RPPG can only be engaged if ELCG
	 * is engaged.
	 */
	if (g->elcg_enabled) {
		if ((g->fgpg_supported_engmask &
					new_engine_bitmask) == new_engine_bitmask) {
			if (new_engine_bitmask != g->fgpg_engine_bitmask) {
				nvgpu_pmu_fgpg_engine_enable_disable(g, new_engine_bitmask);
			} else {
				nvgpu_log_fn(g, "Same FGPG bitmask is already enabled");
			}
		} else {
			nvgpu_log_fn(g, "FGPG is not supported for the selected bitmask");
		}
	} else {
		nvgpu_log_fn(g,
			"elcg is disabled, cannot enable/disable FG-RPPG");
	}

	nvgpu_mutex_release(&g->cg_pg_lock);
	return err;
}

bool nvgpu_pg_feature_status(struct gk20a *g, u32 lpwr_mon_index)
{
	bool status = false;

	switch(lpwr_mon_index) {
	case PMU_LPWR_MON_SOURCE_ID_FLCG_GPCCLK_INDEX_ID:
		if (g->gpcclk_flcg_enabled) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "GPCCLK FLCG not enabled");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_FLCG_NVDCLK_INDEX_ID:
		if (g->nvdclk_flcg_enabled) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "NVDCLK FLCG not enabled");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_FGPG_GR_INDEX_ID:
	case PMU_LPWR_MON_SOURCE_ID_FGPG_NVENC0_INDEX_ID:
	case PMU_LPWR_MON_SOURCE_ID_FGPG_NVENC1_INDEX_ID:
	case PMU_LPWR_MON_SOURCE_ID_FGPG_NVDEC0_INDEX_ID:
	case PMU_LPWR_MON_SOURCE_ID_FGPG_NVDEC1_INDEX_ID:
	case PMU_LPWR_MON_SOURCE_ID_FGPG_NVJPG0_INDEX_ID:
	case PMU_LPWR_MON_SOURCE_ID_FGPG_NVJPG1_INDEX_ID:
	case PMU_LPWR_MON_SOURCE_ID_FGPG_OFA0_INDEX_ID:
		if (g->fgpg_enabled) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "FGRPPG not enabled");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_ELCG_GR_INDEX_ID:
		if (g->elcg_enabled && g->elcg_engine_bitmask & (1U << CG_SUBFEATURE_ID_ELCG_CTRL_GR)) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "ELCG not enabled for GR");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_ELCG_NVENC0_INDEX_ID:
		if (g->elcg_enabled &&  g->elcg_engine_bitmask & (1U << CG_SUBFEATURE_ID_ELCG_CTRL_NVENC0)) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "ELCG not enabled for NVENC0");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_ELCG_NVENC1_INDEX_ID:
		if (g->elcg_enabled &&  g->elcg_engine_bitmask & (1U << CG_SUBFEATURE_ID_ELCG_CTRL_NVENC1)) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "ELCG not enabled for NVENC1");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_ELCG_NVDEC0_INDEX_ID:
		if (g->elcg_enabled &&  g->elcg_engine_bitmask & (1U << CG_SUBFEATURE_ID_ELCG_CTRL_NVDEC0)) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "ELCG not enabled for NVDEC0");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_ELCG_NVDEC1_INDEX_ID:
		if (g->elcg_enabled &&  g->elcg_engine_bitmask & (1U << CG_SUBFEATURE_ID_ELCG_CTRL_NVDEC1)) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "ELCG not enabled for NVDEC1");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_ELCG_NVJPG0_INDEX_ID:
		if (g->elcg_enabled &&  g->elcg_engine_bitmask & (1U << CG_SUBFEATURE_ID_ELCG_CTRL_NVJPG0)) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "ELCG not enabled for NVJPG0");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_ELCG_NVJPG1_INDEX_ID:
		if (g->elcg_enabled &&  g->elcg_engine_bitmask & (1U << CG_SUBFEATURE_ID_ELCG_CTRL_NVJPG1)) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "ELCG not enabled for NVJPG1");
		}
		break;
	case PMU_LPWR_MON_SOURCE_ID_ELCG_OFA0_INDEX_ID:
		if (g->elcg_enabled &&  g->elcg_engine_bitmask & (1U << CG_SUBFEATURE_ID_ELCG_CTRL_OFA0)) {
			status = true;
		} else {
			nvgpu_pmu_dbg(g, "ELCG not enabled for OFA0");
		}
		break;
	default:
		nvgpu_err(g, "Invalid LPWR_MON_INDEX: %d", lpwr_mon_index);
		status = false;
	}
	return status;
}

s32 nvgpu_pg_lpwr_lp_mon_start_stop(struct gk20a *g, u32 lpwr_mon_index,
		bool start)
{
	struct lpwr_mon_params *lpwr_mon;
	s32 status = 0;

	if (!g->support_ls_pmu || (g->pmu == NULL)) {
		return 0;
	}

	lpwr_mon = &g->pmu->lpwr_mon;

	g->ops.gr.init.wait_initialized(g);

	nvgpu_log_fn(g, " ");

	/*
	 * Check for mismatch cases btw "monitor_started" and
	 * "start" params, then skip the request
	 */
	if (lpwr_mon->monitor_started && start) {
		if (lpwr_mon->lpwr_mon_index == lpwr_mon_index) {
			nvgpu_info(g, "lpwr monitor for same LPWR_MON index: %d "
					"already started", lpwr_mon->lpwr_mon_index);
		} else {
			nvgpu_err(g, "lpwr monitor for diff LPWR_MON index: %d "
					"already started, stop first", lpwr_mon->lpwr_mon_index);
		}
		goto exit;
	}

	if (lpwr_mon->monitor_started && !start) {
		if (lpwr_mon->lpwr_mon_index != lpwr_mon_index) {
			nvgpu_err(g, "lpwr monitor was started for diff "
					"LPWR_MON index: %d, stop first", lpwr_mon->lpwr_mon_index);
			goto exit;
		}
	}

	if (!lpwr_mon->monitor_started && !start) {
		nvgpu_info(g, "lpwr monitor already stopped");
		goto exit;
	}

	/*
	 * for "start" request, check if the feature corresponding to
	 * "lpwr_mon_index" is enabled or not
	 */
	if (start) {
		status = nvgpu_pg_feature_status(g, lpwr_mon_index);
		if (!status) {
			goto exit;
		}
	}

	/* start/stop lpwr_mon for the requested index */
	status = nvgpu_pmu_lpwr_lp_mon_start(g, lpwr_mon_index, start);
	if (status != 0) {
		goto exit;
	}

	/* update the current lpwr_mon_index and monitor started values */
	lpwr_mon->lpwr_mon_index = lpwr_mon_index;
	lpwr_mon->monitor_started = start;

	nvgpu_pmu_dbg(g, "LPWR_MON INDEX : %d, Monitor Started : %d\n",
			lpwr_mon->lpwr_mon_index,
			lpwr_mon->monitor_started ? 1 : 0);

	nvgpu_log_fn(g, "done");
exit:
	return status;
}

u32 nvgpu_pg_get_lpwr_lp_residency(struct gk20a *g)
{
	u32 residency = 0;
	struct lpwr_mon_params *lpwr_mon;
	int status = 0;

	if (!g->support_ls_pmu || (g->pmu == NULL)) {
		return 0;
	}

	lpwr_mon = &g->pmu->lpwr_mon;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->cg_pg_lock);

	/*
	 * check if the feature corresponding to
	 * lpwr_mon_index is enabled or not
	 */
	status = nvgpu_pg_feature_status(g, lpwr_mon->lpwr_mon_index);
	if (!status) {
		goto done;
	}

	if (lpwr_mon->monitor_started) {
		residency = nvgpu_pmu_lpwr_lp_get_residency(g,
				lpwr_mon->lpwr_mon_index);
		/* Update the lpwr_mon struct */
		lpwr_mon->residency = residency;
	} else {
		nvgpu_err(g, "Monitor not started");
		goto done;
	}

	nvgpu_log_fn(g, "done");
done:
	nvgpu_mutex_release(&g->cg_pg_lock);
	return residency;
}
