/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INIT_TU104_H
#define NVGPU_GR_INIT_TU104_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_ctx;
struct netlist_av64_list;
struct nvgpu_gr_ctx_mappings;

u32 tu104_gr_init_get_bundle_cb_default_size(struct gk20a *g);
u32 tu104_gr_init_get_min_gpm_fifo_depth(struct gk20a *g);
u32 tu104_gr_init_get_bundle_cb_token_limit(struct gk20a *g);
u32 tu104_gr_init_get_attrib_cb_default_size(struct gk20a *g);
u32 tu104_gr_init_get_alpha_cb_default_size(struct gk20a *g);

int tu104_gr_init_load_sw_bundle64(struct gk20a *g,
		struct netlist_av64_list *sw_bundle64_init);

#ifdef CONFIG_NVGPU_GRAPHICS
u32 tu104_gr_init_get_rtv_cb_size(struct gk20a *g);
void tu104_gr_init_commit_rtv_cb(struct gk20a *g, u64 addr,
	struct nvgpu_gr_ctx *gr_ctx, bool patch);
#endif /* CONFIG_NVGPU_GRAPHICS */

#ifdef CONFIG_NVGPU_GFXP
void tu104_gr_init_commit_gfxp_rtv_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, struct nvgpu_gr_ctx_mappings *mappings,
	bool patch);

u32 tu104_gr_init_get_attrib_cb_gfxp_default_size(struct gk20a *g);
u32 tu104_gr_init_get_attrib_cb_gfxp_size(struct gk20a *g);
u32 tu104_gr_init_get_ctx_spill_size(struct gk20a *g);
u32 tu104_gr_init_get_ctx_betacb_size(struct gk20a *g);

u32 tu104_gr_init_get_gfxp_rtv_cb_size(struct gk20a *g);
#endif /* CONFIG_NVGPU_GFXP */

#endif /* NVGPU_GR_INIT_TU104_H */
