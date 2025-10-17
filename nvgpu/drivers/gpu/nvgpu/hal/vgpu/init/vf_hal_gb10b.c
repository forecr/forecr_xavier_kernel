// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "hal/mm/mm_gm20b.h"
#include "hal/mm/mm_gp10b.h"
#include "hal/mm/mm_gv11b.h"
#include "hal/mm/mm_ga10b.h"
#include "hal/mm/gmmu/gmmu_gk20a.h"
#include "hal/mm/gmmu/gmmu_gm20b.h"
#include "hal/mm/gmmu/gmmu_gv11b.h"
#include "hal/mm/mmu_fault/mmu_fault_gv11b.h"
#include "hal/mm/mmu_fault/mmu_fault_gb10b.h"
#include "hal/mm/gmmu/gmmu_ga10b.h"
#include "hal/mm/gmmu/gmmu_gb10b.h"
#include "hal/class/class_gb10b.h"
#include "hal/fifo/preempt_gv11b.h"
#include "hal/fifo/engines_gp10b.h"
#include "hal/fifo/engines_gv11b.h"
#include "hal/fifo/fifo_ga10b.h"
#include "hal/fifo/fifo_intr_ga10b.h"
#include "hal/fifo/pbdma_gm20b.h"
#include "hal/fifo/pbdma_gp10b.h"
#include "hal/fifo/pbdma_gv11b.h"
#include "hal/fifo/pbdma_ga10b.h"
#include "hal/fifo/pbdma_gb10b.h"
#include "hal/fifo/ramfc_gb10b.h"
#include "hal/fifo/ramin_gk20a.h"
#include "hal/fifo/ramin_gm20b.h"
#include "hal/fifo/ramin_gv11b.h"
#include "hal/fifo/ramin_ga10b.h"
#include "hal/fifo/ramin_gb10b.h"
#include "hal/fifo/runlist_ram_gv11b.h"
#include "hal/fifo/runlist_ram_gb10b.h"
#include "hal/fifo/runlist_fifo_ga10b.h"
#include "hal/fifo/runlist_fifo_gb10b.h"
#include "hal/fifo/tsg_ga10b.h"
#include "hal/fifo/tsg_gv11b.h"
#include "hal/fifo/userd_gk20a.h"
#include "hal/fifo/userd_gv11b.h"
#include "hal/fifo/userd_gb10b.h"
#include "hal/fifo/usermode_tu104.h"
#include "hal/fuse/fuse_ga10b.h"
#include "hal/gin/gin_gb10b.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_ga10b.h"
#include "hal/gr/gr/gr_ga10b.h"
#include "hal/gr/gr/gr_gb10b.h"
#include "hal/gr/init/gr_init_ga10b.h"
#include "hal/gr/intr/gr_intr_gb10b.h"
#include <nvgpu/gr/fecs_trace.h>
#ifdef CONFIG_NVGPU_FECS_TRACE
#include "hal/gr/fecs_trace/fecs_trace_gm20b.h"
#include "hal/gr/fecs_trace/fecs_trace_gv11b.h"
#include "hal/gr/fecs_trace/fecs_trace_gb10b.h"
#endif
#include "hal/init/hal_gb10b_litter.h"
#include "hal/netlist/netlist_ga10b.h"
#include "hal/ptimer/ptimer_gv11b.h"
#ifdef CONFIG_NVGPU_DEBUGGER
#include "hal/regops/regops_gb10b.h"
#include "hal/regops/allowlist_gb10b.h"
#endif
#include "hal/perf/perf_ga10b.h"
#include "hal/perf/perf_gb10b.h"
#include "hal/gr/fecs_trace/fecs_trace_gv11b.h"
#ifdef CONFIG_NVGPU_GRAPHICS
#include "hal/gr/zbc/zbc_gb10b.h"
#endif
#include "hal/gr/hwpm_map/hwpm_map_gv100.h"
#include "hal/gr/hwpm_map/hwpm_map_gb10b.h"
#include "hal/ltc/ltc_gm20b.h"
#include "hal/ltc/ltc_gb10b.h"
#include "hal/fb/fb_gm20b.h"
#include "hal/fb/fb_gp10b.h"
#include "hal/fb/fb_gb10b.h"
#include "hal/fb/fb_mmu_fault_gb10b.h"
#include "hal/fb/intr/fb_intr_ga10b.h"
#include "hal/fb/fb_mmu_fault_gv11b.h"
#include "hal/func/func_gb10b.h"
#include "hal/gr/init/gr_init_gm20b.h"
#include "hal/gr/init/gr_init_gp10b.h"
#include "hal/gr/init/gr_init_gv11b.h"
#include "hal/gr/init/gr_init_tu104.h"
#include "hal/gr/init/gr_init_gb10b.h"
#include "hal/gr/intr/gr_intr_gv11b.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_gm20b.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_gp10b.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_gv11b.h"
#include "hal/gr/ctxsw_prog/ctxsw_prog_gb10b.h"
#include "hal/gr/gr/gr_gk20a.h"
#include "hal/gr/gr/gr_gm20b.h"
#include "hal/gr/gr/gr_gv11b.h"
#include "hal/sync/syncpt_cmdbuf_gv11b.h"
#include "hal/sync/sema_cmdbuf_gv11b.h"
#include "hal/fifo/channel_ga10b.h"

#include "hal/vgpu/fb/fb_gb10b_vf.h"
#include "hal/vgpu/fb/intr/fb_intr_gb10b_vf.h"
#include "hal/vgpu/fifo/fifo_gv11b_vgpu.h"
#include "hal/vgpu/init/vf_hal_gb10b.h"
#include "hal/vgpu/fifo/pbdma_gb10b_vgpu.h"

#include "common/clk_arb/clk_arb_gp10b.h"

#include <nvgpu/errata.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/gr_intr.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/error_notifier.h>
#include <nvgpu/acr.h>
#include <nvgpu/ce.h>
#include <nvgpu/ce_app.h>
#include <nvgpu/runlist.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/clk_arb.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/perfbuf.h>
#include <nvgpu/soc.h>

#include "common/vgpu/init/init_vgpu.h"
#include "common/vgpu/fb/fb_vgpu.h"
#include "common/vgpu/top/top_vgpu.h"
#include "common/vgpu/fifo/fifo_vgpu.h"
#include "common/vgpu/fifo/channel_vgpu.h"
#include "common/vgpu/fifo/channel_vf.h"
#include "common/vgpu/fifo/tsg_vgpu.h"
#include "common/vgpu/fifo/preempt_vgpu.h"
#include "common/vgpu/fifo/runlist_vf.h"
#include "common/vgpu/fifo/userd_vgpu.h"
#include "common/vgpu/gr/gr_vgpu.h"
#include "common/vgpu/gr/gr_vf.h"
#include "common/vgpu/gr/obj_ctx_vf.h"
#include "common/vgpu/ltc/ltc_vgpu.h"
#include "common/vgpu/mm/mm_vgpu.h"
#include "common/vgpu/debugger_vgpu.h"
#include "common/vgpu/debugger_vf.h"
#include "common/vgpu/pm_reservation_vgpu.h"
#include "common/vgpu/perf/perf_vgpu.h"
#include "common/vgpu/gr/fecs_trace_vgpu.h"
#include "common/vgpu/perf/cyclestats_snapshot_vgpu.h"
#include "common/vgpu/ptimer/ptimer_vgpu.h"
#include "common/vgpu/profiler/profiler_vgpu.h"
#include "common/vgpu/rc/rc_vf.h"
#include "common/vgpu/profiler/profiler_vf.h"
#include "common/vgpu/perf/perf_vf.h"

#include <nvgpu/debugger.h>
#include <nvgpu/gr/setup.h>
#include <nvgpu/vgpu/ce_vgpu.h>

static int vf_gb10b_init_gpu_characteristics(struct gk20a *g)
{
	int err;

	nvgpu_log_fn(g, " ");

	err = vgpu_init_gpu_characteristics(g);
	if (err != 0) {
		nvgpu_err(g, "failed to init GPU characteristics");
		return err;
	}

	nvgpu_set_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS, true);
	nvgpu_set_enabled(g, NVGPU_USE_COHERENT_SYSMEM, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_IO_COHERENCE, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SPARSE_ALLOCS, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_MAP_BUFFER_BATCH, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_MAPPING_MODIFY, true);
	if (nvgpu_has_syncpoints(g)) {
		nvgpu_set_enabled(g, NVGPU_SUPPORT_SYNCPOINT_ADDRESS, true);
		nvgpu_set_enabled(g, NVGPU_SUPPORT_USER_SYNCPOINT, true);
	}
	nvgpu_set_enabled(g, NVGPU_SUPPORT_USERMODE_SUBMIT, true);
#ifdef CONFIG_NVGPU_GRAPHICS
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SCG, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_ZBC_STENCIL, true);
#endif
	nvgpu_set_enabled(g, NVGPU_SUPPORT_PREEMPTION_GFXP, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SET_CTX_MMU_DEBUG_MODE, true);
#ifdef CONFIG_NVGPU_FECS_TRACE
	nvgpu_set_enabled(g, NVGPU_FECS_TRACE_VA, true);
	nvgpu_set_enabled(g, NVGPU_FECS_TRACE_FEATURE_CONTROL, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_FECS_CTXSW_TRACE, true);
#endif
#ifdef CONFIG_NVGPU_PROFILER
	nvgpu_set_enabled(g, NVGPU_SUPPORT_PROFILER_V2_DEVICE, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_PROFILER_V2_CONTEXT, false);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SMPC_GLOBAL_MODE, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_VAB_ENABLED, false);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_CWD_HES, true);
#endif

	return 0;
}

static const struct gops_acr vf_gb10b_ops_acr = {
	.acr_init = nvgpu_acr_init,
	.acr_construct_execute = nvgpu_acr_construct_execute,
};

#ifdef CONFIG_NVGPU_DGPU
static const struct gops_bios vf_gb10b_ops_bios = {
	/* This is specific to dGPU. Disable it for iGPU. */
	.bios_sw_init = NULL,
};
#endif

static const struct gops_ltc_intr vf_gb10b_ops_ltc_intr = {
	.configure = NULL,
	.isr = NULL,
	.en_illegal_compstat = NULL,
};

static const struct gops_ltc vf_gb10b_ops_ltc = {
	.init_ltc_support = nvgpu_init_ltc_support,
	.ltc_remove_support = nvgpu_ltc_remove_support,
	.determine_L2_size_bytes = vgpu_determine_L2_size_bytes,
	.init_fs_state = vgpu_ltc_init_fs_state,
	.flush = NULL,
#if defined(CONFIG_NVGPU_NON_FUSA) || defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT)
	.set_enabled = NULL,
#endif
#ifdef CONFIG_NVGPU_GRAPHICS
	.set_zbc_s_entry = NULL,
	.set_zbc_color_entry = NULL,
	.set_zbc_depth_entry = NULL,
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	.pri_is_ltc_addr = gm20b_ltc_pri_is_ltc_addr,
	.is_pltcg_ltcs_addr = gm20b_ltc_is_pltcg_ltcs_addr,
	.is_ltcs_ltss_addr = gm20b_ltc_is_ltcs_ltss_addr,
	.is_ltcn_ltss_addr = gm20b_ltc_is_ltcn_ltss_addr,
	.split_lts_broadcast_addr = gm20b_ltc_split_lts_broadcast_addr,
	.split_ltc_broadcast_addr = gm20b_ltc_split_ltc_broadcast_addr,
	.set_l2_max_ways_evict_last = NULL,
	.get_l2_max_ways_evict_last = NULL,
	.set_l2_sector_promotion = gb10b_set_l2_sector_promotion,
#endif /* CONFIG_NVGPU_DEBUGGER */
};

#ifdef CONFIG_NVGPU_COMPRESSION
static const struct gops_cbc vf_gb10b_ops_cbc = {
	.cbc_init_support = nvgpu_cbc_init_support,
	.cbc_remove_support = nvgpu_cbc_remove_support,
	.init = NULL,
	.ctrl = NULL,
	.alloc_comptags = NULL,
};
#endif

static const struct gops_ce vf_gb10b_ops_ce = {
	.ce_init_support = NULL,
	/* Sysmem Fast SCRUB */
#ifdef CONFIG_NVGPU_COMPRESSION_RAW
	.ce_app_init_support = nvgpu_ce_app_init_support,
	.ce_app_suspend = nvgpu_ce_app_suspend,
	.ce_app_destroy = nvgpu_ce_app_destroy,
#endif
	.isr_stall = NULL,
#ifdef CONFIG_NVGPU_NONSTALL_INTR
	.isr_nonstall = NULL,
#endif
	.get_num_pce = vgpu_ce_get_num_pce,
};

static const struct gops_gr_ctxsw_prog vf_gb10b_ops_gr_ctxsw_prog = {
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
#ifdef CONFIG_NVGPU_GRAPHICS
	.set_zcull_ptr = gv11b_ctxsw_prog_set_zcull_ptr,
	.set_zcull = gm20b_ctxsw_prog_set_zcull,
	.set_zcull_mode_no_ctxsw = gm20b_ctxsw_prog_set_zcull_mode_no_ctxsw,
	.is_zcull_mode_separate_buffer = gm20b_ctxsw_prog_is_zcull_mode_separate_buffer,
	.set_graphics_preemption_mode_gfxp = gp10b_ctxsw_prog_set_graphics_preemption_mode_gfxp,
	.set_pmu_options_boost_clock_frequencies = NULL,
	.set_full_preemption_ptr = gv11b_ctxsw_prog_set_full_preemption_ptr,
	.set_full_preemption_ptr_veid0 = gv11b_ctxsw_prog_set_full_preemption_ptr_veid0,
#endif /* CONFIG_NVGPU_GRAPHICS */
#ifdef CONFIG_NVGPU_CILP
	.set_compute_preemption_mode_cilp = gp10b_ctxsw_prog_set_compute_preemption_mode_cilp,
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
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
	.set_cde_enabled = gm20b_ctxsw_prog_set_cde_enabled,
	.set_pc_sampling = gm20b_ctxsw_prog_set_pc_sampling,
	.check_main_image_header_magic = ga10b_ctxsw_prog_check_main_image_header_magic,
	.check_local_header_magic = ga10b_ctxsw_prog_check_local_header_magic,
	.get_num_gpcs = gm20b_ctxsw_prog_get_num_gpcs,
	.get_num_tpcs = gm20b_ctxsw_prog_get_num_tpcs,
	.get_extended_buffer_size_offset = gm20b_ctxsw_prog_get_extended_buffer_size_offset,
	.get_ppc_info = gm20b_ctxsw_prog_get_ppc_info,
	.get_local_priv_register_ctl_offset = gm20b_ctxsw_prog_get_local_priv_register_ctl_offset,
	.hw_get_pm_gpc_gnic_stride = gb10b_ctxsw_prog_hw_get_pm_gpc_gnic_stride,
#endif /* CONFIG_NVGPU_DEBUGGER */
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
	.hw_get_perf_counter_register_stride = gv11b_ctxsw_prog_hw_get_perf_counter_register_stride,
	.set_context_buffer_ptr = gv11b_ctxsw_prog_set_context_buffer_ptr,
	.set_type_per_veid_header = gv11b_ctxsw_prog_set_type_per_veid_header,
#ifdef CONFIG_DEBUG_FS
	.dump_ctxsw_stats = NULL,
#endif
};

static const struct gops_gr_config vf_gb10b_ops_gr_config = {
	.get_gpc_mask = vgpu_gr_get_gpc_mask,
	.get_gpc_tpc_mask = vgpu_gr_get_gpc_tpc_mask,
	.init_sm_id_table = vgpu_gr_init_sm_id_table,
	.get_gpc_rop_mask = vgpu_gr_config_get_gpc_rop_mask,
};

static const struct gops_gr_setup vf_gb10b_ops_gr_setup = {
	.init_golden_image = vgpu_vf_gr_obj_ctx_init_golden_image,
	.alloc_obj_ctx = nvgpu_gr_setup_alloc_obj_ctx,
	.free_gr_ctx = nvgpu_gr_setup_free_gr_ctx,
	.free_subctx = nvgpu_gr_setup_free_subctx,
#ifdef CONFIG_NVGPU_GRAPHICS
	.bind_ctxsw_zcull = nvgpu_gr_setup_bind_ctxsw_zcull,
#endif /* CONFIG_NVGPU_GRAPHICS */
	.set_preemption_mode = nvgpu_gr_setup_set_preemption_mode,
};

#ifdef CONFIG_NVGPU_GRAPHICS
static const struct gops_gr_zbc vf_gb10b_ops_gr_zbc = {
	.add_color = NULL,
	.load_default_sw_table = NULL,
	.add_depth = NULL,
	.set_table = vgpu_gr_add_zbc,
	.query_table = vgpu_gr_query_zbc,
	.add_stencil = gb10b_gr_zbc_add_stencil,
	.get_gpcs_swdx_dss_zbc_c_format_reg = NULL,
	.get_gpcs_swdx_dss_zbc_z_format_reg = NULL,
};
#endif

#ifdef CONFIG_NVGPU_GRAPHICS
static const struct gops_gr_zcull vf_gb10b_ops_gr_zcull = {
	.get_zcull_info = vgpu_gr_get_zcull_info,
	.program_zcull_mapping = NULL,
};
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
static const struct gops_gr_hwpm_map vf_gb10b_ops_gr_hwpm_map = {
	.align_regs_perf_pma = gv100_gr_hwpm_map_align_regs_perf_pma,
	.add_ctxsw_buffer_gpc_perf_entries_to_map = gb10b_hwpm_map_add_gpc_perf_entries_to_map,
	.add_ctxsw_buffer_pm_ucgpc_entries_to_map = gb10b_hwpm_map_add_pm_ucgpc_entries_to_map,
	.add_ctxsw_buffer_lrcc_entries_to_map = gb10b_hwpm_map_add_lrcc_entries_to_map,
};
#endif

static const struct gops_gr_falcon vf_gb10b_ops_gr_falcon = {
	.init_ctx_state = vgpu_gr_init_ctx_state,
	.load_ctxsw_ucode = NULL,
};

#ifdef CONFIG_NVGPU_FECS_TRACE
static const struct gops_gr_fecs_trace vf_gb10b_ops_gr_fecs_trace = {
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

static const struct gops_gr_init vf_gb10b_ops_gr_init = {
	.get_no_of_sm = nvgpu_gr_get_no_of_sm,
	.get_nonpes_aware_tpc = gv11b_gr_init_get_nonpes_aware_tpc,
	.get_rtv_cb_size = tu104_gr_init_get_rtv_cb_size,
	.commit_rtv_cb = gb10b_gr_init_commit_rtv_cb,
	.get_bundle_cb_default_size = gv11b_gr_init_get_bundle_cb_default_size,
	.get_min_gpm_fifo_depth = gb10b_gr_init_get_min_gpm_fifo_depth,
	.get_bundle_cb_token_limit = gb10b_gr_init_get_bundle_cb_token_limit,
	.get_attrib_cb_default_size = gb10b_gr_init_get_attrib_cb_default_size,
	.get_alpha_cb_default_size = gv11b_gr_init_get_alpha_cb_default_size,
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
	.get_ctx_attrib_cb_size = gp10b_gr_init_get_ctx_attrib_cb_size,
	.commit_cbes_reserve = gb10b_gr_init_commit_cbes_reserve,
	.get_max_subctx_count = gv11b_gr_init_get_max_subctx_count,
	.get_patch_slots = gv11b_gr_init_get_patch_slots,
	.detect_sm_arch = vgpu_gr_detect_sm_arch,
	.get_supported__preemption_modes = gp10b_gr_init_get_supported_preemption_modes,
	.get_default_preemption_modes = gp10b_gr_init_get_default_preemption_modes,
#ifdef CONFIG_NVGPU_GRAPHICS
	.get_attrib_cb_gfxp_default_size = gb10b_gr_init_get_attrib_cb_gfxp_size,
	.get_attrib_cb_gfxp_size = gb10b_gr_init_get_attrib_cb_gfxp_size,
	.get_gfxp_rtv_cb_size = tu104_gr_init_get_gfxp_rtv_cb_size,
	.commit_gfxp_rtv_cb = gb10b_gr_init_commit_gfxp_rtv_cb,
	.get_ctx_spill_size = gb10b_gr_init_get_ctx_spill_size,
	.get_ctx_pagepool_size = gp10b_gr_init_get_ctx_pagepool_size,
	.get_ctx_betacb_size = gb10b_gr_init_get_ctx_betacb_size,
	.commit_ctxsw_spill = gv11b_gr_init_commit_ctxsw_spill,
	.gfxp_wfi_timeout = gv11b_gr_init_commit_gfxp_wfi_timeout,
#endif /* CONFIG_NVGPU_GRAPHICS */
#ifdef CONFIG_NVGPU_SET_FALCON_ACCESS_MAP
	.get_access_map = gb10b_gr_init_get_access_map,
#endif
};

static const struct gops_gr_intr vf_gb10b_ops_gr_intr = {
	.handle_gcc_exception = NULL,
	.handle_gpc_gpcmmu_exception = NULL,
	.handle_gpc_gpccs_exception = NULL,
	.get_tpc_exception = gb10b_gr_intr_get_tpc_exception,
	.handle_tpc_mpc_exception = NULL,
	.handle_tex_exception = NULL,
	.flush_channel_tlb = nvgpu_gr_intr_flush_channel_tlb,
	.get_sm_no_lock_down_hww_global_esr_mask = gv11b_gr_intr_get_sm_no_lock_down_hww_global_esr_mask,
#ifdef CONFIG_NVGPU_DEBUGGER
	.tpc_enabled_exceptions = vgpu_gr_gk20a_tpc_enabled_exceptions,
	.handle_cilp_save_completed = gv11b_gr_intr_handle_cilp_save_completed,
#endif
};

static const struct gops_gr vf_gb10b_ops_gr = {
	.gr_init_support = nvgpu_gr_init_support,
	.gr_suspend = nvgpu_gr_suspend,
	.vab_reserve = NULL,
	.vab_configure = NULL,
#ifdef CONFIG_NVGPU_DEBUGGER
	.set_alpha_circular_buffer_size = NULL,
	.set_circular_buffer_size = NULL,
	.get_sm_dsm_perf_regs = gv11b_gr_get_sm_dsm_perf_regs,
	.get_sm_dsm_perf_ctrl_regs = gv11b_gr_get_sm_dsm_perf_ctrl_regs,
	.get_sm_dbgr_ctrl_base = gb10b_get_sm_dbgr_ctrl_base,
	.get_gpcs_pri_mmu_debug_ctrl_reg = gb10b_get_gpcs_pri_mmu_debug_ctrl_reg,
#ifdef CONFIG_NVGPU_TEGRA_FUSE
	.set_gpc_tpc_mask = NULL,
#endif
	.dump_gr_regs = NULL,
	.update_pc_sampling = vgpu_gr_update_pc_sampling,
	.init_sm_dsm_reg_info = gv11b_gr_init_sm_dsm_reg_info,
	.init_cyclestats = vgpu_gr_init_cyclestats,
	.set_sm_debug_mode = gv11b_gr_set_sm_debug_mode,
	.bpt_reg_info = NULL,
	.update_smpc_ctxsw_mode = gr_gk20a_update_smpc_ctxsw_mode,
	.update_hwpm_ctxsw_mode = gr_gk20a_update_hwpm_ctxsw_mode,
	.clear_sm_error_state = vgpu_vf_gr_clear_sm_error_state,
	.suspend_contexts = vgpu_vf_gr_suspend_contexts,
	.resume_contexts = vgpu_vf_gr_resume_contexts,
	.trigger_suspend = NULL,
	.wait_for_pause = gr_gk20a_wait_for_pause,
	.resume_from_pause = NULL,
	.clear_sm_errors = NULL,
	.sm_debugger_attached = NULL,
	.suspend_single_sm = NULL,
	.suspend_all_sms = NULL,
	.resume_single_sm = NULL,
	.resume_all_sms = NULL,
	.lock_down_sm = NULL,
	.wait_for_sm_lock_down = NULL,
	.init_ovr_sm_dsm_perf = gv11b_gr_init_ovr_sm_dsm_perf,
	.get_ovr_perf_regs = gv11b_gr_get_ovr_perf_regs,
	.set_boosted_ctx = NULL,
	.pre_process_sm_exception = NULL,
	.set_bes_crop_debug3 = NULL,
	.set_bes_crop_debug4 = NULL,
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
	.get_offset_in_gpccs_segment = gr_gk20a_get_offset_in_gpccs_segment,
	.set_debug_mode = gm20b_gr_set_debug_mode,
	.set_mmu_debug_mode = gm20b_gr_set_mmu_debug_mode,
	.set_sched_wait_for_errbar = gb10b_gr_set_sched_wait_for_errbar,
	.check_warp_esr_error = ga10b_gr_check_warp_esr_error,
#endif
	.set_mmu_nack_pending = vgpu_vf_gr_set_mmu_nack_pending,
};

static const struct gops_nvenc vf_gb10b_ops_nvenc = {
	.multimedia_alloc_ctx = nvgpu_multimedia_setup_ctx,
	.multimedia_free_ctx = nvgpu_multimedia_free_ctx,
};

static const struct gops_ofa vf_gb10b_ops_ofa = {
	.multimedia_alloc_ctx = nvgpu_multimedia_setup_ctx,
	.multimedia_free_ctx = nvgpu_multimedia_free_ctx,
};

static const struct gops_nvdec vf_gb10b_ops_nvdec = {
	.multimedia_alloc_ctx = nvgpu_multimedia_setup_ctx,
	.multimedia_free_ctx = nvgpu_multimedia_free_ctx,
};

static const struct gops_nvjpg vf_gb10b_ops_nvjpg = {
	.multimedia_alloc_ctx = nvgpu_multimedia_setup_ctx,
	.multimedia_free_ctx = nvgpu_multimedia_free_ctx,
};

static const struct gops_class vf_gb10b_ops_gpu_class = {
	.is_valid = gb10b_class_is_valid,
	.is_valid_compute = gb10b_class_is_valid_compute,
	.is_valid_gfx = gb10b_class_is_valid_gfx,
	.is_valid_multimedia = gb10b_class_is_valid_multimedia,
	.is_valid_nvenc = gb10b_class_is_valid_nvenc,
	.is_valid_ofa = gb10b_class_is_valid_ofa,
	.is_valid_nvdec = gb10b_class_is_valid_nvdec,
	.is_valid_nvjpg = gb10b_class_is_valid_nvjpg,
};

static const struct gops_fb_intr vf_gb10b_ops_fb_intr = {
	.enable = vf_gb10b_fb_intr_enable,
	.disable = vf_gb10b_fb_intr_disable,
	.isr = ga10b_fb_intr_isr,
	.is_mmu_fault_pending = NULL,
};

static const struct gops_fb vf_gb10b_ops_fb = {
	.init_hw = vf_gb10b_fb_init_hw,
	.init_fs_state = NULL,
	.set_mmu_page_size = NULL,
#ifdef CONFIG_NVGPU_COMPRESSION
	.set_use_full_comp_tag_line = NULL,
	.compression_page_size = gb10b_fb_compression_page_size,
	.compressible_page_size = gb10b_fb_compressible_page_size,
	.compression_align_mask = gb10b_fb_compression_align_mask,
#endif
#ifdef CONFIG_NVGPU_VPR
	.vpr_info_fetch = NULL,
	.dump_vpr_info = NULL,
#endif
	.dump_wpr_info = NULL,
	.read_wpr_info = NULL,
#ifdef CONFIG_NVGPU_DEBUGGER
	.is_debug_mode_enabled = NULL,
	.set_debug_mode = vgpu_mm_mmu_set_debug_mode,
	.set_mmu_debug_mode = vgpu_fb_set_mmu_debug_mode,
#endif
	.tlb_invalidate = gb10b_func_tlb_invalidate,
	.tlb_flush = gb10b_func_tlb_flush,
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
};

static const struct gops_cg vf_gb10b_ops_cg = {
	.slcg_bus_load_gating_prod = NULL,
	.slcg_ce2_load_gating_prod = NULL,
	.slcg_chiplet_load_gating_prod = NULL,
	.slcg_fb_load_gating_prod = NULL,
	.slcg_fifo_load_gating_prod = NULL,
	.slcg_gr_load_gating_prod = NULL,
	.slcg_ltc_load_gating_prod = NULL,
	.slcg_perf_load_gating_prod = NULL,
	.slcg_priring_load_gating_prod = NULL,
	.slcg_pmu_load_gating_prod = NULL,
	.slcg_therm_load_gating_prod = NULL,
	.slcg_xbar_load_gating_prod = NULL,
	.blcg_bus_load_gating_prod = NULL,
	.blcg_ce_load_gating_prod = NULL,
	.blcg_fb_load_gating_prod = NULL,
	.blcg_fifo_load_gating_prod = NULL,
	.blcg_gr_load_gating_prod = NULL,
	.blcg_ltc_load_gating_prod = NULL,
	.blcg_pmu_load_gating_prod = NULL,
	.blcg_xbar_load_gating_prod = NULL,
};

static const struct gops_fifo vf_gb10b_ops_fifo = {
	.fifo_init_support = nvgpu_fifo_init_support,
	.fifo_suspend = nvgpu_fifo_suspend,
	.init_fifo_setup_hw = vgpu_gv11b_init_fifo_setup_hw,
	.preempt_channel = vgpu_fifo_preempt_channel,
	.preempt_tsg = vgpu_fifo_preempt_tsg,
	.is_preempt_pending = gv11b_fifo_is_preempt_pending,
	.reset_enable_hw = NULL,
#ifdef CONFIG_NVGPU_RECOVERY
	.recover = vgpu_vf_fifo_recover,
#endif
	.setup_sw = vgpu_fifo_setup_sw,
	.cleanup_sw = vgpu_fifo_cleanup_sw,
	.set_sm_exception_type_mask = vgpu_set_sm_exception_type_mask,
	.intr_0_enable = NULL,
	.intr_1_enable = NULL,
	.intr_0_isr = NULL,
	.intr_1_isr = NULL,
	.handle_sched_error = NULL,
	.handle_ctxsw_timeout = NULL,
	.ctxsw_timeout_enable = NULL,
	.trigger_mmu_fault = NULL,
	.get_mmu_fault_info = NULL,
	.get_mmu_fault_desc = NULL,
	.get_mmu_fault_client_desc = NULL,
	.get_mmu_fault_gpc_desc = NULL,
	.mmu_fault_id_to_pbdma_id = ga10b_fifo_mmu_fault_id_to_pbdma_id,
};

static const struct gops_engine vf_gb10b_ops_engine = {
	.is_fault_engine_subid_gpc = gv11b_is_fault_engine_subid_gpc,
	.init_ce_info = gp10b_engine_init_ce_info,
};

static const struct gops_pbdma vf_gb10b_ops_pbdma = {
	.setup_sw = NULL,
	.cleanup_sw = NULL,
	.setup_hw = NULL,
	.intr_enable = NULL,
	.acquire_val = gm20b_pbdma_acquire_val,
	.get_signature = gp10b_pbdma_get_signature,
	.dump_status = NULL,
	.handle_intr_0 = NULL,
	.handle_intr_1 = gv11b_pbdma_handle_intr_1,
	.handle_intr = gm20b_pbdma_handle_intr,
	.read_data = NULL,
	.reset_header = NULL,
	.device_fatal_0_intr_descs = NULL,
	.channel_fatal_0_intr_descs = NULL,
	.restartable_0_intr_descs = NULL,
	.format_gpfifo_entry = gm20b_pbdma_format_gpfifo_entry,
	.get_gp_base = gm20b_pbdma_get_gp_base,
	.get_gp_base_hi = gb10b_pbdma_get_gp_base_hi,
	.get_fc_pb_header = gb10b_pbdma_get_fc_pb_header,
	.get_fc_subdevice = gb10b_pbdma_get_fc_subdevice,
	.get_fc_target = gb10b_pbdma_get_fc_target,
	.get_ctrl_hce_priv_mode_yes = gm20b_pbdma_get_ctrl_hce_priv_mode_yes,
	.get_config_auth_level_privileged = gb10b_pbdma_get_config_auth_level_privileged,
	.set_channel_info_veid = gv11b_pbdma_set_channel_info_veid,
	.set_channel_info_chid = ga10b_pbdma_set_channel_info_chid,
	.set_intr_notify = gb10b_pbdma_set_intr_notify,
	.config_userd_writeback_enable = gv11b_pbdma_config_userd_writeback_enable,
	.get_mmu_fault_id = vgpu_gb10b_pbdma_get_mmu_fault_id,
	.get_num_of_pbdmas = gb10b_pbdma_get_num_of_pbdmas,
};

#ifdef CONFIG_TEGRA_GK20A_NVHOST
static const struct gops_sync_syncpt vf_gb10b_ops_sync_syncpt = {
	.get_sync_ro_map = gv11b_syncpt_get_sync_ro_map,
	.alloc_buf = gv11b_syncpt_alloc_buf,
	.free_buf = gv11b_syncpt_free_buf,
#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	.add_wait_cmd = gv11b_syncpt_add_wait_cmd,
	.get_wait_cmd_size = gv11b_syncpt_get_wait_cmd_size,
	.add_incr_cmd = gv11b_syncpt_add_incr_cmd,
	.get_incr_cmd_size = gv11b_syncpt_get_incr_cmd_size,
	.get_incr_per_release = gv11b_syncpt_get_incr_per_release,
#endif
};
#endif

#if defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT) && \
	defined(CONFIG_NVGPU_SW_SEMAPHORE)
static const struct gops_sync_sema vf_gb10b_ops_sync_sema = {
	.add_wait_cmd = gv11b_sema_add_wait_cmd,
	.get_wait_cmd_size = gv11b_sema_get_wait_cmd_size,
	.add_incr_cmd = gv11b_sema_add_incr_cmd,
	.get_incr_cmd_size = gv11b_sema_get_incr_cmd_size,
};
#endif

static const struct gops_sync vf_gb10b_ops_sync = {
};

static const struct gops_engine_status vf_gb10b_ops_engine_status = {
	.read_engine_status_info = NULL,
	.dump_engine_status = NULL,
};

static const struct gops_pbdma_status vf_gb10b_ops_pbdma_status = {
	.read_pbdma_status_info = NULL,
};

static const struct gops_ramfc vf_gb10b_ops_ramfc = {
	.setup = gb10b_ramfc_setup,
	.capture_ram_dump = gb10b_ramfc_capture_ram_dump_2,
	.commit_userd = NULL,
	.get_syncpt = NULL,
	.set_syncpt = NULL,
	.set_channel_info = gb10b_set_channel_info,
};

static const struct gops_ramin vf_gb10b_ops_ramin = {
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

static const struct gops_runlist vf_gb10b_ops_runlist = {
	.reschedule = NULL,
	.update = nvgpu_runlist_update,
	.reload = nvgpu_runlist_reload,
	.count_max = gb10b_runlist_count_max,
	.entry_size = gv11b_runlist_entry_size,
	.length_max = ga10b_runlist_length_max,
	.get_tsg_entry = gv11b_runlist_get_tsg_entry,
	.get_ch_entry = gb10b_runlist_get_ch_entry,
	.hw_submit = vgpu_vf_runlist_hw_submit,
	.check_pending = vgpu_vf_runlist_check_pending,
	.init_enginfo = nvgpu_runlist_init_enginfo,
	.get_max_channels_per_tsg = gv11b_runlist_get_max_channels_per_tsg,
	.get_chan_userd_ptr_hi = gb10b_chan_userd_ptr_hi,
};

static const struct gops_userd vf_gb10b_ops_userd = {
#ifdef CONFIG_NVGPU_USERD
	.setup_sw = vgpu_userd_setup_sw,
	.cleanup_sw = vgpu_userd_cleanup_sw,
	.init_mem = gb10b_userd_init_mem,
	.gp_get = gb10b_sema_based_gp_get,
	.gp_put = gv11b_userd_gp_put,
	.pb_get = NULL,
#endif
	.entry_size = gk20a_userd_entry_size,
};

static const struct gops_channel vf_gb10b_ops_channel = {
	.alloc_inst = nvgpu_channel_alloc_inst,
	.free_inst = nvgpu_channel_free_inst,
	.bind = ga10b_channel_bind,
	.unbind = ga10b_channel_unbind,
	.clear = vgpu_vf_channel_clear,
	.enable = vgpu_vf_channel_enable, // gb10b: moved to dev_vm
	.disable = vgpu_vf_channel_disable, // gb10b: moved to dev_vm
	.count = vgpu_channel_count,
	.read_state = vgpu_vf_channel_read_state,
	.force_ctx_reload = vgpu_vf_channel_force_ctx_reload,
	.abort_clean_up = nvgpu_channel_abort_clean_up,
	.suspend_all_serviceable_ch = nvgpu_channel_suspend_all_serviceable_ch,
	.resume_all_serviceable_ch = nvgpu_channel_resume_all_serviceable_ch,
	.set_error_notifier = nvgpu_set_err_notifier,
	.reset_faulted = vgpu_vf_channel_reset_faulted,
};

static const struct gops_tsg vf_gb10b_ops_tsg = {
	.enable = gv11b_tsg_enable,
	.disable = gv11b_tsg_disable,
	.init_subctx_state = gv11b_tsg_init_subctx_state,
	.deinit_subctx_state = gv11b_tsg_deinit_subctx_state,
	.add_subctx_channel_hw = gv11b_tsg_add_subctx_channel_hw,
	.remove_subctx_channel_hw = gv11b_tsg_remove_subctx_channel_hw,
	.init_eng_method_buffers = gv11b_tsg_init_eng_method_buffers,
	.deinit_eng_method_buffers = gv11b_tsg_deinit_eng_method_buffers,
	.bind_channel = NULL,
	.bind_channel_eng_method_buffers = gv11b_tsg_bind_channel_eng_method_buffers,
	.unbind_channel = NULL,
	.unbind_channel_check_hw_state = nvgpu_tsg_unbind_channel_hw_state_check,
	.unbind_channel_check_hw_next = ga10b_tsg_unbind_channel_check_hw_next,
	.unbind_channel_check_ctx_reload = nvgpu_tsg_unbind_channel_ctx_reload_check,
	.unbind_channel_check_eng_faulted = gv11b_tsg_unbind_channel_check_eng_faulted,
#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	.check_ctxsw_timeout = nvgpu_tsg_check_ctxsw_timeout,
#endif
#ifdef CONFIG_NVGPU_CHANNEL_TSG_CONTROL
	.force_reset = vgpu_tsg_force_reset_ch,
	.post_event_id = nvgpu_tsg_post_event_id,
#endif
#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
	.set_timeslice = nvgpu_tsg_set_timeslice,
	.set_long_timeslice = nvgpu_tsg_set_long_timeslice,
#endif
	.default_timeslice_us = nvgpu_tsg_default_timeslice_us,
};

static const struct gops_usermode vf_gb10b_ops_usermode = {
	.setup_hw = NULL,
	.base = tu104_usermode_base,
	.bus_base = tu104_usermode_bus_base,
	.ring_doorbell = tu104_usermode_ring_doorbell,
	.doorbell_token = tu104_usermode_doorbell_token,
};

static const struct gops_netlist vf_gb10b_ops_netlist = {
	.get_netlist_name = ga10b_netlist_get_name,
	.is_fw_defined = ga10b_netlist_is_firmware_defined,
};

static const struct gops_mm_mmu_fault vf_gb10b_ops_mm_mmu_fault = {
	.setup_sw = gv11b_mm_mmu_fault_setup_sw,
	.setup_hw = gv11b_mm_mmu_fault_setup_hw,
	.info_mem_destroy = gv11b_mm_mmu_fault_info_mem_destroy,
	.disable_hw = gv11b_mm_mmu_fault_disable_hw,
	.parse_mmu_fault_info = gb10b_mm_mmu_fault_parse_mmu_fault_info,
};

static const struct gops_mm_cache vf_gb10b_ops_mm_cache = {
	.fb_flush = vgpu_mm_fb_flush,
	.l2_invalidate = gb10b_func_l2_invalidate,
	.l2_flush = vgpu_mm_l2_flush,
#ifdef CONFIG_NVGPU_COMPRESSION
	.cbc_clean = NULL,
#endif
};

static const struct gops_mm_gmmu vf_gb10b_ops_mm_gmmu = {
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

static const struct gops_mm vf_gb10b_ops_mm = {
	.init_mm_support = nvgpu_init_mm_support,
	.pd_cache_init = nvgpu_pd_cache_init,
	.mm_suspend = nvgpu_mm_suspend,
	.vm_bind_channel = nvgpu_vm_bind_channel,
	.setup_hw = NULL,
	.is_bar1_supported = gv11b_mm_is_bar1_supported,
	.init_inst_block = gv11b_mm_init_inst_block,
	.init_inst_block_core = gv11b_mm_init_inst_block_core,
	.bar2_vm_size = ga10b_mm_bar2_vm_size,
	.init_bar2_vm = gp10b_mm_init_bar2_vm,
	.remove_bar2_vm = gp10b_mm_remove_bar2_vm,
	.get_default_va_sizes = gp10b_mm_get_default_va_sizes,
};

static const struct gops_therm vf_gb10b_ops_therm = {
	.init_therm_support = NULL,
	.init_therm_setup_hw = NULL,
	.init_elcg_mode = NULL,
	.init_blcg_mode = NULL,
	.elcg_init_idle_filters = NULL,
};

#ifdef CONFIG_NVGPU_LS_PMU
static const struct gops_pmu vf_gb10b_ops_pmu = {
	.pmu_early_init = NULL,
	.pmu_rtos_init = NULL,
	.pmu_pstate_sw_setup = NULL,
	.pmu_pstate_pmu_setup = NULL,
	.pmu_destroy = NULL,
	.pmu_setup_elpg = NULL,
	.pmu_get_queue_head = NULL,
	.pmu_get_queue_head_size = NULL,
	.pmu_get_queue_tail = NULL,
	.pmu_get_queue_tail_size = NULL,
	.pmu_reset = NULL,
	.pmu_queue_head = NULL,
	.pmu_queue_tail = NULL,
	.pmu_msgq_tail = NULL,
	.pmu_mutex_size = NULL,
	.pmu_mutex_acquire = NULL,
	.pmu_mutex_release = NULL,
	.pmu_is_interrupted = NULL,
	.pmu_isr = NULL,
	.pmu_init_perfmon_counter = NULL,
	.pmu_pg_idle_counter_config = NULL,
	.pmu_read_idle_counter = NULL,
	.pmu_reset_idle_counter = NULL,
	.pmu_read_idle_intr_status = NULL,
	.pmu_clear_idle_intr_status = NULL,
	.pmu_dump_elpg_stats = NULL,
	.pmu_dump_falcon_stats = NULL,
	.get_intr_ctrl_msg = NULL,
	.pmu_enable_irq = NULL,
	.write_dmatrfbase = NULL,
	.dump_secure_fuses = NULL,
	.reset_engine = NULL,
	.is_engine_in_reset = NULL,
	.pmu_ns_bootstrap = NULL,
	.is_pmu_supported = NULL,
};
#endif

#ifdef CONFIG_NVGPU_CLK_ARB
static const struct gops_clk_arb vf_gb10b_ops_clk_arb = {
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
static const struct gops_regops vf_gb10b_ops_regops = {
	.exec_regops = vf_exec_regops,
	.exec_ctx_regops = vf_exec_ctx_ops,
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
	.get_hwpm_mm_register_ranges = gb10b_get_hwpm_mm_register_ranges,
	.get_hes_register_ranges = gb10b_get_hes_register_ranges,
	.is_hwpm_pma_reg_context_switched = gb10b_is_hwpm_pma_reg_context_switched,
};
#endif

static const struct gops_mc vf_gb10b_ops_mc = {
	.get_chip_details = NULL,
	.intr_mask = NULL,
	.intr_enable = NULL,
	.intr_stall_unit_config = NULL,
	.intr_nonstall_unit_config = NULL,
	.isr_stall = NULL,
	.intr_stall = NULL,
	.intr_stall_pause = NULL,
	.intr_stall_resume = NULL,
	.intr_nonstall = NULL,
	.intr_nonstall_pause = NULL,
	.intr_nonstall_resume = NULL,
	.isr_nonstall = NULL,
	.is_intr1_pending = NULL,
	.is_intr_hub_pending = NULL,
	.log_pending_intrs = NULL,
	.is_enabled = NULL,
	.fb_reset = NULL,
	.is_mmu_fault_pending = NULL,
	.enable_units = NULL,
	.enable_dev = NULL,
	.enable_devtype = NULL,
};

static const struct gops_debug vf_gb10b_ops_debug = {
	.show_dump = NULL,
};

#ifdef CONFIG_NVGPU_DEBUGGER
static const struct gops_debugger vf_gb10b_ops_debugger = {
	.post_events = nvgpu_dbg_gpu_post_events,
	.dbg_set_powergate = vgpu_dbg_set_powergate,
};
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
static const struct gops_perf vf_gb10b_ops_perf = {
	.get_pmmsys_per_chiplet_offset = gb10b_perf_get_pmmsys_per_chiplet_offset,
	.get_pmmgpc_per_chiplet_offset = ga10b_perf_get_pmmgpc_per_chiplet_offset,
	.get_pmmfbp_per_chiplet_offset = ga10b_perf_get_pmmfbp_per_chiplet_offset,
	.update_get_put = vgpu_perf_update_get_put,
	.reset_pm_trigger_masks = vf_perf_reset_pm_trigger_masks,
	.enable_pm_trigger = vf_perf_enable_pm_trigger,
	.get_pma_cblock_instance_count = gb10b_perf_get_pma_cblock_instance_count,
	.get_pma_channel_count = gb10b_perf_get_pma_channel_count,
	.get_pma_channels_per_cblock = gb10b_perf_get_pma_channels_per_cblock,
	.init_inst_block = vf_perf_init_inst_block,
	.deinit_inst_block = vf_perf_deinit_inst_block,
};
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
static const struct gops_perfbuf vf_gb10b_ops_perfbuf = {
	.perfbuf_enable = vgpu_perfbuffer_enable,
	.perfbuf_disable = vgpu_perfbuffer_disable,
	.init_inst_block = nvgpu_perfbuf_init_inst_block,
	.deinit_inst_block = nvgpu_perfbuf_deinit_inst_block,
	.update_get_put = nvgpu_perfbuf_update_get_put,
};
#endif

#ifdef CONFIG_NVGPU_PROFILER
static const struct gops_pm_reservation vf_gb10b_ops_pm_reservation = {
	.acquire = vgpu_pm_reservation_acquire,
	.release = vgpu_pm_reservation_release,
	.release_all_per_vmid = NULL,
};
#endif

#ifdef CONFIG_NVGPU_PROFILER
static const struct gops_profiler vf_gb10b_ops_profiler = {
	.bind_hwpm = vf_profiler_bind_hwpm,
	.unbind_hwpm = vf_profiler_unbind_hwpm,
	.bind_pma_streamout = vf_profiler_bind_pma_streamout,
	.unbind_pma_streamout = vf_profiler_unbind_pma_streamout,
	.bind_smpc = vf_gb10b_profiler_bind_smpc,
	.unbind_smpc = vf_gb10b_profiler_unbind_smpc,
	.bind_hes_cwd = vf_profiler_bind_hes_cwd,
	.set_hs_credit_per_chiplet = vf_profiler_set_hs_credit_per_chiplet,
	.get_hs_credit_per_gpc_chiplet = gb10b_perf_get_hs_credit_per_gpc_chiplet,
	.get_hs_credit_per_fbp_chiplet = gb10b_perf_get_hs_credit_per_fbp_chiplet,
	.get_hs_credit_per_sys_pipe_for_profiling = gb10b_perf_get_hs_credit_per_sys_pipe_for_profiling,
	.get_hs_credit_per_sys_pipe_for_hes = gb10b_perf_get_hs_credit_per_sys_pipe_for_hes,
};
#endif

static const struct gops_bus vf_gb10b_ops_bus = {
	.init_hw = NULL,
	.isr = NULL,
	.bar1_bind = NULL,
	.bar2_bind = gb10b_func_bar2_bind,
#ifdef CONFIG_NVGPU_DGPU
	.set_bar0_window = NULL,
#endif
};

static const struct gops_ptimer vf_gb10b_ops_ptimer = {
	.isr = NULL,
	.read_ptimer = vgpu_read_ptimer,
#ifdef CONFIG_NVGPU_IOCTL_NON_FUSA
	.get_timestamps_zipper = vgpu_get_timestamps_zipper,
#endif
#ifdef CONFIG_NVGPU_PROFILER
	.get_timer_reg_offsets = gv11b_ptimer_get_timer_reg_offsets,
#endif
};

#if defined(CONFIG_NVGPU_CYCLESTATS)
static const struct gops_css vf_gb10b_ops_css = {
	.enable_snapshot = vgpu_css_enable_snapshot_buffer,
	.disable_snapshot = vgpu_css_release_snapshot_buffer,
	.check_data_available = vgpu_css_flush_snapshots,
	.detach_snapshot = vgpu_css_detach,
	.set_handled_snapshots = NULL,
	.allocate_perfmon_ids = NULL,
	.release_perfmon_ids = NULL,
	.get_max_buffer_size = vgpu_css_get_buffer_size,
};
#endif

static const struct gops_falcon vf_gb10b_ops_falcon = {
	.falcon_sw_init = nvgpu_falcon_sw_init,
	.falcon_sw_free = nvgpu_falcon_sw_free,
};

static const struct gops_priv_ring vf_gb10b_ops_priv_ring = {
	.enable_priv_ring = NULL,
	.isr = NULL,
	.enum_ltc = NULL,
	.get_gpc_count = vgpu_gr_get_gpc_count,
};

static const struct gops_fuse vf_gb10b_ops_fuse = {
	.is_opt_ecc_enable = NULL,
	.is_opt_feature_override_disable = NULL,
	.fuse_status_opt_fbio = NULL,
	.fuse_status_opt_fbp = NULL,
	.fuse_status_opt_emc = ga10b_fuse_status_opt_emc,
	.fuse_status_opt_l2_fbp = NULL,
	.fuse_status_opt_tpc_gpc = NULL,
	.fuse_ctrl_opt_tpc_gpc = NULL,
	.fuse_opt_sec_debug_en = NULL,
	.fuse_opt_priv_sec_en = NULL,
	.read_vin_cal_fuse_rev = NULL,
	.read_vin_cal_slope_intercept_fuse = NULL,
	.read_vin_cal_gain_offset_fuse = NULL,
};

static const struct gops_top vf_gb10b_ops_top = {
	.get_max_gpc_count = vgpu_gr_get_max_gpc_count,
	.get_max_fbps_count = vgpu_gr_get_max_fbps_count,
	.get_max_ltc_per_fbp = vgpu_gr_get_max_ltc_per_fbp,
	.get_max_lts_per_ltc = vgpu_gr_get_max_lts_per_ltc,
	.parse_next_device = vgpu_top_parse_next_dev,
	.get_max_rop_per_gpc = vgpu_top_get_max_rop_per_gpc,
};

static const struct gops_grmgr vf_gb10b_ops_grmgr = {
	.init_gr_manager = nvgpu_init_gr_manager,
};

static const struct gops_fb_vab vf_gb10b_ops_fb_vab = {
	.init = NULL,
	.set_vab_buffer_address = NULL,
	.reserve = NULL,
	.dump_and_clear = NULL,
	.release = NULL,
	.recover = NULL,
	.teardown = NULL,
};

static const struct gops_gin vf_gb10b_ops_gin = {
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

int vf_gb10b_init_hal(struct gk20a *g)
{
	struct gpu_ops *gops = &g->ops;
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	gops->acr = vf_gb10b_ops_acr;
#ifdef CONFIG_NVGPU_DGPU
	gops->bios = vf_gb10b_ops_bios;
#endif /* CONFIG_NVGPU_DGPU */
	gops->ltc = vf_gb10b_ops_ltc;
	gops->ltc.intr = vf_gb10b_ops_ltc_intr;
#ifdef CONFIG_NVGPU_COMPRESSION
	gops->cbc = vf_gb10b_ops_cbc;
#endif
	gops->ce = vf_gb10b_ops_ce;
	gops->gr = vf_gb10b_ops_gr;
	gops->gr.ctxsw_prog = vf_gb10b_ops_gr_ctxsw_prog;
	gops->gr.config = vf_gb10b_ops_gr_config;
	gops->gr.setup = vf_gb10b_ops_gr_setup;
#ifdef CONFIG_NVGPU_GRAPHICS
	gops->gr.zbc = vf_gb10b_ops_gr_zbc;
	gops->gr.zcull = vf_gb10b_ops_gr_zcull;
#endif /* CONFIG_NVGPU_GRAPHICS */
#ifdef CONFIG_NVGPU_DEBUGGER
	gops->gr.hwpm_map = vf_gb10b_ops_gr_hwpm_map;
#endif
	gops->gr.falcon = vf_gb10b_ops_gr_falcon;
#ifdef CONFIG_NVGPU_FECS_TRACE
	gops->gr.fecs_trace = vf_gb10b_ops_gr_fecs_trace;
#endif /* CONFIG_NVGPU_FECS_TRACE */
	gops->gr.init = vf_gb10b_ops_gr_init;
	gops->gr.intr = vf_gb10b_ops_gr_intr;
	gops->gpu_class = vf_gb10b_ops_gpu_class;
	gops->fb = vf_gb10b_ops_fb;
	gops->fb.intr = vf_gb10b_ops_fb_intr;
	gops->fb.vab = vf_gb10b_ops_fb_vab;
	gops->nvenc = vf_gb10b_ops_nvenc;
	gops->ofa = vf_gb10b_ops_ofa;
	gops->nvdec = vf_gb10b_ops_nvdec;
	gops->nvjpg = vf_gb10b_ops_nvjpg;
	gops->cg = vf_gb10b_ops_cg;
	gops->fifo = vf_gb10b_ops_fifo;
	gops->engine = vf_gb10b_ops_engine;
	gops->pbdma = vf_gb10b_ops_pbdma;
	gops->sync = vf_gb10b_ops_sync;
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	gops->sync.syncpt = vf_gb10b_ops_sync_syncpt;
#endif /* CONFIG_TEGRA_GK20A_NVHOST */
#if defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT) && \
	defined(CONFIG_NVGPU_SW_SEMAPHORE)
	gops->sync.sema = vf_gb10b_ops_sync_sema;
#endif
	gops->engine_status = vf_gb10b_ops_engine_status;
	gops->pbdma_status = vf_gb10b_ops_pbdma_status;
	gops->ramfc = vf_gb10b_ops_ramfc;
	gops->ramin = vf_gb10b_ops_ramin;
	gops->runlist = vf_gb10b_ops_runlist;
	gops->userd = vf_gb10b_ops_userd;
	gops->channel = vf_gb10b_ops_channel;
	gops->tsg = vf_gb10b_ops_tsg;
	gops->usermode = vf_gb10b_ops_usermode;
	gops->netlist = vf_gb10b_ops_netlist;
	gops->mm = vf_gb10b_ops_mm;
	gops->mm.mmu_fault = vf_gb10b_ops_mm_mmu_fault;
	gops->mm.cache = vf_gb10b_ops_mm_cache;
	gops->mm.gmmu = vf_gb10b_ops_mm_gmmu;
	gops->therm = vf_gb10b_ops_therm;
#ifdef CONFIG_NVGPU_LS_PMU
	gops->pmu = vf_gb10b_ops_pmu;
#endif
#ifdef CONFIG_NVGPU_CLK_ARB
	gops->clk_arb = vf_gb10b_ops_clk_arb;
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	gops->regops = vf_gb10b_ops_regops;
#endif
	gops->mc = vf_gb10b_ops_mc;
	gops->debug = vf_gb10b_ops_debug;
#ifdef CONFIG_NVGPU_DEBUGGER
	gops->debugger = vf_gb10b_ops_debugger;
	gops->perf = vf_gb10b_ops_perf;
	gops->perfbuf = vf_gb10b_ops_perfbuf;
#endif
#ifdef CONFIG_NVGPU_PROFILER
	gops->pm_reservation = vf_gb10b_ops_pm_reservation;
	gops->profiler = vf_gb10b_ops_profiler;
#endif
	gops->bus = vf_gb10b_ops_bus;
	gops->ptimer = vf_gb10b_ops_ptimer;
#if defined(CONFIG_NVGPU_CYCLESTATS)
	gops->css = vf_gb10b_ops_css;
#endif
	gops->falcon = vf_gb10b_ops_falcon;
	gops->priv_ring = vf_gb10b_ops_priv_ring;
	gops->fuse = vf_gb10b_ops_fuse;
	gops->top = vf_gb10b_ops_top;
	gops->grmgr = vf_gb10b_ops_grmgr;
	gops->gin = vf_gb10b_ops_gin;

	/* Lone functions */
	gops->chip_init_gpu_characteristics = vf_gb10b_init_gpu_characteristics;
	gops->get_litter_value = gb10b_get_litter_value;
	gops->semaphore_wakeup = nvgpu_channel_semaphore_wakeup;

#ifdef CONFIG_NVGPU_CLK_ARB
	if (priv->constants.can_set_clkrate && nvgpu_platform_is_silicon(g)) {
		nvgpu_set_enabled(g, NVGPU_CLK_ARB_ENABLED, true);
	} else {
		gops->clk_arb.get_arbiter_clk_domains = NULL;
		nvgpu_set_enabled(g, NVGPU_CLK_ARB_ENABLED, false);
	}
#endif

	if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)){
		nvgpu_set_errata(g, NVGPU_ERRATA_2969956, true);
	}
	nvgpu_set_errata(g, NVGPU_ERRATA_200601972, true);
	nvgpu_set_errata(g, NVGPU_ERRATA_200391931, true);
	nvgpu_set_errata(g, NVGPU_ERRATA_SYNCPT_INVALID_ID_0, true);

	nvgpu_set_errata(g, NVGPU_ERRATA_3288192, true);

	nvgpu_set_enabled(g, NVGPU_SUPPORT_ROP_IN_GPC, true);

#ifdef CONFIG_NVGPU_SM_DIVERSITY
	/*
	 * To achieve permanent fault coverage, the CTAs launched by each kernel
	 * in the mission and redundant contexts must execute on different
	 * hardware resources. This feature proposes modifications in the
	 * software to modify the virtual SM id to TPC mapping across the
	 * mission and redundant contexts.
	 *
	 * The virtual SM identifier to TPC mapping is done by the nvgpu
	 * when setting up the golden context. Once the table with this mapping
	 * is initialized, it is used by all subsequent contexts created.
	 * The proposal is for setting up the virtual SM identifier to TPC
	 * mapping on a per-context basis and initializing this virtual SM
	 * identifier to TPC mapping differently for the mission and
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
	if (priv->constants.max_sm_diversity_config_count > 1U) {
		nvgpu_set_enabled(g, NVGPU_SUPPORT_SM_DIVERSITY, true);
	}
#else
	priv->constants.max_sm_diversity_config_count =
		NVGPU_DEFAULT_SM_DIVERSITY_CONFIG_COUNT;
#endif
	g->max_sm_diversity_config_count =
		priv->constants.max_sm_diversity_config_count;

#ifdef CONFIG_NVGPU_COMPRESSION
	if (nvgpu_platform_is_silicon(g)) {
		nvgpu_set_enabled(g, NVGPU_SUPPORT_COMPRESSION, true);
	} else {
		nvgpu_set_enabled(g, NVGPU_SUPPORT_COMPRESSION, false);
	}

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_COMPRESSION)) {
		g->cbc_use_raw_mode = true;
		nvgpu_set_enabled(g, NVGPU_SUPPORT_POST_L2_COMPRESSION, true);
	} else {
		gops->cbc.init = NULL;
		gops->cbc.ctrl = NULL;
		gops->cbc.alloc_comptags = NULL;
	}
#endif

#ifdef CONFIG_NVGPU_RECOVERY
	nvgpu_set_enabled(g, NVGPU_SUPPORT_FAULT_RECOVERY, true);
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	nvgpu_set_enabled(g, NVGPU_L2_MAX_WAYS_EVICT_LAST_ENABLED, false);
	nvgpu_set_enabled(g, NVGPU_SCHED_EXIT_WAIT_FOR_ERRBAR_SUPPORTED, true);
#endif

#ifdef CONFIG_NVS_PRESENT
	nvgpu_set_enabled(g, NVGPU_SUPPORT_NVS, false);
#endif
	nvgpu_set_enabled(g, NVGPU_MM_HONORS_APERTURE, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_SEMA_BASED_GPFIFO_GET, true);
	nvgpu_set_enabled(g, NVGPU_SUPPORT_GPU_MMIO, true);

	g->name = "gb10b";

	return 0;
}
