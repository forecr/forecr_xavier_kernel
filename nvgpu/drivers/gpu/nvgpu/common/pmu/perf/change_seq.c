// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/pmu.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/timers.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/pmu/perf.h>
#include "common/pmu/clk/clk.h"
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/pmu/super_surface.h>
#include <nvgpu/pmu/pmu_pstate.h>
#include <nvgpu/pmu/perf.h>
#include <nvgpu/pmu/volt.h>
#include <common/pmu/boardobj/boardobj.h>
#include "common/pmu/volt/volt_rail.h"
#include "common/pmu/clk/ucode_clk_inf.h"
#include "common/pmu/clk/clk_domain.h"

#include "change_seq.h"
#include "perf.h"

/*
 * Determine the correct voltage rail for GB20C based on clock domain
 */
static u8 gb20c_get_volt_rail_idx(u32 clk_domain)
{
	switch (clk_domain) {
	case CTRL_CLK_DOMAIN_GPCCLK:
		/* GPCCLK uses GPC rail */
		return GB20C_VOLT_RAIL_GPC_IDX;

	case CTRL_CLK_DOMAIN_XBARCLK:
	case CTRL_CLK_DOMAIN_SYSCLK:
	case CTRL_CLK_DOMAIN_NVDCLK:
	case CTRL_CLK_DOMAIN_UPROCCLK:
		/* Other domains use GPM rail */
		return GB20C_VOLT_RAIL_GPM_IDX;

	default:
		/* Default to GPC rail for unknown domains */
		return GB20C_VOLT_RAIL_GPC_IDX;
	}
}

static int perf_change_seq_sw_setup_super(struct gk20a *g,
		struct change_seq *p_change_seq)
{
	int status = 0;

	nvgpu_pmu_dbg(g, " ");

	/* Initialize parameters */
	p_change_seq->client_lock_mask = 0;

	p_change_seq->version = CTRL_PERF_CHANGE_SEQ_VERSION_35;

	status = nvgpu_boardobjgrpmask_init(
		&p_change_seq->clk_domains_exclusion_mask.super,
		32U, ((void*)0));
	if (status != 0) {
		nvgpu_err(g, "clk_domains_exclusion_mask failed to init %d",
			status);
		goto perf_change_seq_sw_setup_super_exit;
	}

	status = nvgpu_boardobjgrpmask_init(
		&p_change_seq->clk_domains_inclusion_mask.super,
		32U, ((void*)0));
	if (status != 0) {
		nvgpu_err(g, "clk_domains_inclusion_mask failed to init %d",
			status);
		goto perf_change_seq_sw_setup_super_exit;
	}

perf_change_seq_sw_setup_super_exit:
	return status;
}

int perf_change_seq_sw_setup(struct gk20a *g)
{
	struct change_seq_pmu *perf_change_seq_pmu =
		&(g->pmu->perf_pmu->changeseq_pmu);
	int status = 0;

	nvgpu_pmu_dbg(g, " ");

	(void) memset(perf_change_seq_pmu, 0,
			sizeof(struct change_seq_pmu));

	status = perf_change_seq_sw_setup_super(g, &perf_change_seq_pmu->super);
	if (status != 0) {
		goto exit;
	}

	perf_change_seq_pmu->super.b_enabled_pmu_support = true;
	/*exclude MCLK, may not be needed as MCLK is already fixed */
	perf_change_seq_pmu->super.clk_domains_exclusion_mask.super.data[0]
		= 0x04U;
	perf_change_seq_pmu->b_vf_point_check_ignore = false;
	perf_change_seq_pmu->b_lock = false;
	perf_change_seq_pmu->cpu_step_id_mask = 0;
	perf_change_seq_pmu->cpu_adverised_step_id_mask = 0;
	perf_change_seq_pmu->change_state = 0U;

exit:
	return status;
}

static void build_change_seq_boot (struct gk20a *g)
{
	struct nvgpu_pmu *pmu = g->pmu;
	struct change_seq_pmu *perf_change_seq_pmu =
		&(g->pmu->perf_pmu->changeseq_pmu);
	struct change_seq_pmu_script *script_last =
		&perf_change_seq_pmu->script_last;
	u8 num_domains = 0U;

	nvgpu_pmu_dbg(g, " ");

	script_last->super_surface_offset =
		nvgpu_pmu_get_ss_member_offset(g, pmu,
		NV_PMU_SUPER_SURFACE_MEMBER_CHANGE_SEQ_SET) +
		(u32)(sizeof(struct perf_change_seq_pmu_script) *
		SEQ_SCRIPT_LAST);

	nvgpu_mem_rd_n(g, nvgpu_pmu_super_surface_mem(g,
		pmu, pmu->super_surface),
		script_last->super_surface_offset,
		&script_last->buf,
		(u32) sizeof(struct perf_change_seq_pmu_script));

	script_last->buf.change.data.flags = CTRL_PERF_CHANGE_SEQ_CHANGE_NONE;

	num_domains = nvgpu_pmu_clk_domain_update_clk_info(g,
			&script_last->buf.change.data.clk_list);
	script_last->buf.change.data.clk_list.num_domains = num_domains;

	nvgpu_pmu_dbg(g,"Total domains = %d\n",
		script_last->buf.change.data.clk_list.num_domains);

	/* Assume everything is P0 - Need to find the index for P0  */
	script_last->buf.change.data.pstate_index =
			(u32)perf_pstate_get_table_entry_idx(g, CTRL_PERF_PSTATE_P0);

	nvgpu_mem_wr_n(g, nvgpu_pmu_super_surface_mem(g,
		pmu, pmu->super_surface),
		script_last->super_surface_offset,
		&script_last->buf,
		(u32) sizeof(struct perf_change_seq_pmu_script));

	return;
}

int perf_change_seq_pmu_setup(struct gk20a *g)
{
	struct nv_pmu_rpc_perf_change_seq_info_get info_get;
	struct nv_pmu_rpc_perf_change_seq_info_set info_set;
	struct nvgpu_pmu *pmu = g->pmu;
	struct change_seq_pmu *perf_change_seq_pmu =
		&(g->pmu->perf_pmu->changeseq_pmu);
	int status;

	/* Do this  till we enable performance table */
	build_change_seq_boot(g);

	(void) memset(&info_get, 0,
			sizeof(struct nv_pmu_rpc_perf_change_seq_info_get));
	(void) memset(&info_set, 0,
			sizeof(struct nv_pmu_rpc_perf_change_seq_info_set));

	PMU_RPC_EXECUTE_CPB(status, pmu, PERF, CHANGE_SEQ_INFO_GET, &info_get, 0);
	if (status != 0) {
		nvgpu_err(g,
			"Failed to execute Change Seq GET RPC status=0x%x",
			status);
		goto perf_change_seq_pmu_setup_exit;
	}

	info_set.info_set.super.version = perf_change_seq_pmu->super.version;

	status = nvgpu_boardobjgrpmask_export(
		&perf_change_seq_pmu->super.clk_domains_exclusion_mask.super,
		perf_change_seq_pmu->
		super.clk_domains_exclusion_mask.super.bitcount,
		&info_set.info_set.super.clk_domains_exclusion_mask.super);
	if ( status != 0 ) {
		nvgpu_err(g, "Could not export clkdomains exclusion mask");
		goto perf_change_seq_pmu_setup_exit;
	}

	status = nvgpu_boardobjgrpmask_export(
		&perf_change_seq_pmu->super.clk_domains_inclusion_mask.super,
		perf_change_seq_pmu->
		super.clk_domains_inclusion_mask.super.bitcount,
		&info_set.info_set.super.clk_domains_inclusion_mask.super);
	if ( status != 0 ) {
		nvgpu_err(g, "Could not export clkdomains inclusion mask");
		goto perf_change_seq_pmu_setup_exit;
	}

	info_set.info_set.b_vf_point_check_ignore =
		perf_change_seq_pmu->b_vf_point_check_ignore;
	info_set.info_set.cpu_step_id_mask =
		perf_change_seq_pmu->cpu_step_id_mask;
	info_set.info_set.b_lock =
		perf_change_seq_pmu->b_lock;

	perf_change_seq_pmu->script_last.super_surface_offset =
		nvgpu_pmu_get_ss_member_offset(g, pmu,
		NV_PMU_SUPER_SURFACE_MEMBER_CHANGE_SEQ_SET) +
		(u32)(sizeof(struct perf_change_seq_pmu_script) *
		SEQ_SCRIPT_LAST);

	nvgpu_mem_rd_n(g, nvgpu_pmu_super_surface_mem(g,
		pmu, pmu->super_surface),
		perf_change_seq_pmu->script_last.super_surface_offset,
		&perf_change_seq_pmu->script_last.buf,
		(u32) sizeof(struct perf_change_seq_pmu_script));

	/* Assume everything is P0 - Need to find the index for P0  */
	perf_change_seq_pmu->script_last.buf.change.data.pstate_index =
			(u32)perf_pstate_get_table_entry_idx(g, CTRL_PERF_PSTATE_P0);

	nvgpu_mem_wr_n(g, nvgpu_pmu_super_surface_mem(g,
		pmu, pmu->super_surface),
		perf_change_seq_pmu->script_last.super_surface_offset,
		&perf_change_seq_pmu->script_last.buf,
		(u32) sizeof(struct perf_change_seq_pmu_script));

	/* Continue with PMU setup, assume FB map is done  */
	PMU_RPC_EXECUTE_CPB(status, pmu, PERF, CHANGE_SEQ_INFO_SET, &info_set, 0);
	if (status != 0) {
		nvgpu_err(g,
			"Failed to execute Change Seq SET RPC status=0x%x",
			status);
		goto perf_change_seq_pmu_setup_exit;
	}

perf_change_seq_pmu_setup_exit:
	return status;
}

/*
 * Todo: Remove this function and
 * the obsolete gv100 clk_arb code that uses this function.
 * For now return success.
 */
int nvgpu_pmu_perf_changeseq_set_clks(struct gk20a *g,
		struct nvgpu_clk_slave_freq *vf_point)
{
	(void) g;
	(void) vf_point;

	return 0;
}

/*
 * Helper function to read current voltage from a voltage rail
 */
static int get_current_rail_voltage(struct gk20a *g, u8 rail_idx, u32 *current_voltage_uv)
{
	void *pmu_status_buf = NULL;
	int status = 0;

	/* Get PMU voltage status */
	status = nvgpu_volt_rail_get_status(g, &pmu_status_buf);
	if (status != 0) {
		nvgpu_err(g, "Failed to get PMU voltage status: %d", status);
		return status;
	}

	/* Read voltage based on rail index */
	if (rail_idx == GB20C_VOLT_RAIL_GPC_IDX) {
		status = nvgpu_volt_rail_get_gpc_voltage(g, pmu_status_buf, current_voltage_uv);
		if (status != 0) {
			nvgpu_err(g, "Failed to get GPC voltage: %d", status);
			return status;
		}
	} else if (rail_idx == GB20C_VOLT_RAIL_GPM_IDX) {
		status = nvgpu_volt_rail_get_gpm_voltage(g, pmu_status_buf, current_voltage_uv);
		if (status != 0) {
			nvgpu_err(g, "Failed to get GPM voltage: %d", status);
			return status;
		}
	} else {
		nvgpu_err(g, "Unsupported rail index: %u", rail_idx);
		return -EINVAL;
	}

	nvgpu_pmu_dbg(g, "Current voltage for rail %u: %u uV", rail_idx, *current_voltage_uv);
	return 0;
}


/*
 * Set voltage only via change sequencer without affecting clocks
 */
int nvgpu_pmu_perf_changeseq_set_voltage_only(struct gk20a *g, u8 rail_idx, u32 voltage_uv)
{
	struct nvgpu_pmu *pmu = g->pmu;
	struct nv_pmu_rpc_perf_change_seq_queue_change rpc;
	struct ctrl_perf_change_seq_change_input change_input;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	int status = 0;
	u32 vmin_uv = 0U, vmax_uv = 0U;
	/* Voltage rail selection logic */
	u32 volt_rails_mask = 0U;
	u32 i;
	bool gpc_rail_active = false;
	bool gpm_rail_active = false;
	u32 gpc_idx = 0, xbar_idx = 0, nvd_idx = 0, uproc_idx = 0;
	u32 current_gpc_freq_khz, current_xbar_freq_khz, current_nvd_freq_khz, current_uproc_freq_khz;
	u32 other_rail_idx, other_vmin_uv = 0U, other_vmax_uv = 0U;
	u32 other_current_voltage_uv = 0U;
	/* PTO counter register definitions for each clock domain */
	struct pto_counter_regs {
		u32 cfg;
		u32 cnt0;
		u32 cnt1;
	};
	const struct pto_counter_regs pto_gpc_regs = {
		.cfg  = 0x00132a74U,
		.cnt0 = 0x00132a78U,
		.cnt1 = 0x00132a7cU,
	};
	const struct pto_counter_regs pto_sys_regs = {
		.cfg  = 0x00136e74U,
		.cnt0 = 0x00136e78U,
		.cnt1 = 0x00136e7cU,
	};
	const struct pto_counter_regs pto_nvd_regs = {
		.cfg  = 0x00137574U,
		.cnt0 = 0x00137578U,
		.cnt1 = 0x0013757cU,
	};
	const struct pto_counter_regs pto_uproc_regs = {
		.cfg  = 0x00138274U,
		.cnt0 = 0x00138278U,
		.cnt1 = 0x0013827cU,
	};
	bool elcg_was_enabled = false;

	nvgpu_pmu_dbg(g, " ");

	/* Check if power management data is available */
	if (!pm_data) {
		nvgpu_err(g, "Power management data not initialized");
		return -ENODEV;
	}

	/* Set clock mode to MANUAL if not already set */
	if (pm_data->clock_mode != CLOCK_MODE_MANUAL) {
		pm_data->clock_mode = CLOCK_MODE_MANUAL;
		nvgpu_pmu_dbg(g, "Set clock mode to MANUAL");
	}

	(void) memset(&change_input, 0,
		sizeof(struct ctrl_perf_change_seq_change_input));

	// Set PState index to P0.
	change_input.pstate_index = 6;
	change_input.flags = (u32)CTRL_PERF_CHANGE_SEQ_CHANGE_FORCE;
	change_input.vf_points_cache_counter = 0xFFFFFFFFU;

	/* Set up clock domains and frequencies like nvgpu_pmu_perf_changeseq_set_clks_50 */

	// TODO: This is a temporary fix to get the clock domains.
	// The optimal solution is to use Get_Change_Seq controller to get the clock domains.
	/* Get all domain indices like the working clock function does */
	status = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_GPCCLK, &gpc_idx);
	if (status != 0)
		goto done;
	status = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_XBARCLK, &xbar_idx);
	if (status != 0)
		goto done;
	status = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_NVDCLK, &nvd_idx);
	if (status != 0)
		goto done;
	status = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_UPROCCLK, &uproc_idx);
	if (status != 0)
		goto done;

	/* Set clock domain mask to include all domains */
	change_input.clk_domains_mask.super.data[0] = BIT32(gpc_idx) | BIT32(xbar_idx) |
						      BIT32(nvd_idx) | BIT32(uproc_idx);

	/* Get CURRENT frequencies using PTO counter reading - no PMU interaction */
	/* Temporarily disable ELCG to get accurate clock measurements */

	if (nvgpu_is_enabled(g, NVGPU_GPU_CAN_ELCG) && g->elcg_enabled) {
		elcg_was_enabled = true;
		nvgpu_cg_elcg_set_elcg_enabled(g, false);
		nvgpu_msleep(1); /* Wait for clocks to stabilize */
	}
	/* Helper macro to read PTO frequency - direct hardware register access */
	#define READ_PTO_FREQ(regs) ({ \
		u32 cfg_val, cnt0_val, cnt1_val; \
		u64 start_count, end_count, freq_hz; \
		s64 t_start, t_end, elapsed_us; \
		\
		/* PTO counter measurement sequence */ \
		gk20a_writel(g, (regs)->cfg, 0x29000000U); \
		nvgpu_usleep_range(1000, 2000); \
		gk20a_writel(g, (regs)->cfg, 0x28000000U); \
		gk20a_writel(g, (regs)->cfg, 0x2A000000U); \
		\
		/* Start counter measurement */ \
		cfg_val = 0x22000000U; \
		gk20a_writel(g, (regs)->cfg, cfg_val); \
		\
		cnt0_val = gk20a_readl(g, (regs)->cnt0); \
		cnt1_val = gk20a_readl(g, (regs)->cnt1); \
		start_count = ((u64)(cnt1_val & 0xFU) << 32) | cnt0_val; \
		\
		/* Resume counter */ \
		cfg_val = 0x2A000000U; \
		gk20a_writel(g, (regs)->cfg, cfg_val); \
		\
		/* Time measurement window */ \
		t_start = nvgpu_current_time_us(); \
		nvgpu_usleep_range(1000, 2000); \
		t_end = nvgpu_current_time_us(); \
		elapsed_us = t_end - t_start; \
		\
		/* Stop counter and read final count */ \
		cfg_val = 0x22000000U; \
		gk20a_writel(g, (regs)->cfg, cfg_val); \
		\
		cnt0_val = gk20a_readl(g, (regs)->cnt0); \
		cnt1_val = gk20a_readl(g, (regs)->cnt1); \
		end_count = ((u64)(cnt1_val & 0xFU) << 32) | cnt0_val; \
		\
		/* Resume counter */ \
		cfg_val = 0x2A000000U; \
		gk20a_writel(g, (regs)->cfg, cfg_val); \
		\
		/* Calculate frequency: (clock_cycles * 1e6) / elapsed_us */ \
		if (elapsed_us > 0) { \
			freq_hz = (end_count - start_count) * 1000000ULL / (u64)elapsed_us; \
		} else { \
			freq_hz = 0ULL; /* Fallback if timing failed */ \
		} \
		(u32)(freq_hz / 1000ULL); /* Convert Hz to kHz */ \
	})
	/* Read current frequencies for each clock domain using PTO counters */
	current_gpc_freq_khz = READ_PTO_FREQ(&pto_gpc_regs);
	current_xbar_freq_khz = READ_PTO_FREQ(&pto_sys_regs);  /* SYS = XBAR */
	current_nvd_freq_khz = READ_PTO_FREQ(&pto_nvd_regs);
	current_uproc_freq_khz = READ_PTO_FREQ(&pto_uproc_regs);
	#undef READ_PTO_FREQ
	/* Restore ELCG state if we disabled it */
	if (elcg_was_enabled)
		nvgpu_cg_elcg_set_elcg_enabled(g, true);
	nvgpu_pmu_dbg(g,
		   "PTO counter readings: GPC=%u kHz, XBAR=%u kHz, NVD=%u kHz, UPROC=%u kHz",
		   current_gpc_freq_khz, current_xbar_freq_khz,
		   current_nvd_freq_khz, current_uproc_freq_khz);

	/* Set frequencies for each clock domain */
	change_input.clk[gpc_idx].clk_freq_khz = current_gpc_freq_khz;
	change_input.clk[xbar_idx].clk_freq_khz = current_xbar_freq_khz;
	change_input.clk[nvd_idx].clk_freq_khz = current_nvd_freq_khz;
	change_input.clk[uproc_idx].clk_freq_khz = current_uproc_freq_khz;

	nvgpu_pmu_dbg(g, "Voltage-only change: using CURRENT frequencies GPC:%u XBAR:%u NVD:%u UPROC:%u kHz",
		   change_input.clk[gpc_idx].clk_freq_khz,
		   change_input.clk[xbar_idx].clk_freq_khz,
		   change_input.clk[nvd_idx].clk_freq_khz,
		   change_input.clk[uproc_idx].clk_freq_khz);

	/* Check which clock domains are being set and determine voltage rails */
	for (i = 0U; i < CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS; i++) {
		if ((change_input.clk_domains_mask.super.data[0] & BIT32(i)) != 0U) {
			u32 domain_api = 0U;

			/* Get the API domain from the domain index */
			if (clkClkDomainGetApiDomainByIndex(g, &domain_api, i) == 0) {
				u8 rail_idx_for_domain = gb20c_get_volt_rail_idx(domain_api);

				volt_rails_mask |= BIT32(rail_idx_for_domain);

				if (rail_idx_for_domain == GB20C_VOLT_RAIL_GPC_IDX)
					gpc_rail_active = true;
				else if (rail_idx_for_domain == GB20C_VOLT_RAIL_GPM_IDX)
					gpm_rail_active = true;

				nvgpu_pmu_dbg(g, "Domain index %u -> API 0x%x -> rail %u",
					   i, domain_api, rail_idx_for_domain);
			} else {
				nvgpu_warn(g, "Failed to get API domain for index %u", i);
			}
		}
	}

	// TODO: This is a temporary fix to get the voltage limits.
	// The optimal solution is to use Get_Change_Seq controller to get the voltage limits.
	/* Set voltage for the target rail */
	status = nvgpu_volt_rail_get_limits(g, rail_idx, &vmin_uv, &vmax_uv);
	if (status != 0) {
		nvgpu_err(g, "Failed to get voltage limits for rail %u: %d", rail_idx, status);
		goto done;
	}

	nvgpu_pmu_dbg(g, "Rail %u voltage limits: Vmin: %u uV, Vmax: %u uV", rail_idx, vmin_uv, vmax_uv);

	/* Validate and clamp voltage */
	if (voltage_uv < vmin_uv) {
		nvgpu_pmu_dbg(g, "Requested voltage %u uV is below vmin %u uV for rail %u, clamping to vmin",
			   voltage_uv, vmin_uv, rail_idx);
		voltage_uv = vmin_uv;
	} else if (voltage_uv > vmax_uv) {
		nvgpu_pmu_dbg(g, "Requested voltage %u uV is above vmax %u uV for rail %u, clamping to vmax",
			   voltage_uv, vmax_uv, rail_idx);
		voltage_uv = vmax_uv;
	}

	change_input.volt[rail_idx].voltage_uv = voltage_uv;
	change_input.volt[rail_idx].voltage_min_noise_unaware_uv = vmin_uv;

	/* For the other rail, get current voltage and maintain it */
	other_rail_idx = (rail_idx == GB20C_VOLT_RAIL_GPC_IDX) ?
			  GB20C_VOLT_RAIL_GPM_IDX : GB20C_VOLT_RAIL_GPC_IDX;

	status = get_current_rail_voltage(g, other_rail_idx, &other_current_voltage_uv);
	if (status == 0 && other_current_voltage_uv > 0) {
		change_input.volt[other_rail_idx].voltage_uv = other_current_voltage_uv;
		change_input.volt[other_rail_idx].voltage_min_noise_unaware_uv =
			other_current_voltage_uv;
		nvgpu_pmu_dbg(g, "Maintaining other rail %u at current voltage %u uV",
			   other_rail_idx, other_current_voltage_uv);
	} else {
		/* Fallback to minimum voltage if current voltage reading fails */
		status = nvgpu_volt_rail_get_limits(g, other_rail_idx, &other_vmin_uv, &other_vmax_uv);
		if (status == 0) {
			change_input.volt[other_rail_idx].voltage_uv = other_vmin_uv;
			change_input.volt[other_rail_idx].voltage_min_noise_unaware_uv = other_vmin_uv;
			nvgpu_pmu_dbg(g, "Using vmin %u uV as fallback for other rail %u",
				   other_vmin_uv, other_rail_idx);
		} else {
			nvgpu_err(g, "Failed to get limits for other rail %u: %d",
				   other_rail_idx, status);
			goto done;
		}
	}

	/* Set the voltage rail mask to include both rails */
	change_input.volt_rails_mask.super.data[0] = volt_rails_mask |= BIT32(rail_idx);

	nvgpu_pmu_dbg(g, "Final voltage values: GPC=%u uV, GPM=%u uV",
		   change_input.volt[GB20C_VOLT_RAIL_GPC_IDX].voltage_uv,
		   change_input.volt[GB20C_VOLT_RAIL_GPM_IDX].voltage_uv);

	/* RPC to PMU to queue to execute change sequence request*/
	(void) memset(&rpc, 0,
			sizeof(struct nv_pmu_rpc_perf_change_seq_queue_change));
	rpc.change = change_input;

	PMU_RPC_EXECUTE_CPB(status, pmu, PERF, CHANGE_SEQ_QUEUE_CHANGE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute voltage-only Change Seq RPC - status: %d", status);
		goto done;
	}

	nvgpu_pmu_dbg(g, "Successfully set rail %u voltage to %u uV", rail_idx, voltage_uv);

	/* Update current settings to the voltages set */
	g->pmu->clk_pmu->clk_domainobjs_50->currentClocksVfInfo.gpc_volt_uv =
		change_input.volt[GB20C_VOLT_RAIL_GPC_IDX].voltage_uv;
	g->pmu->clk_pmu->clk_domainobjs_50->currentClocksVfInfo.gpm_volt_uv =
		change_input.volt[GB20C_VOLT_RAIL_GPM_IDX].voltage_uv;

done:
	nvgpu_pmu_dbg(g, "done status %d", status);
	return status;
}

int nvgpu_pmu_perf_changeseq_set_clks_50(struct gk20a *g,
		struct nvgpu_clk_vf_point_5x *target_vf_info)
{
	struct nvgpu_pmu *pmu = g->pmu;
	struct nv_pmu_rpc_perf_change_seq_queue_change rpc;
	struct ctrl_perf_change_seq_change_input change_input;
	struct change_seq_pmu *change_seq_pmu = &g->pmu->perf_pmu->changeseq_pmu;
	int status = 0;
	u32 domainIdx = 0U;
	u32 vmin_uv = 0U, vmax_uv = 0U;
	/* Voltage rail selection logic */
	u32 volt_rails_mask = 0U;
	u32 i;
	bool gpc_rail_active = false;
	bool gpm_rail_active = false;

	nvgpu_pmu_dbg(g, " ");

	(void) memset(&change_input, 0,
		sizeof(struct ctrl_perf_change_seq_change_input));


	// Set PState index to P0.
	change_input.pstate_index = 6;
	change_input.flags = (u32)CTRL_PERF_CHANGE_SEQ_CHANGE_FORCE;
	change_input.vf_points_cache_counter = 0xFFFFFFFFU;

	change_input.clk_domains_mask.super.data[0] = target_vf_info->clk_domain_mask;
	// set clock frequencies for each clock domain
	status = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_GPCCLK, &domainIdx);
	if (status != 0) {
		goto done;
	}
	nvgpu_pmu_dbg(g, "Using gpc target: %d Mhz", target_vf_info->gpc_mhz);
	change_input.clk[domainIdx].clk_freq_khz = target_vf_info->gpc_mhz * 1000U;
	status = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_XBARCLK, &domainIdx);
	if (status != 0) {
		goto done;
	}
	change_input.clk[domainIdx].clk_freq_khz = target_vf_info->sys_mhz * 1000U;
	status = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_NVDCLK, &domainIdx);
	if (status != 0) {
		goto done;
	}
	change_input.clk[domainIdx].clk_freq_khz = target_vf_info->nvd_mhz * 1000U;
	status = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_UPROCCLK, &domainIdx);
	if (status != 0)
		goto done;
	change_input.clk[domainIdx].clk_freq_khz = target_vf_info->uproc_mhz * 1000U;

	/* Check which clock domains are being set and determine voltage rails */
	nvgpu_pmu_dbg(g, "Using Knobs: %d Mhz", target_vf_info->gpc_mhz);

	nvgpu_pmu_dbg(g, "change_input: clk_domains_mask = 0x%x",
			change_input.clk_domains_mask.super.data[0]);
	nvgpu_pmu_dbg(g, "change_input:   clk freqs:  %d  %d  %d  %d  %d",
			change_input.clk[0].clk_freq_khz, change_input.clk[1].clk_freq_khz,
			change_input.clk[2].clk_freq_khz, change_input.clk[3].clk_freq_khz,
			change_input.clk[4].clk_freq_khz);

	for (i = 0U; i < CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS; i++) {
		if ((change_input.clk_domains_mask.super.data[0] & BIT32(i)) != 0U) {
			u32 domain_api = 0U;

			/* Get the API domain from the domain index */
			if (clkClkDomainGetApiDomainByIndex(g, &domain_api, i) == 0) {
				u8 rail_idx = gb20c_get_volt_rail_idx(domain_api);

				volt_rails_mask |= BIT32(rail_idx);

				if (rail_idx == GB20C_VOLT_RAIL_GPC_IDX)
					gpc_rail_active = true;
				else if (rail_idx == GB20C_VOLT_RAIL_GPM_IDX)
					gpm_rail_active = true;

				nvgpu_pmu_dbg(g, "Domain index %u -> API 0x%x -> rail %u",
					i, domain_api, rail_idx);
			} else {
				nvgpu_warn(g, "Failed to get API domain for index %u", i);
			}
		}
	}

	/* Set voltage for active rails */
	if (gpc_rail_active) {
		status = nvgpu_volt_rail_get_limits(g, GB20C_VOLT_RAIL_GPC_IDX, &vmin_uv, &vmax_uv);

		nvgpu_pmu_dbg(g, "GPC: chip voltage limits: Vmin: %d Vmax: %d",
				vmin_uv, vmax_uv);

		if (status != 0) {
			nvgpu_err(g, "Get vmin,vmax failed, proceeding with evaluated Volt value");
		} else if (vmin_uv > target_vf_info->gpc_volt_uv) {
			nvgpu_pmu_dbg(g, "GPC Vmin %d is higher than requested Volt %d, set to vmin",
					vmin_uv, target_vf_info->gpc_volt_uv);
			target_vf_info->gpc_volt_uv = vmin_uv;
		} else if (target_vf_info->gpc_volt_uv > vmax_uv) {
			nvgpu_pmu_dbg(g, "GPC: Requested GPC voltage %d is more than chip max %d, set to vmax",
					target_vf_info->gpc_volt_uv, vmax_uv);
			target_vf_info->gpc_volt_uv = vmax_uv;
		}
		change_input.volt[GB20C_VOLT_RAIL_GPC_IDX].voltage_uv = target_vf_info->gpc_volt_uv;
		change_input.volt[GB20C_VOLT_RAIL_GPC_IDX].voltage_min_noise_unaware_uv = vmin_uv;
	}

	if (gpm_rail_active) {
		status = nvgpu_volt_rail_get_limits(g, GB20C_VOLT_RAIL_GPM_IDX, &vmin_uv, &vmax_uv);

		nvgpu_pmu_dbg(g, "GPM: chip voltage limits: Vmin: %d Vmax: %d", vmin_uv, vmax_uv);

		if (status != 0) {
			nvgpu_err(g, "Get vmin,vmax failed, proceeding with evaluated Volt value");
		} else if (vmin_uv > target_vf_info->gpm_volt_uv) {
			nvgpu_pmu_dbg(g, "GPM Vmin %d is higher than requested Volt %d, set to vmin",
					vmin_uv, target_vf_info->gpm_volt_uv);
			target_vf_info->gpm_volt_uv = vmin_uv;
		} else if (target_vf_info->gpm_volt_uv > vmax_uv) {
			nvgpu_pmu_dbg(g, "GPM: Requested GPM voltage %d is more than chip max %d, set to vmax",
					target_vf_info->gpm_volt_uv, vmax_uv);
			target_vf_info->gpm_volt_uv = vmax_uv;
		}
		change_input.volt[GB20C_VOLT_RAIL_GPM_IDX].voltage_uv = target_vf_info->gpm_volt_uv;
		change_input.volt[GB20C_VOLT_RAIL_GPM_IDX].voltage_min_noise_unaware_uv = vmin_uv;
	}

	/* Set the voltage rail mask */
	change_input.volt_rails_mask.super.data[0] = volt_rails_mask;

	nvgpu_pmu_dbg(g, "voltage rails mask: 0x%02x (GPC:%s, GPM:%s)",
			volt_rails_mask,
			gpc_rail_active ? "active" : "inactive",
			gpm_rail_active ? "active" : "inactive");
	nvgpu_pmu_dbg(g, "change_input  GPC-volt: %d  GPM-volt: %d",
			change_input.volt[GB20C_VOLT_RAIL_GPC_IDX].voltage_uv,
			change_input.volt[GB20C_VOLT_RAIL_GPM_IDX].voltage_uv);

	/* RPC to PMU to queue to execute change sequence request*/
	(void) memset(&rpc, 0,
			sizeof(struct nv_pmu_rpc_perf_change_seq_queue_change));
	rpc.change = change_input;
	rpc.change.pstate_index = change_input.pstate_index;
	change_seq_pmu->change_state = 0U;
	change_seq_pmu->start_time = nvgpu_current_time_us();
	nvgpu_pmu_dbg(g, "change_input:  flags: 0x%x  PSindex: %d vfCacheCounter: 0x%x",
			change_input.flags, rpc.change.pstate_index,
			change_input.vf_points_cache_counter);
	nvgpu_pmu_dbg(g, "change_input:  vRailsMask.data: 0x%02x",
			change_input.volt_rails_mask.super.data[0]);
	nvgpu_pmu_dbg(g, "Change_Seq  RPC:  VFPtChkIgnore: 0x%x  change_state: 0x%02x  stepIdMask: 0x%02x",
			change_seq_pmu->b_vf_point_check_ignore, change_seq_pmu->change_state,
			change_seq_pmu->cpu_step_id_mask);

	nvgpu_pmu_dbg(g, "Executing Change_Seq  RPC ");
	PMU_RPC_EXECUTE_CPB(status, pmu, PERF,
			CHANGE_SEQ_QUEUE_CHANGE, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute Change Seq RPC - status: %d",
			status);
	}

	/* Wait for sync change to complete. */
	if ((rpc.change.flags & CTRL_PERF_CHANGE_SEQ_CHANGE_ASYNC) == 0U) {
		/* wait till RPC execute in PMU & ACK */
		if (nvgpu_pmu_wait_fw_ack_status(g, pmu,
				nvgpu_get_poll_timeout(g),
				&change_seq_pmu->change_state, 1U) != 0) {
			nvgpu_err(g, "PMU wait timeout expired.");
			status = -ETIMEDOUT;
		}
	}
	change_seq_pmu->stop_time = nvgpu_current_time_us();
done:
	nvgpu_pmu_dbg(g, "Done - status: %d", status);
	return status;
}

void nvgpu_perf_change_seq_execute_time(struct gk20a *g, s64 *change_time)
{
	struct change_seq_pmu *change_seq_pmu =
			&g->pmu->perf_pmu->changeseq_pmu;
	s64 diff = change_seq_pmu->stop_time - change_seq_pmu->start_time;

	*change_time = diff;
}
