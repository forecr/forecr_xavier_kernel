/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INIT_GB10B_H
#define NVGPU_GR_INIT_GB10B_H

#include <nvgpu/types.h>

#define FECS_CTXSW_RESET_DELAY_US 10U

struct gk20a;
struct nvgpu_gr_config;
struct nvgpu_gr_ctx;
struct nvgpu_gr_ctx_mappings;
struct nvgpu_gr_obj_ctx_gfx_regs;

#ifdef CONFIG_NVGPU_GRAPHICS
int gb10b_gr_init_rop_mapping(struct gk20a *g,
			      struct nvgpu_gr_config *gr_config);
u32 gb10b_gr_init_get_attrib_cb_gfxp_size(struct gk20a *g);
u32 gb10b_gr_init_get_ctx_spill_size(struct gk20a *g);
u32 gb10b_gr_init_get_ctx_betacb_size(struct gk20a *g);
void gb10b_gr_init_commit_rtv_cb(struct gk20a *g, u64 addr,
	struct nvgpu_gr_ctx *gr_ctx, bool patch);
u32 gb10b_gpcs_rops_crop_debug1_off(void);
#endif /* CONFIG_NVGPU_GRAPHICS */
void gb10b_gr_init_fs_state(struct gk20a *g);
int gb10b_gr_init_sm_id_config(struct gk20a *g, u32 *tpc_sm_id,
				struct nvgpu_gr_config *gr_config,
				struct nvgpu_gr_ctx *gr_ctx,
				bool patch);
u32 gb10b_gr_init_get_sm_id_size(void);
void gb10b_gr_init_sm_id_numbering(struct gk20a *g, u32 gpc, u32 tpc, u32 smid,
				struct nvgpu_gr_config *gr_config,
				struct nvgpu_gr_ctx *gr_ctx,
				bool patch);
u32 gb10b_gr_init_get_attrib_cb_default_size(struct gk20a *g);
u32 gb10b_gr_init_get_alpha_cb_default_size(struct gk20a *g);
void gb10b_gr_init_override_context_reset(struct gk20a *g);
void gb10b_gr_init_detect_sm_arch(struct gk20a *g);
void gb10b_gr_init_set_sm_l1tag_surface_collector(struct gk20a *g);
void gb10b_gr_init_commit_global_pagepool(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, u64 addr, size_t size, bool patch,
	 bool global_ctx);
#ifdef CONFIG_NVGPU_GFXP
void gb10b_gr_init_commit_gfxp_rtv_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, struct nvgpu_gr_ctx_mappings *mappings,
	bool patch);
void gb10b_gr_init_commit_cbes_reserve(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, bool patch);
#endif /* CONFIG_NVGPU_GFXP */
void gb10b_gr_init_gpc_mmu(struct gk20a *g);
void gb10b_gr_init_commit_global_attrib_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, struct nvgpu_gr_ctx_mappings *mappings,
	u32 tpc_count, u32 max_tpc, u64 addr, bool patch);
void gb10b_gr_init_commit_global_bundle_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, u64 addr, u32 size, bool patch);
void gb10b_gr_init_commit_global_cb_manager(struct gk20a *g,
	struct nvgpu_gr_config *config, struct nvgpu_gr_ctx *gr_ctx, bool patch);
void gb10b_gr_init_commit_global_timeslice(struct gk20a *g);
void gb10b_gr_init_capture_gfx_regs(struct gk20a *g, struct nvgpu_gr_obj_ctx_gfx_regs *gfx_regs);
u32 gb10b_gr_init_get_min_gpm_fifo_depth(struct gk20a *g);
u32 gb10b_gr_init_get_bundle_cb_token_limit(struct gk20a *g);
void gb10b_gr_init_commit_rops_crop_override(struct gk20a *g,
				struct nvgpu_gr_ctx *gr_ctx, bool patch);
void gb10b_gr_init_set_default_gfx_regs(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
		struct nvgpu_gr_obj_ctx_gfx_regs *gfx_regs);
void gb10b_gr_set_num_gfx_capable_tpcs(struct gk20a *g);
void gb10b_gr_init_set_gpcs_reset(struct gk20a *g, bool assert_reset);
void gb10b_gr_init_set_engine_reset(struct gk20a *g, bool assert_reset);
int gb10b_gr_init_fecs_pwr_blcg_force_on(struct gk20a *g, bool force_on);
#ifdef CONFIG_NVGPU_SET_FALCON_ACCESS_MAP
void gb10b_gr_init_get_access_map(struct gk20a *g,
				   u32 **gr_access_map,
				   u32 *gr_access_map_num_entries);
#endif
#endif /* NVGPU_GR_INIT_GB10B_H */
