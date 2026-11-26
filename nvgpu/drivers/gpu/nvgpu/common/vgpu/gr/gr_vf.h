/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_VF_H
#define NVGPU_GR_VF_H

struct gk20a;

bool vgpu_vf_gr_set_mmu_nack_pending(struct gk20a *g, u32 gr_instance_id,
		u32 gr_ctx_id);
int vgpu_vf_gr_clear_sm_error_state(struct gk20a *g,
		struct nvgpu_channel *ch, u32 sm_id);
int vgpu_vf_gr_suspend_contexts(struct gk20a *g,
		struct dbg_session_gk20a *dbg_s,
		int *ctx_resident_ch_fd);
int vgpu_vf_gr_resume_contexts(struct gk20a *g,
		struct dbg_session_gk20a *dbg_s,
		int *ctx_resident_ch_fd);

#endif
