// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#ifndef NVGPU_GB20C_PERF
#define NVGPU_GB20C_PERF

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;

u32 gb20c_perf_get_cblock_instance_count(void);
u32 gb20c_perf_get_hwpm_fbp_fbpgs_ltcs_base_addr(void);
u32 gb20c_perf_get_gpc_tpc_start_dg_idx(void);
u32 gb20c_perf_get_gpc_tpc_end_dg_idx(void);
void gb20c_perf_get_num_hwpm_perfmon(struct gk20a *g, u32 *num_sys_perfmon,
				     u32 *num_fbp_perfmon, u32 *num_gpc_perfmon);
void gb20c_update_sys_dg_map_status_mask(struct gk20a *g,
					 u32 gr_instance_id, u32 *sys_dg_map_mask);
bool gb20c_perf_is_perfmon_simulated(void);
int gb20c_perf_set_secure_config_for_ccu_prof(struct gk20a *g, u32 gr_instance_id,
					     u32 pma_channel_id, bool dg_enable,
					     bool is_ctx_resident, struct nvgpu_tsg *tsg);
u32 gb20c_perf_get_num_cwd_hems(struct gk20a *g);
void gb20c_perf_update_sys_hem_cwd_dg_map_mask(struct gk20a *g, u32 gr_instance_id,
					     u32 *sys_dg_map_mask,
					     u32 *start_hem_idx, u32 *num_hems);
u32 gb20c_perf_get_gpc_tpc0_cau0_dg_idx(void);
u32 gb20c_perf_get_ctx_switched_pma_channel_count(struct gk20a *g);

u32 gb20c_perf_get_sysrouter_channel_stride(void);
u32 gb20c_perf_get_pmasys_channel_stride(void);
u32 gb20c_perf_get_pmasys_cmdslice_stride(void);
u32 gb20c_perf_get_pmasys_fwd_channel_stride(void);
u32 gb20c_perf_get_pmmfbprouter_per_chiplet_offset(void);
void gb20c_perf_get_sys_perfmon_range(u32 *perfmon_start_idx,
					u32 *perfmon_end_idx);
u32 gb20c_perf_get_sys_perfmon_stride(void);
void gb20c_perf_get_tpc_perfmon_range(u32 *perfmon_start_idx,
	u32 *perfmon_end_idx);
u32 gb20c_perf_get_max_num_gpc_perfmons(void);
int gb20c_perf_supports_extended_b2cc(struct gk20a *g, struct nvgpu_tsg *tsg,
				     bool *is_ctx_resident);
int gb20c_perf_write_hwpm_register(struct gk20a *g, u32 reg_offset, u32 val,
				    struct nvgpu_tsg *tsg, bool is_ctx_resident, u32 mask);
int gb20c_perf_read_hwpm_register(struct gk20a *g, u32 reg_offset, u32 *val,
				 struct nvgpu_tsg *tsg, bool is_ctx_resident, u32 mask);
int gb20c_perf_reset_pm_trigger_masks(struct gk20a *g, u32 pma_channel_id,
				     u32 gr_instance_id, u32 reservation_id,
				     struct nvgpu_tsg *tsg, bool is_ctx_resdient);
int gb20c_perf_enable_pm_trigger(struct gk20a *g, u32 gr_instance_id,
				u32 pma_channel_id, u32 reservation_id, struct nvgpu_tsg *tsg,
				bool is_ctx_resident);
int gb20c_perf_enable_hes_event_trigger(struct gk20a *g, u32 gr_instance_id,
				       u32 pma_channel_id, struct nvgpu_tsg *tsg,
				       bool is_ctx_resident);
int gb20c_perf_enable_pma_trigger(struct gk20a *g, u32 pma_channel_id,
				 struct nvgpu_tsg *tsg, bool is_ctx_resident);
int gb20c_perf_update_get_put(struct gk20a *g, u32 pma_channel_id,
			u64 bytes_consumed, bool update_available_bytes, u64 *put_ptr,
			bool *overflowed, struct nvgpu_tsg *tsg, bool is_ctx_resident,
			u32 *ctx_bytes_available);
int gb20c_perf_get_membuf_overflow_status(struct gk20a *g, u32 pma_channel_id,
				bool *overflowed, struct nvgpu_tsg *tsg,
				bool is_ctx_resident);
int gb20c_perf_disable_ccuprof(struct gk20a *g,
	u32 gr_instance_id, bool is_ctxsw, u32 reservation_id);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
