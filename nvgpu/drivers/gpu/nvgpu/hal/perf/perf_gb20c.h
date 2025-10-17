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
					      u32 pma_channel_id, bool dg_enable);
u32 gb20c_perf_get_num_cwd_hems(void);
void gb20c_perf_update_sys_hem_cwd_dg_map_mask(struct gk20a *g, u32 gr_instance_id,
					     u32 *sys_dg_map_mask,
					     u32 *start_hem_idx, u32 *num_hems);
u32 gb20c_perf_get_gpc_tpc0_cau0_dg_idx(void);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
