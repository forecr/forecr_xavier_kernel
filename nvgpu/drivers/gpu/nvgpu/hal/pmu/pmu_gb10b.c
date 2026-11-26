// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pmu.h>
#include <nvgpu/io.h>
#include <nvgpu/mm.h>
#include <nvgpu/firmware.h>
#include <nvgpu/pmu/debug.h>
#include <nvgpu/pmu/pmu_pg.h>
#include <nvgpu/soc.h>
#include <nvgpu/device.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/timers.h>

#include "pmu_gb10b.h"

#include <nvgpu/hw/gb10b/hw_pwr_gb10b.h>

#define NV_PPWR_PMU_IDLE_MASK_CLEAR_ALL(type) \
		nvgpu_writel(g, pwr_pmu_idle_mask_r(IDLE_COUNTER_##type), 0x0U);\
		nvgpu_writel(g, pwr_pmu_idle_mask_1_r(IDLE_COUNTER_##type), 0x0U);\
		nvgpu_writel(g, pwr_pmu_idle_mask_2_r(IDLE_COUNTER_##type), 0x0U);\
		nvgpu_writel(g, pwr_pmu_idle_mask_3_r(IDLE_COUNTER_##type), 0x0U)

void gb10b_pmu_dump_elpg_stats(struct nvgpu_pmu *pmu)
{
	struct gk20a *g = pmu->g;

	nvgpu_pmu_dbg(g, "pwr_pmu_idle_mask_supp_r(3): 0x%08x",
		nvgpu_readl(g, pwr_pmu_idle_mask_supp_r(3)));
	nvgpu_pmu_dbg(g, "pwr_pmu_idle_mask_1_supp_r(3): 0x%08x",
		nvgpu_readl(g, pwr_pmu_idle_mask_1_supp_r(3)));
	nvgpu_pmu_dbg(g, "pwr_pmu_idle_ctrl_supp_r(3): 0x%08x",
		nvgpu_readl(g, pwr_pmu_idle_ctrl_supp_r(3)));
	nvgpu_pmu_dbg(g, "pwr_pmu_pg_idle_cnt_r(0): 0x%08x",
		nvgpu_readl(g, pwr_pmu_pg_idle_cnt_r(0)));
	nvgpu_pmu_dbg(g, "pwr_pmu_pg_intren_r(0): 0x%08x",
		nvgpu_readl(g, pwr_pmu_pg_intren_r(0)));

	nvgpu_pmu_dbg(g, "pwr_pmu_idle_count_r(3): 0x%08x",
		nvgpu_readl(g, pwr_pmu_idle_count_r(3)));
	nvgpu_pmu_dbg(g, "pwr_pmu_idle_count_r(4): 0x%08x",
		nvgpu_readl(g, pwr_pmu_idle_count_r(4)));
	nvgpu_pmu_dbg(g, "pwr_pmu_idle_count_r(7): 0x%08x",
		nvgpu_readl(g, pwr_pmu_idle_count_r(7)));
}

void gb10b_pmu_init_elcg_mode(struct gk20a *g, u32 mode, u32 engine)
{
	u32 gate_ctrl;

	if (!nvgpu_is_enabled(g, NVGPU_GPU_CAN_ELCG)) {
		return;
	}

	gate_ctrl = nvgpu_readl(g, pwr_gate_ctrl_r(engine));

	switch (mode) {
	case ELCG_RUN:
		gate_ctrl = set_field(gate_ctrl,
			pwr_gate_ctrl_eng_clk_m(),
			pwr_gate_ctrl_eng_clk_run_f());
		gate_ctrl = set_field(gate_ctrl,
			pwr_gate_ctrl_idle_holdoff_m(),
			pwr_gate_ctrl_idle_holdoff_on_f());
		break;
	case ELCG_STOP:
		gate_ctrl = set_field(gate_ctrl,
			pwr_gate_ctrl_eng_clk_m(),
			pwr_gate_ctrl_eng_clk_stop_f());
		break;
	case ELCG_AUTO:
		gate_ctrl = set_field(gate_ctrl,
			pwr_gate_ctrl_eng_clk_m(),
			pwr_gate_ctrl_eng_clk_auto_f());
		break;
	default:
		nvgpu_err(g, "invalid elcg mode %d", mode);
	break;
}

	nvgpu_writel(g, pwr_gate_ctrl_r(engine), gate_ctrl);
}

int gb10b_pmu_elcg_init_idle_filters(struct gk20a *g)
{
	u32 gate_ctrl, idle_filter;
	u32 i;
	const struct nvgpu_device *dev;
	struct nvgpu_fifo *f = &g->fifo;

	if (nvgpu_platform_is_simulation(g)) {
			return 0;
	}

	nvgpu_log_info(g, "init clock/power gate reg");

	for (i = 0; i < f->num_engines; i++) {
		dev = f->active_engines[i];

		gate_ctrl = nvgpu_readl(g, pwr_gate_ctrl_r(dev->engine_id));
		gate_ctrl = set_field(gate_ctrl,
				pwr_gate_ctrl_eng_idle_filt_exp_m(),
				pwr_gate_ctrl_eng_idle_filt_exp__prod_f());
		gate_ctrl = set_field(gate_ctrl,
				pwr_gate_ctrl_eng_idle_filt_mant_m(),
				pwr_gate_ctrl_eng_idle_filt_mant__prod_f());
		gate_ctrl = set_field(gate_ctrl,
				pwr_gate_ctrl_eng_delay_before_m(),
				pwr_gate_ctrl_eng_delay_before__prod_f());
		gate_ctrl = set_field(gate_ctrl,
				pwr_gate_ctrl_eng_delay_after_m(),
				pwr_gate_ctrl_eng_delay_after__prod_f());
		nvgpu_writel(g, pwr_gate_ctrl_r(dev->engine_id), gate_ctrl);
	}

	idle_filter = nvgpu_readl(g, pwr_sys_pri_hub_idle_filter_r());
	idle_filter = set_field(idle_filter,
				pwr_sys_pri_hub_idle_filter_value_m(),
				pwr_sys_pri_hub_idle_filter_value__prod_f());
	nvgpu_writel(g, pwr_sys_pri_hub_idle_filter_r(), idle_filter);

	idle_filter = nvgpu_readl(g, pwr_hubmmu_idle_filter_r());
	idle_filter = set_field(idle_filter,
				pwr_hubmmu_idle_filter_value_m(),
				pwr_hubmmu_idle_filter_value__prod_f());
	nvgpu_writel(g, pwr_hubmmu_idle_filter_r(), idle_filter);

	return 0;
}

void gb10b_pmu_init_blcg_mode(struct gk20a *g, u32 mode, u32 engine)
{
	u32 gate_ctrl;
	bool error_status = false;

	if (!nvgpu_is_enabled(g, NVGPU_GPU_CAN_BLCG)) {
		return;
	}

	gate_ctrl = nvgpu_readl(g, pwr_gate_ctrl_r(engine));

	switch (mode) {
	case BLCG_RUN:
		gate_ctrl = set_field(gate_ctrl,
				pwr_gate_ctrl_blk_clk_m(),
				pwr_gate_ctrl_blk_clk_run_f());
		break;
	case BLCG_AUTO:
		gate_ctrl = set_field(gate_ctrl,
				pwr_gate_ctrl_blk_clk_m(),
				pwr_gate_ctrl_blk_clk_auto_f());
		break;
	default:
		nvgpu_err(g,
			"invalid blcg mode %d", mode);
		error_status = true;
		break;
	}

	if (error_status == true) {
		return;
	}

	nvgpu_writel(g, pwr_gate_ctrl_r(engine), gate_ctrl);
}

bool gb10b_is_cg_supported_by_pmu(struct gk20a *g)
{
	(void)g;
	return true;
}

/* error handler */
void gb10b_clear_pmu_bar0_host_err_status(struct gk20a *g)
{
	u32 status;

	status = nvgpu_readl(g, pwr_pmu_bar0_host_error_r());
	nvgpu_writel(g, pwr_pmu_bar0_host_error_r(), status);
}

u32 gb10b_pmu_read_idle_counter(struct gk20a *g, u32 counter_id)
{
	return pwr_pmu_idle_count_value_v(
		nvgpu_readl(g, pwr_pmu_idle_count_r(counter_id)));
}

void gb10b_pmu_reset_idle_counter(struct gk20a *g, u32 counter_id)
{
	nvgpu_writel(g, pwr_pmu_idle_count_r(counter_id),
		pwr_pmu_idle_count_reset_f(1));
}

u32 gb10b_pmu_read_idle_intr_status(struct gk20a *g)
{
	return pwr_pmu_idle_intr_status_intr_v(
		gk20a_readl(g, pwr_pmu_idle_intr_status_r()));
}

void gb10b_pmu_clear_idle_intr_status(struct gk20a *g)
{
	nvgpu_writel(g, pwr_pmu_idle_intr_status_r(),
		pwr_pmu_idle_intr_status_intr_f(1));
}

u32 gb10b_pmu_get_mutex_reg(struct gk20a *g, u32 i)
{
	return nvgpu_readl(g, pwr_pmu_mutex_r(i));
}

void gb10b_pmu_set_mutex_reg(struct gk20a *g, u32 i, u32 data)
{
	nvgpu_writel(g, pwr_pmu_mutex_r(i), data);
}

u32 gb10b_pmu_get_mutex_id(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_mutex_id_r());
}

u32 gb10b_pmu_get_mutex_id_release(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_mutex_id_release_r());
}

void gb10b_pmu_set_mutex_id_release(struct gk20a *g, u32 data)
{
	nvgpu_writel(g, pwr_pmu_mutex_id_release_r(), data);
}

u32 gb10b_pmu_falcon_base_addr(void)
{
	return pwr_falcon_irqsset_r();
}

u32 gb10b_pmu_falcon2_base_addr(void)
{
	return pwr_falcon2_pwr_base_r();
}

u32 gb10b_pmu_get_irqstat(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_falcon_irqstat_r());
}

void gb10b_pmu_set_mailbox1(struct gk20a *g, u32 val)
{
	nvgpu_writel(g, pwr_falcon_mailbox1_r(), val);
}

u32 gb10b_pmu_get_ecc_address(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_falcon_ecc_address_r());
}

u32 gb10b_pmu_get_ecc_status(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_falcon_ecc_status_r());
}

void gb10b_pmu_set_ecc_status(struct gk20a *g, u32 val)
{
	nvgpu_writel(g, pwr_pmu_falcon_ecc_status_r(), val);
}

void gb10b_pmu_set_irqsclr(struct gk20a *g, u32 intr)
{
	nvgpu_writel(g, pwr_falcon_irqsclr_r(), intr);
}

void gb10b_pmu_set_irqsset(struct gk20a *g, u32 intr)
{
	nvgpu_writel(g, pwr_falcon_irqsset_r(), intr);
}

u32 gb10b_pmu_get_exterrstat(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_falcon_exterrstat_r());
}

void gb10b_pmu_set_exterrstat(struct gk20a *g, u32 intr)
{
	nvgpu_writel(g, pwr_falcon_exterrstat_r(), intr);
}

u32 gb10b_pmu_get_exterraddr(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_falcon_exterraddr_r());
}

u32 gb10b_pmu_get_bar0_addr(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_addr_r());
}

u32 gb10b_pmu_get_bar0_data(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_data_r());
}

u32 gb10b_pmu_get_bar0_timeout(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_timeout_r());
}

u32 gb10b_pmu_get_bar0_ctl(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_ctl_r());
}

u32 gb10b_pmu_get_bar0_error_status(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_error_status_r());
}

void gb10b_pmu_set_bar0_error_status(struct gk20a *g, u32 val)
{
	return nvgpu_writel(g, pwr_pmu_bar0_error_status_r(), val);
}

u32 gb10b_pmu_get_bar0_fecs_error(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_fecs_error_r());
}

void gb10b_pmu_set_bar0_fecs_error(struct gk20a *g, u32 val)
{
	return nvgpu_writel(g, pwr_pmu_bar0_fecs_error_r(), val);
}

u32 gb10b_pmu_get_mailbox(struct gk20a *g, u32 i)
{
	return nvgpu_readl(g, pwr_pmu_mailbox_r(i));
}

u32 gb10b_pmu_get_pmu_debug(struct gk20a *g, u32 i)
{
	return nvgpu_readl(g, pwr_pmu_debug_r(i));
}

u32 gb10b_pmu_get_pmu_msgq_head(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_msgq_head_r());
}

void gb10b_pmu_set_pmu_msgq_head(struct gk20a *g, u32 data)
{
       nvgpu_writel(g, pwr_pmu_msgq_head_r(), data );
}

u32 gb10b_pmu_queue_head_r(u32 i)
{
	return pwr_pmu_queue_head_r(i);
}

u32 gb10b_pmu_queue_head__size_1_v(void)
{
	return pwr_pmu_queue_head__size_1_v();
}

u32 gb10b_pmu_queue_tail_r(u32 i)
{
	return pwr_pmu_queue_tail_r(i);
}

u32 gb10b_pmu_queue_tail__size_1_v(void)
{
	return pwr_pmu_queue_tail__size_1_v();
}

void gb10b_pmu_pg_idle_counter_config(struct gk20a *g, u32 pg_engine_id)
{
	nvgpu_writel(g, pwr_pmu_pg_idlefilth_r(pg_engine_id),
		PMU_PG_IDLE_THRESHOLD);
	nvgpu_writel(g, pwr_pmu_pg_ppuidlefilth_r(pg_engine_id),
		PMU_PG_POST_POWERUP_IDLE_THRESHOLD);
}

void gb10b_pmu_set_new_instblk(struct gk20a *g, u32 data)
{
	nvgpu_writel(g, pwr_pmu_new_instblk_r(), data);
}

#ifdef CONFIG_NVGPU_LS_PMU
void gb10b_write_dmatrfbase(struct gk20a *g, u32 addr)
{
	nvgpu_writel(g, pwr_falcon_dmatrfbase_r(), addr);
	nvgpu_writel(g, pwr_falcon_dmatrfbase1_r(), 0x0U);
}
#endif /* CONFIG_NVGPU_LS_PMU */

void gb10b_pmu_init_perfmon_counter(struct gk20a *g)
{
	u32 data;

	nvgpu_writel(g, pwr_pmu_idle_intr_r(),
		pwr_pmu_idle_intr_en_f(0));

	/* set BASE cntr threshold to detect overflow and idle mode to ALWAYS */
	nvgpu_writel(g, pwr_pmu_idle_threshold_r(IDLE_COUNTER_BASE),
		pwr_pmu_idle_threshold_value_f(PMU_IDLE_THRESHOLD_V));

	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_BASE), 0x0U);
	data = nvgpu_readl(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_BASE));
	data = set_field(data, pwr_pmu_idle_ctrl_value_m() |
		pwr_pmu_idle_ctrl_filter_m(),
		pwr_pmu_idle_ctrl_value_always_f() |
		pwr_pmu_idle_ctrl_filter_disabled_f());
	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_BASE), data);

	/* set GR cntr idle mode to BUSY, first clear all idle masks */
	NV_PPWR_PMU_IDLE_MASK_CLEAR_ALL(GR);
	nvgpu_writel(g, pwr_pmu_idle_mask_r(IDLE_COUNTER_GR),
		pwr_pmu_idle_mask_gr_enabled_f() |
		pwr_pmu_idle_mask_ce_2_enabled_f());

	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_GR), 0x0U);
	data = nvgpu_readl(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_GR));
	data = set_field(data, pwr_pmu_idle_ctrl_value_m() |
		pwr_pmu_idle_ctrl_filter_m(),
		pwr_pmu_idle_ctrl_value_busy_f() |
		pwr_pmu_idle_ctrl_filter_disabled_f());
	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_GR), data);

	/* set NVD cntr idle mode to BUSY, first clear all idle masks */
	NV_PPWR_PMU_IDLE_MASK_CLEAR_ALL(NVD);
	nvgpu_writel(g, pwr_pmu_idle_mask_r(IDLE_COUNTER_NVD),
		pwr_pmu_idle_mask_nvenc_0_enabled_f() |
		pwr_pmu_idle_mask_nvenc_1_enabled_f() |
		pwr_pmu_idle_mask_nvdec_0_enabled_f());
    /*  MM engines are defined on multiple idle_mask so set for each */
	nvgpu_writel(g, pwr_pmu_idle_mask_2_r(IDLE_COUNTER_NVD),
		pwr_pmu_idle_mask_2_nvdec_1_enabled_f() |
		pwr_pmu_idle_mask_2_ofa_enabled_f() |
		pwr_pmu_idle_mask_2_nvjpg_0_enabled_f());

	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVD), 0x0U);
	data = nvgpu_readl(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVD));
	data = set_field(data, pwr_pmu_idle_ctrl_value_m() |
		pwr_pmu_idle_ctrl_filter_m(),
		pwr_pmu_idle_ctrl_value_busy_f() |
		pwr_pmu_idle_ctrl_filter_disabled_f());
	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVD), data);

	/* set NVENC0 cntr idle mode to BUSY, first clear all idle masks */
	NV_PPWR_PMU_IDLE_MASK_CLEAR_ALL(NVENC0);
	nvgpu_writel(g, pwr_pmu_idle_mask_r(IDLE_COUNTER_NVENC0),
		pwr_pmu_idle_mask_nvenc_0_enabled_f());

	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVENC0), 0x0U);
	data = nvgpu_readl(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVENC0));
	data = set_field(data, pwr_pmu_idle_ctrl_value_m() |
		pwr_pmu_idle_ctrl_filter_m(),
		pwr_pmu_idle_ctrl_value_busy_f() |
		pwr_pmu_idle_ctrl_filter_disabled_f());
	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVENC0), data);

	/* set NVENC1 cntr idle mode to BUSY, first clear all idle masks */
	NV_PPWR_PMU_IDLE_MASK_CLEAR_ALL(NVENC1);
	nvgpu_writel(g, pwr_pmu_idle_mask_r(IDLE_COUNTER_NVENC1),
		pwr_pmu_idle_mask_nvenc_1_enabled_f());

	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVENC1), 0x0U);
	data = nvgpu_readl(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVENC1));
	data = set_field(data, pwr_pmu_idle_ctrl_value_m() |
		pwr_pmu_idle_ctrl_filter_m(),
		pwr_pmu_idle_ctrl_value_busy_f() |
		pwr_pmu_idle_ctrl_filter_disabled_f());
	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVENC1), data);

	/* set NVDEC0 cntr idle mode to BUSY, first clear all idle masks */
	NV_PPWR_PMU_IDLE_MASK_CLEAR_ALL(NVDEC0);
	nvgpu_writel(g, pwr_pmu_idle_mask_r(IDLE_COUNTER_NVDEC0),
		pwr_pmu_idle_mask_nvdec_0_enabled_f());

	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVDEC0), 0x0U);
	data = nvgpu_readl(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVDEC0));
	data = set_field(data, pwr_pmu_idle_ctrl_value_m() |
		pwr_pmu_idle_ctrl_filter_m(),
		pwr_pmu_idle_ctrl_value_busy_f() |
		pwr_pmu_idle_ctrl_filter_disabled_f());
	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVDEC0), data);

	/* set NVDEC1 cntr idle mode to BUSY, first clear all idle masks */
	NV_PPWR_PMU_IDLE_MASK_CLEAR_ALL(NVDEC1);
	nvgpu_writel(g, pwr_pmu_idle_mask_2_r(IDLE_COUNTER_NVDEC1),
		pwr_pmu_idle_mask_2_nvdec_1_enabled_f());

	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVDEC1), 0x0U);
	data = nvgpu_readl(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVDEC1));
	data = set_field(data, pwr_pmu_idle_ctrl_value_m() |
		pwr_pmu_idle_ctrl_filter_m(),
		pwr_pmu_idle_ctrl_value_busy_f() |
		pwr_pmu_idle_ctrl_filter_disabled_f());
	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVDEC1), data);

	/* set OFA cntr idle mode to BUSY, first clear all idle masks */
	NV_PPWR_PMU_IDLE_MASK_CLEAR_ALL(OFA);
	nvgpu_writel(g, pwr_pmu_idle_mask_2_r(IDLE_COUNTER_OFA),
		pwr_pmu_idle_mask_2_ofa_enabled_f());

	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_OFA), 0x0U);
	data = nvgpu_readl(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_OFA));
	data = set_field(data, pwr_pmu_idle_ctrl_value_m() |
		pwr_pmu_idle_ctrl_filter_m(),
		pwr_pmu_idle_ctrl_value_busy_f() |
		pwr_pmu_idle_ctrl_filter_disabled_f());
	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_OFA), data);

	/* set NVJPG0 cntr idle mode to BUSY, first clear all idle masks */
	/* gb10b HW can monitor idle on NVJPG0 only or on NVJPG1 only, not both
	   simultaneously, if NV_PPWR_PMU_NVJPG_IDLE_MASK is set accordingly.
	   By default, NVJPG0 idle cycle counts for NVJPG0/NVJPG1 aggregated result */
	NV_PPWR_PMU_IDLE_MASK_CLEAR_ALL(NVJPG0);
	nvgpu_writel(g, pwr_pmu_idle_mask_2_r(IDLE_COUNTER_NVJPG0),
		pwr_pmu_idle_mask_2_nvjpg_0_enabled_f());

	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVJPG0), 0x0U);
	data = nvgpu_readl(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVJPG0));
	data = set_field(data, pwr_pmu_idle_ctrl_value_m() |
		pwr_pmu_idle_ctrl_filter_m(),
		pwr_pmu_idle_ctrl_value_busy_f() |
		pwr_pmu_idle_ctrl_filter_disabled_f());
	nvgpu_writel(g, pwr_pmu_idle_ctrl_r(IDLE_COUNTER_NVJPG0), data);

	/* Clear idle_intr_status, Reset counters */
	nvgpu_writel(g, pwr_pmu_idle_intr_status_r(),
		pwr_pmu_idle_intr_status_intr_f(1));
	nvgpu_writel(g, pwr_pmu_idle_count_r(IDLE_COUNTER_BASE),
		pwr_pmu_idle_count_reset_f(1));
	nvgpu_writel(g, pwr_pmu_idle_count_r(IDLE_COUNTER_GR),
		pwr_pmu_idle_count_reset_f(1));
	nvgpu_writel(g, pwr_pmu_idle_count_r(IDLE_COUNTER_NVD),
		pwr_pmu_idle_count_reset_f(1));
	nvgpu_writel(g, pwr_pmu_idle_count_r(IDLE_COUNTER_NVENC0),
		pwr_pmu_idle_count_reset_f(1));
	nvgpu_writel(g, pwr_pmu_idle_count_r(IDLE_COUNTER_NVENC1),
		pwr_pmu_idle_count_reset_f(1));
	nvgpu_writel(g, pwr_pmu_idle_count_r(IDLE_COUNTER_NVDEC0),
		pwr_pmu_idle_count_reset_f(1));
	nvgpu_writel(g, pwr_pmu_idle_count_r(IDLE_COUNTER_NVDEC1),
		pwr_pmu_idle_count_reset_f(1));
	nvgpu_writel(g, pwr_pmu_idle_count_r(IDLE_COUNTER_OFA),
		pwr_pmu_idle_count_reset_f(1));
	nvgpu_writel(g, pwr_pmu_idle_count_r(IDLE_COUNTER_NVJPG0),
		pwr_pmu_idle_count_reset_f(1));
}

u32 gb10b_pmu_get_irqmask(struct gk20a *g)
{
	u32 mask = 0U;

	nvgpu_pmu_dbg(g, "RISCV core INTR");
	mask = nvgpu_readl(g, pwr_riscv_irqmask_r());
	mask &= nvgpu_readl(g, pwr_riscv_irqdest_r());

	return mask;
}

bool gb10b_pmu_is_engine_in_reset(struct gk20a *g)
{
	u32 reg_reset;
	bool status = false;

	reg_reset = gk20a_readl(g, pwr_falcon_engine_r());
	if (reg_reset == pwr_falcon_engine_reset_true_f()) {
		status = true;
	}

	return status;
}

void gb10b_pmu_engine_reset(struct gk20a *g, bool do_reset)
{
	if (g->is_fusa_sku) {
		return;
	}

	/*
	* From GP10X onwards, we are using PPWR_FALCON_ENGINE for reset. And as
	* it may come into same behavior, reading NV_PPWR_FALCON_ENGINE again
	* after Reset.
	*/
	if (do_reset) {
		nvgpu_writel(g, pwr_falcon_engine_r(),
			pwr_falcon_engine_reset_false_f());
		(void) nvgpu_readl(g, pwr_falcon_engine_r());
	} else {
		nvgpu_writel(g, pwr_falcon_engine_r(),
			pwr_falcon_engine_reset_true_f());
		(void) nvgpu_readl(g, pwr_falcon_engine_r());
	}
}

#ifdef CONFIG_NVGPU_INJECT_HWERR
void gb10b_pmu_inject_ecc_error(struct gk20a *g,
                struct nvgpu_hw_err_inject_info *err, u32 error_info)
{
	(void)error_info;
	nvgpu_info(g, "Injecting PMU fault %s", err->name);
	nvgpu_writel(g, err->get_reg_addr(), err->get_reg_val(1U));
}

static inline u32 pmu_falcon_ecc_control_r(void)
{
	return pwr_pmu_falcon_ecc_control_r();
}

static inline u32 pmu_falcon_ecc_control_inject_corrected_err_f(u32 v)
{
	return pwr_pmu_falcon_ecc_control_inject_corrected_err_f(v);
}

static inline u32 pmu_falcon_ecc_control_inject_uncorrected_err_f(u32 v)
{
	return pwr_pmu_falcon_ecc_control_inject_uncorrected_err_f(v);
}

static struct nvgpu_hw_err_inject_info pmu_ecc_err_desc[] = {
	NVGPU_ECC_ERR("falcon_imem_ecc_corrected",
		gb10b_pmu_inject_ecc_error,
		pmu_falcon_ecc_control_r,
		pmu_falcon_ecc_control_inject_corrected_err_f),
	NVGPU_ECC_ERR("falcon_imem_ecc_uncorrected",
		gb10b_pmu_inject_ecc_error,
		pmu_falcon_ecc_control_r,
		pmu_falcon_ecc_control_inject_uncorrected_err_f),
};

static struct nvgpu_hw_err_inject_info_desc pmu_err_desc;

struct nvgpu_hw_err_inject_info_desc *
gb10b_pmu_intr_get_err_desc(struct gk20a *g)
{
	(void)g;
	pmu_err_desc.info_ptr = pmu_ecc_err_desc;
	pmu_err_desc.info_size = nvgpu_safe_cast_u64_to_u32(
			sizeof(pmu_ecc_err_desc) /
			sizeof(struct nvgpu_hw_err_inject_info));

	return &pmu_err_desc;
}

#endif /* CONFIG_NVGPU_INJECT_HWERR */

u32 gb10b_pmu_msgq_tail_r(void)
{
	return pwr_pmu_msgq_tail_r();
}

static void gb10b_pmu_stall_handler(struct gk20a *g, u64 cookie)
{
	(void)cookie;
	g->ops.pmu.pmu_isr(g);
}

void gb10b_pmu_get_intr_ctrl_msg(struct gk20a *g, bool enable,
							u32 *intr_ctrl_msg)
{
	u32 vector = 0U;

	vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_PMU, 0);

	if (enable) {
		nvgpu_gin_set_stall_handler(g, vector, &gb10b_pmu_stall_handler, 0);
		*intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_PMU, 0,
					NVGPU_GIN_INTR_ENABLE);
	} else {
		*intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, vector,
			NVGPU_GIN_CPU_DISABLE, NVGPU_GIN_GSP_DISABLE);
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_PMU, 0,
					NVGPU_GIN_INTR_DISABLE);
	}
	return;
}

/*
 *
 * Interrupts required for LS-PMU are configured by LS-PMU ucode as part of
 * LS-PMU init code, so just enable/disable PMU interrupt from MC.
 *
 */
void gb10b_pmu_enable_irq(struct nvgpu_pmu *pmu, bool enable)
{
	struct gk20a *g = pmu->g;
	u32 intr_ctrl_msg = 0U;

	nvgpu_log_fn(g, " ");

	if (g->ops.pmu.get_intr_ctrl_msg != NULL) {
		g->ops.pmu.get_intr_ctrl_msg(g, enable,
		&intr_ctrl_msg);
	} else {
		nvgpu_err(g, "Un-supported PMU interrupt msg ctrl");
	}

	nvgpu_writel(g, pwr_falcon_intr_ctrl_r(0), intr_ctrl_msg);
}

bool gb10b_pmu_is_debug_mode_en(struct gk20a *g)
{
	u32 ctl_stat =  nvgpu_readl(g, pwr_falcon_hwcfg2_r());

	nvgpu_writel(g, pwr_falcon_hwcfg2_r(),
			pwr_falcon_hwcfg2_dbgmode_enable_v());

	if (pwr_falcon_hwcfg2_dbgmode_v(ctl_stat) ==
			pwr_falcon_hwcfg2_dbgmode_enable_v()) {
		nvgpu_pmu_dbg(g, "DEBUG MODE");
		return true;
	} else {
		nvgpu_pmu_dbg(g, "PROD MODE");
		return false;
	}

}

void gb10b_pmu_ns_setup_apertures(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	/* setup apertures - virtual */
	nvgpu_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_UCODE),
		pwr_fbif_transcfg_mem_type_virtual_f());
	nvgpu_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_VIRT),
		pwr_fbif_transcfg_mem_type_virtual_f());
	/* setup apertures - physical */
	nvgpu_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_PHYS_VID),
		pwr_fbif_transcfg_mem_type_physical_f() |
		pwr_fbif_transcfg_target_local_fb_f());
	nvgpu_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_PHYS_SYS_COH),
		pwr_fbif_transcfg_mem_type_physical_f() |
		pwr_fbif_transcfg_target_coherent_sysmem_f());
	nvgpu_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_PHYS_SYS_NCOH),
		pwr_fbif_transcfg_mem_type_physical_f() |
		pwr_fbif_transcfg_target_noncoherent_sysmem_f());
}

unsigned long long gb10b_get_amap_extmem2_start(void)
{
	return 0x9400000000000000ull;
}
