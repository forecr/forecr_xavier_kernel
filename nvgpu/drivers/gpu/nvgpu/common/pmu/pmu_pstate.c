// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/string.h>
#include <nvgpu/bios.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pmu.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/boardobjgrp_e255.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/pmu/pmgr.h>
#include <nvgpu/pmu/therm.h>
#include <nvgpu/pmu/perf.h>
#include <nvgpu/pmu/volt.h>
#include <nvgpu/pmu/perf_cf.h>
#include <nvgpu/pmu/pmu_pstate.h>
#include <nvgpu/pmu/pmu_model_20.h>
#include <nvgpu/pmu/debug.h>
#include <nvgpu/pmu/cmd.h>

#include "boardobj/boardobj.h"
#include "clk/ucode_clk_inf.h"
#include "clk/clk.h"

void nvgpu_pmu_pstate_deinit(struct gk20a *g)
{
	pmgr_pmu_free_pmupstate(g);
	nvgpu_pmu_therm_deinit(g, g->pmu);

	if (g->pmu->perf_pmu != NULL) {
		nvgpu_pmu_perf_deinit(g);
	}

	if (g->pmu->volt != NULL) {
		nvgpu_pmu_volt_deinit(g);
	}

	nvgpu_pmu_clk_deinit(g);

	if (g->ops.clk.mclk_deinit != NULL) {
		g->ops.clk.mclk_deinit(g);
	}

	if (g->pmu->perf_cf != NULL) {
		nvgpu_pmu_perf_cf_deinit(g);
	}
}

static int pmu_pstate_init(struct gk20a *g)
{
	int err;
	nvgpu_log_fn(g, " ");

	err = nvgpu_pmu_therm_init(g, g->pmu);
	if (err != 0) {
		nvgpu_pmu_therm_deinit(g, g->pmu);
		return err;
	}

	err = nvgpu_pmu_clk_init(g);
	if (err != 0) {
		return err;
	}

	err = nvgpu_pmu_perf_init(g);
	if (err != 0) {
		nvgpu_pmu_perf_deinit(g);
		return err;
	}

	err = nvgpu_pmu_volt_init(g);
	if (err != 0) {
		return err;
	}

	err = pmgr_pmu_init_pmupstate(g);
	if (err != 0) {
		pmgr_pmu_free_pmupstate(g);
		return err;
	}

	err = nvgpu_pmu_perf_cf_init(g);
	if (err != 0) {
		nvgpu_err(g, "Perf CF init failed, status: 0x%x", err);
		return err;
	}

	return 0;
}

/*sw setup for pstate components*/
int nvgpu_pmu_pstate_sw_setup(struct gk20a *g)
{
	int err;
	nvgpu_log_fn(g, " ");

	err = nvgpu_pmu_wait_fw_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU not ready to process pstate requests");
		return err;
	}
	err = pmu_pstate_init(g);
	if (err != 0) {
		nvgpu_err(g, "Pstate init failed");
		return err;
	}
	err = nvgpu_pmu_therm_sw_setup(g, g->pmu);
	if (err != 0) {
		return err;
	}

	err = nvgpu_pmu_volt_sw_setup(g);
	if (err != 0) {
		nvgpu_err(g, "Volt sw setup failed");
		return err;
	}

	err = nvgpu_pmu_perf_sw_setup(g);
	if (err != 0) {
		nvgpu_err(g, "Perf sw setup failed");
		return err;
	}

	err = nvgpu_pmu_clk_sw_setup(g);
	if (err != 0) {
		nvgpu_err(g, "Clk sw setup failed");
		return err;
	}

	err = nvgpu_pmu_perf_cf_sw_setup(g);
	if (err != 0) {
		nvgpu_err(g, "Perf CF sw setup failed");
		return err;
	}

	/* TODO: Temporarily disable P-State SW setup and enable it once the P-State is supported */
	/*
	 *
	 * if (g->ops.clk.support_pmgr_domain) {
		err = pmgr_domain_sw_setup(g);
		if (err != 0) {
			goto err_pmgr_pmu_init_pmupstate;
		}
	}

	return 0;

err_pmgr_pmu_init_pmupstate:
	pmgr_pmu_free_pmupstate(g);
err_therm_pmu_init_pmupstate:
	nvgpu_pmu_therm_deinit(g, g->pmu);
err_perf_pmu_init_pmupstate:
	nvgpu_pmu_perf_deinit(g);
	*/

	return err;
}

/*sw setup for pstate components*/
int nvgpu_pmu_pstate_pmu_setup(struct gk20a *g)
{
	u32 unit_id;
	int err = 0;
	nvgpu_log_fn(g, " ");

	/* reset pstate rpc pending flags */
	(void) memset(&g->pmu->pmu_unit_rpc_pending, 0,
				sizeof(g->pmu->pmu_unit_rpc_pending));

	err = nvgpu_pmu_therm_pmu_setup(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "Therm pmu setup failed");
		goto exit_none;
	}

	err = nvgpu_pmu_perf_pmu_setup(g);
	if (err != 0) {
		nvgpu_err(g, "Perf pmu setup failed");
		goto exit_none;
	}

	err = nvgpu_pmu_volt_pmu_setup(g);
	if (err != 0) {
		nvgpu_err(g, "Volt pmu setup failed");
		goto exit_none;
	}

	err = nvgpu_pmu_clk_pmu_setup(g);
	if (err != 0) {
		nvgpu_err(g, "Failed to send CLK pmu setup");
		goto exit_none;
	}

	/* Send CLK LOAD RPC with ADC feature */
	err = nvgpu_pmu_clk_load_with_feature(g, NV_RM_PMU_CLK_LOAD_FEATURE_ADC,
			NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_FULL);
	if (err != 0) {
		nvgpu_err(g, "Failed to send ADC perf_load RPC status=0x%x", err);
		goto exit_none;
	}

	/* Send CLK LOAD RPC with FREQ_EFFECTIVE_AVG feature */
	err = nvgpu_pmu_clk_load_with_feature(g, NV_RM_PMU_CLK_LOAD_FEATURE_FREQ_EFFECTIVE_AVG,
		(NV_RM_PMU_CLK_LOAD_ACTION_MASK_FREQ_EFFECTIVE_AVG_CALLBACK_YES << 2));
	if (err != 0) {
		nvgpu_err(g, "Failed to send FREQ_EFFECTIVE_AVG perf_load RPC status=0x%x", err);
		goto exit_none;
	}

	/* Send VOLT LOAD RPC */
	err = nvgpu_pmu_volt_load(g);
	if (err != 0) {
		nvgpu_err(g,
			"Failed to send VOLT LOAD CMD to PMU: status = 0x%08x.",
			err);
		goto exit_none;
	}

	err = nvgpu_pmu_perf_post_clk_domain_and_volt_rail_init(g);
	if (err != 0) {
		nvgpu_err(g, "Failed to send POST_CLK_DOMAIN_AND_VOLT_RAIL_INIT");
		goto exit_none;
	}

	err = nvgpu_pmu_clk_load_with_feature(g, NV_RM_PMU_CLK_LOAD_FEATURE_CLK_DOMAIN, 0);
	if (err != 0) {
		nvgpu_err(g, "Failed to send CLK_DOMAIN perf_load RPC status=0x%x", err);
		goto exit_none;
	}

	err = nvgpu_pmu_perf_load(g);
	if (err != 0) {
		nvgpu_err(g, "Failed to send PERF_LOAD RPC, status=0x%x", err);
		goto exit_none;
	}

	err = nvgpu_pmu_perf_cf_pmu_setup(g);
	if (err != 0) {
		nvgpu_err(g, "Failed to setup PERF CF, status=0x%x", err);
		goto exit_none;
	}

/*
	if (g->ops.clk.mclk_init != NULL) {
		err = g->ops.clk.mclk_init(g);
		if (err != 0) {
			goto exit_none;
		}
	}
*/
	/* TODO: Remove this once pstate is fully supported*/

/*
#ifdef CONFIG_NVGPU_DGPU

	err = nvgpu_pmu_clk_pmu_setup(g);
	if (err != 0) {
		goto exit_none;
	}

	if (g->ops.clk.support_pmgr_domain) {
		err = pmgr_domain_pmu_setup(g);
		if (err != 0) {
			goto exit_none;
		}
	}

	err = g->ops.clk.perf_pmu_vfe_load(g);
	if (err != 0) {
		goto exit_none;
	}
*/

	/* Wait for pstate pmu setup RPCs sent above to be acked by PMU */
	for (unit_id = 0; unit_id < PMU_UNIT_END; unit_id++) {
		if (!PMU_PSTATE_RPC_UNIT_ID(unit_id)) {
			continue;
		}
		pmu_wait_message_cond(g->pmu,
			nvgpu_get_poll_timeout(g),
			&g->pmu->pmu_unit_rpc_pending[unit_id], 0);
		if (g->pmu->pmu_unit_rpc_pending[unit_id]) {
			nvgpu_err(g, "pstate pmu setup fail, pending on unit id %d", unit_id);
			nvgpu_pmu_dump_falcon_stats(g->pmu);
			err = -ETIMEDOUT;
			goto exit_none;
		}
	}

	err = nvgpu_pmu_driver_register_send(g,
		register_complete, PMU_DRIVER_REGISTER_CLIENT_PSTATE);
	if (err != 0) {
		nvgpu_err(g, "Failed sending driver register complete");
		err = -EINVAL;
		goto exit_none;
	}
	nvgpu_pmu_dbg(g, "PMU register done PSTATE");

exit_none:
	return err;
}

