// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/pmu/pmu_pg.h>
#include <nvgpu/pmu/pmuif/pg.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/soc.h>
#include <nvgpu/power_features/pg.h>

#include "common/pmu/pg/pmu_pg.h"
#include "common/pmu/pg/pg_sw_gm20b.h"
#include "common/pmu/pg/pg_sw_ga10b.h"
#include "common/pmu/pg/pg_sw_gb10b.h"
#include "common/pmu/pg/pg_sw_gb20c.h"
#include "pg_sw_gb20c.h"

u32 gb20c_pmu_pg_engines_list(struct gk20a *g)
{
	u32 engines_list = BIT32(PMU_PG_ELPG_ENGINE_ID_GRAPHICS);

	if (nvgpu_is_enabled(g, NVGPU_ELPG_MS_ENABLED)) {
		engines_list |= BIT32(PMU_PG_ELPG_ENGINE_ID_MS_LTC);
	}
	if (nvgpu_is_enabled(g, NVGPU_GPC_RG_ENABLED)) {
		engines_list |= BIT32(PMU_PG_ELPG_ENGINE_ID_GR_RG);
	}

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MULTIMEDIA)) {
		if (nvgpu_is_enabled(g, NVGPU_NVDEC_ELPG_ENABLED)) {
			engines_list |= BIT32(PMU_PG_ELPG_ENGINE_ID_VID_NVDEC);
		}
		if (nvgpu_is_enabled(g, NVGPU_NVENC_ELPG_ENABLED)) {
			engines_list |= BIT32(PMU_PG_ELPG_ENGINE_ID_VID_NVENC);
		}
		if (nvgpu_is_enabled(g, NVGPU_OFA_ELPG_ENABLED)) {
			engines_list |= BIT32(PMU_PG_ELPG_ENGINE_ID_VID_OFA);
		}
	}
	return engines_list;
}

int gb20c_pmu_pg_pre_init(struct gk20a *g, struct nvgpu_pmu *pmu)
{
	struct pmu_rpc_struct_lpwr_loading_pre_init rpc;
	int status;
	u32 idx;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
		sizeof(struct pmu_rpc_struct_lpwr_loading_pre_init));

	/* ARCH Support MAsk */
	if (!nvgpu_platform_is_silicon(g)) {
		/* We need to have Idle threshold check only for Silicon */
		rpc.arch_sf_support_mask =
			NV_PMU_LPWR_FSM_FEATURE_ID_IDLE_SNAP_DBG |
			NV_PMU_LPWR_FSM_FEATURE_ID_LPWR_LP_IDLE_SNAP_NOTIFY;
	} else {

		rpc.arch_sf_support_mask =
			NV_PMU_LPWR_FSM_FEATURE_ID_IDLE_SNAP_DBG |
			NV_PMU_LPWR_FSM_FEATURE_ID_IDLE_THRESHOLD_CHECK |
			NV_PMU_LPWR_FSM_FEATURE_ID_LPWR_LP_IDLE_SNAP_NOTIFY;
	}

	rpc.base_period_ms = NV_PMU_PG_AP_BASE_PERIOD_MS;
	rpc.b_no_pstate_vbios = true;

	/* Initialize LPWR GR/MS/VID grp data for GRAPHICS/MS_LTC/VID engine */
	for (idx = 0; idx < NV_PMU_LPWR_GRP_CTRL_ID__COUNT; idx++) {

		if (idx == NV_PMU_LPWR_GRP_CTRL_ID_GR) {
			rpc.grp_ctrl_mask[idx] =
				BIT(PMU_PG_ELPG_ENGINE_ID_GRAPHICS);
		}

		if (nvgpu_is_enabled(g, NVGPU_ELPG_MS_ENABLED)) {
			if (idx == NV_PMU_LPWR_GRP_CTRL_ID_MS) {
				rpc.grp_ctrl_mask[idx] =
					BIT(PMU_PG_ELPG_ENGINE_ID_MS_LTC);
			}
		}

		if (nvgpu_is_enabled(g, NVGPU_GPC_RG_ENABLED)) {
			if (idx == NV_PMU_LPWR_GRP_CTRL_ID_GR) {
				rpc.grp_ctrl_mask[idx] |=
					BIT(PMU_PG_ELPG_ENGINE_ID_GR_RG);
			}
		}

		if (nvgpu_is_enabled(g, NVGPU_NVDEC_ELPG_ENABLED)) {
			if (idx == NV_PMU_LPWR_GRP_CTRL_ID_VID) {
				rpc.grp_ctrl_mask[idx] |=
					BIT(PMU_PG_ELPG_ENGINE_ID_VID_NVDEC);
			}
		}

		if (nvgpu_is_enabled(g, NVGPU_NVENC_ELPG_ENABLED)) {
			if (idx == NV_PMU_LPWR_GRP_CTRL_ID_VID) {
				rpc.grp_ctrl_mask[idx] |=
					BIT(PMU_PG_ELPG_ENGINE_ID_VID_NVENC);
			}
		}

		if (nvgpu_is_enabled(g, NVGPU_OFA_ELPG_ENABLED)) {
			if (idx == NV_PMU_LPWR_GRP_CTRL_ID_VID) {
				rpc.grp_ctrl_mask[idx] |=
					BIT(PMU_PG_ELPG_ENGINE_ID_VID_OFA);
			}
		}
	}

	PMU_RPC_EXECUTE_CPB(status, pmu, PG_LOADING, PRE_INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb20c_pmu_pg_init(struct gk20a *g, struct nvgpu_pmu *pmu,
		u8 pg_engine_id)
{
	struct pmu_rpc_struct_lpwr_loading_pg_ctrl_init rpc;
	int status;

	nvgpu_log_fn(g, " ");

	/* init ELPG */
	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_loading_pg_ctrl_init));
	rpc.ctrl_id = (u32)pg_engine_id;
	/* GR sub-Feature mask */
	if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_GRAPHICS) {
		/* set the default mask */
		rpc.support_mask = NVGPU_PMU_GR_FEATURE_MASK_DEFAULT_GB10B;
		if (g->gpcclk_flcg_enabled) {
			/* append the mask if GPCCLK FLCG is enabled */
			rpc.support_mask |=
				NVGPU_PMU_GR_FEATURE_MASK_FLCG_GPCCLK;
		}
	}
	/* ELPG_MS sub-feature mask */
	if (nvgpu_is_enabled(g, NVGPU_ELPG_MS_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_MS_LTC) {
			rpc.support_mask =
				NV_PMU_LPWR_FSM_CTRL_MS_LTC_FEATURE_MASK_ALL;
		}
	}
	/* GPC_RG sub-feature mask */
	if (nvgpu_is_enabled(g, NVGPU_GPC_RG_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_GR_RG) {
			rpc.support_mask =
				NV_PMU_LPWR_FSM_CTRL_GR_RG_FEATURE_MASK_ALL;
		}
	}
	/* NVDEC_ELPG sub-feature mask */
	if (nvgpu_is_enabled(g, NVGPU_NVDEC_ELPG_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_VID_NVDEC) {
			rpc.support_mask =
				NV_PMU_LPWR_FSM_CTRL_VID_NVDEC_PG_FEATURE_MASK_ALL;
		}
	}
	/* NVENC_ELPG sub-feature mask */
	if (nvgpu_is_enabled(g, NVGPU_NVENC_ELPG_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_VID_NVENC) {
			rpc.support_mask =
				NV_PMU_LPWR_FSM_CTRL_VID_NVENC_PG_FEATURE_MASK_ALL;
		}
	}
	/* OFA_ELPG sub-feature mask */
	if (nvgpu_is_enabled(g, NVGPU_OFA_ELPG_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_VID_OFA) {
			rpc.support_mask =
				NV_PMU_LPWR_FSM_CTRL_VID_OFA_PG_FEATURE_MASK_ALL;
		}
	}

	PMU_RPC_EXECUTE_CPB(status, pmu, PG_LOADING, INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x",
			status);
	}

	return status;
}

int gb20c_pmu_pg_threshold_update(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 pg_engine_id)
{
	struct pmu_rpc_struct_lpwr_pg_ctrl_threshold_update rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
		sizeof(struct pmu_rpc_struct_lpwr_pg_ctrl_threshold_update));
	rpc.ctrl_id = (u32)pg_engine_id;

#ifdef CONFIG_NVGPU_SIM
	if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)) {
		rpc.threshold_cycles.idle = PMU_PG_IDLE_THRESHOLD_SIM;
		rpc.threshold_cycles.ppu = PMU_PG_POST_POWERUP_IDLE_THRESHOLD_SIM;
	} else
#endif
	{
		switch (pg_engine_id) {
		case PMU_PG_ELPG_ENGINE_ID_GRAPHICS:
		case PMU_PG_ELPG_ENGINE_ID_VID_NVDEC:
		case PMU_PG_ELPG_ENGINE_ID_VID_NVENC:
		case PMU_PG_ELPG_ENGINE_ID_VID_OFA:
			rpc.threshold_cycles.idle = PMU_PG_IDLE_THRESHOLD;
			rpc.threshold_cycles.ppu = PMU_PG_POST_POWERUP_IDLE_THRESHOLD;
			break;
		case PMU_PG_ELPG_ENGINE_ID_GR_RG:
			rpc.threshold_cycles.idle = PMU_PG_IDLE_THRESHOLD_GPC_RG;
			rpc.threshold_cycles.ppu = PMU_PG_POST_POWERUP_IDLE_THRESHOLD_GPC_RG;
			break;
		default:
			rpc.threshold_cycles.idle = PMU_PG_IDLE_THRESHOLD;
			rpc.threshold_cycles.ppu = PMU_PG_POST_POWERUP_IDLE_THRESHOLD;
			break;
		}
	}

	nvgpu_pmu_dbg(g, "pg_engine_id=0x%x, idle_threshold=0x%x, ppu_threshold=0x%x",
		pg_engine_id, rpc.threshold_cycles.idle, rpc.threshold_cycles.ppu);

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, THRESHOLD_UPDATE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x",
			status);
	}

	return status;
}

int gb20c_pmu_pg_sfm_update(struct gk20a *g,
	struct nvgpu_pmu *pmu, u8 pg_engine_id, bool enable)
{
	struct pmu_rpc_struct_lpwr_pg_ctrl_sfm_update rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
		sizeof(struct pmu_rpc_struct_lpwr_pg_ctrl_sfm_update));

	rpc.ctrl_id = (u32)pg_engine_id;

	if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_GRAPHICS) {
		rpc.enabled_mask =
			NV_PMU_SUB_FEATURE_SUPPORT_GPCCLK_FLCG_MASK(enable);
	}

	if (nvgpu_is_enabled(g, NVGPU_ELPG_MS_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_MS_LTC) {
			rpc.enabled_mask =
				NV_PMU_LPWR_FSM_CTRL_MS_LTC_FEATURE_MASK_ALL;
		}
	}

	if (nvgpu_is_enabled(g, NVGPU_GPC_RG_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_GR_RG) {
			rpc.enabled_mask =
				NV_PMU_LPWR_FSM_CTRL_GR_RG_FEATURE_MASK_ALL;
		}
	}

	if (nvgpu_is_enabled(g, NVGPU_NVDEC_ELPG_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_VID_NVDEC) {
			rpc.enabled_mask =
				NV_PMU_LPWR_FSM_CTRL_VID_NVDEC_PG_FEATURE_MASK_ALL;
		}
	}

	if (nvgpu_is_enabled(g, NVGPU_NVENC_ELPG_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_VID_NVENC) {
			rpc.enabled_mask =
				NV_PMU_LPWR_FSM_CTRL_VID_NVENC_PG_FEATURE_MASK_ALL;
		}
	}

	if (nvgpu_is_enabled(g, NVGPU_OFA_ELPG_ENABLED)) {
		if (pg_engine_id == PMU_PG_ELPG_ENGINE_ID_VID_OFA) {
			rpc.enabled_mask =
				NV_PMU_LPWR_FSM_CTRL_VID_OFA_PG_FEATURE_MASK_ALL;
		}
	}

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, SFM_UPDATE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x",
			status);
	}

	return status;
}

int gb20c_pmu_pg_fgpg_init(struct gk20a *g,
		struct nvgpu_pmu *pmu)
{
	struct pmu_rpc_struct_lpwr_loading_seq_fgpg_init rpc;
	int status;

	nvgpu_log_fn(g, " ");

	if (!g->fgpg_enabled) {
		nvgpu_pmu_dbg(g, "FGPG not enabled");
		return 0;
	}

	(void) memset(&rpc, 0,
		sizeof(struct pmu_rpc_struct_lpwr_loading_seq_fgpg_init));

	rpc.fgpg_supported = true;
	/* enable GR-FGRPPG */
	rpc.support_mask = BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_GR);

	/* if multimedia flasg is set then only enable FGRPPG for MM engines */
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MULTIMEDIA)) {
		rpc.support_mask |= BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_NVDEC0) |
			BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_NVENC0) |
			BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_OFA);
	}

	g->fgpg_supported_engmask = rpc.support_mask;

	g->fgpg_engine_bitmask = nvgpu_pg_get_fgpg_all_engine_mask(g);

	PMU_RPC_EXECUTE_CPB(status, pmu, PG_LOADING, SEQ_FGPG_INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb20c_pmu_pg_cg_elcg_state_change(struct gk20a *g,
		struct nvgpu_pmu *pmu, u32 elcg_engine_status[])
{
	struct pmu_rpc_struct_lpwr_cg_elcg_state_change rpc;
	const struct nvgpu_device *dev = NULL;
	struct nvgpu_fifo *f = &g->fifo;
	int status;
	u32 n;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
		sizeof(struct pmu_rpc_struct_lpwr_cg_elcg_state_change));

	for (n = 0; n < f->num_engines; n++) {
		dev = f->active_engines[n];

		/* if MM support is disabled skip ELCG for MM engines */
		if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_MULTIMEDIA)) {
			if (dev->engine_id == CG_ENGINE_ID_ELCG_NVDEC0_GB20C ||
			dev->engine_id == CG_ENGINE_ID_ELCG_NVENC0_GB20C ||
			dev->engine_id == CG_ENGINE_ID_ELCG_OFA0_GB20C) {
				/* do not modify ELCG */
				continue;
			}
		}
		if (elcg_engine_status[dev->engine_id] == ELCG_AUTO) {
			rpc.enable_req[dev->engine_id] = true;
		} else if (elcg_engine_status[dev->engine_id] == ELCG_RUN) {
			rpc.disable_req[dev->engine_id] = true;
		}
	}

	g->elcg_engine_bitmask = g->elcg_enabled ?
			nvgpu_cg_get_elcg_all_engine_mask(g) : 0;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CG_ELCG_STATE_CHANGE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

void nvgpu_gb20c_pg_sw_init(struct gk20a *g,
		struct nvgpu_pmu_pg *pg)
{
	nvgpu_log_fn(g, " ");

	pg->elpg_statistics = ga10b_pmu_elpg_statistics;
	pg->init_param = NULL;
	pg->supported_engines_list = gb20c_pmu_pg_engines_list;
	pg->engines_feature_list = NULL;
	pg->set_sub_feature_mask = NULL;
	pg->save_zbc = gm20b_pmu_save_zbc;
	pg->allow = ga10b_pmu_pg_allow;
	pg->disallow = ga10b_pmu_pg_disallow;
	pg->init = gb20c_pmu_pg_init;
	pg->alloc_dmem = NULL;
	pg->load_buff = NULL;
	pg->hw_load_zbc = NULL;
	pg->aelpg_init = ga10b_pmu_pg_aelpg_init;
	pg->aelpg_init_and_enable = ga10b_pmu_pg_aelpg_init_and_enable;
	pg->aelpg_enable = ga10b_pmu_pg_aelpg_enable;
	pg->aelpg_disable = ga10b_pmu_pg_aelpg_disable;
	pg->pg_loading_rpc_handler = ga10b_pg_loading_rpc_handler;
	pg->pg_rpc_handler = ga10b_pg_rpc_handler;
	pg->init_send = gb10b_pmu_pg_init_send;
	pg->pre_init = gb20c_pmu_pg_pre_init;
	pg->post_init = gb10b_pmu_pg_post_init;
	pg->process_pg_event = ga10b_pmu_pg_process_pg_event;
	pg->pmu_get_pg_buf_gpu_va = gb10b_pmu_get_pg_buf_gpu_va;
	pg->pmu_get_pg_buf_gpu_va_hi = gb10b_pmu_get_pg_buf_gpu_va_hi;
	pg->pmu_pg_threshold_update = gb20c_pmu_pg_threshold_update;
	pg->pmu_pg_sfm_update = gb20c_pmu_pg_sfm_update;
	pg->fgpg_init = gb20c_pmu_pg_fgpg_init;
	pg->fgpg_state_change = gb10b_pmu_pg_fgpg_state_change;
	pg->fgpg_threshold_update = gb10b_pmu_pg_fgpg_threshold_update;
	pg->priv_blocker_init = gb10b_pmu_pg_priv_blocker_init;
	pg->cg_elcg_get_entry_count = gb10b_pmu_pg_cg_elcg_get_entry_count;
	pg->cg_elcg_get_residency = gb10b_pmu_pg_cg_elcg_get_residency;
	pg->cg_elcg_mon_start = gb10b_pmu_pg_cg_elcg_mon_start;
	pg->cg_elcg_mon_config = gb10b_pmu_pg_cg_elcg_mon_config;
	pg->flcg_init = gb10b_pmu_pg_cg_flcg_init;
	pg->cg_flcg_ctrl_enable = gb10b_pmu_pg_cg_flcg_ctrl_enable;
	pg->cg_flcg_ctrl_threshold_update = gb10b_pmu_pg_cg_flcg_ctrl_threshold_update;
	pg->lpwr_lp_rpc_handler = gb10b_pmu_pg_lpwr_lp_rpc_handler;
	pg->lpwr_lp_mon_start = gb10b_pmu_pg_lpwr_lp_mon_start;
	pg->lpwr_lp_get_residency = gb10b_pmu_pg_lpwr_lp_get_residency;
	pg->cg_elcg_state_change = gb20c_pmu_pg_cg_elcg_state_change;
	pg->csd_ctrl_enable = gb10b_pmu_pg_cg_csd_ctrl_enable;
/* On FPGA (not for SCSIM) if CSD INIT is triggered, PMU halt happens at PMU code:
lpwrCsdSysClkInit_GB20B(void)
regVal = THERM_REG_RD32(NV_CPWR_THERM_INTR_MONITOR_CTRL(pLpwrCsdCtrl->monIndex));
*/
//	pg->csd_init = gb10b_pmu_pg_cg_csd_init;
	pg->csd_ctrl_threshold_update =
					gb10b_pmu_pg_cg_csd_ctrl_threshold_update;
	pg->csd_get_entry_count = gb10b_pmu_pg_cg_csd_get_entry_count;
}
