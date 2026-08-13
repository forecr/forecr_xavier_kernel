/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INIT_GA100_H
#define NVGPU_GR_INIT_GA100_H

#include <nvgpu/types.h>

#define FECS_CTXSW_RESET_DELAY_US 10U

struct gk20a;
struct nvgpu_gr_ctx;




void ga100_gr_init_set_sm_l1tag_surface_collector(struct gk20a *g);
#ifdef CONFIG_NVGPU_GRAPHICS
u32 ga100_gr_init_get_attrib_cb_gfxp_default_size(struct gk20a *g);
u32 ga100_gr_init_get_attrib_cb_gfxp_size(struct gk20a *g);
u32 ga100_gr_init_get_ctx_spill_size(struct gk20a *g);
u32 ga100_gr_init_get_ctx_betacb_size(struct gk20a *g);
#endif
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void ga100_gr_init_commit_global_bundle_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, u64 addr, u32 size, bool patch);
void ga100_gr_init_override_context_reset(struct gk20a *g);
u32 ga100_gr_init_get_min_gpm_fifo_depth(struct gk20a *g);
u32 ga100_gr_init_get_bundle_cb_token_limit(struct gk20a *g);
u32 ga100_gr_init_get_attrib_cb_default_size(struct gk20a *g);
#endif /* CONFIG_NVGPU_HAL_NON_FUSA */
#endif /* NVGPU_GR_INIT_GA100_H */
