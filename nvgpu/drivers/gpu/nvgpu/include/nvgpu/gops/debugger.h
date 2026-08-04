/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GOPS_DEBUGGER_H
#define NVGPU_GOPS_DEBUGGER_H

#ifdef CONFIG_NVGPU_DEBUGGER
struct gops_regops {
	int (*exec_regops)(struct gk20a *g,
				struct nvgpu_tsg *tsg,
				struct nvgpu_dbg_reg_op *ops,
				u32 num_ops,
				u32 ctx_wr_count,
				u32 ctx_rd_count,
				u32 *flags);
	int (*exec_ctx_regops)(struct nvgpu_tsg *tsg,
			    struct nvgpu_dbg_reg_op *ctx_ops, u32 num_ops,
			    u32 num_ctx_wr_ops, u32 num_ctx_rd_ops,
			    u32 *flags);
	const struct regop_offset_range* (
			*get_global_whitelist_ranges)(void);
	u64 (*get_global_whitelist_ranges_count)(void);
	const struct regop_offset_range* (
			*get_context_whitelist_ranges)(void);
	u64 (*get_context_whitelist_ranges_count)(void);
	const u32* (*get_runcontrol_whitelist)(void);
	u64 (*get_runcontrol_whitelist_count)(void);
	u32 (*get_hwpm_perfmon_register_stride)(void);
	u32 (*get_hwpm_router_register_stride)(void);
	u32 (*get_hwpm_pma_channel_register_stride)(void);
	u32 (*get_hwpm_pma_trigger_register_stride)(void);
	u32 (*get_smpc_register_stride)(void);
	u32 (*get_cau_register_stride)(void);
	const u32 *(*get_hwpm_perfmon_register_offset_allowlist)(u32 *count);
	const u32 *(*get_hwpm_router_register_offset_allowlist)(u32 *count);
	const u32 *(*get_hwpm_pma_channel_register_offset_allowlist)(u32 *count);
	const u32 *(*get_hwpm_pma_trigger_register_offset_allowlist)(u32 *count);
	const u32 *(*get_smpc_register_offset_allowlist)(u32 *count);
	const u32 *(*get_cau_register_offset_allowlist)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_hwpm_perfmon_register_ranges)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_hwpm_router_register_ranges)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_hwpm_pma_channel_register_ranges)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_hwpm_pc_sampler_register_ranges)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_hwpm_pma_trigger_register_ranges)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_smpc_register_ranges)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_cau_register_ranges)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_hwpm_perfmux_register_ranges)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_hes_register_ranges)(u32 *count);
	const struct nvgpu_pm_resource_register_range *
		(*get_hwpm_mm_register_ranges)(u32 *count);
	bool (*is_hwpm_pma_reg_context_switched)(void);
};
struct gops_debugger {
	void (*post_events)(struct gk20a *g, u32 gfid, u32 tsgid);
	int (*dbg_set_powergate)(struct dbg_session_gk20a *dbg_s,
				bool disable_powergate);
};
struct gops_perf {
	void (*enable_membuf)(struct gk20a *g, u32 pma_channel_id, u32 size,
			      u64 buf_addr);
	void (*disable_membuf)(struct gk20a *g, u32 pma_channel_id);
	void (*bind_mem_bytes_buffer_addr)(struct gk20a *g, u32 pma_channel_id,
					   u64 buf_addr);
	int (*init_inst_block)(struct gk20a *g, u32 pma_channel_id,
			u32 inst_blk_ptr, u32 aperture, u32 gfid);
	void (*deinit_inst_block)(struct gk20a *g, u32 pma_channel_id);
	void (*membuf_reset_streaming)(struct gk20a *g, u32 pma_channel_id);
	u32 (*get_membuf_pending_bytes)(struct gk20a *g, u32 pma_channel_id);
	void (*set_membuf_handled_bytes)(struct gk20a *g, u32 pma_channel_id,
					 u32 entries, u32 entry_size);
	bool (*get_membuf_overflow_status)(struct gk20a *g, u32 pma_channel_id);
	u32 (*get_pmmsys_per_chiplet_offset)(void);
	u32 (*get_pmmgpc_per_chiplet_offset)(void);
	u32 (*get_pmmgpcrouter_per_chiplet_offset)(void);
	u32 (*get_pmmfbprouter_per_chiplet_offset)(void);
	u32 (*get_pmmfbp_per_chiplet_offset)(void);
	int (*update_get_put)(struct gk20a *g, u32 pma_channel_id,
			      u64 bytes_consumed, bool update_available_bytes,
			      u64 *put_ptr, bool *overflowed);
	const u32 *(*get_hwpm_sys_perfmon_regs)(u32 *count);
	const u32 *(*get_hwpm_fbp_perfmon_regs)(u32 *count);
	const u32 *(*get_hwpm_gpc_perfmon_regs)(u32 *count);
	u32 (*get_hwpm_fbp_perfmon_regs_base)(struct gk20a *g);
	u32 (*get_hwpm_gpc_perfmon_regs_base)(struct gk20a *g);
	u32 (*get_hwpm_fbprouter_perfmon_regs_base)(struct gk20a *g);
	u32 (*get_hwpm_gpcrouter_perfmon_regs_base)(struct gk20a *g);
	void (*init_hwpm_pmm_register)(struct gk20a *g, u32 gr_instance_id);
	void (*get_num_hwpm_perfmon)(struct gk20a *g, u32 *num_sys_perfmon,
				     u32 *num_fbp_perfmon,
				     u32 *num_gpc_perfmon);
	void (*set_pmm_register)(struct gk20a *g, u32 offset, u32 val,
			 u32 num_chiplets, u32 chiplet_stride, u32 num_perfmons);
	void (*reset_hwpm_pmm_registers)(struct gk20a *g, u32 gr_instance_id);
	void (*pma_stream_enable)(struct gk20a *g, u32 pma_channel_id,
				  bool enable);
	void (*disable_all_perfmons)(struct gk20a *g, u32 gr_instance_id);
	int (*wait_for_idle_pmm_routers)(struct gk20a *g, u32 gr_instance_id, u32 pma_channel_id);
	int (*wait_for_idle_pma)(struct gk20a *g);
	void (*reset_cwd_hem_registers)(struct gk20a *g, u32 gr_instance_id);
	int (*set_secure_config_for_cwd_hem)(struct gk20a *g, u32 gr_instance_id,
					     u32 pma_channel_id, bool dg_enable);
	int (*set_secure_config_for_cau)(struct gk20a *g, u32 gr_instance_id,
					 u32 pma_channel_id, bool dg_enable);
	void (*update_sys_dg_map_status_mask)(struct gk20a *g,
					      u32 gr_instance_id, u32 *sys_dg_map_mask);
	void (*update_cau_dg_map_status_mask)(struct gk20a *g, u32 gr_instance_id,
					      u32 num_gpcs, u32 perfmon_idx, u32 num_perfmons,
					      u32 *dgmap_mask, u32 dgmap_mask_size);
	int (*set_secure_config_for_ccu_prof)(struct gk20a *g, u32 gr_instance_id,
						u32 pma_channel_id, bool enable);

#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
	void (*enable_hs_streaming)(struct gk20a *g, bool enable);
	void (*reset_hs_streaming_credits)(struct gk20a *g);
	void (*enable_pmasys_legacy_mode)(struct gk20a *g, bool enable);
#endif
	void (*reset_hwpm_pma_registers)(struct gk20a *g);
	void (*reset_hwpm_pma_trigger_registers)(struct gk20a *g);
	void (*reset_pmasys_channel_registers)(struct gk20a *g);
	int (*set_secure_config_for_hwpm_dg)(struct gk20a *g, u32 gr_instance_id,
					     u32 pma_channel_id, bool dg_enable);
	u32 (*get_pma_cblock_instance_count)(void);
	u32 (*get_pma_channel_count)(struct gk20a *g);
	u32 (*get_pma_channels_per_cblock)(void);
	void (*reset_pm_trigger_masks)(struct gk20a *g, u32 pma_channel_id,
			u32 gr_instance_id, u32 reservation_id);
	void (*enable_pm_trigger)(struct gk20a *g, u32 gr_instance_id, u32 pma_channel_id,
			u32 reservation_id);
	void (*enable_hes_event_trigger)(struct gk20a *g, u32 gr_instance_id, u32 pma_channel_id);
	void (*enable_pma_trigger)(struct gk20a *g, u32 pma_channel_id);
	u32 (*get_hwpm_fbp_fbpgs_ltcs_base_addr)(void);
	u32 (*get_hwpm_gpcgs_gpctpca_base_addr)(void);
	u32 (*get_hwpm_gpcgs_gpctpcb_base_addr)(void);
	u32 (*get_hwpm_gpcs_base_addr)(void);
	u32 (*get_hwpm_gpcsrouter_base_addr)(void);
	u32 (*get_hwpm_fbps_base_addr)(void);
	u32 (*get_hwpm_fbpsrouter_base_addr)(void);
	void (*set_pmm_register_for_chiplet_range)(struct gk20a *g, u32 offset, u32 val,
						   u32 start_chiplet, u32 num_chiplets,
						   u32 chiplet_stride, u32 num_perfmons,
						   u32 *dg_map_mask, u32 dg_map_mask_size);
	int (*alloc_mem_for_sys_partition_dg_map)(struct gk20a *g);
	void (*program_sys_pmm_secure_config)(struct gk20a *g, u32 offset, u32 val,
			u32 dg_map_mask);
	void (*get_tpc_perfmon_range)(u32 *perfmon_start_idx,
					u32 *perfmon_end_idx);
	u32 (*get_max_num_gpc_perfmons)(void);
	u32 (*get_gpc_perfmon_stride)(void);
	u32 (*get_gpc_tpc_start_dg_idx)(void);
	u32 (*get_gpc_tpc_end_dg_idx)(void);
	bool (*is_perfmon_simulated)(void);
	u32 (*get_num_cwd_hems)(void);
	void (*update_sys_hem_cwd_dg_map_mask)(struct gk20a *g, u32 gr_instance_id,
					     u32 *sys_dg_map_mask,
					     u32 *start_hem_idx, u32 *num_hems);
	u32 (*get_gpc_tpc0_cau0_dg_idx)(void);
};
struct gops_perfbuf {
	int (*perfbuf_enable)(struct gk20a *g, u32 pma_channel_id, u64 offset,
			      u32 size);
	int (*perfbuf_disable)(struct gk20a *g, u32 pma_channel_id);
	int (*init_inst_block)(struct gk20a *g, struct perfbuf *perfbuf,
				u32 pma_channel_id);
	void (*deinit_inst_block)(struct gk20a *g,
			struct nvgpu_mem *inst_block, u32 pma_channel_id);
	int (*update_get_put)(struct gk20a *g, u32 pma_channel_id,
			      u64 bytes_consumed, u64 *bytes_available,
			      void *cpuva, bool wait, u64 *put_ptr,
			      bool *overflowed);
};
#endif

#endif /* NVGPU_GOPS_DEBUGGER_H */
