/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GOPS_PROFILER_H
#define NVGPU_GOPS_PROFILER_H

#ifdef CONFIG_NVGPU_PROFILER
struct gops_pm_reservation {
	int (*acquire)(struct gk20a *g, u32 gpu_instance_id, u32 reservation_id,
		enum nvgpu_profiler_pm_resource_type pm_resource,
		enum nvgpu_profiler_pm_reservation_scope scope,
		u32 vmid, u32 *pma_channel_id);
	int (*release)(struct gk20a *g, u32 gpu_instance_id, u32 reservation_id,
		enum nvgpu_profiler_pm_resource_type pm_resource,
		u32 vmid, u32 pma_channel_id);
	void (*release_all_per_vmid)(struct gk20a *g, u32 gpu_instance_id, u32 vmid);
};
struct gops_profiler {
	int (*bind_hwpm)(struct gk20a *g,
			u32 gr_instance_id,
			bool is_ctxsw,
			struct nvgpu_tsg *tsg, bool streamout,
			u32 reservation_id);
	int (*unbind_hwpm)(struct gk20a *g,
			u32 gr_instance_id,
			bool is_ctxsw,
			struct nvgpu_tsg *tsg);
	int (*bind_pma_streamout)(struct gk20a *g,
			u32 gr_instance_id,
			u32 pma_channel_id,
			u64 pma_buffer_va,
			u32 pma_buffer_size,
			u64 pma_bytes_available_buffer_va,
			u32 reservation_id);
	int (*unbind_pma_streamout)(struct gk20a *g,
			u32 gr_instance_id,
			u32 pma_channel_id,
			bool is_ctxsw,
			struct nvgpu_tsg *tsg,
			void *pma_bytes_available_buffer_cpuva,
			u32 reservation_id);
	int (*bind_smpc)(struct gk20a *g,
			u32 gr_instance_id,
			bool is_ctxsw,
			struct nvgpu_tsg *tsg,
			u32 reservation_id);
	int (*unbind_smpc)(struct gk20a *g,
			u32 gr_instance_id,
			bool is_ctxsw,
			struct nvgpu_tsg *tsg,
			u32 reservation_id);
	void (*bind_hes_cwd)(struct gk20a *g, u32 gr_instance_id,
			u32 reservation_id, u32 pma_channel_id);
	int (*unbind_hes_cwd)(struct gk20a *g,
			u32 gr_instance_id,
			bool is_ctxsw);
	void (*bind_ccuprof)(struct gk20a *g, u32 gr_instance_id,
			u32 reservation_id, u32 pma_channel_id);
	int (*unbind_ccuprof)(struct gk20a *g,
			u32 gr_instance_id,
			bool is_ctxsw);
	u32 (*get_hs_credit_per_gpc_chiplet)(struct gk20a *g);
	u32 (*get_hs_credit_per_fbp_chiplet)(struct gk20a *g);
	u32 (*get_hs_credit_per_sys_pipe_for_profiling)(struct gk20a *g);
	u32 (*get_hs_credit_per_sys_pipe_for_hes)(struct gk20a *g);
	void (*set_hs_credit_per_chiplet)(struct gk20a *g, u32 gr_instance_id,
			u32 pma_channel_id, u32 chiplet_type,
			u32 chiplet_local_index, u32 num_of_credits);
};
#endif

#endif /* NVGPU_GOPS_PROFILER_H */
