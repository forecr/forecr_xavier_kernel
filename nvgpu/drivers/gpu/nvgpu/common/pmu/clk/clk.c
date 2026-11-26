// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/pmu.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/string.h>
#include <nvgpu/kmem.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/pmu/volt.h>
#include <nvgpu/pmu/perf.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/timers.h>
#include <nvgpu/pmu/pmu_pstate.h>

#include <common/pmu/perf/ucode_perf_change_seq_inf.h>

#include "ucode_clk_inf.h"
#include "clk_domain.h"
#include "clk_enum.h"
#include "clk_vf_rel.h"
#include "clk_prop_regime.h"
#include "clk_prop_top.h"
#include "clk_prop_top_rel.h"
#include "clk3_freq_domain.h"
#include "clk_vin.h"
#include "clk_fll.h"
#include "clk_vf_point.h"
#include "clk.h"

static int clk_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmu->clk_pmu != NULL) {
		return 0;
	}

	g->pmu->clk_pmu = nvgpu_kzalloc(g, sizeof(*g->pmu->clk_pmu));
	if (g->pmu->clk_pmu == NULL) {
		return -ENOMEM;
	}

	return 0;
}

static void clk_free_pmupstate(struct gk20a *g)
{
	nvgpu_kfree(g, g->pmu->clk_pmu);
	g->pmu->clk_pmu = NULL;
}

s32 nvgpu_pmu_clk_resume_clks(struct gk20a *g)
{
	s32 status = 0;
	u32 freq;

	/* get the last set GPC frequency to resume to */
	freq = g->pmu->clk_pmu->clk_domainobjs_50->currentClocksVfInfo.gpc_mhz;
	nvgpu_pmu_dbg(g, "Resume GPC clock to %d MHz in Auto mode", freq);
	status = set_clock_rate_via_pmu_rpc(g, freq,
		CTRL_CLK_DOMAIN_GPCCLK, CLOCK_MODE_AUTO);

	return status;
}

/*
 * nvgpu_pmu_clk_set_boot_clk - Set boot clocks to pstate fmax (2002 MHz)
 * @g: pointer to GPU context
 *
 * This function sets GPU clocks to pstate fmax during boot
 * initialization. It first sets the clock domain regime to FR to ensure
 * the clock rate is Min(What SW wants and what voltage can give), then set the
 * clock to pstate fmax.
 *
 * Returns: 0 on success, negative error code on failure
 */
int nvgpu_pmu_clk_set_boot_clk(struct gk20a *g)
{
	int ret = 0;
	u32 fmax = 2002U; /* default to use if VBIOS value unavailable */

	if (g == NULL) {
		ret = -EINVAL;
		goto done;
	}

	nvgpu_log_fn(g, " ");

	/* First, set all clock domain regimes to FR (0x02) to ensure rates stick */
	ret = nvgpu_pmu_clk_domains_fll_set_regime_50(g, CTRL_CLK_FLL_REGIME_ID_FR, NV2080_CTRL_CLK_DOMAIN_LEGACY_GPCCLK);
	if (ret != 0) {
		nvgpu_err(g, "Failed to set GPC domain regime to FR: %d", ret);
		goto done;
	}

	ret = nvgpu_pmu_clk_domains_fll_set_regime_50(g, CTRL_CLK_FLL_REGIME_ID_FR, NV2080_CTRL_CLK_DOMAIN_LEGACY_XBARCLK);
	if (ret != 0) {
		nvgpu_err(g, "Failed to set SYS domain regime to FR: %d", ret);
		goto done;
	}

	ret = nvgpu_pmu_clk_domains_fll_set_regime_50(g, CTRL_CLK_FLL_REGIME_ID_FR, NV2080_CTRL_CLK_DOMAIN_LEGACY_NVDCLK);
	if (ret != 0) {
		nvgpu_err(g, "Failed to set NVD domain regime to FR: %d", ret);
		goto done;
	}

	ret = nvgpu_pmu_clk_domains_fll_set_regime_50(g, CTRL_CLK_FLL_REGIME_ID_FR, NV2080_CTRL_CLK_DOMAIN_LEGACY_UPROCCLK);
	if (ret != 0) {
		nvgpu_err(g, "Failed to set UPROC domain regime to FR: %d", ret);
		goto done;
	}

	nvgpu_info(g, "Set all clock domain regimes to FR (0x%02x) before setting boot clocks", CTRL_CLK_FLL_REGIME_ID_FR);

	/* Get GPC clock Drive mode Fmax from VBIOS */
	(void) clk_domain_get_gpc_drive_mode_freq(g, &fmax);

	/* Set GPC clock to Drive mode fmax using PMU RPC */
	ret = set_clock_rate_via_pmu_rpc(g, fmax, CTRL_CLK_DOMAIN_GPCCLK,
					CLOCK_MODE_AUTO);
	if (ret != 0) {
		nvgpu_err(g, "Failed to set GPC clock to %d MHz: %d", fmax, ret);
		goto done;
	}

	nvgpu_info(g, "Successfully set boot clocks: GPC=%d MHz (Drive mode fmax)", fmax);

done:
	return ret;
}

/* Helper function to set clock rate using PMU RPC */
int set_clock_rate_via_pmu_rpc(struct gk20a *g, u32 requested_rate_mhz,
					u32 clk_domain, u32 mode)
{
	struct nvgpu_clk_vf_point_5x vf_point;
	u32 temp_freq, temp_volt;
	struct nvgpu_clk_domain *p_domain;
	u16 clk_domain_idx = 0xffff;
	char *clkName = "";
	int ret = 0;

	/* Only actually set clocks for GPC domain in auto mode, return success for others */
	if (clk_domain != CTRL_CLK_DOMAIN_GPCCLK && mode == CLOCK_MODE_AUTO) {
		nvgpu_pmu_dbg(g, "Auto mode: ignoring non-GPC domain %u request for %u MHz (success)",
				clk_domain, requested_rate_mhz);
		goto exit_set_clk_rate;
	}

	/* Step 0: Fetch all VF points ONCE to eliminate redundant RPC to PMU
	 * in each of the F->V and V->F steps below.
	 */
	ret = clk_domain_get_vf_tables(g);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get VF Curves from PMU");
		goto exit_set_clk_rate;
	}

	if (clk_domain == CTRL_CLK_DOMAIN_GPCCLK && mode == CLOCK_MODE_AUTO) {
		clkName = "GPC";
		/* Step 1: Find VF point for GPC rate in auto mode */
		ret = nvgpu_clk_find_vf_point_for_gpc_rate(g, requested_rate_mhz, &vf_point);
		if (ret) {
			nvgpu_err(g, "Failed to find VF point for GPC rate %u MHz: %d",
				requested_rate_mhz, ret);
			goto done;
		}
	} else {
		/* In Manual mode clocks are set independently */

		/* Step 1: copy current clock state */
		vf_point = g->pmu->clk_pmu->clk_domainobjs_50->currentClocksVfInfo;

		nvgpu_pmu_dbg(g, "Current VF point: GPC=%u MHz, SYS=%u MHz, NVD=%u MHz, UPROC=%u MHz",
			vf_point.gpc_mhz, vf_point.sys_mhz, vf_point.nvd_mhz, vf_point.uproc_mhz);
		nvgpu_pmu_dbg(g, "Current GPC_VDD at %u uV and GPM_VDD at %u uV",
			vf_point.gpc_volt_uv, vf_point.gpm_volt_uv);

		/* Step 2: Get the idx of the requested clock via api domain Id */
		p_domain = clkDomainsFindByApiDomain(g, clk_domain);
		if (p_domain == NULL) {
			nvgpu_err(g, "Could not find api domain: 0x%x", clk_domain);
			ret = -EINVAL;
			goto done;
		}
		ret = nvgpu_pmu_clk_domain_get_index_by_domain(g, p_domain->domain,
				(u32 *)&clk_domain_idx);
		if (ret != 0) {
			nvgpu_err(g, "Failed to get clock domain index");
			goto done;
		}

		/* Step 3: Get actual VF point for the clock */
		temp_freq = requested_rate_mhz;
		temp_volt = 0;
		ret = nvgpu_pmu_clk_domain_freq_to_volt(g, clk_domain_idx, &temp_freq, &temp_volt);
		if (ret != 0) {
			nvgpu_err(g, "Failed to get voltage for Clk Idx: %d freq %u MHz",
					clk_domain_idx, temp_freq);
			goto done;
		}
		/* temp_volt not needed, only frequency is set in manual mode. */
		(void) temp_volt;

		/* Step 4: Use the actual frequency value from VF point */
		nvgpu_pmu_dbg(g, "DEBUG: Clk Idx: %d - Final CLK freq=%u MHz",
				clk_domain_idx, temp_freq);
		switch (clk_domain) {
		case CTRL_CLK_DOMAIN_GPCCLK:
			clkName = "GPC";
			vf_point.gpc_mhz = temp_freq;
			break;

		case CTRL_CLK_DOMAIN_XBARCLK:
			clkName = "SYS";
			vf_point.sys_mhz = temp_freq;
			break;

		case CTRL_CLK_DOMAIN_NVDCLK:
			clkName = "NVD";
			vf_point.nvd_mhz = temp_freq;
			break;

		case CTRL_CLK_DOMAIN_UPROCCLK:
			clkName = "UPROC";
			vf_point.uproc_mhz = temp_freq;
			break;

		default:
			ret = -EINVAL;
			break;
		}

		/* Avoid MISRA complaint by moving goto out of default: case */
		if (ret != 0) {
			nvgpu_err(g, "Unsupported clk domain");
			goto done;
		}
	}

	/* Last Step: Send VF point to PMU via change sequencer */
	ret = nvgpu_pmu_perf_changeseq_set_clks_50(g, &vf_point);
	if (ret) {
		nvgpu_err(g, "Failed to set clocks via PMU RPC: %d", ret);
		goto done;
	}

	/* Update current clocks settings to actual data sent to PMU */
	g->pmu->clk_pmu->clk_domainobjs_50->currentClocksVfInfo = vf_point;

	/* Persist the GPC target frequency instead of actual set frequency.
	 * The VF curve can change and we always need to restore
	 * to the target frequency to ensure we get the closest actual frequency.
	 */
	if (clk_domain == CTRL_CLK_DOMAIN_GPCCLK)
		g->pmu->clk_pmu->clk_domainobjs_50->currentClocksVfInfo.gpc_mhz =
				requested_rate_mhz;

	nvgpu_pmu_dbg(g, "Successfully set %s clock rate to %u MHz (VF point: GPC=%u MHz, SYS=%u MHz, NVD=%u MHz, UPROC=%u MHz)",
			clkName, requested_rate_mhz,
			vf_point.gpc_mhz, vf_point.sys_mhz, vf_point.nvd_mhz, vf_point.uproc_mhz);
	nvgpu_pmu_dbg(g, "Successfully set GPC_VDD to %u uV and GPM_VDD to %u uV",
			vf_point.gpc_volt_uv, vf_point.gpm_volt_uv);

done:
	clk_domain_free_vf_tables(g);
exit_set_clk_rate:
	return ret;
}

/*
 * nvgpu_pmu_clk_restore_saved_state - Restore clocks from saved currentClocksVfInfo
 * @g: pointer to GPU context
 *
 * This function restores GPU clocks from the saved currentClocksVfInfo state
 * during resume scenarios. It accesses the saved VF point and restores all
 * clock domains and voltages.
 *
 * Returns: 0 on success, negative error code on failure
 */
int nvgpu_pmu_clk_restore_saved_state(struct gk20a *g)
{
	int ret = 0;

	if (g == NULL || g->pmu == NULL || g->pmu->clk_pmu == NULL)
		return -EINVAL;

	if (g->pmu->clk_pmu->clk_domainobjs_50 == NULL) {
		nvgpu_pmu_dbg(g, "clk_domainobjs_50 not initialized, skipping clock restore");
		return 0;
	}

#ifdef CONFIG_PM
	/* Only restore if we have saved clock state */
	if (!g->has_suspended_clk_state || g->suspended_clk_vf_info == NULL) {
		nvgpu_pmu_dbg(g, "No saved clock state to restore");
		return 0;
	}

	/* Only restore if we have valid saved clock settings */
	if (g->suspended_clk_vf_info->gpc_mhz == 0 && g->suspended_clk_vf_info->sys_mhz == 0) {
		nvgpu_pmu_dbg(g, "saved clock state not valid, skipping clock restore");
		return 0;
	}

	ret = nvgpu_pmu_perf_changeseq_set_clks_50(g, g->suspended_clk_vf_info);
	if (ret != 0) {
		nvgpu_err(g, "failed to restore saved clocks: %d", ret);
		return ret;
	}

	/* Update currentClocksVfInfo with restored values */
	g->pmu->clk_pmu->clk_domainobjs_50->currentClocksVfInfo = *g->suspended_clk_vf_info;

	nvgpu_pmu_dbg(g, "clocks restored from saved state: GPC=%u MHz, SYS=%u MHz, NVD=%u MHz, UPROC=%u MHz",
		g->suspended_clk_vf_info->gpc_mhz, g->suspended_clk_vf_info->sys_mhz,
		g->suspended_clk_vf_info->nvd_mhz, g->suspended_clk_vf_info->uproc_mhz);
	nvgpu_pmu_dbg(g, "voltages restored: GPC_VDD=%u uV, GPM_VDD=%u uV",
		g->suspended_clk_vf_info->gpc_volt_uv, g->suspended_clk_vf_info->gpm_volt_uv);

	/* Clean up saved state after successful restoration */
	nvgpu_kfree(g, g->suspended_clk_vf_info);
	g->suspended_clk_vf_info = NULL;
	g->has_suspended_clk_state = false;
#endif

	return ret;
}

unsigned long nvgpu_pmu_clk_mon_init_domains(struct gk20a *g)
{
	unsigned long domain_mask;

	(void)g;

	domain_mask = (CTRL_CLK_DOMAIN_MCLK |
			CTRL_CLK_DOMAIN_XBARCLK 	|
			CTRL_CLK_DOMAIN_SYSCLK		|
			CTRL_CLK_DOMAIN_HUBCLK		|
			CTRL_CLK_DOMAIN_GPCCLK		|
			CTRL_CLK_DOMAIN_HOSTCLK		|
			CTRL_CLK_DOMAIN_UTILSCLK	|
			CTRL_CLK_DOMAIN_PWRCLK		|
			CTRL_CLK_DOMAIN_NVDCLK		|
			CTRL_CLK_DOMAIN_XCLK		|
			CTRL_CLK_DOMAIN_NVL_COMMON	|
			CTRL_CLK_DOMAIN_PEX_REFCLK	);
	return domain_mask;
}

u32 nvgpu_pmu_clk_mon_init_domains_5x(struct gk20a *g)
{
	u32 domain_mask;

	(void)g;

	domain_mask = (
			CTRL_CLK_DOMAIN_MCLK		|
			CTRL_CLK_DOMAIN_XBARCLK		|
			CTRL_CLK_DOMAIN_GPCCLK		|
			CTRL_CLK_DOMAIN_UPROCCLK	|
			CTRL_CLK_DOMAIN_NVDCLK);
	return domain_mask;
}

int nvgpu_pmu_clk_pmu_setup(struct gk20a *g)
{
	int err = 0;
	nvgpu_log_fn(g, " ");

	err = clk_domain_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

	err = clk_enum_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

	err = clk_vf_rel_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

	err = clk_prop_regime_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

	err = clk_prop_top_rel_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

	err = clk_prop_top_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

	err = clk3_freq_domain_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

	err = clk_vin_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

	err = clk_fll_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

	err = clk_vf_point_pmu_setup(g);
	if (err != 0) {
		goto done;
	}

done:
	nvgpu_log_fn(g, "Done: %d", err);
	return err;
}

int nvgpu_pmu_clk_sw_setup(struct gk20a *g)
{
	int err = 0;
	nvgpu_log_fn(g, " ");

	err = clk_domain_sw_setup(g);
	if (err != 0) {
		clk_domain_free_pmupstate(g);
		goto done;
	}
	err = clk_vin_sw_setup(g);
	if (err != 0) {
		clk_vin_free_pmupstate(g);
		goto done;
	}

	err = clk_fll_sw_setup(g);
	if (err != 0) {
		clk_fll_free_pmupstate(g);
		goto done;
	}

	err = clk_enum_sw_setup(g);
	if (err != 0) {
		clk_enum_free_pmupstate(g);
		goto done;
	}

	err = clk_prop_regime_sw_setup(g);
	if (err != 0) {
		clk_prop_regime_free_pmupstate(g);
		goto done;
	}

	err = clk_prop_top_rel_sw_setup(g);
	if (err != 0) {
		clk_prop_top_rel_free_pmupstate(g);
		goto done;
	}

	err = clk_prop_top_sw_setup(g);
	if (err != 0) {
		clk_prop_top_free_pmupstate(g);
		goto done;
	}

	err = clk_vf_point_sw_setup(g);
	if (err != 0) {
		clk_vf_point_free_pmupstate(g);
		goto done;
	}

	err = clk_vf_rel_sw_setup(g);
	if (err != 0) {
		clk_vf_rel_free_pmupstate(g);
		goto done;
	}

	err = clk3_freq_domain_sw_setup(g);
	if (err != 0) {
		clk3_freq_domain_free_pmupstate(g);
		goto done;
	}

done:
	nvgpu_log_fn(g, "Done: %d", err);
	return err;
}

int nvgpu_pmu_clk_init(struct gk20a *g)
{
	int err = 0;
	nvgpu_log_fn(g, " ");

	err = clk_init_pmupstate(g);
	if (err != 0) {
		clk_free_pmupstate(g);
		goto done;
	}

	err = clk_domain_init_pmupstate(g);
	if (err != 0) {
		clk_domain_free_pmupstate(g);
		goto done;
	}

	err = clk_enum_init_pmupstate(g);
	if (err != 0) {
		clk_enum_free_pmupstate(g);
		goto done;
	}

	err = clk_vf_point_init_pmupstate(g);
	if (err != 0) {
		clk_vf_point_free_pmupstate(g);
		goto done;
	}

	err = clk_vin_init_pmupstate(g);
	if (err != 0) {
		clk_vin_free_pmupstate(g);
		goto done;
	}

	err = clk_fll_init_pmupstate(g);
	if (err != 0) {
		clk_fll_free_pmupstate(g);
		goto done;
	}

	err = clk_vf_rel_init_pmupstate(g);
	if (err != 0) {
		clk_vf_rel_free_pmupstate(g);
		goto done;
	}

	err = clk_prop_regime_init_pmupstate(g);
	if (err != 0) {
		clk_prop_regime_free_pmupstate(g);
		goto done;
	}

	err = clk_prop_top_rel_init_pmupstate(g);
	if (err != 0) {
		clk_prop_top_rel_free_pmupstate(g);
		goto done;
	}

	err = clk_prop_top_init_pmupstate(g);
	if (err != 0) {
		clk_prop_top_free_pmupstate(g);
		goto done;
	}

	err = clk3_freq_domain_init_pmupstate(g);
	if (err != 0) {
		clk3_freq_domain_free_pmupstate(g);
		goto done;
	}

done:
	nvgpu_log_fn(g, "Done: %d", err);
	return err;
}

int nvgpu_pmu_clk_load_with_feature(struct gk20a *g, u8 feature, u32 action_mask)
{
	struct nvgpu_pmu *pmu = g->pmu;
	struct nv_pmu_rpc_struct_clk_load rpc;
	int status = 0;

	(void) memset(&rpc, 0, sizeof(struct nv_pmu_rpc_struct_clk_load));

	/* Set up the RPC header */
	rpc.hdr.unit_id = PMU_UNIT_CLK;
	rpc.hdr.function = NV_PMU_RPC_ID_CLK_LOAD;
	rpc.hdr.flags = 0x0;

	/* Set up the CLK load parameters */
	rpc.clk_load.feature = feature;
	rpc.clk_load.action_mask = action_mask;

	/* Execute the RPC */
	status = nvgpu_pmu_rpc_execute(pmu, (u8 *)&rpc,
		(u16)(sizeof(rpc) - sizeof(rpc.scratch)), 0, NULL, NULL, true);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute CLK LOAD RPC (feature=0x%x) status=0x%x",
			feature, status);
	}

	return status;
}

int nvgpu_pmu_clk_load(struct gk20a *g)
{
	/* Default CLK load with no specific feature */
	return nvgpu_pmu_clk_load_with_feature(g, NV_RM_PMU_CLK_LOAD_FEATURE_INVALID, 0);
}

int nvgpu_pmu_clk_adc_perf_load(struct gk20a *g)
{
	int status;
	u32 action_mask = NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_FULL;

	/* Send CLK LOAD RPC with ADC feature and action mask (ADC_CALLBACK + ADC_HW_CAL_PROGRAM) */
	status = nvgpu_pmu_clk_load_with_feature(g, NV_RM_PMU_CLK_LOAD_FEATURE_ADC, action_mask);
	if (status != 0) {
		nvgpu_err(g, "Failed to send ADC perf_load RPC status=0x%x", status);
		return status;
	}

	nvgpu_log_info(g, "ADC perf_load completed successfully with action_mask=0x%x", action_mask);
	return 0;
}

void nvgpu_pmu_clk_rpc_handler(struct gk20a *g, struct nv_pmu_rpc_header *rpc)
{
	switch (rpc->function) {
	case NV_PMU_RPC_ID_CLK_BOARD_OBJ_GRP_IFACE_MODEL_10_CMD:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_CLK_BOARD_OBJ_GRP_IFACE_MODEL_10_CMD: %d",
			rpc->function);
		break;
	case NV_PMU_RPC_ID_CLK_BOARD_OBJ_GRP_IFACE_MODEL_PMU_INIT_1X_CMD:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_CLK_BOARD_OBJ_GRP_IFACE_MODEL_PMU_INIT_1X_CMD: %d",
			rpc->function);
		break;
	case NV_PMU_RPC_ID_CLK_CNTR_SAMPLE_DOMAIN:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_CLK_CNTR_SAMPLE_DOMAIN: %d",
			rpc->function);
		break;
	case NV_PMU_RPC_ID_CLK_CLK_DOMAIN_PROG_VOLT_TO_FREQ:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_CLK_CLK_DOMAIN_PROG_VOLT_TO_FREQ: %d",
			rpc->function);
		break;
	case NV_PMU_RPC_ID_CLK_CLK_DOMAIN_PROG_FREQ_TO_VOLT:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_CLK_CLK_DOMAIN_PROG_FREQ_TO_VOLT: %d",
			rpc->function);
		break;
	case NV_PMU_RPC_ID_CLK_LOAD:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_CLK_LOAD");
		break;
	default:
		nvgpu_err(g, "invalid CLK RPC reply, function=0x%x", rpc->function);
		break;
	}
}

void nvgpu_pmu_clk_deinit(struct gk20a *g)
{
	if ((g->pmu != NULL) && (g->pmu->clk_pmu != NULL)) {
		clk_domain_free_pmupstate(g);
		clk_vf_point_free_pmupstate(g);
		clk_fll_free_pmupstate(g);
		clk_vin_free_pmupstate(g);
		clk_enum_free_pmupstate(g);
		clk_vf_rel_free_pmupstate(g);
		clk_prop_top_rel_free_pmupstate(g);
		clk_prop_top_free_pmupstate(g);
		clk3_freq_domain_free_pmupstate(g);
		clk_free_pmupstate(g);
	}
}
