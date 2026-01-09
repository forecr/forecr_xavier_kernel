/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PROF_VF_H
#define NVGPU_PROF_VF_H

struct gk20a;

int vf_profiler_bind_hwpm(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg, bool streamout,
		u32 reservation_id);
int vf_profiler_unbind_hwpm(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg);

int vf_gb10b_profiler_bind_smpc(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		u32 reservation_id);
int vf_gb10b_profiler_unbind_smpc(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		u32 reservation_id);
void vf_profiler_bind_hes_cwd(struct gk20a *g,
		u32 gr_instance_id,
		u32 reservation_id,
		u32 pma_channel_id);
void vf_profiler_set_hs_credit_per_chiplet(struct gk20a *g,
		u32 gr_instance_id,
		u32 pma_channel_id,
		u32 chiplet_type,
		u32 chiplet_local_index,
		u32 num_of_credits);
int vf_profiler_bind_pma_streamout(struct gk20a *g,
		u32 gr_instance_id,
		u32 pma_channel_id,
		u64 pma_buffer_va,
		u32 pma_buffer_size,
		u64 pma_bytes_available_buffer_va,
		u32 reservation_id);
int vf_profiler_unbind_pma_streamout(struct gk20a *g,
		u32 gr_instance_id,
		u32 pma_channel_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		void *pma_bytes_available_buffer_cpuva,
		u32 reservation_id);
#endif
