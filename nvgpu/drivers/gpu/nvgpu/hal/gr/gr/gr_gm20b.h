/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
/*
 * GM20B GPC MMU
 */

#ifndef NVGPU_GM20B_GR_GM20B_H
#define NVGPU_GM20B_GR_GM20B_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_warpstate;
enum nvgpu_event_id_type;

u32 gr_gm20b_get_gr_status(struct gk20a *g);
int gm20b_gr_tpc_disable_override(struct gk20a *g, u32 mask);
int gr_gm20b_commit_global_cb_manager(struct gk20a *g,
			struct nvgpu_gr_ctx *gr_ctx, bool patch);
void gr_gm20b_set_alpha_circular_buffer_size(struct gk20a *g, u32 data);
void gr_gm20b_set_circular_buffer_size(struct gk20a *g, u32 data);
void gr_gm20b_init_sm_dsm_reg_info(void);
void gr_gm20b_get_sm_dsm_perf_regs(struct gk20a *g,
					  u32 *num_sm_dsm_perf_regs,
					  u32 **sm_dsm_perf_regs,
					  u32 *perf_register_stride);
void gr_gm20b_get_sm_dsm_perf_ctrl_regs(struct gk20a *g,
					       u32 *num_sm_dsm_perf_ctrl_regs,
					       u32 **sm_dsm_perf_ctrl_regs,
					       u32 *ctrl_register_stride);
#ifdef CONFIG_NVGPU_TEGRA_FUSE
void gr_gm20b_set_gpc_tpc_mask(struct gk20a *g, u32 gpc_index);
#endif
int gr_gm20b_dump_gr_status_regs(struct gk20a *g,
			   struct nvgpu_debug_context *o);
int gr_gm20b_update_pc_sampling(struct nvgpu_channel *c,
				       bool enable);
void gr_gm20b_init_cyclestats(struct gk20a *g);
void gr_gm20b_bpt_reg_info(struct gk20a *g, struct nvgpu_warpstate *w_state);
int gm20b_gr_clear_sm_error_state(struct gk20a *g,
		struct nvgpu_channel *ch, u32 sm_id);
void gm20b_gr_set_debug_mode(struct gk20a *g, bool enable);
int gm20b_gr_set_mmu_debug_mode(struct gk20a *g,
		struct nvgpu_channel *ch, bool enable, bool enable_gcc);
bool gm20b_gr_esr_bpt_pending_events(u32 global_esr,
				     enum nvgpu_event_id_type bpt_event);
u32 gm20b_get_gpcs_pri_mmu_debug_ctrl_reg(void);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GM20B_GR_GM20B_H */
