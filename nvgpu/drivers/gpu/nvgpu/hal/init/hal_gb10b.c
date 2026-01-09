// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/soc.h>
#include <nvgpu/errata.h>
#include <nvgpu/acr.h>
#include <nvgpu/ce.h>
#include <nvgpu/ce_app.h>
#include <nvgpu/pmu.h>
#ifdef CONFIG_NVGPU_LS_PMU
#include <nvgpu/pmu/pmu_pstate.h>
#include <nvgpu/pmu/seq.h>
#endif
#include <nvgpu/therm.h>
#include <nvgpu/clk_arb.h>
#include <nvgpu/fuse.h>
#include <nvgpu/pbdma.h>
#include <nvgpu/engines.h>
#include <nvgpu/preempt.h>
#include <nvgpu/regops.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/fs_state.h>
#include <nvgpu/gr/obj_ctx.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/dma.h>
#ifdef CONFIG_NVGPU_LS_PMU
#include <nvgpu/pmu/pmu_perfmon.h>
#endif
#include <nvgpu/profiler.h>
#ifdef CONFIG_NVGPU_POWER_PG
#include <nvgpu/pmu/pmu_pg.h>
#endif
#include <nvgpu/nvenc.h>
#include <nvgpu/ofa.h>
#include <nvgpu/nvdec.h>
#include <nvgpu/nvjpg.h>

#include "hal/mm/mm_gp10b.h"
#include "hal/mm/mm_gv11b.h"
#include "hal/mm/mm_ga10b.h"
#include "hal/mm/cache/flush_gk20a.h"
#include "hal/mm/cache/flush_gv11b.h"
#include "hal/mm/cache/flush_gb10b.h"
#include "hal/mm/gmmu/gmmu_gm20b.h"
#include "hal/mm/gmmu/gmmu_gp10b.h"
#include "hal/mm/gmmu/gmmu_gv11b.h"
#include "hal/mm/gmmu/gmmu_gb10b.h"
#include "hal/mm/mmu_fault/mmu_fault_gv11b.h"
#include "hal/mm/mmu_fault/mmu_fault_ga10b.h"
#include "hal/mm/mmu_fault/mmu_fault_gb10b.h"
#include "hal/mc/mc_gm20b.h"
#include "hal/mc/mc_gp10b.h"
#include "hal/mc/mc_gv11b.h"
#include "hal/mc/mc_tu104.h"
#include "hal/mc/mc_ga10b.h"
#include "hal/mc/mc_gb10b.h"
#include "hal/mc/mc_intr_ga10b.h"
#include "hal/mc/mc_intr_gb10b.h"
#include "hal/gin/gin_gb10b.h"
#include "hal/bus/bus_gk20a.h"
#include "hal/bus/bus_gp10b.h"
#include "hal/bus/bus_gm20b.h"
#include "hal/bus/bus_gv11b.h"
#include "hal/xal_ep/xal_ep_gb10b.h"
#include "hal/ce/ce_gv11b.h"
#include "hal/class/class_gv11b.h"
#include "hal/class/class_ga10b.h"
#include "hal/class/class_gb10b.h"
#include "hal/priv_ring/priv_ring_gm20b.h"
#include "hal/priv_ring/priv_ring_gp10b.h"
#include "hal/priv_ring/priv_ring_ga10b.h"
#include "hal/priv_ring/priv_ring_gb10b.h"
#include "hal/gr/config/gr_config_gv100.h"
#include "hal/gr/config/gr_config_gb10b.h"
#include "hal/power_features/cg/ga10b_gating_reglist.h"
#ifdef CONFIG_NVGPU_COMPRESSION
#include "hal/cbc/cbc_gp10b.h"
#include "hal/cbc/cbc_gv11b.h"
#include "hal/cbc/cbc_tu104.h"
#include "hal/cbc/cbc_ga10b.h"
#include "hal/cbc/cbc_gb10b.h"
#endif
#include "hal/ce/ce_gp10b.h"
#include "hal/ce/ce_ga10b.h"
#include "hal/ce/ce_gb10b.h"
#include "hal/therm/therm_gm20b.h"
#include "hal/therm/therm_gv11b.h"
#include "hal/therm/therm_ga10b.h"
#include "hal/ltc/ltc_gm20b.h"
#include "hal/ltc/ltc_gp10b.h"
#include "hal/ltc/ltc_gv11b.h"
#include "hal/ltc/ltc_tu104.h"
#include "hal/ltc/ltc_ga10b.h"
#include "hal/ltc/ltc_gb10b.h"
#include "hal/ltc/intr/ltc_intr_gv11b.h"
#include "hal/ltc/intr/ltc_intr_ga10b.h"
#include "hal/ltc/intr/ltc_intr_gb10b.h"
#include "hal/ctc_chi/ctc_chi_gb10b.h"
#include "hal/fb/fb_gm20b.h"
#include "hal/fb/fb_gp10b.h"
#include "hal/fb/fb_gv11b.h"
#include "hal/fb/fb_tu104.h"
#include "hal/fb/fb_ga10b.h"
#include "hal/fb/fb_gb10b.h"
#include "hal/fb/fb_mmu_fault_gv11b.h"
#include "hal/fb/fb_mmu_fault_gb10b.h"
#include "hal/fb/ecc/fb_ecc_ga10b.h"
#include "hal/fb/ecc/fb_ecc_gv11b.h"
#include "hal/fb/intr/fb_intr_gv11b.h"
#include "hal/fb/intr/fb_intr_ga10b.h"
#include "hal/fb/intr/fb_intr_gb10b.h"
#include "hal/fb/intr/fb_intr_ecc_gv11b.h"
#include "hal/fb/intr/fb_intr_ecc_ga10b.h"
#include "hal/fb/intr/fb_intr_ecc_gb10b.h"
#include "hal/fb/vab/vab_ga10b.h"
#include "hal/func/func_ga10b.h"
#include "hal/func/func_gb10b.h"
#include "hal/fuse/fuse_gm20b.h"
#include "hal/fuse/fuse_gp10b.h"
#include "hal/fuse/fuse_gv11b.h"
#include "hal/fuse/fuse_ga10b.h"
#include "hal/fuse/fuse_gb10b.h"
#include "hal/ptimer/ptimer_gk20a.h"
#include "hal/ptimer/ptimer_gp10b.h"
#include "hal/ptimer/ptimer_gv11b.h"
#include "hal/ptimer/ptimer_ga10b.h"
#ifdef CONFIG_NVGPU_DEBUGGER
#include "hal/regops/regops_gb10b.h"
#include "hal/regops/allowlist_gb10b.h"
#endif
#ifdef CONFIG_NVGPU_RECOVERY
#include "hal/rc/rc_gv11b.h"
#endif
#include "hal/fifo/fifo_gk20a.h"
#include "hal/fifo/fifo_gv11b.h"
#include "hal/fifo/fifo_ga10b.h"
#include "hal/fifo/fifo_gb10b.h"
#include "hal/fifo/pbdma_gm20b.h"
#include "hal/fifo/pbdma_gp10b.h"
#include "hal/fifo/pbdma_gv11b.h"
#include "hal/fifo/pbdma_ga10b.h"
#include "hal/fifo/pbdma_gb10b.h"
#include "hal/fifo/preempt_gv11b.h"
#include "hal/fifo/preempt_ga10b.h"
#include "hal/fifo/engine_status_gv100.h"
#include "hal/fifo/engine_status_ga10b.h"
#include "hal/fifo/pbdma_status_gm20b.h"
#include "hal/fifo/pbdma_status_ga10b.h"
#include "hal/fifo/engines_gp10b.h"
#include "hal/fifo/engines_gv11b.h"
#include "hal/fifo/ramfc_gp10b.h"
#include "hal/fifo/ramfc_gv11b.h"
#include "hal/fifo/ramfc_ga10b.h"
#include "hal/fifo/ramfc_gb10b.h"
#include "hal/fifo/ramin_gk20a.h"
#include "hal/fifo/ramin_gm20b.h"
#include "hal/fifo/ramin_gv11b.h"
#include "hal/fifo/ramin_ga10b.h"
#include "hal/fifo/ramin_gb10b.h"
#include "hal/fifo/runlist_ram_gk20a.h"
#include "hal/fifo/runlist_ram_gv11b.h"
#include "hal/fifo/runlist_ram_gb10b.h"
#include "hal/fifo/runlist_fifo_gk20a.h"
#include "hal/fifo/runlist_fifo_gv11b.h"
#include "hal/fifo/runlist_fifo_ga10b.h"
#include "hal/fifo/runlist_fifo_gb10b.h"
#include "hal/fifo/runlist_ga10b.h"
#include "hal/fifo/tsg_ga10b.h"
#include "hal/fifo/tsg_gv11b.h"
#include "hal/fifo/tsg_gb10b.h"
#include "hal/fifo/userd_gk20a.h"
#include "hal/fifo/userd_gv11b.h"
#include "hal/fifo/userd_ga10b.h"
#include "hal/fifo/userd_gb10b.h"
#include "hal/fifo/usermode_gv11b.h"
#include "hal/fifo/usermode_tu104.h"
#include "hal/fifo/usermode_gb10b.h"
#include "hal/fifo/fifo_intr_gk20a.h"
#include "hal/fifo/fifo_intr_gv11b.h"
#include "hal/fifo/fifo_intr_ga10b.h"
#include "hal/fifo/fifo_intr_gb10b.h"
#include "hal/fifo/ctxsw_timeout_gv11b.h"
#include "hal/fifo/ctxsw_timeout_ga10b.h"
#include "hal/gr/ecc/ecc_gv11b.h"
#include "hal/gr/ecc/ecc_ga10b.h"
#include "hal/gr/ecc/ecc_gb10b.h"
#ifdef CONFIG_NVGPU_FECS_TRACE
#include "hal/gr/fecs_trace/fecs_trace_gm20b.h"
#include "hal/gr/fecs_trace/fecs_trace_gv11b.h"
#include "hal/gr/fecs_trace/fecs_trace_gb10b.h"
#endif
#include "hal/gr/falcon/gr_falcon_gm20b.h"
#include "hal/gr/falcon/gr_falcon_gp10b.h"
#include "hal/gr/falcon/gr_falcon_gv11b.h"
#include "hal/gr/falcon/gr_falcon_tu104.h"
#include "hal/gr/falcon/gr_falcon_ga100.h"
#include "hal/gr/falcon/gr_falcon_ga10b.h"
#include "hal/gr/falcon/gr_falcon_gb10b.h"
#include "hal/gr/config/gr_config_gm20b.h"
#include "hal/gr/config/gr_config_gv11b.h"
#include "hal/gr/config/gr_config_ga10b.h"
#ifdef CONFIG_NVGPU_GRAPHICS
#include "hal/gr/zbc/zbc_gm20b.h"
#include "hal/gr/zbc/zbc_gp10b.h"
#include "hal/gr/zbc/zbc_gv11b.h"
#include "hal/gr/zbc/zbc_ga10b.h"
#include "hal/gr/zbc/zbc_gb10b.h"
#include "hal/gr/zcull/zcull_gm20b.h"
#include "hal/gr/zcull/zcull_gv11b.h"
#include "hal/gr/zcull/zcull_gb10b.h"
#endif
#include "hal/gr/init/gr_init_gm20b.h"
#include "hal/gr/init/gr_init_gp10b.h"
#include "hal/gr/init/gr_init_gv11b.h"
#include "hal/gr/init/gr_init_tu104.h"
#include "hal/gr/init/gr_init_ga10b.h"
#include "hal/gr/init/gr_init_gb10b.h"
#include "hal/gr/init/gr_init_ga100.h"
#include "hal/gr/intr/gr_intr_gm20b.h"
#include "hal/gr/intr/gr_intr_gp10b.h"
#include "hal/gr/intr/gr_intr_gv11b.h"
#include "hal/gr/intr/gr_intr_ga10b.h"
#include "hal/gr/intr/gr_intr_gb10b.h"
#ifdef CONFIG_NVGPU_DEBUGGER
#include "hal/gr/hwpm_map/hwpm_map_gv100.h"
#include "hal/gr/hwpm_map/hwpm_map_gb10b.h"
#endif
#include "hal/gr/ctxsw_prog/ctxsw_prog_gm20b.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_gp10b.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_gv11b.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_ga10b.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_ga100.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_gb10b.h"
#ifdef CONFIG_NVGPU_DEBUGGER
#include "hal/gr/gr/gr_gk20a.h"
#include "hal/gr/gr/gr_gm20b.h"
#include "hal/gr/gr/gr_gp10b.h"
#include "hal/gr/gr/gr_gv100.h"
#include "hal/gr/gr/gr_gv11b.h"
#include "hal/gr/gr/gr_tu104.h"
#include "hal/gr/gr/gr_ga10b.h"
#include "hal/gr/gr/gr_gb10b.h"
#include "hal/gr/gr/gr_ga100.h"
#endif
#include "hal/pmu/pmu_gk20a.h"
#ifdef CONFIG_NVGPU_LS_PMU
#include "hal/pmu/pmu_gm20b.h"
#endif
#include "hal/pmu/pmu_gv11b.h"
#include "hal/pmu/pmu_ga10b.h"
#include "hal/pmu/pmu_gb10b.h"
#include "hal/gsp/gsp_ga10b.h"
#include "hal/gsp/gsp_gb10b.h"
#include "hal/sync/syncpt_cmdbuf_gv11b.h"
#include "hal/sync/sema_cmdbuf_gv11b.h"
#include "hal/falcon/falcon_gk20a.h"
#include "hal/falcon/falcon_ga10b.h"
#include "hal/falcon/falcon_gb10b.h"
#ifdef CONFIG_NVGPU_DEBUGGER
#include "hal/perf/perf_gv11b.h"
#include "hal/perf/perf_ga10b.h"
#include "hal/perf/perf_gb10b.h"
#endif
#include "hal/netlist/netlist_ga10b.h"
#include "hal/netlist/netlist_gb10b.h"
#include "hal/top/top_gm20b.h"
#include "hal/top/top_gp10b.h"
#include "hal/top/top_gv11b.h"
#include "hal/top/top_ga10b.h"
#include "hal/top/top_gb10b.h"
#include "hal/sysctrl/sysctrl_gb10b.h"
#include "hal/nvenc/nvenc_gb10b.h"
#include "hal/ofa/ofa_gb10b.h"
#include "hal/nvdec/nvdec_gb10b.h"
#include "hal/nvjpg/nvjpg_gb10b.h"

#ifdef CONFIG_NVGPU_LS_PMU
#include "common/pmu/pg/pg_sw_gm20b.h"
#include "common/pmu/pg/pg_sw_gp106.h"
#include "common/pmu/pg/pg_sw_gv11b.h"
#endif

#ifdef CONFIG_NVGPU_CLK_ARB
#include "common/clk_arb/clk_arb_gp10b.h"
#endif

#include "hal/fifo/channel_gk20a.h"
#include "hal/fifo/channel_gm20b.h"
#include "hal/fifo/channel_gv11b.h"
#include "hal/fifo/channel_ga10b.h"

#ifdef CONFIG_NVGPU_STATIC_POWERGATE
#include "hal/tpc/tpc_gv11b.h"
#endif

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
#include "hal/mssnvlink/mssnvlink_ga10b.h"
#endif
#include "hal_gb10b_litter.h"

#include <nvgpu/ptimer.h>
#include <nvgpu/error_notifier.h>
#include <nvgpu/debugger.h>
#include <nvgpu/pm_reservation.h>
#include <nvgpu/runlist.h>
#include <nvgpu/fifo/userd.h>
#include <nvgpu/perfbuf.h>
#include <nvgpu/cyclestats_snapshot.h>
#include <nvgpu/gr/zbc.h>
#include <nvgpu/gr/setup.h>
#include <nvgpu/gr/fecs_trace.h>
#include <nvgpu/gr/gr_intr.h>
#include "hal/xbar/xbar_gb10b.h"
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/cic_mon.h>
#ifndef CONFIG_NVGPU_MIG
#include <nvgpu/grmgr.h>
#endif
#include <nvgpu/gr/gr_instances.h>
#ifdef CONFIG_NVGPU_SIM
#include <nvgpu/sim.h>
#include "hal/sim/sim_ga10b.h"
#endif

#include "hal/grmgr/grmgr_ga10b.h"
#include "hal/grmgr/grmgr_gb10b.h"
#include "hal/power_features/cg/gb10b_gating_reglist.h"
#include "hal/cic/mon/cic_ga10b.h"
#include "hal/cic/mon/cic_gb10b.h"
#include "hal/xtl_ep/xtl_ep_gb10b.h"
#include "hal/ppriv/ppriv_gb10b.h"
#include "hal/lrc/lrc_gb10b.h"

#include "hal_gb10b.h"

static int gb10b_init_gpu_characteristics(struct gk20a *g)
{
	int err;

	err = nvgpu_init_gpu_characteristics(g);
	if (err != 0) {
		nvgpu_err(g, "failed to init GPU characteristics");
		return err;
	}

	nvgpu_set_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS, true);
#ifdef CONFIG_NVGPU_GRAPHICS
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SCG, true);
#endif
#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
	nvgpu_set_enabled(g, NVGPU_SUPPORT_RESCHEDULE_RUNLIST, true);
#endif
	if (nvgpu_has_syncpoints(g)) {
		nvgpu_set_enabled(g, NVGPU_SUPPORT_SYNCPOINT_ADDRESS, true);
		nvgpu_set_enabled(g, NVGPU_SUPPORT_USER_SYNCPOINT, true);
	}
	/*
	 * Usermode submits are not tied to syncpoints and can be safely enabled.
	 */
	nvgpu_set_enabled(g, NVGPU_SUPPORT_USERMODE_SUBMIT, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_DEVICE_EVENTS, true);

	return 0;
}

static const struct gops_acr gb10b_ops_acr = {
	.acr_init = nvgpu_acr_init,
	.acr_construct_execute = nvgpu_acr_construct_execute,
};

static const struct gops_func gb10b_ops_func = {
	.get_full_phys_offset = ga10b_func_get_full_phys_offset,
};

#ifdef CONFIG_NVGPU_DGPU
static const struct gops_bios gb10b_ops_bios = {
	/* This is specific to dGPU. Disable it for iGPU. */
	.bios_sw_init = NULL,
};
#endif

static const struct gops_ecc gb10b_ops_ecc = {
	.ecc_init_support = nvgpu_ecc_init_support,
	.ecc_finalize_support = nvgpu_ecc_finalize_support,
	.ecc_remove_support = nvgpu_ecc_remove_support,
};

static const struct gops_ltc_intr gb10b_ops_ltc_intr = {
	.configure = gb10b_ltc_intr_configure,
	.configure_intr1 = gb10b_ltc_intr1_configure,
	.configure_intr2 = gb10b_ltc_intr2_configure,
	.configure_intr3 = gb10b_ltc_intr3_configure,
	.isr = ga10b_ltc_intr_isr,
	.handle_intr1 = gb10b_ltc_intr_handle_lts_intr,
	.handle_intr2 = gb10b_ltc_intr_handle_lts_intr2,
	.handle_intr3 = gb10b_ltc_intr_handle_lts_intr3,
	.handle_ecc_intr3 = ga10b_ltc_intr3_ecc_interrupts,
	.handle_xbar_intfc_ecc_intr = gb10b_xbar_handle_intfc_ecc_intr,
	.isr_extra = NULL,
	.ltc_intr3_configure_extra = NULL,
#ifdef CONFIG_NVGPU_NON_FUSA
	.en_illegal_compstat = NULL,
	.handle_illegal_compstat = NULL,
#endif
	.read_intr1 = gb10b_ltc_intr_read_intr1,
	.read_intr2 = gb10b_ltc_intr_read_intr2,
	.read_intr3 = gb10b_ltc_intr_read_intr3,
	.write_intr1 = gb10b_ltc_intr_write_intr1,
	.write_intr2 = gb10b_ltc_intr_write_intr2,
	.write_intr3 = gb10b_ltc_intr_write_intr3,
	.retrigger = gb10b_ltc_intr_retrigger,
	.report_non_ecc_uncorrected_err = gb10b_ltc_intr_report_non_ecc_uncorrected_err,
};

static const struct gops_ltc gb10b_ops_ltc = {
	.ecc_init = ga10b_lts_ecc_init,
	.init_ltc_support = nvgpu_init_ltc_support,
	.ltc_remove_support = nvgpu_ltc_remove_support,
	.determine_L2_size_bytes = gb10b_determine_L2_size_bytes,
	.init_fs_state = gb10b_ltc_init_fs_state,
	.ltc_lts_set_mgmt_setup = NULL,
	.flush = gb10b_flush_ltc,
	.compute_lts_mask = gb10b_ltc_compute_lts_mask,
#ifdef CONFIG_NVGPU_MIG
	.set_remote_swizzle_id = gb10b_ltc_set_remote_swizzle_id,
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) || defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT)
	.set_enabled = gb10b_ltc_set_enabled,
#endif
#ifdef CONFIG_NVGPU_GRAPHICS
	.set_zbc_s_entry = gb10b_ltc_set_zbc_stencil_entry,
	.set_zbc_color_entry = gb10b_ltc_set_zbc_color_entry,
	.set_zbc_depth_entry = gb10b_ltc_set_zbc_depth_entry,
#endif /* CONFIG_NVGPU_GRAPHICS */
#ifdef CONFIG_NVGPU_DEBUGGER
	.pri_is_ltc_addr = gb10b_ltc_pri_is_ltc_addr,
	.is_pltcg_ltcs_addr = gb10b_ltc_is_pltcg_ltcs_addr,
	.is_ltcs_ltss_addr = gb10b_ltc_is_ltcs_ltss_addr,
	.is_ltcn_ltss_addr = gm20b_ltc_is_ltcn_ltss_addr,
	.split_lts_broadcast_addr = gm20b_ltc_split_lts_broadcast_addr,
	.split_ltc_broadcast_addr = gm20b_ltc_split_ltc_broadcast_addr,
	.pri_is_lts_tstg_addr = tu104_ltc_pri_is_lts_tstg_addr,
	.pri_shared_addr = gb10b_ltc_pri_shared_addr,
	/* ga10b_gb10b_fix_mismatch */
	.set_l2_max_ways_evict_last = NULL,
	/* ga10b_gb10b_fix_mismatch */
	.get_l2_max_ways_evict_last = NULL,
	.set_l2_sector_promotion = gb10b_set_l2_sector_promotion,
#endif /* CONFIG_NVGPU_DEBUGGER */
#ifndef CONFIG_NVGPU_NON_FUSA
	.set_default_l2_max_ways_evict_last = NULL,
#endif /*not defined CONFIG_NVGPU_NON_FUSA*/
#ifdef CONFIG_NVGPU_NON_FUSA
	.get_ltcs_ltss_tstg_cfg1_active_sets = gb10b_ltcs_ltss_tstg_cfg1_active_sets,
#endif /* CONFIG_NVGPU_NON_FUSA */
};

#ifdef CONFIG_NVGPU_COMPRESSION
static const struct gops_cbc gb10b_ops_cbc = {
	.cbc_init_support = gb10b_cbc_init_support,
	.cbc_remove_support = gb10b_cbc_remove_support,
};
#endif

static const struct gops_ctc_chi gb10b_ops_ctc_chi = {
	.init_hw = gb10b_ctc_chi_init_hw,
};

static const struct gops_ce gb10b_ops_ce = {
	.ce_init_support = nvgpu_ce_init_support,
	/* Sysmem Fast SCRUB */
#ifdef CONFIG_NVGPU_COMPRESSION_RAW
	.ce_app_init_support = nvgpu_ce_app_init_support,
	.ce_app_suspend = nvgpu_ce_app_suspend,
	.ce_app_destroy = nvgpu_ce_app_destroy,
#endif
	.intr_enable = gb10b_ce_intr_enable,
	.isr_stall = gb10b_ce_stall_isr,
	.intr_retrigger = ga10b_ce_intr_retrigger,
	.get_num_pce = gv11b_ce_get_num_pce,
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	.mthd_buffer_fault_in_bar2_fault = gv11b_ce_mthd_buffer_fault_in_bar2_fault,
#endif
	.init_prod_values = NULL,
	.halt_engine = gv11b_ce_halt_engine,
	.request_idle = ga10b_ce_request_idle,
	.get_inst_ptr_from_lce = gv11b_ce_get_inst_ptr_from_lce,
	.pce_map_value = gb10b_ce_pce_map_value,
	.set_pce2lce_mapping = gb10b_ce_set_pce2lce_mapping,
	.init_pce2lce_configs = gb10b_ce_init_pce2lce_configs,
	.init_grce_configs = gb10b_ce_init_grce_configs,
	.set_engine_reset = gb10b_ce_set_engine_reset,
	.reset = nvgpu_ce_reset,
	.grce_config_size = gb10b_grce_config_size,
};

static const struct gops_gr_ecc gb10b_ops_gr_ecc = {
	.detect = ga10b_ecc_detect_enabled_units,
	.gpc_tpc_ecc_init = ga10b_gr_gpc_tpc_ecc_init,
	.fecs_ecc_init = gv11b_gr_fecs_ecc_init,
	.gpc_tpc_ecc_deinit = ga10b_gr_gpc_tpc_ecc_deinit,
	.fecs_ecc_deinit = gv11b_gr_fecs_ecc_deinit,
#ifdef CONFIG_NVGPU_INJECT_HWERR
	.get_mmu_err_desc = ga10b_gr_ecc_get_mmu_err_desc,
	.get_gcc_err_desc = gv11b_gr_intr_get_gcc_err_desc,
	.get_sm_err_desc = gb10b_gr_intr_get_sm_err_desc,
	.get_gpccs_err_desc = gv11b_gr_intr_get_gpccs_err_desc,
	.get_fecs_err_desc = gv11b_gr_intr_get_fecs_err_desc,
#endif /* CONFIG_NVGPU_INJECT_HWERR */
};

static const struct gops_gr_ctxsw_prog gb10b_ops_gr_ctxsw_prog = {
	.hw_get_fecs_header_size = ga10b_ctxsw_prog_hw_get_fecs_header_size,
	.get_patch_count = gm20b_ctxsw_prog_get_patch_count,
	.set_patch_count = gm20b_ctxsw_prog_set_patch_count,
	.set_patch_addr = gm20b_ctxsw_prog_set_patch_addr,
	.set_compute_preemption_mode_cta = gp10b_ctxsw_prog_set_compute_preemption_mode_cta,
	.get_main_image_ctx_id = gm20b_ctxsw_prog_get_main_image_ctx_id,
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	.init_ctxsw_hdr_data = gp10b_ctxsw_prog_init_ctxsw_hdr_data,
	.disable_verif_features = gm20b_ctxsw_prog_disable_verif_features,
#endif
#ifdef CONFIG_NVGPU_SET_FALCON_ACCESS_MAP
	.set_priv_access_map_config_mode = gm20b_ctxsw_prog_set_config_mode_priv_access_map,
	.set_priv_access_map_addr = gm20b_ctxsw_prog_set_addr_priv_access_map,
#endif
	.set_context_buffer_ptr = gv11b_ctxsw_prog_set_context_buffer_ptr,
	.set_type_per_veid_header = gv11b_ctxsw_prog_set_type_per_veid_header,
#ifdef CONFIG_NVGPU_GRAPHICS
	.set_zcull_ptr = gv11b_ctxsw_prog_set_zcull_ptr,
	.set_zcull = gm20b_ctxsw_prog_set_zcull,
	.set_zcull_mode_no_ctxsw = gm20b_ctxsw_prog_set_zcull_mode_no_ctxsw,
	.is_zcull_mode_separate_buffer = gm20b_ctxsw_prog_is_zcull_mode_separate_buffer,
#endif /* CONFIG_NVGPU_GRAPHICS */
#ifdef CONFIG_NVGPU_GFXP
	.set_graphics_preemption_mode_gfxp = gp10b_ctxsw_prog_set_graphics_preemption_mode_gfxp,
	.set_full_preemption_ptr = gv11b_ctxsw_prog_set_full_preemption_ptr,
	.set_full_preemption_ptr_veid0 = gv11b_ctxsw_prog_set_full_preemption_ptr_veid0,
#endif /* CONFIG_NVGPU_GFXP */
#ifdef CONFIG_NVGPU_CILP
	.set_compute_preemption_mode_cilp = gp10b_ctxsw_prog_set_compute_preemption_mode_cilp,
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	/* fix_ga10b_gb10b_missing */
	.hw_get_gpccs_header_size = gb10b_ctxsw_prog_hw_get_gpccs_header_size,
	.hw_get_extended_buffer_segments_size_in_bytes = gm20b_ctxsw_prog_hw_get_extended_buffer_segments_size_in_bytes,
	.hw_extended_marker_size_in_bytes = gm20b_ctxsw_prog_hw_extended_marker_size_in_bytes,
	.hw_get_perf_counter_control_register_stride = gm20b_ctxsw_prog_hw_get_perf_counter_control_register_stride,
	.set_pm_ptr = gv11b_ctxsw_prog_set_pm_ptr,
	.set_pm_mode = gm20b_ctxsw_prog_set_pm_mode,
	.set_pm_smpc_mode = gm20b_ctxsw_prog_set_pm_smpc_mode,
	.hw_get_pm_mode_no_ctxsw = gm20b_ctxsw_prog_hw_get_pm_mode_no_ctxsw,
	.hw_get_pm_mode_ctxsw = gm20b_ctxsw_prog_hw_get_pm_mode_ctxsw,
	.hw_get_pm_mode_stream_out_ctxsw = gv11b_ctxsw_prog_hw_get_pm_mode_stream_out_ctxsw,
	.set_cde_enabled = NULL,
	.set_pc_sampling = NULL,
	.check_main_image_header_magic = ga10b_ctxsw_prog_check_main_image_header_magic,
	.check_local_header_magic = ga10b_ctxsw_prog_check_local_header_magic,
	.get_num_gpcs = gm20b_ctxsw_prog_get_num_gpcs,
	.get_num_tpcs = gm20b_ctxsw_prog_get_num_tpcs,
	.get_extended_buffer_size_offset = gm20b_ctxsw_prog_get_extended_buffer_size_offset,
	.get_ppc_info = gm20b_ctxsw_prog_get_ppc_info,
	.get_local_priv_register_ctl_offset = gm20b_ctxsw_prog_get_local_priv_register_ctl_offset,
	.set_pmu_options_boost_clock_frequencies = NULL,
	.hw_get_perf_counter_register_stride = gv11b_ctxsw_prog_hw_get_perf_counter_register_stride,
	.hw_get_pm_gpc_gnic_stride = gb10b_ctxsw_prog_hw_get_pm_gpc_gnic_stride,
	.hw_get_main_header_size = ga10b_ctxsw_prog_hw_get_main_header_size,
	/* fix_ga10b_gb10b_missing */
	.hw_get_gpccs_header_stride = gb10b_ctxsw_prog_hw_get_gpccs_header_stride,
	.get_compute_sysreglist_offset = ga10b_ctxsw_prog_get_compute_sysreglist_offset,
	.get_gfx_sysreglist_offset = ga10b_ctxsw_prog_get_gfx_sysreglist_offset,
	.get_ltsreglist_offset = ga10b_ctxsw_prog_get_ltsreglist_offset,
	.get_compute_gpcreglist_offset = ga10b_ctxsw_prog_get_compute_gpcreglist_offset,
	.get_gfx_gpcreglist_offset = ga10b_ctxsw_prog_get_gfx_gpcreglist_offset,
	.get_compute_tpcreglist_offset = ga10b_ctxsw_prog_get_compute_tpcreglist_offset,
	.get_gfx_tpcreglist_offset = ga10b_ctxsw_prog_get_gfx_tpcreglist_offset,
	.get_compute_ppcreglist_offset = ga10b_ctxsw_prog_get_compute_ppcreglist_offset,
	.get_gfx_ppcreglist_offset = ga10b_ctxsw_prog_get_gfx_ppcreglist_offset,
	.get_compute_etpcreglist_offset = ga10b_ctxsw_prog_get_compute_etpcreglist_offset,
	.get_gfx_etpcreglist_offset = ga10b_ctxsw_prog_get_gfx_etpcreglist_offset,
	.get_tpc_segment_pri_layout = ga10b_ctxsw_prog_get_tpc_segment_pri_layout,
#endif /* CONFIG_NVGPU_DEBUGGER */
#ifdef CONFIG_DEBUG_FS
	.dump_ctxsw_stats = ga10b_ctxsw_prog_dump_ctxsw_stats,
#endif
#ifdef CONFIG_NVGPU_FECS_TRACE
	.hw_get_ts_tag_invalid_timestamp = gb10b_ctxsw_prog_hw_get_ts_tag_invalid_timestamp,
	.hw_get_ts_tag = gb10b_ctxsw_prog_hw_get_ts_tag,
	.hw_record_ts_timestamp = gb10b_ctxsw_prog_hw_record_ts_timestamp,
	.hw_get_ts_record_size_in_bytes = gm20b_ctxsw_prog_hw_get_ts_record_size_in_bytes,
	.is_ts_valid_record = gm20b_ctxsw_prog_is_ts_valid_record,
	.get_ts_buffer_aperture_mask = NULL,
	.set_ts_num_records = gm20b_ctxsw_prog_set_ts_num_records,
	.set_ts_buffer_ptr = gm20b_ctxsw_prog_set_ts_buffer_ptr,
#endif
};

static const struct gops_gr_config gb10b_ops_gr_config = {
	.get_gpc_mask = gm20b_gr_config_get_gpc_mask,
	.get_gpc_tpc_mask = gm20b_gr_config_get_gpc_tpc_mask,
	.get_gpc_pes_mask = gv11b_gr_config_get_gpc_pes_mask,
	.set_live_pes_mask = gb10b_gr_config_set_live_pes_mask,
	.get_gpc_rop_mask = ga10b_gr_config_get_gpc_rop_mask,
	.get_tpc_count_in_gpc = gm20b_gr_config_get_tpc_count_in_gpc,
	.get_pes_tpc_mask = gb10b_gr_config_get_pes_tpc_mask,
	.get_pd_dist_skip_table_size = gm20b_gr_config_get_pd_dist_skip_table_size,
	.init_sm_id_table = gb10b_gr_config_init_sm_id_table,
#ifdef CONFIG_NVGPU_GRAPHICS
	.get_zcull_count_in_gpc = gm20b_gr_config_get_zcull_count_in_gpc,
#endif /* CONFIG_NVGPU_GRAPHICS */
};

#ifdef CONFIG_NVGPU_FECS_TRACE
static const struct gops_gr_fecs_trace gb10b_ops_gr_fecs_trace = {
	.alloc_user_buffer = nvgpu_gr_fecs_trace_ring_alloc,
	.free_user_buffer = nvgpu_gr_fecs_trace_ring_free,
	.get_mmap_user_buffer_info = nvgpu_gr_fecs_trace_get_mmap_buffer_info,
	.init = nvgpu_gr_fecs_trace_init,
	.deinit = nvgpu_gr_fecs_trace_deinit,
	.enable = nvgpu_gr_fecs_trace_enable,
	.disable = nvgpu_gr_fecs_trace_disable,
	.is_enabled = nvgpu_gr_fecs_trace_is_enabled,
	.reset = nvgpu_gr_fecs_trace_reset,
	.flush = NULL,
	.poll = nvgpu_gr_fecs_trace_poll,
	.bind_channel = nvgpu_gr_fecs_trace_bind_channel,
	.unbind_channel = nvgpu_gr_fecs_trace_unbind_channel,
	.max_entries = nvgpu_gr_fecs_trace_max_entries,
	.get_buffer_full_mailbox_val = gv11b_fecs_trace_get_buffer_full_mailbox_val,
	.get_read_index = gb10b_fecs_trace_get_read_index,
	.get_write_index = gb10b_fecs_trace_get_write_index,
	.set_read_index = gb10b_fecs_trace_set_read_index,
};
#endif

static const struct gops_gr_setup gb10b_ops_gr_setup = {
	.init_golden_image = nvgpu_gr_obj_ctx_init_golden_context_image,
	.alloc_obj_ctx = nvgpu_gr_setup_alloc_obj_ctx,
	.free_gr_ctx = nvgpu_gr_setup_free_gr_ctx,
	.free_subctx = nvgpu_gr_setup_free_subctx,
#ifdef CONFIG_NVGPU_GRAPHICS
	.bind_ctxsw_zcull = nvgpu_gr_setup_bind_ctxsw_zcull,
#endif /* CONFIG_NVGPU_GRAPHICS */
	.set_preemption_mode = nvgpu_gr_setup_set_preemption_mode,
#ifdef CONFIG_NVGPU_DEBUGGER
	.setup_pri_address_buffer = nvgpu_gr_setup_pri_address_buffer,
#endif
};

#ifdef CONFIG_NVGPU_GRAPHICS
static const struct gops_gr_zbc gb10b_ops_gr_zbc = {
	.add_color = gb10b_gr_zbc_add_color,
	.load_default_sw_table = ga10b_gr_zbc_load_static_table,
	.add_depth = gb10b_gr_zbc_add_depth,
	.set_table = nvgpu_gr_zbc_set_table,
	.query_table = nvgpu_gr_zbc_query_table,
	.add_stencil = gb10b_gr_zbc_add_stencil,
	.get_gpcs_swdx_dss_zbc_c_format_reg = gb10b_gr_zbc_get_gpcs_swdx_dss_zbc_c_format_reg,
	.get_gpcs_swdx_dss_zbc_z_format_reg = gb10b_gr_zbc_get_gpcs_swdx_dss_zbc_z_format_reg,
	.init_table_indices = gb10b_gr_zbc_init_table_indices,
	.get_gr_pri_gpcs_rops_crop_zbc_index_address_max = gb10b_gr_pri_gpcs_rops_crop_zbc_index_address_max,
};
#endif

#ifdef CONFIG_NVGPU_GRAPHICS
static const struct gops_gr_zcull gb10b_ops_gr_zcull = {
	.init_zcull_hw = gb10b_gr_init_zcull_hw,
	.get_zcull_info = gm20b_gr_get_zcull_info,
	.program_zcull_mapping = gb10b_gr_program_zcull_mapping,
};
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
static const struct gops_gr_hwpm_map gb10b_ops_gr_hwpm_map = {
	.align_regs_perf_pma = gv100_gr_hwpm_map_align_regs_perf_pma,
	.add_ctxsw_buffer_gpc_perf_entries_to_map = gb10b_hwpm_map_add_gpc_perf_entries_to_map,
	.add_ctxsw_buffer_pm_ucgpc_entries_to_map = gb10b_hwpm_map_add_pm_ucgpc_entries_to_map,
	.add_ctxsw_buffer_lrcc_entries_to_map = gb10b_hwpm_map_add_lrcc_entries_to_map,
};
#endif

static const struct gops_gr_init gb10b_ops_gr_init = {
	.get_no_of_sm = nvgpu_gr_get_no_of_sm,
	.get_nonpes_aware_tpc = gv11b_gr_init_get_nonpes_aware_tpc,
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	.wait_initialized = nvgpu_gr_wait_initialized,
#endif
	/* Since ecc scrubbing is moved to ctxsw ucode, setting HAL to NULL */
	.ecc_scrub_reg = NULL,
	.lg_coalesce = NULL,
	.su_coalesce = NULL,
	.pes_vsc_stream = gm20b_gr_init_pes_vsc_stream,
	.gpc_mmu = gb10b_gr_init_gpc_mmu,
	.eng_config = ga10b_gr_init_eng_config,
	.set_gpcs_reset = gb10b_gr_init_set_gpcs_reset,
	.set_engine_reset = gb10b_gr_init_set_engine_reset,
	.setup_gfx_tpcs = gb10b_gr_set_num_gfx_capable_tpcs,
	.fifo_access = gm20b_gr_init_fifo_access,
	.set_sm_l1tag_surface_collector = gb10b_gr_init_set_sm_l1tag_surface_collector,
#ifdef CONFIG_NVGPU_SET_FALCON_ACCESS_MAP
	.get_access_map = gb10b_gr_init_get_access_map,
#endif
	.get_sm_id_size = gb10b_gr_init_get_sm_id_size,
	.sm_id_config_early = nvgpu_gr_init_sm_id_early_config,
	.load_sm_id_config = nvgpu_gr_load_sm_id_config,
	.sm_id_config = gb10b_gr_init_sm_id_config,
	.sm_id_numbering = gb10b_gr_init_sm_id_numbering,
	.tpc_mask = NULL,
	.fs_state = gb10b_gr_init_fs_state,
	.pd_tpc_per_gpc = gm20b_gr_init_pd_tpc_per_gpc,
	.pd_skip_table_gpc = gm20b_gr_init_pd_skip_table_gpc,
	.cwd_gpcs_tpcs_num = gm20b_gr_init_cwd_gpcs_tpcs_num,
	.gr_load_tpc_mask = NULL,
	.wait_empty = ga10b_gr_init_wait_empty,
	.wait_idle = ga10b_gr_init_wait_idle,
	.wait_fe_idle = gm20b_gr_init_wait_fe_idle,
#ifdef CONFIG_NVGPU_GR_GOLDEN_CTX_VERIFICATION
	.restore_stats_counter_bundle_data = gv11b_gr_init_restore_stats_counter_bundle_data,
#endif
	.fe_pwr_mode_force_on = gb10b_gr_init_fecs_pwr_blcg_force_on,
	.override_context_reset = gb10b_gr_init_override_context_reset,
	.fe_go_idle_timeout = ga10b_gr_init_fe_go_idle_timeout,
	.auto_go_idle = ga10b_gr_init_auto_go_idle,
	.load_method_init = gm20b_gr_init_load_method_init,
	.commit_global_timeslice = gb10b_gr_init_commit_global_timeslice,
	.get_bundle_cb_default_size = gv11b_gr_init_get_bundle_cb_default_size,
	.get_min_gpm_fifo_depth = gb10b_gr_init_get_min_gpm_fifo_depth,
	.get_bundle_cb_token_limit = gb10b_gr_init_get_bundle_cb_token_limit,
	.get_attrib_cb_default_size = gb10b_gr_init_get_attrib_cb_default_size,
	.get_alpha_cb_default_size = gb10b_gr_init_get_alpha_cb_default_size,
	.get_attrib_cb_size = gv11b_gr_init_get_attrib_cb_size,
	.get_alpha_cb_size = gv11b_gr_init_get_alpha_cb_size,
	.get_global_attr_cb_size = gv11b_gr_init_get_global_attr_cb_size,
	.get_global_ctx_cb_buffer_size = gm20b_gr_init_get_global_ctx_cb_buffer_size,
	.get_global_ctx_pagepool_buffer_size = gm20b_gr_init_get_global_ctx_pagepool_buffer_size,
	.commit_global_bundle_cb = gb10b_gr_init_commit_global_bundle_cb,
	.pagepool_default_size = gp10b_gr_init_pagepool_default_size,
	.commit_global_pagepool = gb10b_gr_init_commit_global_pagepool,
	.commit_global_attrib_cb = gb10b_gr_init_commit_global_attrib_cb,
	.commit_global_cb_manager = gb10b_gr_init_commit_global_cb_manager,
	.pipe_mode_override = gm20b_gr_init_pipe_mode_override,
#ifdef CONFIG_NVGPU_NON_FUSA
	.enable_mme_config_ptimer = ga10b_gr_init_enable_mme_config_ptimer,
#endif
#ifdef CONFIG_NVGPU_GR_GOLDEN_CTX_VERIFICATION
	.load_sw_bundle_init = gv11b_gr_init_load_sw_bundle_init,
#else
	.load_sw_bundle_init = gm20b_gr_init_load_sw_bundle_init,
#endif
	.load_sw_veid_bundle = gv11b_gr_init_load_sw_veid_bundle,
	.load_sw_bundle64 = tu104_gr_init_load_sw_bundle64,
	.get_max_subctx_count = gv11b_gr_init_get_max_subctx_count,
	.get_patch_slots = gv11b_gr_init_get_patch_slots,
	.detect_sm_arch = gb10b_gr_init_detect_sm_arch,
	.capture_gfx_regs = gb10b_gr_init_capture_gfx_regs,
	.set_default_gfx_regs = gb10b_gr_init_set_default_gfx_regs,
#ifndef CONFIG_NVGPU_NON_FUSA
	.set_default_compute_regs = ga10b_gr_init_set_default_compute_regs,
#endif
	.get_supported__preemption_modes = gp10b_gr_init_get_supported_preemption_modes,
	.get_default_preemption_modes = gp10b_gr_init_get_default_preemption_modes,
	.is_allowed_sw_bundle = gm20b_gr_init_is_allowed_sw_bundle,
#ifdef CONFIG_NVGPU_GRAPHICS
	.rop_mapping = gb10b_gr_init_rop_mapping,
	.get_rtv_cb_size = tu104_gr_init_get_rtv_cb_size,
	.commit_rtv_cb = gb10b_gr_init_commit_rtv_cb,
	.commit_rops_crop_override = gb10b_gr_init_commit_rops_crop_override,
	.get_gpcs_rops_crop_debug1_off = gb10b_gpcs_rops_crop_debug1_off,
#endif /* CONFIG_NVGPU_GRAPHICS */
#ifdef CONFIG_NVGPU_GFXP
	.preemption_state = gv11b_gr_init_preemption_state,
	.get_ctx_attrib_cb_size = gp10b_gr_init_get_ctx_attrib_cb_size,
	.commit_cbes_reserve = gb10b_gr_init_commit_cbes_reserve,
	.commit_gfxp_rtv_cb = gb10b_gr_init_commit_gfxp_rtv_cb,
	.get_gfxp_rtv_cb_size = tu104_gr_init_get_gfxp_rtv_cb_size,
	.get_attrib_cb_gfxp_default_size = gb10b_gr_init_get_attrib_cb_gfxp_size,
	.get_attrib_cb_gfxp_size = gb10b_gr_init_get_attrib_cb_gfxp_size,
	.gfxp_wfi_timeout = gv11b_gr_init_commit_gfxp_wfi_timeout,
	.get_ctx_spill_size = gb10b_gr_init_get_ctx_spill_size,
	.get_ctx_pagepool_size = gp10b_gr_init_get_ctx_pagepool_size,
	.get_ctx_betacb_size = gb10b_gr_init_get_ctx_betacb_size,
	.commit_ctxsw_spill = gv11b_gr_init_commit_ctxsw_spill,
#ifdef CONFIG_NVGPU_MIG
	.is_allowed_reg = ga10b_gr_init_is_allowed_reg,
#endif
#endif /* CONFIG_NVGPU_GFXP */
};

static const struct gops_gr_intr gb10b_ops_gr_intr = {
	.handle_fecs_error = gb10b_gr_intr_handle_fecs_error,
	.is_valid_sw_method = gb10b_gr_intr_is_valid_sw_method,
	.handle_sw_method = gb10b_gr_intr_handle_sw_method,
#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
	.is_valid_compute_sw_method = ga10b_gr_intr_is_valid_compute_sw_method,
	.handle_compute_sw_method = ga10b_gr_intr_handle_compute_sw_method,
#endif
#if defined(CONFIG_NVGPU_DEBUGGER) && defined(CONFIG_NVGPU_GRAPHICS)
	.is_valid_gfx_sw_method = ga10b_gr_intr_is_valid_gfx_sw_method,
	.handle_gfx_sw_method = ga10b_gr_intr_handle_gfx_sw_method,
#endif
	.handle_class_error = gp10b_gr_intr_handle_class_error,
	.clear_pending_interrupts = gm20b_gr_intr_clear_pending_interrupts,
	.read_pending_interrupts = ga10b_gr_intr_read_pending_interrupts,
	.is_illegal_method_pending = ga10b_gr_intr_is_illegal_method_pending,
	.handle_exceptions = gb10b_gr_intr_handle_exceptions,
	.read_gpc_tpc_exception = gb10b_gr_intr_read_gpc_tpc_exception,
	.read_gpc_exception = gm20b_gr_intr_read_gpc_exception,
	.read_exception1 = gm20b_gr_intr_read_exception1,
	.trapped_method_info = gm20b_gr_intr_get_trapped_method_info,
	.get_fault_context = nvgpu_gr_intr_get_fault_context,
	.handle_semaphore_pending = nvgpu_gr_intr_handle_semaphore_pending,
	.handle_notify_pending = nvgpu_gr_intr_handle_notify_pending,
	.handle_gcc_exception = gb10b_gr_intr_handle_gcc_exception,
	.handle_gpc_gpcmmu_exception = ga10b_gr_intr_handle_gpc_gpcmmu_exception,
	.handle_gpc_prop_exception = gv11b_gr_intr_handle_gpc_prop_exception,
	.handle_gpc_zcull_exception = gv11b_gr_intr_handle_gpc_zcull_exception,
	.handle_gpc_setup_exception = gv11b_gr_intr_handle_gpc_setup_exception,
	.handle_gpc_pes_exception = gv11b_gr_intr_handle_gpc_pes_exception,
	.handle_gpc_gpccs_exception = gb10b_gr_intr_handle_gpc_gpccs_exception,
	.handle_gpc_zrop_hww = ga10b_gr_intr_handle_gpc_zrop_hww,
	.handle_gpc_crop_hww = ga10b_gr_intr_handle_gpc_crop_hww,
	.get_gpc0_rop0_crop_hww_esr_offset = ga10b_gpc0_rop0_crop_hww_esr_offset,
	.handle_gpc_rrh_hww = ga10b_gr_intr_handle_gpc_rrh_hww,
	.handle_gpc_wdxps_hww = gb10b_gr_intr_handle_gpc_wdxps_hww,
	.get_gpc0_rop0_rrh_status_offset = ga10b_gpc0_rop0_rrh_status_offset,
	.get_tpc_exception = gb10b_gr_intr_get_tpc_exception,
	.handle_tpc_mpc_exception = gb10b_gr_intr_handle_tpc_mpc_exception,
	.handle_tpc_pe_exception = NULL,
	.handle_tpc_tpccs_exception = gb10b_gr_intr_handle_tpc_tpccs_exception,
	.enable_hww_exceptions = gb10b_gr_intr_enable_hww_exceptions,
	.enable_mask = ga10b_gr_intr_enable_mask,
	.enable_interrupts = gb10b_gr_intr_enable_interrupts,
	.enable_gpc_exceptions = gb10b_gr_intr_enable_gpc_exceptions,
	/* ga10b_gb10b_mismatch */
	.enable_gpc_crop_hww = gb10b_gr_intr_enable_gpc_crop_hww,
	/* ga10b_gb10b_mismatch */
	.enable_gpc_zrop_hww = gb10b_gr_intr_enable_gpc_zrop_hww,
	.enable_exceptions = gb10b_gr_intr_enable_exceptions,
	.nonstall_isr = NULL,
	.handle_sm_exception = nvgpu_gr_intr_handle_sm_exception,
	.stall_isr = nvgpu_gr_intr_stall_isr,
	.retrigger = ga10b_gr_intr_retrigger,
	.flush_channel_tlb = nvgpu_gr_intr_flush_channel_tlb,
	.set_hww_esr_report_mask = gb10b_gr_intr_set_hww_esr_report_mask,
	.handle_tpc_sm_ecc_exception = gb10b_gr_intr_handle_tpc_sm_ecc_exception,
	.get_esr_sm_sel = gb10b_gr_intr_get_esr_sm_sel,
	.clear_sm_hww = gb10b_gr_intr_clear_sm_hww,
	.handle_ssync_hww = gv11b_gr_intr_handle_ssync_hww,
	/* gv11b_gb10b_mismatch */
	.record_sm_error_state = gb10b_gr_intr_record_sm_error_state,
	/* gv11b_gb10b_mismatch */
	.get_sm_hww_warp_esr = gb10b_gr_intr_get_warp_esr_sm_hww,
	.get_sm_hww_warp_esr_pc = gb10b_gr_intr_get_warp_esr_pc_sm_hww,
	.get_sm_hww_global_esr = gb10b_gr_intr_get_sm_hww_global_esr,
	.get_sm_hww_cga_esr = gb10b_gr_intr_get_sm_hww_cga_esr,
	.handle_sm_hww_cga_esr = gb10b_gr_intr_handle_sm_hww_cga_esr,
	.get_sm_no_lock_down_hww_global_esr_mask = gv11b_gr_intr_get_sm_no_lock_down_hww_global_esr_mask,
	.get_ctxsw_checksum_mismatch_mailbox_val = gv11b_gr_intr_ctxsw_checksum_mismatch_mailbox_val,
	.sm_ecc_status_errors = ga10b_gr_intr_sm_ecc_status_errors,
	.handle_sm_poison_error = gb10b_handle_sm_poison_error,
	.sm_l1_data_ecc_status_errors = gb10b_gr_intr_sm_l1_data_ecc_status_errors,
	.sm_l1_tag_ecc_status_errors = gb10b_gr_intr_sm_l1_tag_ecc_status_errors,
	.sm_rams_ecc_status_errors = gb10b_gr_intr_sm_rams_ecc_status_errors,
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	.handle_tex_exception = NULL,
	.set_shader_exceptions = gb10b_gr_intr_set_shader_exceptions,
	.tpc_exception_sm_enable = gb10b_gr_intr_tpc_exception_sm_enable,
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	/* gm20b_gb10b_mismatch */
	.tpc_exception_sm_disable = gb10b_gr_intr_tpc_exception_sm_disable,
	.tpc_enabled_exceptions = gb10b_gr_intr_tpc_enabled_exceptions,
	.handle_cilp_save_completed = gv11b_gr_intr_handle_cilp_save_completed,
#endif
};

static const struct gops_gr_falcon gb10b_ops_gr_falcon = {
	.handle_fecs_ecc_error = gv11b_gr_falcon_handle_fecs_ecc_error,
	.read_fecs_ctxsw_mailbox = gm20b_gr_falcon_read_mailbox_fecs_ctxsw,
	.fecs_host_clear_intr = gm20b_gr_falcon_fecs_host_clear_intr,
	.fecs_host_intr_status = gb10b_gr_falcon_fecs_host_intr_status,
	.fecs_base_addr = gb10b_gr_falcon_fecs_base_addr,
	.fecs2_base_addr = gb10b_gr_falcon_fecs2_base_addr,
	.gpccs_base_addr = gb10b_gr_falcon_gpccs_base_addr,
	.gpccs2_base_addr = gb10b_gr_falcon_gpccs2_base_addr,
	.set_current_ctx_invalid = gm20b_gr_falcon_set_current_ctx_invalid,
	.dump_stats = gb10b_gr_falcon_dump_stats,
	.fecs_ctxsw_mailbox_size = ga10b_gr_falcon_get_fecs_ctxsw_mailbox_size,
	.fecs_ctxsw_clear_mailbox = ga10b_gr_falcon_fecs_ctxsw_clear_mailbox,
	.get_fecs_ctx_state_store_major_rev_id = gm20b_gr_falcon_get_fecs_ctx_state_store_major_rev_id,
	.start_gpccs = gb10b_gr_falcon_start_gpccs,
	.start_fecs = gb10b_gr_falcon_start_fecs,
	.get_gpccs_start_reg_offset = gb10b_gr_falcon_get_gpccs_start_reg_offset,
	.bind_instblk = NULL,
	.wait_mem_scrubbing = gb10b_gr_falcon_wait_mem_scrubbing,
	.wait_ctxsw_ready = gm20b_gr_falcon_wait_ctxsw_ready,
	.ctrl_ctxsw = gb10b_gr_falcon_ctrl_ctxsw,
	.get_current_ctx = gm20b_gr_falcon_get_current_ctx,
	.get_ctx_ptr = gm20b_gr_falcon_get_ctx_ptr,
	.get_current_ctx_id = gv11b_gr_falcon_get_current_ctx_id,
	.get_fecs_current_ctx_data = gm20b_gr_falcon_get_fecs_current_ctx_data,
	.get_fecs_current_ctx_hi_data =
		gb10b_gr_falcon_get_fecs_current_ctx_hi_data,
	.init_ctx_state = gp10b_gr_falcon_init_ctx_state,
	.get_zcull_image_size = gm20b_gr_falcon_get_zcull_image_size,
	.fecs_host_int_enable = gb10b_gr_falcon_fecs_host_int_enable,
	.read_fecs_ctxsw_status0 = gm20b_gr_falcon_read_status0_fecs_ctxsw,
	.read_fecs_ctxsw_status1 = gm20b_gr_falcon_read_status1_fecs_ctxsw,
	.reset_ctxsw_mailbox_scratch = gb10b_gr_falcon_reset_ctxsw_mailbox_scratch,
#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
	.load_ctxsw_ucode_header = gb10b_gr_falcon_load_ctxsw_ucode_header,
	.load_ctxsw_ucode_boot = gb10b_gr_falcon_load_ctxsw_ucode_boot,
	.load_gpccs_dmem = gb10b_gr_falcon_load_gpccs_dmem,
	.gpccs_dmemc_write = gb10b_gr_falcon_gpccs_dmemc_write,
	.load_fecs_dmem = gb10b_gr_falcon_load_fecs_dmem,
	.fecs_dmemc_write = gb10b_gr_falcon_fecs_dmemc_write,
	.load_gpccs_imem = gb10b_gr_falcon_load_gpccs_imem,
	.gpccs_imemc_write = gb10b_gr_falcon_gpccs_imemc_write,
	.load_fecs_imem = gb10b_gr_falcon_load_fecs_imem,
	.fecs_imemc_write = gb10b_gr_falcon_fecs_imemc_write,
	.start_ucode = gb10b_gr_falcon_start_ucode,
	.load_ctxsw_ucode = nvgpu_gr_falcon_load_ctxsw_ucode,
#endif
#ifdef CONFIG_NVGPU_SIM
	.configure_fmodel = gb10b_gr_falcon_configure_fmodel,
#endif
	.get_fw_name = gm20b_gr_falcon_get_fw_name,
	.get_compute_preemption_mode = gb10b_gr_falcon_get_compute_preemption_mode,
	.set_null_fecs_method_data = gb10b_gr_falcon_set_null_fecs_method_data,
};

static const struct gops_gr gb10b_ops_gr = {
	.gr_init_support = nvgpu_gr_init_support,
	.gr_suspend = nvgpu_gr_suspend,
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	.vab_reserve = gb10b_gr_vab_reserve,
	.vab_configure = gb10b_gr_vab_configure,
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	.get_gr_status = gr_gm20b_get_gr_status,
	.set_alpha_circular_buffer_size = gr_gv11b_set_alpha_circular_buffer_size,
	.get_cbm_alpha_cb_size = gb10b_gr_gpc0_ppc0_cbm_alpha_cb_size,
	.set_circular_buffer_size = gb10b_gr_set_circular_buffer_size,
	.get_sm_dsm_perf_regs = gv11b_gr_get_sm_dsm_perf_regs,
	.get_sm_dsm_perf_ctrl_regs = gv11b_gr_get_sm_dsm_perf_ctrl_regs,
	.get_sm_dbgr_ctrl_base = gb10b_get_sm_dbgr_ctrl_base,
	.get_sm_dbgr_status_base = gb10b_get_sm_dbgr_status_base,
	.get_sm_hww_global_esr_base = gb10b_get_sm_hww_global_esr_base,
	.get_sm_hww_warp_esr_base = gb10b_get_sm_hww_warp_esr_base,
	.get_gpcs_pri_mmu_debug_ctrl_reg = gb10b_get_gpcs_pri_mmu_debug_ctrl_reg,
	.get_gpcs_tpcs_tex_in_dbg_reg = gb10b_get_gpcs_tpcs_tex_in_dbg_reg,
	.get_gpcs_tpcs_sm_l1tag_ctrl_reg = gb10b_get_gpcs_tpcs_sm_l1tag_ctrl_reg,
#ifdef CONFIG_NVGPU_TEGRA_FUSE
	.set_gpc_tpc_mask = gr_gv11b_set_gpc_tpc_mask,
#endif
	.dump_gr_regs = gb10b_gr_dump_gr_status_regs,
	.dump_fecs_gr_regs = gb10b_gr_dump_fecs_gr_status_regs,
	.update_pc_sampling = NULL,
	.init_sm_dsm_reg_info = gv11b_gr_init_sm_dsm_reg_info,
	.init_cyclestats = gr_gm20b_init_cyclestats,
	.set_sm_debug_mode = gv11b_gr_set_sm_debug_mode,
	.bpt_reg_info = gb10b_gr_bpt_reg_info,
	.update_smpc_ctxsw_mode = gr_gk20a_update_smpc_ctxsw_mode,
	.update_smpc_global_mode = tu104_gr_update_smpc_global_mode,
	.update_hwpm_ctxsw_mode = gr_gk20a_update_hwpm_ctxsw_mode,
	.disable_cau = gb10b_gr_disable_cau,
	.disable_smpc = gb10b_gr_disable_smpc,
	.get_hwpm_cau_init_data = gb10b_gr_get_hwpm_cau_init_data,
	.init_cau = gb10b_gr_init_cau,
	.clear_sm_error_state = gv11b_gr_clear_sm_error_state,
	.suspend_contexts = gr_gp10b_suspend_contexts,
	.resume_contexts = gr_gk20a_resume_contexts,
	.trigger_suspend = NULL,
	.wait_for_pause = NULL,
	.resume_from_pause = NULL,
	.clear_sm_errors = gr_gk20a_clear_sm_errors,
	.is_tsg_ctx_resident = gk20a_is_tsg_ctx_resident,
	.sm_debugger_attached = gv11b_gr_sm_debugger_attached,
	.suspend_single_sm = gv11b_gr_suspend_single_sm,
	.suspend_all_sms = gv11b_gr_suspend_all_sms,
	.resume_single_sm = gv11b_gr_resume_single_sm,
	.resume_all_sms = gv11b_gr_resume_all_sms,
	.lock_down_sm = gv11b_gr_lock_down_sm,
	.wait_for_sm_lock_down = gb10b_gr_wait_for_sm_lock_down,
	.init_ovr_sm_dsm_perf = NULL,
	.get_ovr_perf_regs = NULL,
#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
	.set_boosted_ctx = NULL,
#endif
	.pre_process_sm_exception = gr_gv11b_pre_process_sm_exception,
	.set_bes_crop_debug3 = NULL,
	.set_bes_crop_debug4 = gb10b_gr_set_gpcs_rops_crop_debug4,
	.is_etpc_addr = gv11b_gr_pri_is_etpc_addr,
	.egpc_etpc_priv_addr_table = gv11b_gr_egpc_etpc_priv_addr_table,
	.get_egpc_base = gb10b_gr_get_egpc_base,
	.get_egpc_shared_base = gb10b_gr_get_egpc_shared_base,
	.get_egpc_etpc_num = gv11b_gr_get_egpc_etpc_num,
	.is_egpc_addr = gv11b_gr_pri_is_egpc_addr,
	.decode_egpc_addr = gv11b_gr_decode_egpc_addr,
	.decode_priv_addr = gr_gb10b_decode_priv_addr,
	.create_priv_addr_table = gr_gb10b_create_priv_addr_table,
	.split_fbpa_broadcast_addr = gr_gk20a_split_fbpa_broadcast_addr,
	.get_offset_in_gpccs_segment = NULL,
	.process_context_buffer_priv_segment =
		gr_ga10b_process_context_buffer_priv_segment,
	.set_debug_mode = gm20b_gr_set_debug_mode,
	.set_mmu_debug_mode = gm20b_gr_set_mmu_debug_mode,
	.set_sched_wait_for_errbar = gb10b_gr_set_sched_wait_for_errbar,
	.esr_bpt_pending_events = gv11b_gr_esr_bpt_pending_events,
	.get_ctx_buffer_offsets = gr_gk20a_get_ctx_buffer_offsets,
	.get_pm_ctx_buffer_offsets = gr_gk20a_get_pm_ctx_buffer_offsets,
	.find_priv_offset_in_buffer =
		gb10b_gr_find_priv_offset_in_buffer,
	.check_warp_esr_error = ga10b_gr_check_warp_esr_error,
	.set_cilp_preempt_pending = gr_gp10b_set_cilp_preempt_pending,
#endif /* CONFIG_NVGPU_DEBUGGER */
};

static const struct gops_nvenc gb10b_ops_nvenc = {
	.base_addr = gb10b_nvenc_base_addr,
	.falcon2_base_addr = gb10b_nvenc_falcon2_base_addr,
	.setup_boot_config = gb10b_nvenc_setup_boot_config,
	.halt_engine = gb10b_nvenc_halt_engine,
	.reset_eng = gb10b_nvenc_reset_engine,
	.get_intr_ctrl_msg = gb10b_nvenc_get_intr_ctrl_msg,
	.enable_irq = gb10b_nvenc_enable_irq,
	.nvenc_isr = gb10b_nvenc_isr,
	.init = nvgpu_nvenc_sw_init,
	.deinit = nvgpu_nvenc_sw_deinit,
	.bootstrap = nvgpu_nvenc_nvriscv_eb_boot,
	.reset_and_boot = nvgpu_nvenc_reset_and_boot,
	.multimedia_alloc_ctx = nvgpu_multimedia_setup_ctx,
	.multimedia_free_ctx = nvgpu_multimedia_free_ctx,
	.dump_engine_status = nvgpu_nvenc_dump_status,
	.get_nvenc_fw = gb10b_nvenc_fw,
	.set_safety_intr = gb10b_nvenc_set_safety_intr,
};

static const struct gops_ofa gb10b_ops_ofa = {
	.base_addr = gb10b_ofa_base_addr,
	.falcon2_base_addr = gb10b_ofa_falcon2_base_addr,
	.setup_boot_config = gb10b_ofa_setup_boot_config,
	.halt_engine = gb10b_ofa_halt_engine,
	.reset_eng = gb10b_ofa_reset_engine,
	.get_intr_ctrl_msg = gb10b_ofa_get_intr_ctrl_msg,
	.set_ecc_intr = gb10b_ofa_set_ecc_intr,
	.enable_irq = gb10b_ofa_enable_irq,
	.ofa_isr = gb10b_ofa_isr,
	.init = nvgpu_ofa_sw_init,
	.deinit = nvgpu_ofa_sw_deinit,
	.bootstrap = nvgpu_ofa_nvriscv_eb_boot,
	.reset_and_boot = nvgpu_ofa_reset_and_boot,
	.multimedia_alloc_ctx = nvgpu_multimedia_setup_ctx,
	.multimedia_free_ctx = nvgpu_multimedia_free_ctx,
	.dump_engine_status = nvgpu_ofa_dump_status,
	.get_ofa_fw = gb10b_ofa_fw,
};

static const struct gops_nvdec gb10b_ops_nvdec = {
	.base_addr = gb10b_nvdec_base_addr,
	.falcon2_base_addr = gb10b_nvdec_falcon2_base_addr,
	.setup_boot_config = gb10b_nvdec_setup_boot_config,
	.halt_engine = gb10b_nvdec_halt_engine,
	.reset_eng = gb10b_nvdec_reset_engine,
	.get_intr_ctrl_msg = gb10b_nvdec_get_intr_ctrl_msg,
	.enable_irq = gb10b_nvdec_enable_irq,
	.nvdec_isr = gb10b_nvdec_isr,
	.nvdec_is_swgen0_set = gb10b_nvdec_is_swgen0_set,
	.init = nvgpu_nvdec_sw_init,
	.deinit = nvgpu_nvdec_sw_deinit,
	.bootstrap = nvgpu_nvdec_nvriscv_br_boot,
	.reset_and_boot = nvgpu_nvdec_reset_and_boot,
	.multimedia_alloc_ctx = nvgpu_multimedia_setup_ctx,
	.multimedia_free_ctx = nvgpu_multimedia_free_ctx,
	.dump_engine_status = nvgpu_nvdec_dump_status,
	.get_nvdec_fw = gb10b_nvdec_fw,
};

static const struct gops_nvjpg gb10b_ops_nvjpg = {
	.base_addr = gb10b_nvjpg_base_addr,
	.falcon2_base_addr = gb10b_nvjpg_falcon2_base_addr,
	.setup_boot_config = gb10b_nvjpg_setup_boot_config,
	.halt_engine = gb10b_nvjpg_halt_engine,
	.reset_eng = gb10b_nvjpg_reset_engine,
	.get_intr_ctrl_msg = gb10b_nvjpg_get_intr_ctrl_msg,
	.enable_irq = gb10b_nvjpg_enable_irq,
	.nvjpg_isr = gb10b_nvjpg_isr,
	.init = nvgpu_nvjpg_sw_init,
	.deinit = nvgpu_nvjpg_sw_deinit,
	.bootstrap = nvgpu_nvjpg_nvriscv_eb_boot,
	.reset_and_boot = nvgpu_nvjpg_reset_and_boot,
	.multimedia_alloc_ctx = nvgpu_multimedia_setup_ctx,
	.multimedia_free_ctx = nvgpu_multimedia_free_ctx,
	.dump_engine_status = nvgpu_nvjpg_dump_status,
	.get_nvjpg_fw = gb10b_nvjpg_fw,
};

static const struct gops_class gb10b_ops_gpu_class = {
	.is_valid = gb10b_class_is_valid,
	.is_valid_compute = gb10b_class_is_valid_compute,
#ifdef CONFIG_NVGPU_GRAPHICS
	.is_valid_gfx = gb10b_class_is_valid_gfx,
#endif
	.is_valid_multimedia = gb10b_class_is_valid_multimedia,
	.is_valid_nvenc = gb10b_class_is_valid_nvenc,
	.is_valid_ofa = gb10b_class_is_valid_ofa,
	.is_valid_nvdec = gb10b_class_is_valid_nvdec,
	.is_valid_nvjpg = gb10b_class_is_valid_nvjpg,
};

static const struct gops_fb_ecc gb10b_ops_fb_ecc = {
	.ecc_counter_init = ga10b_fb_ecc_init,
	.ecc_counter_free = ga10b_fb_ecc_free,
	.l2tlb_error_mask = ga10b_fb_ecc_l2tlb_error_mask,
};

static const struct gops_fb_intr gb10b_ops_fb_intr = {
	.enable = gb10b_fb_intr_enable,
	.disable = gb10b_fb_intr_disable,
	.isr = ga10b_fb_intr_isr,
	.is_mmu_fault_pending = NULL,
	.handle_ecc = gv11b_fb_intr_handle_ecc,
	.handle_ecc_l2tlb = ga10b_fb_intr_handle_ecc_l2tlb,
	.handle_ecc_hubtlb = ga10b_fb_intr_handle_ecc_hubtlb,
	.handle_ecc_fillunit = ga10b_fb_intr_handle_ecc_fillunit,
	.read_l2tlb_ecc_status = gb10b_fb_intr_read_l2tlb_ecc_status,
	.read_hubtlb_ecc_status = gb10b_fb_intr_read_hubtlb_ecc_status,
	.read_fillunit_ecc_status = gb10b_fb_intr_read_fillunit_ecc_status,
	.get_l2tlb_ecc_info = gb10b_fb_intr_get_l2tlb_ecc_info,
	.get_hubtlb_ecc_info = gb10b_fb_intr_get_hubtlb_ecc_info,
	.get_fillunit_ecc_info = gb10b_fb_intr_get_fillunit_ecc_info,
	.clear_ecc_l2tlb = gb10b_fb_intr_clear_ecc_l2tlb,
	.clear_ecc_hubtlb = gb10b_fb_intr_clear_ecc_hubtlb,
	.clear_ecc_fillunit = gb10b_fb_intr_clear_ecc_fillunit,
};

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
static const struct gops_fb_vab gb10b_ops_fb_vab = {
	.init = ga10b_fb_vab_init,
	.set_vab_buffer_address = ga10b_fb_vab_set_vab_buffer_address,
	.reserve = ga10b_fb_vab_reserve,
	.dump_and_clear = ga10b_fb_vab_dump_and_clear,
	.release = ga10b_fb_vab_release,
	.teardown = ga10b_fb_vab_teardown,
	.recover = ga10b_fb_vab_recover,
};
#endif

static const struct gops_xtl gb10b_ops_xtl = {
	.init_hw = gb10b_xtl_ep_init_hw,
};

static const struct gops_fb gb10b_ops_fb = {
	.init_hw = gb10b_fb_init_hw,
	.init_fs_state = gb10b_fb_init_fs_state,
	.set_mmu_page_size = NULL,
	.mmu_ctrl = gb10b_fb_mmu_ctrl,
	.mmu_debug_ctrl = gb10b_fb_mmu_debug_ctrl,
	.mmu_debug_wr = gb10b_fb_mmu_debug_wr,
	.mmu_debug_rd = gb10b_fb_mmu_debug_rd,
#ifdef CONFIG_NVGPU_COMPRESSION
	.compression_page_size = gb10b_fb_compression_page_size,
	.compressible_page_size = gb10b_fb_compressible_page_size,
	.compression_align_mask = gb10b_fb_compression_align_mask,
	.cbc_configure = NULL,
#endif
#ifdef CONFIG_NVGPU_VPR
	.vpr_info_fetch = ga10b_fb_vpr_info_fetch,
	.dump_vpr_info = ga10b_fb_dump_vpr_info,
#endif
	.dump_wpr_info = ga10b_fb_dump_wpr_info,
	.read_wpr_info = gb10b_fb_read_wpr_info,
#ifdef CONFIG_NVGPU_DEBUGGER
	.is_debug_mode_enabled = gb10b_fb_debug_mode_enabled,
	.set_debug_mode = gm20b_fb_set_debug_mode,
	.set_mmu_debug_mode = gb10b_fb_set_mmu_debug_mode,
#endif
	.tlb_invalidate = gb10b_func_tlb_invalidate,
	.tlb_flush = gb10b_func_tlb_flush,
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
	.handle_replayable_fault = gv11b_fb_handle_replayable_mmu_fault,
	.mmu_invalidate_replay = gb10b_fb_mmu_invalidate_replay,
#endif
#ifdef CONFIG_NVGPU_DGPU
	/* This is specific to dGPU. Disable it for iGPU. */
	.mem_unlock = NULL,
	.init_fbpa = NULL,
	.fbpa_ecc_init = NULL,
#endif
	.write_mmu_fault_buffer_lo_hi = gb10b_func_write_mmu_fault_buffer_lo_hi,
	.write_mmu_fault_buffer_get = gb10b_func_write_mmu_fault_buffer_get,
	.write_mmu_fault_buffer_size = gb10b_func_write_mmu_fault_buffer_size,
	.write_mmu_fault_status = gb10b_func_write_mmu_fault_status,
	.read_mmu_fault_buffer_get = gb10b_func_read_mmu_fault_buffer_get,
	.read_mmu_fault_buffer_put = gb10b_func_read_mmu_fault_buffer_put,
	.read_mmu_fault_buffer_size = gb10b_func_read_mmu_fault_buffer_size,
	.read_mmu_fault_addr_lo_hi = gb10b_func_read_mmu_fault_addr_lo_hi,
	.read_mmu_fault_inst_lo_hi = gb10b_func_read_mmu_fault_inst_lo_hi,
	.read_mmu_fault_info = gb10b_func_read_mmu_fault_info,
	.read_mmu_fault_status = gb10b_func_read_mmu_fault_status,
	.is_fault_buf_enabled = gv11b_fb_is_fault_buf_enabled,
	.fault_buf_set_state_hw = gv11b_fb_fault_buf_set_state_hw,
	.fault_buf_configure_hw = gv11b_fb_fault_buf_configure_hw,
	.get_num_active_ltcs = gb10b_fb_get_num_active_ltcs,
#ifdef CONFIG_NVGPU_MIG
	.config_veid_smc_map = gb10b_fb_config_veid_smc_map,
	.set_smc_eng_config = ga10b_fb_set_smc_eng_config,
	.set_remote_swizid = ga10b_fb_set_remote_swizid,
	.set_smc_veid_table_mask = gb10b_fb_set_smc_veid_table_mask,
	.set_swizzid = gb10b_fb_set_swizzid,
#endif
	.set_fbp_mask = gb10b_fb_set_fbp_mask,
	.set_atomic_mode = gb10b_fb_set_atomic_mode,
	.serial_ats_ctrl = gb10b_fb_serial_ats_ctrl,
};

static const struct gops_cg gb10b_ops_cg = {
	.slcg_bus_load_gating_prod = gb10b_slcg_bus_load_gating_prod,
	.slcg_ce2_load_gating_prod = gb10b_slcg_ce2_load_gating_prod,
	.slcg_chiplet_load_gating_prod = gb10b_slcg_chiplet_load_gating_prod,
	.slcg_fb_load_gating_prod = gb10b_slcg_fb_load_gating_prod,
	.slcg_fifo_load_gating_prod = NULL,
	.slcg_runlist_load_gating_prod = gb10b_slcg_runlist_load_gating_prod,
	.slcg_gr_load_gating_prod = gb10b_slcg_gr_load_gating_prod,
	.slcg_ltc_load_gating_prod = gb10b_slcg_ltc_load_gating_prod,
	.slcg_perf_load_gating_prod = gb10b_slcg_perf_load_gating_prod,
	.slcg_priring_load_gating_prod = gb10b_slcg_priring_load_gating_prod,
	.slcg_rs_ctrl_fbp_load_gating_prod =
				gb10b_slcg_rs_ctrl_fbp_load_gating_prod,
	.slcg_rs_ctrl_gpc_load_gating_prod =
				gb10b_slcg_rs_ctrl_gpc_load_gating_prod,
	.slcg_rs_ctrl_sys_load_gating_prod =
				gb10b_slcg_rs_ctrl_sys_load_gating_prod,
	.slcg_rs_fbp_load_gating_prod = gb10b_slcg_rs_fbp_load_gating_prod,
	.slcg_rs_gpc_load_gating_prod = gb10b_slcg_rs_gpc_load_gating_prod,
	.slcg_rs_sys_load_gating_prod = gb10b_slcg_rs_sys_load_gating_prod,
	.slcg_pmu_load_gating_prod = gb10b_slcg_pmu_load_gating_prod,
	.slcg_fbhub_load_gating_prod = gb10b_slcg_fbhub_load_gating_prod,
	.slcg_therm_load_gating_prod = NULL,
	.slcg_xbar_load_gating_prod = gb10b_slcg_xbar_load_gating_prod,
	.slcg_hshub_load_gating_prod = gb10b_slcg_hshub_load_gating_prod,
	.slcg_timer_load_gating_prod = gb10b_slcg_timer_load_gating_prod,
	.slcg_ctrl_load_gating_prod = gb10b_slcg_ctrl_load_gating_prod,
	.slcg_gsp_load_gating_prod = gb10b_slcg_gsp_load_gating_prod,
	.slcg_nvenc_load_gating_prod = gb10b_slcg_nvenc_load_gating_prod,
	.slcg_ofa_load_gating_prod = gb10b_slcg_ofa_load_gating_prod,
	.slcg_nvdec_load_gating_prod = gb10b_slcg_nvdec_load_gating_prod,
	.slcg_nvjpg_load_gating_prod = gb10b_slcg_nvjpg_load_gating_prod,
	.slcg_xal_ep_load_gating_prod = gb10b_slcg_xal_ep_load_gating_prod,
	.blcg_bus_load_gating_prod = gb10b_blcg_bus_load_gating_prod,
	.blcg_ce_load_gating_prod = gb10b_blcg_ce_load_gating_prod,
	.blcg_fb_load_gating_prod = gb10b_blcg_fb_load_gating_prod,
	.blcg_fifo_load_gating_prod = NULL,
	.blcg_runlist_load_gating_prod = gb10b_blcg_runlist_load_gating_prod,
	.blcg_gr_load_gating_prod = gb10b_blcg_gr_load_gating_prod,
	.blcg_ltc_load_gating_prod = gb10b_blcg_ltc_load_gating_prod,
	.blcg_pmu_load_gating_prod = gb10b_blcg_pmu_load_gating_prod,
	.blcg_xbar_load_gating_prod = gb10b_blcg_xbar_load_gating_prod,
	.blcg_hshub_load_gating_prod = gb10b_blcg_hshub_load_gating_prod,
	.blcg_fbhub_load_gating_prod = gb10b_blcg_fbhub_load_gating_prod,
	.blcg_ctrl_load_gating_prod = gb10b_blcg_ctrl_load_gating_prod,
	.blcg_nvenc_load_gating_prod = gb10b_blcg_nvenc_load_gating_prod,
	.blcg_ofa_load_gating_prod = gb10b_blcg_ofa_load_gating_prod,
	.blcg_nvdec_load_gating_prod = gb10b_blcg_nvdec_load_gating_prod,
	.blcg_nvjpg_load_gating_prod = gb10b_blcg_nvjpg_load_gating_prod,
	.elcg_ce_load_gating_prod = gb10b_elcg_ce_load_gating_prod,
	.blcg_xal_ep_load_gating_prod = gb10b_blcg_xal_ep_load_gating_prod,
};

static const struct gops_fifo gb10b_ops_fifo = {
	.fifo_init_support = nvgpu_fifo_init_support,
	.fifo_suspend = nvgpu_fifo_suspend,
	.init_fifo_setup_hw = gb10b_init_fifo_setup_hw,
	.preempt_channel = gv11b_fifo_preempt_channel,
	.preempt_tsg = nvgpu_fifo_preempt_tsg,
	.preempt_trigger = ga10b_fifo_preempt_trigger,
	.check_tsg_preempt_is_complete = gb10b_fifo_check_tsg_preempt_is_complete,
	.preempt_poll_pbdma = gv11b_fifo_preempt_poll_pbdma,
	.is_preempt_pending = gv11b_fifo_is_preempt_pending,
	.reset_enable_hw = ga10b_init_fifo_reset_enable_hw,
#ifdef CONFIG_NVGPU_RECOVERY
	.recover = gv11b_fifo_recover,
	.rc_cleanup_and_reenable_ctxsw = gv11b_fifo_rc_cleanup_and_reenable_ctxsw,
#endif
	.intr_set_recover_mask = ga10b_fifo_intr_set_recover_mask,
	.intr_unset_recover_mask = ga10b_fifo_intr_unset_recover_mask,
	.setup_sw = nvgpu_fifo_setup_sw,
	.cleanup_sw = nvgpu_fifo_cleanup_sw,
#ifdef CONFIG_NVGPU_DEBUGGER
	.set_sm_exception_type_mask = nvgpu_tsg_set_sm_exception_type_mask,
#endif
	.intr_top_enable = gb10b_fifo_intr_top_enable,
	.intr_0_enable = gb10b_fifo_intr_0_enable,
	.intr_1_enable = ga10b_fifo_intr_1_enable,
	.intr_0_isr = ga10b_fifo_intr_0_isr,
	.intr_1_isr = NULL,
	.runlist_intr_retrigger = ga10b_fifo_runlist_intr_retrigger,
	.runlist_intr_0_en_clear_tree = ga10b_runlist_intr_0_en_clear_tree,
	.runlist_intr_0_en_set_tree = ga10b_runlist_intr_0_en_set_tree,
	.handle_sched_error = NULL,
	.ctxsw_timeout_enable = ga10b_fifo_ctxsw_timeout_enable,
	.ctxsw_timeout_config = ga10b_fifo_ctxsw_timeout_config,
	.ctxsw_timeout_config_size_1 = ga10b_ctxsw_timeout_config_size_1,
	.handle_ctxsw_timeout = NULL,
	.trigger_mmu_fault = NULL,
	.get_mmu_fault_info = NULL,
	.get_mmu_fault_desc = NULL,
	.get_mmu_fault_client_desc = NULL,
	.get_mmu_fault_gpc_desc = NULL,
	.get_runlist_timeslice = NULL,
	.get_pb_timeslice = NULL,
	.mmu_fault_id_to_pbdma_id = ga10b_fifo_mmu_fault_id_to_pbdma_id,
	.wait_for_preempt_before_reset = gb10b_runlist_wait_for_preempt_before_reset,
	.runlist_intr_0 = ga10b_runlist_intr_0,
	.runlist_intr_ctrl = gb10b_runlist_intr_ctrl,
	.get_runlist_intr_0_mask = ga10b_runlist_intr_0_mask,
	.get_runlist_intr_0_recover_mask = ga10b_runlist_intr_0_recover_mask,
	.get_runlist_intr_0_recover_unmask = ga10b_runlist_intr_0_recover_unmask,
	.get_runlist_intr_0_ctxsw_timeout_mask = ga10b_runlist_intr_0_ctxsw_timeout_mask,
	.get_runlist_intr_bad_tsg = ga10b_runlist_intr_bad_tsg,
	.get_runlist_intr_retrigger_reg_off = ga10b_runlist_intr_retrigger_reg_off,
	.get_runlist_preempt_reg = ga10b_runlist_preempt_reg,
	.get_rleng_ctxsw_timeout_info_reg_off = ga10b_rleng_ctxsw_timeout_info_reg_off,
};

static const struct gops_engine gb10b_ops_engine = {
	.is_fault_engine_subid_gpc = gv11b_is_fault_engine_subid_gpc,
	.init_ce_info = gp10b_engine_init_ce_info,
};

static const struct gops_pbdma gb10b_ops_pbdma = {
	.setup_sw = nvgpu_pbdma_setup_sw,
	.cleanup_sw = nvgpu_pbdma_cleanup_sw,
	.setup_hw = NULL,
	.intr_enable = ga10b_pbdma_intr_enable,
	/* fix_ga10b_gb10b_missing */
	.intr_0_en_set_tree_mask = gb10b_pbdma_intr_0_en_set_tree_mask,
	.intr_0_en_clear_tree_mask = gb10b_pbdma_intr_0_en_clear_tree_mask,
	.intr_1_en_set_tree_mask = ga10b_pbdma_intr_1_en_set_tree_mask,
	.intr_1_en_clear_tree_mask = ga10b_pbdma_intr_1_en_clear_tree_mask,
	.reset_method = gb10b_pbdma_reset_method,
	.acquire_val = NULL,
	.get_signature = gp10b_pbdma_get_signature,
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	.syncpt_debug_dump = NULL,
	.dump_status = gb10b_pbdma_dump_status,
#endif
	.handle_intr_0 = ga10b_pbdma_handle_intr_0,
	.handle_intr_0_acquire = NULL,
	.intr_0_pbcrc_pending = NULL,
	.handle_intr_1 = ga10b_pbdma_handle_intr_1,
	.handle_intr = ga10b_pbdma_handle_intr,
	.dump_intr_0 = gb10b_pbdma_dump_intr_0,
	.set_clear_intr_offsets = gb10b_pbdma_set_clear_intr_offsets,
	.read_data = gb10b_pbdma_read_data,
	/* ga10b_gb10b_fix_missing */
	.reset_header = gb10b_pbdma_reset_header,
	.device_fatal_0_intr_descs = ga10b_pbdma_device_fatal_0_intr_descs,
	/* ga10b_gb10b_fix_missing */
	.channel_fatal_0_intr_descs = gb10b_pbdma_channel_fatal_0_intr_descs,
	.restartable_0_intr_descs = gm20b_pbdma_restartable_0_intr_descs,
	.format_gpfifo_entry = gm20b_pbdma_format_gpfifo_entry,
	.get_gp_base = gm20b_pbdma_get_gp_base,
	/* gm20b_gb10b_fix_missing */
	.get_gp_base_hi = gb10b_pbdma_get_gp_base_hi,
	.get_fc_formats = NULL,
	/* gv11b_gb10b_fix_missing */
	.get_fc_pb_header = gb10b_pbdma_get_fc_pb_header,
	/* gm20b_gb10b_fix_missing */
	.get_fc_subdevice = gb10b_pbdma_get_fc_subdevice,
	/* ga10b_gb10b_fix_missing */
	.get_fc_target = gb10b_pbdma_get_fc_target,
	.get_ctrl_hce_priv_mode_yes = gm20b_pbdma_get_ctrl_hce_priv_mode_yes,
	.get_userd_aperture_mask = NULL,
	.get_userd_addr = NULL,
	.get_userd_hi_addr = NULL,
	.get_fc_runlist_timeslice = NULL,
	/* gp10b_gb10b_fix_missing */
	.get_config_auth_level_privileged = gb10b_pbdma_get_config_auth_level_privileged,
	.set_channel_info_veid = gv11b_pbdma_set_channel_info_veid,
	.set_channel_info_chid = ga10b_pbdma_set_channel_info_chid,
	/* ga10b_gb10b_fix_missing */
	.set_intr_notify = gb10b_pbdma_set_intr_notify,
	.is_sw_method_subch = gb10b_pbdma_is_sw_method_subch,
	.report_error = gb10b_pbdma_report_error,
	/* gv11b_gb10b_fix_missing */
	.config_userd_writeback_enable = NULL,
	.get_mmu_fault_id = ga10b_pbdma_get_mmu_fault_id,
	.get_num_of_pbdmas = gb10b_pbdma_get_num_of_pbdmas,
	.intr_0_reg_addr = ga10b_intr_0_reg_addr,
	.intr_1_reg_addr = ga10b_intr_1_reg_addr,
	.hce_dbg0_reg_addr = ga10b_hce_dbg0_reg_addr,
	.hce_dbg1_reg_addr = ga10b_hce_dbg1_reg_addr,
};

#ifdef CONFIG_TEGRA_GK20A_NVHOST
static const struct gops_sync_syncpt gb10b_ops_sync_syncpt = {
	.alloc_buf = gv11b_syncpt_alloc_buf,
	.free_buf = gv11b_syncpt_free_buf,
#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	.add_wait_cmd = gv11b_syncpt_add_wait_cmd,
	.get_wait_cmd_size = gv11b_syncpt_get_wait_cmd_size,
	.add_incr_cmd = gv11b_syncpt_add_incr_cmd,
	.get_incr_cmd_size = gv11b_syncpt_get_incr_cmd_size,
	.get_incr_per_release = gv11b_syncpt_get_incr_per_release,
#endif
	.get_sync_ro_map = gv11b_syncpt_get_sync_ro_map,
};
#endif

#ifdef CONFIG_NVGPU_SW_SEMAPHORE
static const struct gops_sync_sema gb10b_ops_sync_sema = {
	.add_wait_cmd = gv11b_sema_add_wait_cmd,
	.get_wait_cmd_size = gv11b_sema_get_wait_cmd_size,
	.add_incr_cmd = gv11b_sema_add_incr_cmd,
	.get_incr_cmd_size = gv11b_sema_get_incr_cmd_size,
};
#endif

static const struct gops_sync gb10b_ops_sync = {
};

static const struct gops_engine_status gb10b_ops_engine_status = {
	.read_engine_status_info = ga10b_read_engine_status_info,
	/* TODO update this hal for ga10b */
	.dump_engine_status = gv100_dump_engine_status,
	.get_rleng_status0_reg_off = ga10b_rleng_status0_reg_off,
	.get_rleng_status1_reg_off = ga10b_rleng_status1_reg_off,
};

static const struct gops_pbdma_status gb10b_ops_pbdma_status = {
	.read_pbdma_status_info = ga10b_read_pbdma_status_info,
	.pbdma_status_sched_reg_addr = ga10b_pbdma_status_sched_reg_addr,
	.pbdma_channel_reg_addr = ga10b_pbdma_channel_reg_addr,
	.pbdma_next_channel_reg_addr = ga10b_pbdma_next_channel_reg_addr,
};

static const struct gops_ramfc gb10b_ops_ramfc = {
	/* ga10b_gb10b_fix_missing */
	.setup = gb10b_ramfc_setup,
	.capture_ram_dump = gb10b_ramfc_capture_ram_dump_2,
	.commit_userd = NULL,
	.get_syncpt = NULL,
	.set_syncpt = NULL,
	.set_channel_info = gb10b_set_channel_info,
};

static const struct gops_ramin gb10b_ops_ramin = {
	.set_gr_ptr = gb10b_ramin_set_gr_ptr,
	.set_big_page_size = gm20b_ramin_set_big_page_size,
	.init_pdb = ga10b_ramin_init_pdb,
	.init_subctx_pdb_map = gv11b_ramin_init_subctx_pdb_map,
	.set_subctx_pdb_info = gv11b_ramin_set_subctx_pdb_info,
	.init_subctx_pdb = gv11b_ramin_init_subctx_pdb,
	.init_subctx_mask = gv11b_ramin_init_subctx_valid_mask,
	.set_adr_limit = NULL,
	.base_shift = gk20a_ramin_base_shift,
	.alloc_size = gk20a_ramin_alloc_size,
	.set_eng_method_buffer = gv11b_ramin_set_eng_method_buffer,
	.set_magic_value = ga10b_ramin_set_magic_value,
};

static const struct gops_runlist gb10b_ops_runlist = {
#ifdef NVGPU_CHANNEL_TSG_SCHEDULING
	.reschedule = gv11b_runlist_reschedule,
	.reschedule_preempt_next_locked = ga10b_fifo_reschedule_preempt_next,
#endif
	.update = nvgpu_runlist_update,
	.reload = nvgpu_runlist_reload,
	.count_max = gb10b_runlist_count_max,
	.entry_size = gv11b_runlist_entry_size,
	.length_max = ga10b_runlist_length_max,
	.get_tsg_entry = gv11b_runlist_get_tsg_entry,
	.get_ch_entry = gb10b_runlist_get_ch_entry,
	.get_ch_entry_veid = NULL,
	.hw_submit = gb10b_runlist_hw_submit,
	.check_pending = ga10b_runlist_check_pending,
	.write_state = ga10b_runlist_write_state,
	.get_runlist_id = ga10b_runlist_get_runlist_id,
	.get_runlist_aperture = ga10b_get_runlist_aperture,
	.get_engine_id_from_rleng_id = ga10b_runlist_get_engine_id_from_rleng_id,
	.get_chram_bar0_offset = ga10b_runlist_get_chram_bar0_offset,
	.get_pbdma_info = ga10b_runlist_get_pbdma_info,
	.get_engine_intr_id = NULL,
	.init_enginfo = nvgpu_runlist_init_enginfo,
	.get_tsg_max_timeslice = gv11b_runlist_max_timeslice,
	.get_esched_fb_thread_id = ga10b_runlist_get_esched_fb_thread_id,
	.get_max_channels_per_tsg = gv11b_runlist_get_max_channels_per_tsg,
	.get_sched_disable_reg_off = ga10b_sched_disable_reg_off,
	.get_chan_userd_ptr_hi = gb10b_chan_userd_ptr_hi,
	.get_virtual_channel_cfg_off = gb10b_virtual_channel_cfg_off,
};

static const struct gops_userd gb10b_ops_userd = {
#ifdef CONFIG_NVGPU_USERD
	.setup_sw = nvgpu_userd_setup_sw,
	.cleanup_sw = nvgpu_userd_cleanup_sw,
	/* ga10b_gb10b_fix_missing */
	.init_mem = gb10b_userd_init_mem,
#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	.gp_get = gb10b_sema_based_gp_get,
	.gp_put = gv11b_userd_gp_put,
	/* gv11b_gb10b_fix_missing */
	.pb_get = NULL,
#endif
#endif /* CONFIG_NVGPU_USERD */
	.entry_size = gk20a_userd_entry_size,
};

static const struct gops_channel gb10b_ops_channel = {
	.alloc_inst = nvgpu_channel_alloc_inst,
	.free_inst = nvgpu_channel_free_inst,
	.bind = ga10b_channel_bind,
	.unbind = ga10b_channel_unbind,
	.clear = ga10b_channel_clear,
	.enable = ga10b_channel_enable,
	.disable = ga10b_channel_disable,
	.count = ga10b_channel_count,
	.read_state = ga10b_channel_read_state,
	.force_ctx_reload = ga10b_channel_force_ctx_reload,
	.abort_clean_up = nvgpu_channel_abort_clean_up,
	.suspend_all_serviceable_ch = nvgpu_channel_suspend_all_serviceable_ch,
	.resume_all_serviceable_ch = nvgpu_channel_resume_all_serviceable_ch,
	.set_error_notifier = nvgpu_set_err_notifier_if_empty,
	.reset_faulted = ga10b_channel_reset_faulted,
};

static const struct gops_tsg gb10b_ops_tsg = {
	.enable = gv11b_tsg_enable,
	.disable = gv11b_tsg_disable,
	.init_subctx_state = gv11b_tsg_init_subctx_state,
	.deinit_subctx_state = gv11b_tsg_deinit_subctx_state,
	.add_subctx_channel_hw = gv11b_tsg_add_subctx_channel_hw,
	.remove_subctx_channel_hw = gv11b_tsg_remove_subctx_channel_hw,
	.init_eng_method_buffers = gv11b_tsg_init_eng_method_buffers,
	.deinit_eng_method_buffers = gv11b_tsg_deinit_eng_method_buffers,
	.bind_channel = NULL,
	.bind_channel_eng_method_buffers = gb10b_tsg_bind_channel_eng_method_buffers,
	.unbind_channel = NULL,
	.unbind_channel_check_hw_state = nvgpu_tsg_unbind_channel_hw_state_check,
	.unbind_channel_check_hw_next = ga10b_tsg_unbind_channel_check_hw_next,
	.unbind_channel_check_ctx_reload = nvgpu_tsg_unbind_channel_ctx_reload_check,
	.unbind_channel_check_eng_faulted = gv11b_tsg_unbind_channel_check_eng_faulted,
#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	.check_ctxsw_timeout = nvgpu_tsg_check_ctxsw_timeout,
#endif
#ifdef CONFIG_NVGPU_CHANNEL_TSG_CONTROL
	.force_reset = nvgpu_tsg_force_reset_ch,
	.post_event_id = nvgpu_tsg_post_event_id,
#endif
#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
	.set_timeslice = nvgpu_tsg_set_timeslice,
	.set_long_timeslice = nvgpu_tsg_set_long_timeslice,
#endif
	.default_timeslice_us = nvgpu_tsg_default_timeslice_us,
};

static const struct gops_usermode gb10b_ops_usermode = {
	.setup_hw = gb10b_usermode_setup_hw,
	.base = tu104_usermode_base,
	.bus_base = tu104_usermode_bus_base,
	.ring_doorbell = tu104_usermode_ring_doorbell,
	.doorbell_token = tu104_usermode_doorbell_token,
};

static const struct gops_netlist gb10b_ops_netlist = {
	.get_netlist_name = gb10b_netlist_get_name,
	.is_fw_defined = gb10b_netlist_is_firmware_defined,
	.get_fecs_pkc_name = gb10b_fecs_pkc_get_name,
	.get_gpccs_pkc_name = gb10b_gpccs_pkc_get_name,
	.get_fecs_desc_name = gb10b_fecs_desc_get_name,
	.get_gpccs_desc_name = gb10b_gpccs_desc_get_name,
};

static const struct gops_mm_mmu_fault gb10b_ops_mm_mmu_fault = {
	.setup_sw = gv11b_mm_mmu_fault_setup_sw,
	.setup_hw = gv11b_mm_mmu_fault_setup_hw,
	.info_mem_destroy = gv11b_mm_mmu_fault_info_mem_destroy,
	.disable_hw = gv11b_mm_mmu_fault_disable_hw,
	.parse_mmu_fault_info = gb10b_mm_mmu_fault_parse_mmu_fault_info,
	.mmu_eng_id_ce0_val = gb10b_gmmu_fault_mmu_eng_id_ce0_val,
};

static const struct gops_mm_cache gb10b_ops_mm_cache = {
	.fb_flush = gk20a_mm_fb_flush,
	.token_based_fb_flush = gb10b_mm_fb_flush_internal,
	.l2_invalidate = gb10b_func_l2_invalidate,
	.token_based_l2_invalidate_locked = gb10b_func_l2_invalidate_locked,
	.l2_flush = gv11b_mm_l2_flush,
	.token_based_l2_flush = gb10b_mm_l2_flush_internal,
#ifdef CONFIG_NVGPU_COMPRESSION
	.cbc_clean = gk20a_mm_cbc_clean,
	.token_based_cbc_clean = gb10b_mm_l2_clean_comptags,
#endif
};

static const struct gops_mm_gmmu gb10b_ops_mm_gmmu = {
	.get_mmu_levels = gb10b_mm_get_mmu_levels,
	.get_max_page_table_levels = gb10b_get_max_page_table_levels,
	.map = nvgpu_gmmu_map_locked,
	.unmap = nvgpu_gmmu_unmap_locked,
	.get_big_page_sizes = gm20b_mm_get_big_page_sizes,
	.get_default_big_page_size = nvgpu_gmmu_default_big_page_size,
	.get_default_huge_page_size = gb10b_mm_default_huge_page_size,
	.get_iommu_bit = NULL,
	.gpu_phys_addr = gv11b_gpu_phys_addr,
	.get_gpu_phys_tegra_raw_bit = NULL,
};

static const struct gops_mm gb10b_ops_mm = {
	.init_mm_support = nvgpu_init_mm_support,
	.pd_cache_init = nvgpu_pd_cache_init,
	.mm_suspend = nvgpu_mm_suspend,
	.vm_bind_channel = nvgpu_vm_bind_channel,
	.setup_hw = nvgpu_mm_setup_hw,
	.is_bar1_supported = gv11b_mm_is_bar1_supported,
	.init_inst_block = gv11b_mm_init_inst_block,
	.init_inst_block_core = gv11b_mm_init_inst_block_core,
	.bar2_vm_size = ga10b_mm_bar2_vm_size,
	.init_bar2_vm = gp10b_mm_init_bar2_vm,
	.remove_bar2_vm = gp10b_mm_remove_bar2_vm,
	.get_default_va_sizes = gp10b_mm_get_default_va_sizes,
	.bar1_map_userd = NULL,
};

static const struct gops_therm gb10b_ops_therm = {
	/*
	 * Thermal management like GPU slow throttling,
	 * HW throttling, PWM voltage ctrl etc. are moved
	 * from SOC_THERM to NV_THERM in GB10B. Thus, these
	 * one-time settings will be done by BPMP in GB10B
	 */
	.therm_max_fpdiv_factor = NULL,
	.therm_grad_stepping_pdiv_duration = NULL,
	.init_therm_support = NULL,
	.init_therm_setup_hw = NULL,
	/*
	 * All clock gating registers are moved from
	 * NV_THERM to PMU in GB10B. Thus, these HALs
	 * are also moved to PMU ops.
	 */
	.init_elcg_mode = NULL,
#ifdef CONFIG_NVGPU_NON_FUSA
	.init_blcg_mode = NULL,
#endif
	.elcg_init_idle_filters = NULL,
};

static const struct gops_gsp gb10b_ops_gsp = {
	.is_gsp_supported = true,
	.falcon_base_addr = ga10b_gsp_falcon_base_addr,
	.falcon2_base_addr = ga10b_gsp_falcon2_base_addr,
	.gsp_reset = ga10b_gsp_engine_reset,
	.validate_mem_integrity = NULL,
	/* interrupt */
	.enable_irq = gb10b_gsp_enable_irq,
	.gsp_is_interrupted = ga10b_gsp_is_interrupted,
	.gsp_isr = ga10b_gsp_isr,
	.set_msg_intr = ga10b_gsp_set_msg_intr,
	.gsp_handle_ecc = NULL,

	/* queue */
	.gsp_get_queue_head = ga10b_gsp_queue_head_r,
	.gsp_get_queue_head_size = ga10b_gsp_queue_head__size_1_v,
	.gsp_get_queue_tail = ga10b_gsp_queue_tail_r,
	.gsp_get_queue_tail_size = ga10b_gsp_queue_tail__size_1_v,
	.gsp_copy_to_emem = gb10b_gsp_flcn_copy_to_emem,
	.gsp_copy_from_emem = gb10b_gsp_flcn_copy_from_emem,
	.gsp_queue_head = ga10b_gsp_queue_head,
	.gsp_queue_tail = ga10b_gsp_queue_tail,
	.msgq_tail = ga10b_gsp_msgq_tail,

	.falcon_setup_boot_config = NULL,
};

static const struct gops_pmu gb10b_ops_pmu = {
	.ecc_init = gv11b_pmu_ecc_init,
	.ecc_free = gv11b_pmu_ecc_free,
#ifdef CONFIG_NVGPU_INJECT_HWERR
	.get_pmu_err_desc = gb10b_pmu_intr_get_err_desc,
#endif /* CONFIG_NVGPU_INJECT_HWERR */
	/*
		 * Basic init ops are must, as PMU engine used by ACR to
		 * load & bootstrap GR LS falcons without LS PMU, remaining
		 * ops can be assigned/ignored as per build flag request
		 */
	/* Basic init ops */
	.pmu_early_init = nvgpu_pmu_early_init,
#ifdef CONFIG_NVGPU_POWER_PG
	.pmu_restore_golden_img_state = nvgpu_pmu_restore_golden_img_state,
#endif
	.is_pmu_supported = ga10b_is_pmu_supported,
	.falcon_base_addr = gb10b_pmu_falcon_base_addr,
	.falcon2_base_addr = gb10b_pmu_falcon2_base_addr,
	.pmu_reset = nvgpu_pmu_reset,
	.reset_engine = gb10b_pmu_engine_reset,
	.is_engine_in_reset = gb10b_pmu_is_engine_in_reset,
	.is_debug_mode_enabled = gb10b_pmu_is_debug_mode_en,
	/* aperture set up is moved to acr */
	.setup_apertures = NULL,
	.flcn_setup_boot_config = gv11b_pmu_flcn_setup_boot_config,
	.pmu_clear_bar0_host_err_status = gb10b_clear_pmu_bar0_host_err_status,
	.bar0_error_status = gv11b_pmu_bar0_error_status,
	.validate_mem_integrity = gv11b_pmu_validate_mem_integrity,
	.get_intr_ctrl_msg = gb10b_pmu_get_intr_ctrl_msg,
	.pmu_enable_irq = gb10b_pmu_enable_irq,
	.get_irqdest = gv11b_pmu_get_irqdest,
	.get_irqmask = gb10b_pmu_get_irqmask,
	.set_mailbox1 = gb10b_pmu_set_mailbox1,
	.get_ecc_address = gb10b_pmu_get_ecc_address,
	.get_ecc_status = gb10b_pmu_get_ecc_status,
	.set_ecc_status = gb10b_pmu_set_ecc_status,
	.get_irqstat = gb10b_pmu_get_irqstat,
	.set_irqsclr = gb10b_pmu_set_irqsclr,
	.set_irqsset = gb10b_pmu_set_irqsset,
	.get_exterrstat = gb10b_pmu_get_exterrstat,
	.set_exterrstat = gb10b_pmu_set_exterrstat,
	.get_exterraddr = gb10b_pmu_get_exterraddr,
	.get_bar0_addr = gb10b_pmu_get_bar0_addr,
	.get_bar0_data = gb10b_pmu_get_bar0_data,
	.get_bar0_timeout = gb10b_pmu_get_bar0_timeout,
	.get_bar0_ctl = gb10b_pmu_get_bar0_ctl,
	.get_bar0_error_status = gb10b_pmu_get_bar0_error_status,
	.set_bar0_error_status = gb10b_pmu_set_bar0_error_status,
	.get_bar0_fecs_error = gb10b_pmu_get_bar0_fecs_error,
	.set_bar0_fecs_error = gb10b_pmu_set_bar0_fecs_error,
	.get_mailbox = gb10b_pmu_get_mailbox,
	.get_pmu_debug = gb10b_pmu_get_pmu_debug,
	.get_pmu_msgq_head = gb10b_pmu_get_pmu_msgq_head,
	.set_pmu_msgq_head = gb10b_pmu_set_pmu_msgq_head,
	.set_pmu_new_instblk = gb10b_pmu_set_new_instblk,
	.pmu_isr = gk20a_pmu_isr,
	.handle_ext_irq = ga10b_pmu_handle_ext_irq,
#ifdef CONFIG_NVGPU_LS_PMU
	.get_inst_block_config = ga10b_pmu_get_inst_block_config,
	/* Init */
	.pmu_rtos_init = nvgpu_pmu_rtos_init,
	.pmu_pstate_sw_setup = nvgpu_pmu_pstate_sw_setup,
	.pmu_pstate_pmu_setup = nvgpu_pmu_pstate_pmu_setup,
	.pmu_destroy = nvgpu_pmu_destroy,
	/* ISR */
	.pmu_is_interrupted = ga10b_pmu_is_interrupted,
	.handle_swgen1_irq = ga10b_pmu_handle_swgen1_irq,
	/* pmu sequence */
	.pmu_seq_cleanup = nvgpu_pmu_seq_free_release,
	/* queue */
	.pmu_get_queue_head = gb10b_pmu_queue_head_r,
	.pmu_get_queue_head_size = gb10b_pmu_queue_head__size_1_v,
	.pmu_get_queue_tail = gb10b_pmu_queue_tail_r,
	.pmu_get_queue_tail_size = gb10b_pmu_queue_tail__size_1_v,
	.pmu_queue_head = gk20a_pmu_queue_head,
	.pmu_queue_tail = gk20a_pmu_queue_tail,
	.pmu_msgq_tail = gk20a_pmu_msgq_tail,
	.pmu_get_msgq_tail = gb10b_pmu_msgq_tail_r,
	/* mutex */
	.pmu_mutex_size = gv11b_pmu_mutex__size_1_v,
	.pmu_mutex_owner = gk20a_pmu_mutex_owner,
	.pmu_mutex_acquire = gk20a_pmu_mutex_acquire,
	.pmu_mutex_release = gk20a_pmu_mutex_release,
	.pmu_get_mutex_reg = gb10b_pmu_get_mutex_reg,
	.pmu_set_mutex_reg = gb10b_pmu_set_mutex_reg,
	.pmu_get_mutex_id = gb10b_pmu_get_mutex_id,
	.pmu_get_mutex_id_release = gb10b_pmu_get_mutex_id_release,
	.pmu_set_mutex_id_release = gb10b_pmu_set_mutex_id_release,
	/* power-gating */
	.pmu_setup_elpg = NULL,
	.pmu_pg_idle_counter_config = gb10b_pmu_pg_idle_counter_config,
	.pmu_dump_elpg_stats = gb10b_pmu_dump_elpg_stats,
	/* perfmon */
	.pmu_init_perfmon_counter = gb10b_pmu_init_perfmon_counter,
	.pmu_read_idle_counter = gb10b_pmu_read_idle_counter,
	.pmu_reset_idle_counter = gb10b_pmu_reset_idle_counter,
	.pmu_read_idle_intr_status = gb10b_pmu_read_idle_intr_status,
	.pmu_clear_idle_intr_status = gb10b_pmu_clear_idle_intr_status,
	/* debug */
	.dump_secure_fuses = pmu_dump_security_fuses_gm20b,
	.pmu_dump_falcon_stats = gk20a_pmu_dump_falcon_stats,
	/* PMU ucode */
	.pmu_ns_bootstrap = ga10b_pmu_ns_bootstrap,
	.secured_pmu_start = NULL,
	.write_dmatrfbase = gb10b_write_dmatrfbase,
	/* Clock-gating control */
	.is_cg_supported_by_pmu = gb10b_is_cg_supported_by_pmu,
	.pmu_init_elcg_mode = gb10b_pmu_init_elcg_mode,
#ifdef CONFIG_NVGPU_NON_FUSA
	.pmu_init_blcg_mode = gb10b_pmu_init_blcg_mode,
#endif
	.pmu_elcg_init_idle_filters = gb10b_pmu_elcg_init_idle_filters,

#endif
	.get_amap_extmem2_start = gb10b_get_amap_extmem2_start,
};

#ifdef CONFIG_NVGPU_CLK_ARB
static const struct gops_clk_arb gb10b_ops_clk_arb = {
	.clk_arb_init_arbiter = nvgpu_clk_arb_init_arbiter,
	.check_clk_arb_support = gp10b_check_clk_arb_support,
	.get_arbiter_clk_domains = gp10b_get_arbiter_clk_domains,
	.get_arbiter_f_points = gp10b_get_arbiter_f_points,
	.get_arbiter_clk_range = gp10b_get_arbiter_clk_range,
	.get_arbiter_clk_default = gp10b_get_arbiter_clk_default,
	.arbiter_clk_init = gp10b_init_clk_arbiter,
	.clk_arb_run_arbiter_cb = gp10b_clk_arb_run_arbiter_cb,
	.clk_arb_cleanup = gp10b_clk_arb_cleanup,
};
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
static const struct gops_regops gb10b_ops_regops = {
	.exec_regops = exec_regops_gk20a,
	.get_global_whitelist_ranges = gb10b_get_global_allowlist_ranges,
	.get_global_whitelist_ranges_count = gb10b_get_global_allowlist_ranges_count,
	.get_context_whitelist_ranges = gb10b_get_context_allowlist_ranges,
	.get_context_whitelist_ranges_count = gb10b_get_context_allowlist_ranges_count,
	.get_runcontrol_whitelist = gb10b_get_runcontrol_allowlist,
	.get_runcontrol_whitelist_count = gb10b_get_runcontrol_allowlist_count,
	.get_hwpm_router_register_stride = gb10b_get_hwpm_router_register_stride,
	.get_hwpm_perfmon_register_stride = gb10b_get_hwpm_perfmon_register_stride,
	.get_hwpm_pma_channel_register_stride = gb10b_get_hwpm_pma_channel_register_stride,
	.get_hwpm_pma_trigger_register_stride = gb10b_get_hwpm_pma_trigger_register_stride,
	.get_smpc_register_stride = gb10b_get_smpc_register_stride,
	.get_cau_register_stride = gb10b_get_cau_register_stride,
	.get_hwpm_perfmon_register_offset_allowlist =
		gb10b_get_hwpm_perfmon_register_offset_allowlist,
	.get_hwpm_router_register_offset_allowlist =
		gb10b_get_hwpm_router_register_offset_allowlist,
	.get_hwpm_pma_channel_register_offset_allowlist =
		gb10b_get_hwpm_pma_channel_register_offset_allowlist,
	.get_hwpm_pma_trigger_register_offset_allowlist =
		gb10b_get_hwpm_pma_trigger_register_offset_allowlist,
	.get_smpc_register_offset_allowlist = gb10b_get_smpc_register_offset_allowlist,
	.get_cau_register_offset_allowlist = gb10b_get_cau_register_offset_allowlist,
	.get_hwpm_perfmon_register_ranges = gb10b_get_hwpm_perfmon_register_ranges,
	.get_hwpm_router_register_ranges = gb10b_get_hwpm_router_register_ranges,
	.get_hwpm_pma_channel_register_ranges = gb10b_get_hwpm_pma_channel_register_ranges,
	.get_hwpm_pc_sampler_register_ranges = gb10b_get_hwpm_pc_sampler_register_ranges,
	.get_hwpm_pma_trigger_register_ranges = gb10b_get_hwpm_pma_trigger_register_ranges,
	.get_smpc_register_ranges = gb10b_get_smpc_register_ranges,
	.get_cau_register_ranges = gb10b_get_cau_register_ranges,
	.get_hwpm_perfmux_register_ranges = gb10b_get_hwpm_perfmux_register_ranges,
	.get_hes_register_ranges = gb10b_get_hes_register_ranges,
	.get_hwpm_mm_register_ranges = gb10b_get_hwpm_mm_register_ranges,
	.is_hwpm_pma_reg_context_switched = gb10b_is_hwpm_pma_reg_context_switched,
};
#endif

static const struct gops_mc gb10b_ops_mc = {
	.get_chip_details = gm20b_get_chip_details,
	.enable_units = ga10b_mc_enable_units,
	.enable_dev = gb10b_mc_enable_dev,
	.enable_devtype = gb10b_mc_enable_devtype,
#ifdef CONFIG_NVGPU_NON_FUSA
	.elpg_enable = ga10b_mc_elpg_enable,
	.log_pending_intrs = gb10b_intr_log_pending_intrs,
#endif
#ifdef CONFIG_NVGPU_LS_PMU
	.is_enabled = gb10b_mc_is_enabled,
#endif
	.ltc_isr = mc_tu104_ltc_isr,
};

static const struct gops_debug gb10b_ops_debug = {
	.show_dump = gk20a_debug_show_dump,
};

#ifdef CONFIG_NVGPU_DEBUGGER
static const struct gops_debugger gb10b_ops_debugger = {
	.post_events = nvgpu_dbg_gpu_post_events,
	.dbg_set_powergate = nvgpu_dbg_set_powergate,
};
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
static const struct gops_perf gb10b_ops_perf = {
	.enable_membuf = gb10b_perf_enable_membuf,
	.disable_membuf = gb10b_perf_disable_membuf,
	.bind_mem_bytes_buffer_addr = gb10b_perf_bind_mem_bytes_buffer_addr,
	.init_inst_block = gb10b_perf_init_inst_block,
	.deinit_inst_block = gb10b_perf_deinit_inst_block,
	.membuf_reset_streaming = gb10b_perf_membuf_reset_streaming,
	.get_membuf_pending_bytes = gb10b_perf_get_membuf_pending_bytes,
	.set_membuf_handled_bytes = gb10b_perf_set_membuf_handled_bytes,
	.get_membuf_overflow_status = gb10b_perf_get_membuf_overflow_status,
	.get_pmmsys_per_chiplet_offset = gb10b_perf_get_pmmsys_per_chiplet_offset,
	.get_pmmgpc_per_chiplet_offset = gb10b_perf_get_pmmgpc_per_chiplet_offset,
	.get_pmmfbp_per_chiplet_offset = gb10b_perf_get_pmmfbp_per_chiplet_offset,
	.get_pmmgpcrouter_per_chiplet_offset = gb10b_perf_get_pmmgpcrouter_per_chiplet_offset,
	.get_pmmfbprouter_per_chiplet_offset = gb10b_perf_get_pmmfbprouter_per_chiplet_offset,
	.update_get_put = gb10b_perf_update_get_put,
	.get_hwpm_fbp_perfmon_regs_base = ga10b_get_hwpm_fbp_perfmon_regs_base,
	.get_hwpm_gpc_perfmon_regs_base = ga10b_get_hwpm_gpc_perfmon_regs_base,
	.get_hwpm_sys_perfmon_regs = gb10b_perf_get_hwpm_sys_perfmon_regs,
	.get_hwpm_gpc_perfmon_regs = gb10b_perf_get_hwpm_gpc_perfmon_regs,
	.get_hwpm_fbp_perfmon_regs = gb10b_perf_get_hwpm_fbp_perfmon_regs,
	.get_hwpm_fbprouter_perfmon_regs_base = gb10b_get_hwpm_fbprouter_perfmon_regs_base,
	.get_hwpm_gpcrouter_perfmon_regs_base = gb10b_get_hwpm_gpcrouter_perfmon_regs_base,
	.set_pmm_register = gv11b_perf_set_pmm_register,
	.get_num_hwpm_perfmon = gb10b_perf_get_num_hwpm_perfmon,
	.init_hwpm_pmm_register = gb10b_perf_init_hwpm_pmm_register,
	.reset_hwpm_pmm_registers = gb10b_perf_reset_hwpm_pmm_register,
	.pma_stream_enable = gb10b_perf_pma_stream_enable,
	.disable_all_perfmons = gb10b_perf_disable_all_perfmons,
	.wait_for_idle_pmm_routers = gb10b_perf_wait_for_idle_pmm_routers,
	.wait_for_idle_pma = gb10b_perf_wait_for_idle_pma,
	.enable_hs_streaming = NULL,
	.reset_hs_streaming_credits = NULL,
	.enable_pmasys_legacy_mode = NULL,
	.set_secure_config_for_hwpm_dg = gb10b_set_secure_config_for_hwpm_dg,
	.reset_cwd_hem_registers = gb10b_perf_reset_cwd_hem_registers,
	.set_secure_config_for_cwd_hem = gb10b_perf_set_secure_config_for_cwd_hem,
	.set_secure_config_for_cau = gb10b_perf_set_secure_config_for_cau,
	.update_sys_dg_map_status_mask = gb10b_update_sys_dg_map_status_mask,
	.update_cau_dg_map_status_mask = gb10b_update_cau_dg_map_status_mask,
	.get_pma_cblock_instance_count = gb10b_perf_get_pma_cblock_instance_count,
	.get_pma_channel_count = gb10b_perf_get_pma_channel_count,
	.get_pma_channels_per_cblock = gb10b_perf_get_pma_channels_per_cblock,
	.reset_pm_trigger_masks = gb10b_perf_reset_pm_trigger_masks,
	.enable_pm_trigger = gb10b_perf_enable_pm_trigger,
	.enable_hes_event_trigger = gb10b_perf_enable_hes_event_trigger,
	.enable_pma_trigger = gb10b_perf_enable_pma_trigger,
	.get_hwpm_fbp_fbpgs_ltcs_base_addr = gb10b_perf_get_hwpm_fbp_fbpgs_ltcs_base_addr,
	.get_hwpm_gpcgs_gpctpca_base_addr = gb10b_perf_get_hwpm_gpcgs_gpctpca_base_addr,
	.get_hwpm_gpcgs_gpctpcb_base_addr = gb10b_perf_get_hwpm_gpcgs_gpctpcb_base_addr,
	.get_hwpm_gpcs_base_addr = gb10b_perf_get_hwpm_gpcs_base_addr,
	.get_hwpm_gpcsrouter_base_addr = gb10b_perf_get_hwpm_gpcsrouter_base_addr,
	.get_hwpm_fbps_base_addr = gb10b_perf_get_hwpm_fbps_base_addr,
	.get_hwpm_fbpsrouter_base_addr = gb10b_perf_get_hwpm_fbpsrouter_base_addr,
	.set_pmm_register_for_chiplet_range = gb10b_perf_set_pmm_register_for_chiplet_range,
	.alloc_mem_for_sys_partition_dg_map = gb10b_perf_alloc_mem_for_sys_partition_dg_map,
	.program_sys_pmm_secure_config = gb10b_perf_program_sys_pmm_secure_config,
	.get_tpc_perfmon_range = gb10b_perf_get_tpc_perfmon_range,
	.get_max_num_gpc_perfmons = gb10b_perf_get_max_num_gpc_perfmons,
	.get_gpc_perfmon_stride = gb10b_perf_get_gpc_perfmon_stride,
	.get_gpc_tpc_start_dg_idx = gb10b_perf_get_gpc_tpc_start_dg_idx,
	.get_gpc_tpc_end_dg_idx = gb10b_perf_get_gpc_tpc_end_dg_idx,
	.is_perfmon_simulated = gb10b_perf_is_perfmon_simulated,
	.get_num_cwd_hems = gb10b_perf_get_num_cwd_hems,
	.update_sys_hem_cwd_dg_map_mask = gb10b_perf_update_sys_hem_cwd_dg_map_mask,
	.get_gpc_tpc0_cau0_dg_idx = gb10b_perf_get_gpc_tpc0_cau0_dg_idx,
};
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
static const struct gops_perfbuf gb10b_ops_perfbuf = {
	.perfbuf_enable = nvgpu_perfbuf_enable_locked,
	.perfbuf_disable = nvgpu_perfbuf_disable_locked,
	.init_inst_block = nvgpu_perfbuf_init_inst_block,
	.deinit_inst_block = nvgpu_perfbuf_deinit_inst_block,
	.update_get_put = nvgpu_perfbuf_update_get_put,
};
#endif

#ifdef CONFIG_NVGPU_PROFILER
static const struct gops_pm_reservation gb10b_ops_pm_reservation = {
	.acquire = nvgpu_pm_reservation_acquire,
	.release = nvgpu_pm_reservation_release,
	.release_all_per_vmid = nvgpu_pm_reservation_release_all_per_vmid,
};
#endif

#ifdef CONFIG_NVGPU_PROFILER
static const struct gops_profiler gb10b_ops_profiler = {
	.bind_hwpm = nvgpu_profiler_bind_hwpm,
	.unbind_hwpm = nvgpu_profiler_unbind_hwpm,
	.bind_pma_streamout = nvgpu_profiler_bind_pma_streamout,
	.unbind_pma_streamout = nvgpu_profiler_unbind_pma_streamout,
	.bind_smpc = nvgpu_profiler_bind_smpc,
	.unbind_smpc = nvgpu_profiler_unbind_smpc,
	.bind_hes_cwd = nvgpu_profiler_bind_hes_cwd,
	.get_hs_credit_per_gpc_chiplet = gb10b_perf_get_hs_credit_per_gpc_chiplet,
	.get_hs_credit_per_fbp_chiplet = gb10b_perf_get_hs_credit_per_fbp_chiplet,
	.get_hs_credit_per_sys_pipe_for_profiling = gb10b_perf_get_hs_credit_per_sys_pipe_for_profiling,
	.get_hs_credit_per_sys_pipe_for_hes = gb10b_perf_get_hs_credit_per_sys_pipe_for_hes,
	.set_hs_credit_per_chiplet = gb10b_perf_set_hs_credit_per_chiplet,
};
#endif

/* TODO: Update HALs of bus unit w.r.t. xal_ep. JIRA NVGPU-9321 */
static const struct gops_bus gb10b_ops_bus = {
	.init_hw = gb10b_xal_ep_init_hw,
	.isr = gb10b_xal_ep_isr,
	/* gm20b_gb10b_fix_missing */
	.bar1_bind = gb10b_func_bar1_bind,
	.bar2_bind = gb10b_func_bar2_bind,
	.configure_debug_bus = NULL,
#ifdef CONFIG_NVGPU_DGPU
	.set_bar0_window = gk20a_bus_set_bar0_window,
#endif
};

static const struct gops_ptimer gb10b_ops_ptimer = {
#ifdef CONFIG_NVGPU_IOCTL_NON_FUSA
	.read_ptimer = gk20a_read_ptimer,
	.get_timestamps_zipper = nvgpu_get_timestamps_zipper,
#endif
#ifdef CONFIG_NVGPU_PROFILER
	.get_timer_reg_offsets = gv11b_ptimer_get_timer_reg_offsets,
#endif

};

#if defined(CONFIG_NVGPU_CYCLESTATS)
static const struct gops_css gb10b_ops_css = {
	.enable_snapshot = nvgpu_css_enable_snapshot,
	.disable_snapshot = nvgpu_css_disable_snapshot,
	.check_data_available = nvgpu_css_check_data_available,
	.set_handled_snapshots = nvgpu_css_set_handled_snapshots,
	.allocate_perfmon_ids = nvgpu_css_allocate_perfmon_ids,
	.release_perfmon_ids = nvgpu_css_release_perfmon_ids,
	.get_overflow_status = nvgpu_css_get_overflow_status,
	.get_pending_snapshots = nvgpu_css_get_pending_snapshots,
	.get_max_buffer_size = nvgpu_css_get_max_buffer_size,
};
#endif

static const struct gops_falcon gb10b_ops_falcon = {
	.falcon_sw_init = nvgpu_falcon_sw_init,
	.falcon_sw_free = nvgpu_falcon_sw_free,
	.reset = gk20a_falcon_reset,
	.is_falcon_cpu_halted = ga10b_falcon_is_cpu_halted,
	.is_falcon_idle = ga10b_is_falcon_idle,
	.is_falcon_scrubbing_done = ga10b_is_falcon_scrubbing_done,
	.get_mem_size = gb10b_falcon_get_mem_size,
	.get_ports_count = gk20a_falcon_get_ports_count,
	.copy_to_dmem = gk20a_falcon_copy_to_dmem,
	.copy_to_imem = gk20a_falcon_copy_to_imem,
	.dmemc_blk_mask = ga10b_falcon_dmemc_blk_mask,
	.imemc_blk_field = ga10b_falcon_imemc_blk_field,
	.bootstrap = ga10b_falcon_bootstrap,
	.dump_brom_stats = ga10b_falcon_dump_brom_stats,
	.get_brom_retcode = ga10b_falcon_get_brom_retcode,
	.is_priv_lockdown = ga10b_falcon_is_priv_lockdown,
	.check_brom_passed = ga10b_falcon_check_brom_passed,
	.check_brom_failed = ga10b_falcon_check_brom_failed,
	.check_brom_init_done = gb10b_falcon_check_brom_init_done,
	.set_bcr = ga10b_falcon_set_bcr,
	.brom_config = ga10b_falcon_brom_config,
	.mailbox_read = gk20a_falcon_mailbox_read,
	.mailbox_write = gk20a_falcon_mailbox_write,
	.set_irq = gk20a_falcon_set_irq,
	.get_bootplugin_fw_name = gb10b_get_bootplugin_fw_name,
#ifdef CONFIG_NVGPU_FALCON_DEBUG
	.dump_falcon_stats = ga10b_falcon_dump_stats,
	.dump_falcon_info = gb10b_falcon_dump_info,
	.dump_debug_regs = gb10b_falcon_dump_debug_regs,
#endif
#if defined(CONFIG_NVGPU_FALCON_DEBUG) || defined(CONFIG_NVGPU_FALCON_NON_FUSA)
	.copy_from_dmem = gk20a_falcon_copy_from_dmem,
#endif
#ifdef CONFIG_NVGPU_FALCON_NON_FUSA
	.clear_halt_interrupt_status = gk20a_falcon_clear_halt_interrupt_status,
	.copy_from_imem = gk20a_falcon_copy_from_imem,
	.get_falcon_ctls = gk20a_falcon_get_ctls,
#endif
	.load_ucode = gk20a_falcon_load_ucode_dma,
	.debuginfo_offset = gb10b_falcon_debuginfo_offset,
	.write_dmatrfbase = gb10b_falcon_write_dmatrfbase,
};

static const struct gops_xbar gb10b_ops_xbar = {
	.handle_xbar_read_ecc_err = gb10b_handle_xbar_read_ecc_err,
};

static const struct gops_priv_ring gb10b_ops_priv_ring = {
	/* gm20b_gb10b_fix_missing */
	.enable_priv_ring = gb10b_priv_ring_enable,
	.isr = gp10b_priv_ring_isr,
	.isr_handle_0 = ga10b_priv_ring_isr_handle_0,
	.isr_handle_1 = ga10b_priv_ring_isr_handle_1,
	.intr_retrigger = gb10b_priv_ring_intr_retrigger,
	.decode_error_code = ga10b_priv_ring_decode_error_code,
	.enum_ltc = ga10b_priv_ring_enum_ltc,
	.get_gpc_count = gm20b_priv_ring_get_gpc_count,
	.get_fbp_count = gm20b_priv_ring_get_fbp_count,
	.init_fbp_physical_logical_id_map = gb10b_init_fbp_physical_logical_id_map,
#ifdef CONFIG_NVGPU_MIG
	.config_gr_remap_window = ga10b_priv_ring_config_gr_remap_window,
	.config_gpc_rs_map = gb10b_priv_ring_config_gpc_rs_map,
#endif
#ifdef CONFIG_NVGPU_PROFILER
	.read_pri_fence = ga10b_priv_ring_read_pri_fence,
#endif
};

static const struct gops_fuse gb10b_ops_fuse = {
	.check_priv_security = ga10b_fuse_check_priv_security,
	.is_opt_ecc_enable = ga10b_fuse_is_opt_ecc_enable,
	.is_opt_feature_override_disable = ga10b_fuse_is_opt_feature_override_disable,
	.fuse_status_opt_fbio = ga10b_fuse_status_opt_fbio,
	.fuse_status_opt_fbp = ga10b_fuse_status_opt_fbp,
	.fuse_status_opt_emc = ga10b_fuse_status_opt_emc,
	.fuse_status_opt_l2_fbp = ga10b_fuse_status_opt_l2_fbp,
	.fuse_status_opt_tpc_gpc = ga10b_fuse_status_opt_tpc_gpc,
	.fuse_status_opt_pes_gpc = ga10b_fuse_status_opt_pes_gpc,
	.fuse_status_opt_rop_gpc = ga10b_fuse_status_opt_rop_gpc,
	.fuse_ctrl_opt_tpc_gpc = ga10b_fuse_ctrl_opt_tpc_gpc,
	.fuse_opt_sec_debug_en = ga10b_fuse_opt_sec_debug_en,
	.fuse_opt_priv_sec_en = ga10b_fuse_opt_priv_sec_en,
	.fuse_opt_sm_ttu_en = NULL,
	.opt_sec_source_isolation_en =
			ga10b_fuse_opt_secure_source_isolation_en,
	.read_vin_cal_fuse_rev = NULL,
	.read_vin_cal_slope_intercept_fuse = NULL,
	.read_vin_cal_gain_offset_fuse = NULL,
	.read_gcplex_config_fuse = gb10b_fuse_read_gcplex_config_fuse,
	.fuse_status_opt_gpc = ga10b_fuse_status_opt_gpc,
#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
	.write_feature_override_ecc = ga10b_fuse_write_feature_override_ecc,
	.write_feature_override_ecc_1 = ga10b_fuse_write_feature_override_ecc_1,
#endif
	.read_feature_override_ecc = ga10b_fuse_read_feature_override_ecc,
	.read_per_device_identifier = gb10b_fuse_read_per_device_identifier,
	.fetch_falcon_fuse_settings = NULL,
	.fuse_status_opt_cpc_gpc = gb10b_fuse_status_opt_cpc_gpc,
	.init_hw = gb10b_fuse_init_hw,
	.feature_override_ofa_err_contain_en =
			gb10b_feature_override_ofa_err_contain_en,
	.feature_override_nvenc_err_contain_en =
			gb10b_feature_override_nvenc_err_contain_en,
};

static const struct gops_top gb10b_ops_top = {
	.device_info_parse_enum = NULL,
	.device_info_parse_data = NULL,
	.get_expected_chip_info_cfg_version = gb10b_get_expected_chip_info_cfg_version,
	.parse_next_device = gb10b_top_parse_next_dev,
	.get_max_gpc_count = gm20b_top_get_max_gpc_count,
	.get_max_tpc_per_gpc_count = gm20b_top_get_max_tpc_per_gpc_count,
	.get_max_fbps_count = gm20b_top_get_max_fbps_count,
	.get_max_ltc_per_fbp = gm20b_top_get_max_ltc_per_fbp,
	.get_max_lts_per_ltc = gm20b_top_get_max_lts_per_ltc,
	.get_num_ltcs = gm20b_top_get_num_ltcs,
	.get_num_lce = gv11b_top_get_num_lce,
	.get_max_rop_per_gpc = ga10b_top_get_max_rop_per_gpc,
	.get_max_pes_per_gpc = gv11b_top_get_max_pes_per_gpc,
};

static const struct gops_sysctrl gb10b_ops_sysctrl = {
	.init_hw = gb10b_sysctrl_init_hw,
};

#ifdef CONFIG_NVGPU_STATIC_POWERGATE
static const struct gops_tpc_pg gb10b_ops_tpc_pg = {
	/*
	 * HALs for static-pg will be updated for pre-silicon platform
	 * during HAL init. For silicon, static-pg feature related settings
	 * will be taken care by BPMP.
	 * Silicon: assign the HALs to NULL. TODO: JIRA NVGPU-9265.
	 * Pre-silicon: Currently, HALs are reused from GV11B.
	 * This needs to be updated for GB10B. TODO: JIRA NVGPU-7112.
	 */
	.init_tpc_pg = gv11b_tpc_pg,
	.tpc_pg = gv11b_gr_pg_tpc,
};
#endif

static const struct gops_grmgr gb10b_ops_grmgr = {
#ifdef CONFIG_NVGPU_MIG
	.init_gr_manager = ga10b_grmgr_init_gr_manager,
	.remove_gr_manager = ga10b_grmgr_remove_gr_manager,
	.get_max_sys_pipes = ga10b_grmgr_get_max_sys_pipes,
	.get_mig_config_ptr = gb10b_grmgr_get_mig_config_ptr,
	.get_allowed_swizzid_size = gb10b_grmgr_get_allowed_swizzid_size,
	.get_gpc_instance_gpcgrp_id = ga10b_grmgr_get_gpc_instance_gpcgrp_id,
	.get_mig_gpu_instance_config = ga10b_grmgr_get_mig_gpu_instance_config,
	.get_gpcgrp_count = ga10b_grmgr_get_gpcgrp_count,
	.is_gpc_gfx_capable = gb10b_grmgr_is_gpc_gfx_capable,
	.set_syspipe_gfx_or_compute = gb10b_grmgr_set_syspipe_gfx_or_compute,
	.get_runlist_config_reg_off = gb10b_runlist_config_reg_off,
	.add_grce_syspipe_gfx = gb10b_grmgr_add_grce_syspipe_gfx,
	.setup_veids_mask = gb10b_grmgr_setup_veids_mask,
	.choose_mig_vpr_setting = gb10b_grmgr_choose_mig_vpr_setting,
	.setup_vpr = gb10b_grmgr_setup_vpr,
#else
	.init_gr_manager = nvgpu_init_gr_manager,
#endif
	.setup_ltc_partition = gb10b_grmgr_setup_ltc_partition,
	.setup_fbp_id_mapping = gb10b_grmgr_setup_fbp_id_mapping,
	.load_timestamp_prod = ga10b_grmgr_load_smc_arb_timestamp_prod,
	.discover_gpc_ids = ga10b_grmgr_discover_gpc_ids,
};

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
static const struct gops_mssnvlink gb10b_ops_mssnvlink = {
	.get_links = NULL,
	.init_soc_credits = NULL,
};
#endif

static const struct gops_cic_mon gb10b_ops_cic_mon = {
#ifdef CONFIG_NVGPU_FSI_ERR_INJECTION
	.reg_errinj_cb = nvgpu_cic_mon_reg_errinj_cb,
	.dereg_errinj_cb = nvgpu_cic_mon_dereg_errinj_cb,
#endif
	.init = gb10b_cic_mon_init,
	.report_err = nvgpu_cic_mon_report_err_safety_services
};

static const struct gops_gin gb10b_ops_gin = {
	.init = gb10b_gin_init,
	.get_intr_ctrl_msg = gb10b_gin_get_intr_ctrl_msg,
	.get_vector_from_intr_ctrl_msg = gb10b_gin_get_vector_from_intr_ctrl_msg,
	.read_intr_top = gb10b_gin_read_intr_top,
	.write_intr_top_en_set = gb10b_gin_write_intr_top_en_set,
	.write_intr_top_en_clear = gb10b_gin_write_intr_top_en_clear,
	.subtree_leaf_status = gb10b_gin_subtree_leaf_status,
	.subtree_leaf_clear = gb10b_gin_subtree_leaf_clear,
	.subtree_leaf_enable = gb10b_gin_subtree_leaf_enable,
	.subtree_leaf_disable = gb10b_gin_subtree_leaf_disable,
	.get_num_vectors = gb10b_gin_get_num_vectors,
	.is_mmu_fault_pending = gb10b_gin_is_mmu_fault_pending,
	.is_stall_and_eng_intr_pending = gb10b_gin_is_stall_and_eng_intr_pending,
};

static const struct gops_lrc gb10b_ops_lrc = {
#ifdef CONFIG_NVGPU_DEBUGGER
	.pri_is_lrcc_addr = gb10b_lrc_pri_is_lrcc_addr,
	.pri_is_lrcs_addr = gb10b_lrc_pri_is_lrcs_addr,
	.pri_is_lrcs_lrccs_addr = gb10b_lrc_pri_is_lrcs_lrccs_addr,
	.split_lrcc_broadcast_addr = gb10b_lrc_split_lrcc_broadcast_addr,
	.split_lrc_broadcast_addr = gb10b_lrc_split_lrc_broadcast_addr,
#endif /* CONFIG_NVGPU_DEBUGGER */
	.init_hw = gb10b_lrc_init_hw,
	.is_lrc_supported = true,
};

int gb10b_init_hal(struct gk20a *g)
{
	struct gpu_ops *gops = &g->ops;
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch,
					g->params.gpu_impl);

	gops->acr = gb10b_ops_acr;
	gops->func = gb10b_ops_func;
#ifdef CONFIG_NVGPU_DGPU
	gops->bios = gb10b_ops_bios;
#endif /* CONFIG_NVGPU_DGPU */
	gops->ecc = gb10b_ops_ecc;
	gops->ltc = gb10b_ops_ltc;
	gops->ltc.intr = gb10b_ops_ltc_intr;
#ifdef CONFIG_NVGPU_COMPRESSION
	gops->cbc = gb10b_ops_cbc;
#endif
	gops->ce = gb10b_ops_ce;
	gops->gr = gb10b_ops_gr;
	gops->gr.ecc = gb10b_ops_gr_ecc;
	gops->gr.ctxsw_prog = gb10b_ops_gr_ctxsw_prog;
	gops->gr.config = gb10b_ops_gr_config;
#ifdef CONFIG_NVGPU_FECS_TRACE
	gops->gr.fecs_trace = gb10b_ops_gr_fecs_trace;
#endif /* CONFIG_NVGPU_FECS_TRACE */
	gops->gr.setup = gb10b_ops_gr_setup;
#ifdef CONFIG_NVGPU_GRAPHICS
	gops->gr.zbc = gb10b_ops_gr_zbc;
	gops->gr.zcull = gb10b_ops_gr_zcull;
#endif /* CONFIG_NVGPU_GRAPHICS */
#ifdef CONFIG_NVGPU_DEBUGGER
	gops->gr.hwpm_map = gb10b_ops_gr_hwpm_map;
#endif
	gops->gr.init = gb10b_ops_gr_init;
	gops->gr.intr = gb10b_ops_gr_intr;
	gops->gr.falcon = gb10b_ops_gr_falcon;
	gops->gpu_class = gb10b_ops_gpu_class;
	gops->fb = gb10b_ops_fb;
	gops->xtl = gb10b_ops_xtl;
	gops->sysctrl = gb10b_ops_sysctrl;
	gops->fb.ecc = gb10b_ops_fb_ecc;
	gops->fb.intr = gb10b_ops_fb_intr;
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	gops->fb.vab = gb10b_ops_fb_vab;
#endif
	gops->nvenc = gb10b_ops_nvenc;
	gops->ofa = gb10b_ops_ofa;
	gops->nvdec = gb10b_ops_nvdec;
	gops->nvjpg = gb10b_ops_nvjpg;
	gops->cg = gb10b_ops_cg;
	gops->fifo = gb10b_ops_fifo;
	gops->engine = gb10b_ops_engine;
	gops->pbdma = gb10b_ops_pbdma;
	gops->sync = gb10b_ops_sync;
	gops->ctc_chi = gb10b_ops_ctc_chi;
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	gops->sync.syncpt = gb10b_ops_sync_syncpt;
#endif /* CONFIG_TEGRA_GK20A_NVHOST */
#ifdef CONFIG_NVGPU_SW_SEMAPHORE
	gops->sync.sema = gb10b_ops_sync_sema;
#endif
	gops->xbar = gb10b_ops_xbar;
	gops->engine_status = gb10b_ops_engine_status;
	gops->pbdma_status = gb10b_ops_pbdma_status;
	gops->ramfc = gb10b_ops_ramfc;
	gops->ramin = gb10b_ops_ramin;
	gops->runlist = gb10b_ops_runlist;
	gops->userd = gb10b_ops_userd;
	gops->channel = gb10b_ops_channel;
	gops->tsg = gb10b_ops_tsg;
	gops->usermode = gb10b_ops_usermode;
	gops->netlist = gb10b_ops_netlist;
	gops->mm = gb10b_ops_mm;
	gops->mm.mmu_fault = gb10b_ops_mm_mmu_fault;
	gops->mm.cache = gb10b_ops_mm_cache;
	gops->mm.gmmu = gb10b_ops_mm_gmmu;
	gops->therm = gb10b_ops_therm;
	gops->pmu = gb10b_ops_pmu;
#ifdef CONFIG_NVGPU_CLK_ARB
	gops->clk_arb = gb10b_ops_clk_arb;
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	gops->regops = gb10b_ops_regops;
#endif
	gops->mc = gb10b_ops_mc;
	gops->debug = gb10b_ops_debug;
#ifdef CONFIG_NVGPU_DEBUGGER
	gops->debugger = gb10b_ops_debugger;
	gops->perf = gb10b_ops_perf;
	gops->perfbuf = gb10b_ops_perfbuf;
#endif
#ifdef CONFIG_NVGPU_PROFILER
	gops->pm_reservation = gb10b_ops_pm_reservation;
	gops->profiler = gb10b_ops_profiler;
#endif
	gops->bus = gb10b_ops_bus;
	gops->ptimer = gb10b_ops_ptimer;
#if defined(CONFIG_NVGPU_CYCLESTATS)
	gops->css = gb10b_ops_css;
#endif
	gops->falcon = gb10b_ops_falcon;
	gops->gsp = gb10b_ops_gsp;
	gops->priv_ring = gb10b_ops_priv_ring;
	gops->fuse = gb10b_ops_fuse;
	gops->top = gb10b_ops_top;
#ifdef CONFIG_NVGPU_STATIC_POWERGATE
	gops->tpc_pg = gb10b_ops_tpc_pg;
#endif
	gops->grmgr = gb10b_ops_grmgr;
	gops->cic_mon = gb10b_ops_cic_mon;
	gops->gin = gb10b_ops_gin;
	gops->lrc = gb10b_ops_lrc;
	gops->chip_init_gpu_characteristics = gb10b_init_gpu_characteristics;
	gops->get_litter_value = gb10b_get_litter_value;
	gops->semaphore_wakeup = nvgpu_channel_semaphore_wakeup;

	if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)){
		nvgpu_set_errata(g, NVGPU_ERRATA_2969956, true);
	}
	nvgpu_set_errata(g, NVGPU_ERRATA_200601972, true);
	nvgpu_set_errata(g, NVGPU_ERRATA_200391931, true);
	nvgpu_set_errata(g, NVGPU_ERRATA_200677649, true);
	nvgpu_set_errata(g, NVGPU_ERRATA_3154076, true);
	/*
	 * NVGPU_ERRATA_3288192 is only applicable for auto platforms which are
	 * always virtualized, hence disable this errata on non-virtualized
	 * platforms.
	 */
	if (nvgpu_is_hypervisor_mode(g)) {
		nvgpu_set_errata(g, NVGPU_ERRATA_3288192, true);
	} else {
		nvgpu_set_errata(g, NVGPU_ERRATA_3288192, false);
	}

	nvgpu_set_errata(g, NVGPU_ERRATA_SYNCPT_INVALID_ID_0, true);
	nvgpu_set_errata(g, NVGPU_ERRATA_2557724, true);
	nvgpu_set_errata(g, NVGPU_ERRATA_3524791, true);
	nvgpu_set_errata(g, NVGPU_ERRATA_200075440, true);

	nvgpu_set_enabled(g, NVGPU_GR_USE_DMA_FOR_FW_BOOTSTRAP, false);

	/* It's expected that there is no TTU on GB10B */
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SM_TTU, false);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_ROP_IN_GPC, true);

	/* Read fuses to check if gpu needs to boot in secure/non-secure mode */
	if (gops->fuse.check_priv_security(g) != 0) {
		/* Do not boot gpu */
		return -EINVAL;
	}

	/* priv security dependent ops */
	if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		gops->gr.falcon.load_ctxsw_ucode =
			nvgpu_gr_falcon_load_secure_ctxsw_ucode;
	} else {
#ifdef CONFIG_NVGPU_LS_PMU
		/* non-secure boot */
		gops->pmu.setup_apertures =
			gb10b_pmu_ns_setup_apertures;
#endif
	}

#ifdef CONFIG_NVGPU_FECS_TRACE
	nvgpu_set_enabled(g, NVGPU_FECS_TRACE_VA, true);
	nvgpu_set_enabled(g, NVGPU_FECS_TRACE_FEATURE_CONTROL, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_FECS_CTXSW_TRACE, true);
#endif

#ifdef CONFIG_NVGPU_PROFILER
	nvgpu_set_enabled(g, NVGPU_SUPPORT_PROFILER_V2_DEVICE, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_PROFILER_V2_CONTEXT, false);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_CWD_HES, true);
#endif

	nvgpu_set_enabled(g, NVGPU_SUPPORT_MULTIPLE_WPR, false);
#ifdef CONFIG_NVGPU_GRAPHICS
	nvgpu_set_enabled(g, NVGPU_SUPPORT_ZBC_STENCIL, true);
#endif
#ifdef CONFIG_NVGPU_GFXP
	nvgpu_set_enabled(g, NVGPU_SUPPORT_PREEMPTION_GFXP, true);
#endif
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SET_CTX_MMU_DEBUG_MODE, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_MULTIMEDIA, true);
#ifdef CONFIG_NVGPU_PROFILER
	nvgpu_set_enabled(g, NVGPU_SUPPORT_VAB_ENABLED, false);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SMPC_GLOBAL_MODE, true);
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	nvgpu_set_enabled(g, NVGPU_L2_MAX_WAYS_EVICT_LAST_ENABLED, false);
	nvgpu_set_enabled(g, NVGPU_SCHED_EXIT_WAIT_FOR_ERRBAR_SUPPORTED, true);
#endif

	if (g->ops.pmu.is_pmu_supported != NULL) {
		if (g->ops.pmu.is_pmu_supported(g)) {
			nvgpu_set_enabled(g, NVGPU_SUPPORT_PMU_RTOS_FBQ, true);
			nvgpu_set_enabled(g, NVGPU_SUPPORT_PMU_SUPER_SURFACE, true);
		}
	}

#ifdef CONFIG_NVGPU_GSP_SCHEDULER
	nvgpu_set_enabled(g, NVGPU_SUPPORT_GSP_SCHED, false);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_GSP_STEST, false);
#endif

	/*
	 * L3 allocation is not supported in t264.
	 */
	nvgpu_set_enabled(g, NVGPU_DISABLE_L3_SUPPORT, true);

	/*
	 * gb10b supports non-pasid ats
	 */
	nvgpu_set_enabled(g, NVGPU_MM_SUPPORT_NONPASID_ATS, true);

	nvgpu_set_enabled(g, NVGPU_MM_BYPASSES_IOMMU, false);

	/*
	 * Disable ECC sysfs/stats for NVGPU_MSUBMIT.
	 */
	nvgpu_set_enabled(g, NVGPU_DISABLE_ECC_STATS, false);

	/*
	 * ecc scrub init can get called before
	 * gb10b_init_gpu_characteristics call.
	 */
	g->ops.gr.ecc.detect(g);

#ifndef CONFIG_NVGPU_BUILD_CONFIGURATION_IS_SAFETY
	/*
	 * To achieve permanent fault coverage, the CTAs launched by each kernel
	 * in the mission and redundant contexts must execute on different
	 * hardware resources. This feature proposes modifications in the
	 * software to modify the virtual SM id to TPC mapping across the
	 * mission and redundant contexts.
	 *
	 * The virtual SM identifier to TPC mapping is done by the nvgpu
	 * when setting up the golden context. Once the table with this mapping
	 * is initialized, it is used by all subsequent contexts that are
	 * created. The proposal is for setting up the virtual SM identifier
	 * to TPC mapping on a per-context basis and initializing this
	 * virtual SM identifier to TPC mapping differently for the mission and
	 * redundant contexts.
	 *
	 * The recommendation for the redundant setting is to offset the
	 * assignment by 1 (TPC). This will ensure both GPC and TPC diversity.
	 * The SM and Quadrant diversity will happen naturally.
	 *
	 * For kernels with few CTAs, the diversity is guaranteed to be 100%.
	 * In case of completely random CTA allocation, e.g. large number of
	 * CTAs in the waiting queue, the diversity is 1 - 1/#SM,
	 * or 87.5% for GV11B.
	 */
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SM_DIVERSITY, true);
	g->max_sm_diversity_config_count =
		NVGPU_MAX_SM_DIVERSITY_CONFIG_COUNT;
#else
	g->max_sm_diversity_config_count =
		NVGPU_DEFAULT_SM_DIVERSITY_CONFIG_COUNT;
#endif

#ifdef CONFIG_NVGPU_COMPRESSION
	if (nvgpu_is_hypervisor_mode(g)) {
		nvgpu_set_enabled(g, NVGPU_SUPPORT_COMPRESSION, false);
	} else {
		nvgpu_set_enabled(g, NVGPU_SUPPORT_COMPRESSION, true);
	}

	nvgpu_set_enabled(g, NVGPU_SUPPORT_COMPRESSION, true);
	g->cbc_use_raw_mode = true;

	/* PLC is not supported in GB10B */
	nvgpu_set_enabled(g, NVGPU_SUPPORT_POST_L2_COMPRESSION, false);
	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_COMPRESSION)) {
		gops->cbc.init = NULL;
		gops->cbc.ctrl = NULL;
		gops->cbc.alloc_comptags = NULL;
	}
#endif

#ifdef CONFIG_NVGPU_CLK_ARB
	/* Enable clock arbitration support for silicon */
	if (nvgpu_platform_is_silicon(g)) {
		nvgpu_set_enabled(g, NVGPU_CLK_ARB_ENABLED, true);
	} else {
		nvgpu_set_enabled(g, NVGPU_CLK_ARB_ENABLED, false);
	}
#endif

#ifdef CONFIG_NVGPU_SIM
	/* SIM specific overrides */
	nvgpu_init_sim_support_ga10b(g);
#endif

#ifdef CONFIG_NVGPU_MSUBMIT
	/* Disable static power gating for NVGPU_MSUBMIT. */
	nvgpu_set_enabled(g, NVGPU_DISABLE_STATIC_POWERGATE, true);
#endif

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	gops->mssnvlink = gb10b_ops_mssnvlink;
#endif
	/* There is no emulate mode for gb10b */
	nvgpu_set_enabled(g, NVGPU_SUPPORT_EMULATE_MODE, false);
	#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
	/*
	 * enabled kmd sheduling worker thread
	 */
	nvgpu_set_enabled(g, NVGPU_SUPPORT_KMD_SCHEDULING_WORKER_THREAD, true);
#endif

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SEMA_BASED_GPFIFO_GET, true);
#endif
	nvgpu_set_enabled(g, NVGPU_SUPPORT_PES_FS, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_GPU_MMIO, true);

	/* ToDo: need to update it for gb10b_next */
	g->name = "gb10b";

	if (ver == NVGPU_GPUID_GB10B_NEXT) {
		gops->priv_ring.init_fbp_physical_logical_id_map = NULL;
	}

	return 0;
}
