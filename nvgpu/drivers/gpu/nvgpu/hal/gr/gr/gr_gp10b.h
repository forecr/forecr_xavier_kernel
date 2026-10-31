/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_GP10B_H
#define NVGPU_GR_GP10B_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_gr_ctx;
struct dbg_session_gk20a;
struct nvgpu_debug_context;

#define NVC097_BES_CROP_DEBUG4_CLAMP_FP_BLEND_TO_INF	0x0U
#define NVC097_BES_CROP_DEBUG4_CLAMP_FP_BLEND_TO_MAXVAL 0x1U

int gr_gp10b_set_cilp_preempt_pending(struct gk20a *g, u32 gfid, u32 tsgid);
void gr_gp10b_set_bes_crop_debug3(struct gk20a *g, u32 data);
void gr_gp10b_set_bes_crop_debug4(struct gk20a *g, u32 data);
void gr_gp10b_set_alpha_circular_buffer_size(struct gk20a *g, u32 data);
void gr_gp10b_set_circular_buffer_size(struct gk20a *g, u32 data);
int gr_gp10b_dump_gr_status_regs(struct gk20a *g,
			   struct nvgpu_debug_context *o);
#ifdef CONFIG_NVGPU_TEGRA_FUSE
void gr_gp10b_set_gpc_tpc_mask(struct gk20a *g, u32 gpc_index);
#endif
u32 gp10b_gr_get_sm_hww_warp_esr(struct gk20a *g,
			u32 gpc, u32 tpc, u32 sm);
int gr_gp10b_suspend_contexts(struct gk20a *g,
				struct dbg_session_gk20a *dbg_s,
				int *ctx_resident_ch_fd);
#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
int gr_gp10b_set_boosted_ctx(struct nvgpu_channel *ch,
				    bool boost);
#endif
int gp10b_gr_fuse_override(struct gk20a *g);
bool gr_gp10b_suspend_context(struct nvgpu_channel *ch,
				bool *cilp_preempt_pending);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_GP10B_H */
