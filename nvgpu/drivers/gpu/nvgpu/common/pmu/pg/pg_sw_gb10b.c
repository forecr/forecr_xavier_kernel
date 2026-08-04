// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu.h>
#include <nvgpu/pmu/pmu_pg.h>
#include <nvgpu/pmu/pmuif/pg.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/soc.h>

#include "common/pmu/pg/pmu_pg.h"
#include "common/pmu/pg/pg_sw_gm20b.h"
#include "common/pmu/pg/pg_sw_gv11b.h"
#include "common/pmu/pg/pg_sw_ga10b.h"
#include "pg_sw_gb10b.h"

void gb10b_pmu_pg_lpwr_lp_rpc_handler(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct nv_pmu_rpc_header *rpc, struct rpc_handler_payload *rpc_payload)
{
	nvgpu_log_fn(g, " ");
	switch (rpc->function) {
	case NV_PMU_RPC_ID_LPWR_LP_MON_START:
		nvgpu_pmu_dbg(g, "Reply LPWR_LP_MON_START");
		break;
	case NV_PMU_RPC_ID_LPWR_LP_MON_STAT_GET:
		nvgpu_pmu_dbg(g, "Reply LPWR_LP_MON_STAT_GET");
		break;
	case NV_PMU_RPC_ID_LPWR_LP_CSD_STAT_GET:
		nvgpu_pmu_dbg(g, "Reply LPWR_LP_CSD_STAT_GET");
		break;
	default:
		nvgpu_err(g, "unsupported LPWR_LP rpc function :0x%x",
				rpc->function);
		break;
	}
	(void)pmu;
	(void)rpc_payload;
}

u32 gb10b_pmu_get_pg_buf_gpu_va(struct gk20a *g, struct nvgpu_pmu *pmu,
		u32 buf_id)
{
	u32 data;

	/*
	 * On next-gen chips, the Virtual address is 57 bits.
	 * In 57 bits, the lower bits(0-7) of the lower 32 bits address are
	 * zero. Hence right shifting by 8. And appending the higher bits(24-31) with
	 * lower bits(0-8) of higher address to pass the contiguous valid 32
	 * bits of 49 bits address through methodData to REGLIST_VIRTUAL_ADDRESS.
	 */
	data = u64_lo32(nvgpu_pmu_pg_buf_get_gpu_va(g, pmu, buf_id) >> 8) |
		u64_hi32(nvgpu_pmu_pg_buf_get_gpu_va(g, pmu, buf_id) << 24);

	return data;
}

u32 gb10b_pmu_get_pg_buf_gpu_va_hi(struct gk20a *g, struct nvgpu_pmu *pmu,
		u32 buf_id)
{
	u32 data;

	/*
	 * The higher address 17 bits of 49 bits VA to be passed to
	 * VIRTUAL_ADDRESS_HI. The 8 bits of 17 bits higher address is already
	 * appended and sent to VIRUAL_ADDRESS. Hence the lower bits(0-8) are
	 * right shifted and remaining is passed through method data.
	 */
	data = u64_hi32(nvgpu_pmu_pg_buf_get_gpu_va(g, pmu, buf_id) >> 8);

	return data;
}

u32 gb10b_pmu_pg_engines_list(struct gk20a *g)
{
	return nvgpu_is_enabled(g, NVGPU_ELPG_MS_ENABLED) ?
		(BIT32(PMU_PG_ELPG_ENGINE_ID_GRAPHICS) |
			BIT32(PMU_PG_ELPG_ENGINE_ID_MS_LTC)) :
		(BIT32(PMU_PG_ELPG_ENGINE_ID_GRAPHICS));
}

int gb10b_pmu_pg_pre_init(struct gk20a *g, struct nvgpu_pmu *pmu)
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

	/* Initialize LPWR GR and MS grp data for GRAPHICS and MS_LTC engine */
	for (idx = 0; idx < NV_PMU_LPWR_GRP_CTRL_ID__COUNT; idx++)
	{
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
	}

	PMU_RPC_EXECUTE_CPB(status, pmu, PG_LOADING, PRE_INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_init(struct gk20a *g, struct nvgpu_pmu *pmu,
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

	PMU_RPC_EXECUTE_CPB(status, pmu, PG_LOADING, INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x",
			status);
	}

	return status;
}

int gb10b_pmu_pg_threshold_update(struct gk20a *g,
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
		rpc.threshold_cycles.idle = PMU_PG_IDLE_THRESHOLD;
		rpc.threshold_cycles.ppu = PMU_PG_POST_POWERUP_IDLE_THRESHOLD;
	}

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, THRESHOLD_UPDATE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x",
			status);
	}

	return status;
}

int gb10b_pmu_pg_sfm_update(struct gk20a *g,
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

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, SFM_UPDATE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x",
			status);
	}

	return status;
}

int gb10b_pmu_pg_post_init(struct gk20a *g, struct nvgpu_pmu *pmu)
{
	struct pmu_rpc_struct_lpwr_loading_post_init rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_loading_post_init));
	/*
	 * keep gpcclk flcg disable from here
	 * it gets enabled/disabled duirng elpg entry/exit
	 * Bug - 4129104
	 * GPCCLK FLCG is constrained to GR-RPG/ELPG window
	 * for entire blackwell series
	 */
	rpc.gpc_cg_supported = false;

	rpc.nvd_cg_supported = false;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG_LOADING, POST_INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_init_send(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 pg_engine_id)
{
	int status;
	u32 pg_engine_id_list = 0U;
	u32 flcg_support_mask = 0U;

	nvgpu_log_fn(g, " ");

	status = gb10b_pmu_pg_pre_init(g, pmu);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute PG_PRE_INIT RPC");
		return status;
	}

	if (pmu->pg->supported_engines_list != NULL) {
		pg_engine_id_list = pmu->pg->supported_engines_list(g);
	}

	for (pg_engine_id = PMU_PG_ELPG_ENGINE_ID_GRAPHICS;
			pg_engine_id < PMU_PG_ELPG_ENGINE_ID_INVALID_ENGINE;
			pg_engine_id++) {
		if ((BIT32(pg_engine_id) & pg_engine_id_list) != 0U) {
			status = gb10b_pmu_pg_init(g, pmu, pg_engine_id);
			if (status != 0) {
				nvgpu_err(g, "Failed to execute PG_INIT RPC");
				return status;
			}

			status = gb10b_pmu_pg_threshold_update(g, pmu, pg_engine_id);
			if (status != 0) {
				nvgpu_err(g, "Failed to execute PG_THRESHOLD_UPDATE RPC");
				return status;
			}

			if (g->gpcclk_flcg_enabled) {
				status = gb10b_pmu_pg_sfm_update(g, pmu,
						pg_engine_id, true);
			} else {
				status = gb10b_pmu_pg_sfm_update(g, pmu,
						pg_engine_id, false);
			}
			if (status != 0) {
				nvgpu_err(g, "Failed to execute PG_SFM_UPDATE RPC");
				return status;
			}
		}
	}
	if (pmu->pg->priv_blocker_init != NULL) {
		status = pmu->pg->priv_blocker_init(g, pmu);
		if (status != 0) {
			nvgpu_err(g, "Failed to execute "
					"PG_LOADING_PRIV_BLOCKER_INIT RPC");
			return status;
		}
	}

	if (pmu->pg->fgpg_init != NULL) {
		status = pmu->pg->fgpg_init(g, pmu);

		if (status != 0) {
			nvgpu_err(g, "Failed to execute "
					"PG_LOADING_SEQ_FGPG_INIT RPC");
			return status;
		}
	}
	if (pmu->pg->flcg_init != NULL) {
		/*
		 * Init both NVDCLK and GPCCLK FLCG
		 * This RPC will enable NVDCLK FLCG only.
		 * As GPCCLK FLCG needs to enabled in ELPG window only
		 * Bug - 4129104
		 * GPCCLK FLCG is constrained to GR-RPG/ELPG window
		 * for entire blackwell series
		 */
		nvgpu_pmu_dbg(g, "flcg init");
		flcg_support_mask = BIT32(PMU_CTRL_ID_CG_FLCG_CTRL_NVDCLK) |
					BIT32(PMU_CTRL_ID_CG_FLCG_CTRL_GPCCLK);
		status = pmu->pg->flcg_init(g, pmu, flcg_support_mask);
		if (status != 0) {
			nvgpu_err(g, "Failed to execute "
					"PG_LOADING_FLCG_INIT RPC");
			return status;
		}
	}
	if (pmu->pg->csd_init != NULL) {
		nvgpu_pmu_dbg(g, "CSD INIT");
		status = pmu->pg->csd_init(g, pmu);

		if (status != 0) {
			nvgpu_err(g, "Failed to execute PG_CSD_INIT RPC");
			return status;
		}
	}

	status = gb10b_pmu_pg_post_init(g, pmu);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute PG_POST_INIT RPC");
		return status;
	}

	if (g->sysclk_slowdown_enabled) {
		if (pmu->pg->csd_ctrl_enable != NULL) {
			nvgpu_pmu_dbg(g, "CSD Enable");
			status = pmu->pg->csd_ctrl_enable(g, pmu, true);

			if (status != 0) {
				nvgpu_err(g, "Failed to execute PG_CSD_CTRL_ENABLE RPC");
				return status;
			}
		}

		if (pmu->pg->csd_ctrl_threshold_update != NULL) {
			nvgpu_pmu_dbg(g, "CSD Threshold Update");
			status = pmu->pg->csd_ctrl_threshold_update(g, pmu, 1000);

			if (status != 0) {
				nvgpu_err(g, "Failed to execute PG_CSD_CTRL_THRESHOLD_UPDATE "
								"RPC");
				return status;
			}
		}
	}

	return status;
}

int gb10b_pmu_pg_fgpg_init(struct gk20a *g,
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
	rpc.support_mask = BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_GR) |
			BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_NVDEC0) |
			BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_NVDEC1) |
			BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_NVENC0) |
			BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_NVENC1) |
			BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_NVJPG0) |
			BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_NVJPG1) |
			BIT32(NV_PMU_LPWR_SEQ_FGPG_CTRL_ID_OFA);

	g->fgpg_supported_engmask = rpc.support_mask;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG_LOADING, SEQ_FGPG_INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_fgpg_state_change(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 fgpg_ctrl_id, bool enable)
{
	struct pmu_rpc_struct_lpwr_seq_fgpg_state_change rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
		sizeof(struct pmu_rpc_struct_lpwr_seq_fgpg_state_change));

	rpc.ctrl_id = fgpg_ctrl_id;
	rpc.enable = enable;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, SEQ_FGPG_STATE_CHANGE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_fgpg_threshold_update(struct gk20a *g,
			struct nvgpu_pmu *pmu, struct nvgpu_pmu_fgpg_ctrl *fgpg,
			u8 ctrl_id)
{
	struct pmu_rpc_struct_lpwr_seq_fgpg_threshold_update rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
		sizeof(struct pmu_rpc_struct_lpwr_seq_fgpg_threshold_update));

	rpc.entry_threshold_us  = fgpg->entry_threshold_us;
	rpc.min_resident_time_us = fgpg->min_resident_time_us;
	rpc.ctrl_id = ctrl_id;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, SEQ_FGPG_THRESHOLD_UPDATE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_priv_blocker_init(struct gk20a *g,
		struct nvgpu_pmu *pmu)
{
	struct pmu_rpc_struct_lpwr_loading_priv_blocker_init rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_loading_priv_blocker_init));

	/*
	 * enable Centralized Priv Blocker.PRI blocker is to block PRI
	 * accesses and raise an interrupt to notify PMU of an
	 * incoming/blocked access.
	 */
	rpc.blocker_support_mask = BIT(NV_PMU_PRIV_BLOCKER_ID_CENTRAL);

	PMU_RPC_EXECUTE_CPB(status, pmu, PG_LOADING, PRIV_BLOCKER_INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_cg_elcg_get_entry_count(struct gk20a *g,
	       struct nvgpu_pmu *pmu, u32 ctrl_id, u32 mode, u32 *entry_count)
{
	struct pmu_rpc_struct_lpwr_cg_elcg_stat_get rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_cg_elcg_stat_get));

	rpc.ctrl_id = ctrl_id;
	rpc.mode = mode;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CG_ELCG_STAT_GET, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	} else {
		*entry_count = rpc.val;
	}

	return status;
}

int gb10b_pmu_pg_cg_elcg_get_residency(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 ctrl_id, u32 mode, u32 *delta_sleep_time_us,
	u32 *delta_wall_time_us)
{
	struct pmu_rpc_struct_lpwr_cg_elcg_stat_get rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_cg_elcg_stat_get));

	rpc.ctrl_id = ctrl_id;
	rpc.mode = mode;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CG_ELCG_STAT_GET, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
		goto exit;
	} else {
		*delta_sleep_time_us = rpc.delta_sleep_time_us;
		*delta_wall_time_us  = rpc.delta_wall_time_us;
	}
exit:
	return status;
}

int gb10b_pmu_pg_cg_elcg_mon_start(struct gk20a *g,
		struct nvgpu_pmu *pmu, u32 ctrl_id, bool enable)
{
	struct pmu_rpc_struct_lpwr_cg_elcg_mon_start rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_cg_elcg_mon_start));

	rpc.ctrl_id = ctrl_id;
	if (enable) {
		rpc.start = true;
	} else {
		rpc.start = false;
	}

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CG_ELCG_MON_START, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_cg_elcg_mon_config(struct gk20a *g,
		struct nvgpu_pmu *pmu, u32 mode)
{
	struct pmu_rpc_struct_lpwr_cg_elcg_mon_config rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_cg_elcg_mon_config));

	rpc.mode = mode;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CG_ELCG_MON_CONFIG, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_cg_elcg_state_change(struct gk20a *g,
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
		if (elcg_engine_status[dev->engine_id] == ELCG_AUTO) {
			rpc.enable_req[dev->engine_id] = true;
		} else if (elcg_engine_status[dev->engine_id] == ELCG_RUN) {
			rpc.disable_req[dev->engine_id] = true;
		}
	}

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CG_ELCG_STATE_CHANGE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_cg_flcg_init(struct gk20a *g,
		struct nvgpu_pmu *pmu, u32 support_mask)
{
	struct pmu_rpc_struct_lpwr_loading_cg_flcg_init rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_loading_cg_flcg_init));

	rpc.support_mask = support_mask;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG_LOADING, FLCG_INIT, &rpc, 0);

	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_cg_flcg_ctrl_enable(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 ctrl_id, bool enable)
{
	struct pmu_rpc_struct_lpwr_cg_flcg_ctrl_enable rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_cg_flcg_ctrl_enable));

	rpc.ctrl_id = ctrl_id;
	rpc.enable = enable;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CG_FLCG_CTRL_ENABLE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_cg_flcg_ctrl_threshold_update(struct gk20a *g,
		struct nvgpu_pmu *pmu, u8 ctrl_id, u16 threshold_type,
		u16 threshold_ns)
{
	struct pmu_rpc_struct_lpwr_cg_flcg_ctrl_threshold_update rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
		sizeof(struct
			pmu_rpc_struct_lpwr_cg_flcg_ctrl_threshold_update));

	rpc.ctrl_id = ctrl_id;
	rpc.threshold_type = threshold_type;
	rpc.threshold_ns = threshold_ns;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CG_FLCG_CTRL_THRESHOLD_UPDATE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_lpwr_lp_mon_start(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 lpwr_mon_index, bool enable)
{
	struct pmu_rpc_struct_lpwr_lp_mon_start rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_lp_mon_start));

	rpc.lpwr_mon_index = lpwr_mon_index;
	rpc.start = enable;

	PMU_RPC_EXECUTE_CPB(status, pmu, LPWR_LP, MON_START, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_lpwr_lp_get_residency(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 lpwr_mon_index, u32 *delta_sleep_time_tick,
	u32 *delta_wall_time_tick)
{
	struct pmu_rpc_struct_lpwr_lp_mon_stat_get rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_lpwr_lp_mon_stat_get));

	rpc.lpwr_mon_index = lpwr_mon_index;

	PMU_RPC_EXECUTE_CPB(status, pmu, LPWR_LP, MON_STAT_GET, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
		goto exit;
	} else {
		*delta_sleep_time_tick = rpc.delta_sleep_time_tick;
		*delta_wall_time_tick  = rpc.delta_wall_time_tick;
	}
exit:
	return status;
}

int gb10b_pmu_pg_cg_csd_init(struct gk20a *g, struct nvgpu_pmu *pmu)
{
	struct pmu_rpc_struct_lpwr_csd_init rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0, sizeof(struct pmu_rpc_struct_lpwr_csd_init));

	rpc.support_mask = BIT32(PMU_CTRL_ID_CSD_CTRL_SYSCLK);

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CSD_INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_cg_csd_ctrl_enable(struct gk20a *g,
		struct nvgpu_pmu *pmu, bool enable)
{
	struct pmu_rpc_struct_lpwr_csd_ctrl_enable rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0, sizeof(struct pmu_rpc_struct_lpwr_csd_ctrl_enable));

	rpc.ctrl_id = PMU_CTRL_ID_CSD_CTRL_SYSCLK;

	rpc.enable = enable;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CSD_CTRL_ENABLE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_cg_csd_ctrl_threshold_update(struct gk20a *g,
			struct nvgpu_pmu *pmu, u32 threshold_us)
{
	struct pmu_rpc_struct_lpwr_csd_ctrl_threshold_update rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
		sizeof(struct pmu_rpc_struct_lpwr_csd_ctrl_threshold_update));

	rpc.threshold_us  = threshold_us;
	rpc.ctrl_id = PMU_CTRL_ID_CSD_CTRL_SYSCLK;

	PMU_RPC_EXECUTE_CPB(status, pmu, PG, CSD_CTRL_THRESHOLD_UPDATE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
	}

	return status;
}

int gb10b_pmu_pg_cg_csd_get_entry_count(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 *entry_count)
{
	struct pmu_rpc_struct_lpwr_lp_csd_stat_get rpc;
	int status;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0, sizeof(struct pmu_rpc_struct_lpwr_lp_csd_stat_get));

	rpc.ctrl_id = PMU_CTRL_ID_CSD_CTRL_SYSCLK;

	PMU_RPC_EXECUTE_CPB(status, pmu, LPWR_LP, CSD_STAT_GET, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC status=0x%x", status);
		goto exit;
	} else {
		*entry_count = rpc.entry_count;
	}
exit:
	return status;
}

void nvgpu_gb10b_pg_sw_init(struct gk20a *g,
		struct nvgpu_pmu_pg *pg)
{
	nvgpu_log_fn(g, " ");

	pg->elpg_statistics = ga10b_pmu_elpg_statistics;
	pg->init_param = NULL;
	pg->supported_engines_list = gb10b_pmu_pg_engines_list;
	pg->engines_feature_list = NULL;
	pg->set_sub_feature_mask = NULL;
	pg->save_zbc = gm20b_pmu_save_zbc;
	pg->allow = ga10b_pmu_pg_allow;
	pg->disallow = ga10b_pmu_pg_disallow;
	pg->init = gb10b_pmu_pg_init;
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
	pg->process_pg_event = ga10b_pmu_pg_process_pg_event;
	pg->pmu_get_pg_buf_gpu_va = gb10b_pmu_get_pg_buf_gpu_va;
	pg->pmu_get_pg_buf_gpu_va_hi = gb10b_pmu_get_pg_buf_gpu_va_hi;
	pg->pmu_pg_sfm_update = gb10b_pmu_pg_sfm_update;
	pg->fgpg_init = gb10b_pmu_pg_fgpg_init;
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
	pg->cg_elcg_state_change = gb10b_pmu_pg_cg_elcg_state_change;
	pg->csd_ctrl_enable = gb10b_pmu_pg_cg_csd_ctrl_enable;
	pg->csd_init = gb10b_pmu_pg_cg_csd_init;
	pg->csd_ctrl_threshold_update =
					gb10b_pmu_pg_cg_csd_ctrl_threshold_update;
	pg->csd_get_entry_count = gb10b_pmu_pg_cg_csd_get_entry_count;
}
