// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu.h>
#include <nvgpu/pmu/pmu_pg.h>
#include <nvgpu/pmu/pmuif/pg.h>
#include <nvgpu/soc.h>

#include "common/pmu/pg/pmu_pg.h"
#include "common/pmu/pg/pg_sw_gm20b.h"
#include "common/pmu/pg/pg_sw_ga10b.h"
#include "common/pmu/pg/pg_sw_gb10b.h"
#include "pg_sw_gb20c.h"

void nvgpu_gb20c_pg_sw_init(struct gk20a *g,
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
/* On FPGA (not for SCSIM) if CSD INIT is triggered, PMU halt happens at PMU code:
lpwrCsdSysClkInit_GB20B(void)
regVal = THERM_REG_RD32(NV_CPWR_THERM_INTR_MONITOR_CTRL(pLpwrCsdCtrl->monIndex));
*/
//	pg->csd_init = gb10b_pmu_pg_cg_csd_init;
	pg->csd_ctrl_threshold_update =
					gb10b_pmu_pg_cg_csd_ctrl_threshold_update;
	pg->csd_get_entry_count = gb10b_pmu_pg_cg_csd_get_entry_count;
}
