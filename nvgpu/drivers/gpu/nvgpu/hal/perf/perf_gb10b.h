/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GB10B_PERF
#define NVGPU_GB10B_PERF

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;
enum nvgpu_profiler_pm_resource_type;

u32 gb10b_perf_get_pma_cblock_instance_count(void);
u32 gb10b_perf_get_pma_channel_count(struct gk20a *g);
u32 gb10b_perf_get_pma_channels_per_cblock(void);
bool gb10b_perf_get_membuf_overflow_status(struct gk20a *g, u32 pma_channel_id);
u32 gb10b_perf_get_membuf_pending_bytes(struct gk20a *g, u32 pma_channel_id);
void gb10b_perf_set_membuf_handled_bytes(struct gk20a *g, u32 pma_channel_id,
					 u32 entries, u32 entry_size);
void gb10b_perf_membuf_reset_streaming(struct gk20a *g, u32 pma_channel_id);
void gb10b_perf_enable_membuf(struct gk20a *g, u32 pma_channel_id, u32 size,
			      u64 buf_addr);
void gb10b_perf_disable_membuf(struct gk20a *g, u32 pma_channel_id);
void gb10b_perf_bind_mem_bytes_buffer_addr(struct gk20a *g, u32 pma_channel_id,
					   u64 buf_addr);
int gb10b_perf_init_inst_block(struct gk20a *g, u32 pma_channel_id,
			       u32 inst_blk_ptr, u32 aperture, u32 gfid);
void gb10b_perf_deinit_inst_block(struct gk20a *g, u32 pma_channel_id);
int gb10b_perf_update_get_put(struct gk20a *g, u32 pma_channel_id,
			      u64 bytes_consumed, bool update_available_bytes,
			      u64 *put_ptr, bool *overflowed);
void gb10b_perf_pma_stream_enable(struct gk20a *g, u32 pma_channel_id,
				  bool enable);
int gb10b_perf_wait_for_idle_pma(struct gk20a *g);
void gb10b_perf_get_num_hwpm_perfmon(struct gk20a *g, u32 *num_sys_perfmon,
				u32 *num_fbp_perfmon, u32 *num_gpc_perfmon);
void gb10b_perf_disable_all_perfmons(struct gk20a *g, u32 gr_instance_id);
u32 gb10b_perf_get_pmmfbprouter_per_chiplet_offset(void);

u32 gb10b_get_hwpm_fbprouter_perfmon_regs_base(struct gk20a *g);
u32 gb10b_get_hwpm_gpcrouter_perfmon_regs_base(struct gk20a *g);
u32 gb10b_perf_get_pmmgpcrouter_per_chiplet_offset(void);
u32 gb10b_perf_get_pmmsys_per_chiplet_offset(void);
const u32 *gb10b_perf_get_hwpm_sys_perfmon_regs(u32 *count);
const u32 *gb10b_perf_get_hwpm_gpc_perfmon_regs(u32 *count);
const u32 *gb10b_perf_get_hwpm_fbp_perfmon_regs(u32 *count);
u32 gb10b_perf_get_pmmgpc_per_chiplet_offset(void);
u32 gb10b_perf_get_pmmfbp_per_chiplet_offset(void);
int gb10b_perf_wait_for_idle_pmm_routers(struct gk20a *g, u32 gr_instance_id, u32 pma_channel_id);
int gb10b_set_secure_config_for_hwpm_dg(struct gk20a *g, u32 gr_instance_id,
					u32 pma_channel_id, bool dg_enable);
void gb10b_perf_init_hwpm_pmm_register(struct gk20a *g, u32 gr_instance_id);
void gb10b_perf_reset_hwpm_pmm_register(struct gk20a *g, u32 gr_instance_id);
void gb10b_perf_reset_pm_trigger_masks(struct gk20a *g, u32 pma_channel_id,
			u32 gr_instance_id, u32 reservation_id);
void gb10b_perf_enable_pm_trigger(struct gk20a *g, u32 gr_instance_id, u32 pma_channel_id,
									u32 reservation_id);
void gb10b_perf_enable_hes_event_trigger(struct gk20a *g, u32 gr_instance_id, u32 pma_channel_id);
void gb10b_perf_enable_pma_trigger(struct gk20a *g, u32 pma_channel_id);
u32 gb10b_perf_get_hwpm_fbp_fbpgs_ltcs_base_addr(void);
u32 gb10b_perf_get_hwpm_gpcgs_gpctpca_base_addr(void);
u32 gb10b_perf_get_hwpm_gpcgs_gpctpcb_base_addr(void);
u32 gb10b_perf_get_hwpm_gpcs_base_addr(void);
u32 gb10b_perf_get_hwpm_gpcsrouter_base_addr(void);
u32 gb10b_perf_get_hwpm_fbps_base_addr(void);
u32 gb10b_perf_get_hwpm_fbpsrouter_base_addr(void);
void gb10b_perf_reset_cwd_hem_registers(struct gk20a *g, u32 gr_instance_id);
int gb10b_perf_set_secure_config_for_cwd_hem(struct gk20a *g, u32 gr_instance_id,
					     u32 pma_channel_id, bool dg_enable);
int gb10b_perf_set_secure_config_for_cau(struct gk20a *g, u32 gr_instance_id,
					 u32 pma_channel_id, bool dg_enable);
void gb10b_perf_set_pmm_register_for_chiplet_range(struct gk20a *g, u32 offset, u32 val,
						   u32 start_chiplet, u32 num_chiplets,
						   u32 chiplet_stride, u32 num_perfmons,
						   u32 *dg_map_mask, u32 dg_map_mask_size);
int gb10b_perf_alloc_mem_for_sys_partition_dg_map(struct gk20a *g);
void gb10b_perf_program_sys_pmm_secure_config(struct gk20a *g, u32 offset, u32 val,
		u32 dg_map_mask);
void gb10b_perf_get_tpc_perfmon_range(u32 *perfmon_start_idx,
					u32 *perfmon_end_idx);
u32 gb10b_perf_get_max_num_gpc_perfmons(void);
u32 gb10b_perf_get_gpc_perfmon_stride(void);

u32 gb10b_perf_get_hs_credit_per_gpc_chiplet(struct gk20a *g);
u32 gb10b_perf_get_hs_credit_per_fbp_chiplet(struct gk20a *g);
u32 gb10b_perf_get_hs_credit_per_sys_pipe_for_profiling(struct gk20a *g);
u32 gb10b_perf_get_hs_credit_per_sys_pipe_for_hes(struct gk20a *g);
void gb10b_perf_set_hs_credit_per_chiplet(struct gk20a *g, u32 gr_instance_id,
			u32 pma_channel_id, u32 chiplet_type,
			u32 chiplet_local_index, u32 num_of_credits);
void gb10b_perf_set_pma_stream_gfid(struct gk20a *g, u32 pma_channel_id, u32 gfid);
u32 gb10b_perf_get_gpc_tpc_start_dg_idx(void);
u32 gb10b_perf_get_gpc_tpc_end_dg_idx(void);
void gb10b_update_sys_dg_map_status_mask(struct gk20a *g, u32 gr_instance_id,
					 u32 *sys_dg_map_mask);
void
gb10b_update_cau_dg_map_status_mask(struct gk20a *g, u32 gr_instance_id, u32 num_gpcs,
				    u32 perfmon_idx, u32 num_perfmons, u32 *dgmap_mask,
				    u32 dgmap_mask_size);
void gb10b_update_fbp_dg_map_status_mask(u32 num_chiplets, u32 perfmon_idx,
			u32 num_perfmons, u32 *dgmap_mask, u32 dgmap_mask_size);
bool gb10b_perf_is_perfmon_simulated(void);
void gb10b_perf_program_pmm_register_for_chiplet_range(struct gk20a *g,
		u32 offset, u32 val, u32 start_chiplet, u32 num_chiplets,
		u32 chiplet_stride, u32 *dg_map_mask);
int poll_dgmap_status_fbp_perfmons_mapped(struct gk20a *g, u32 *dgmap_mask,
					  u32 dgmap_mask_size, bool dg_enable,
					  u32 cur_gr_instance);
int poll_dgmap_status_gpc_perfmons_mapped(struct gk20a *g, u32 *dgmap_mask,
		u32 dgmap_mask_size, bool dg_enable, u32 cur_gr_instance);
u32 gb10b_perf_get_num_cwd_hems(void);
void gb10b_perf_update_sys_hem_cwd_dg_map_mask(struct gk20a *g, u32 gr_instance_id,
					     u32 *sys_dg_map_mask,
					     u32 *start_hem_idx, u32 *num_hems);
u32 gb10b_perf_get_gpc_tpc0_cau0_dg_idx(void);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
