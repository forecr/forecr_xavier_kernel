/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INIT_GV100_H
#define NVGPU_GR_INIT_GV100_H

#include <nvgpu/types.h>

struct gk20a;

u32 gv100_gr_init_get_bundle_cb_default_size(struct gk20a *g);
u32 gv100_gr_init_get_min_gpm_fifo_depth(struct gk20a *g);
u32 gv100_gr_init_get_bundle_cb_token_limit(struct gk20a *g);
u32 gv100_gr_init_get_attrib_cb_default_size(struct gk20a *g);
u32 gv100_gr_init_get_alpha_cb_default_size(struct gk20a *g);
u32 gv100_gr_init_get_attrib_cb_gfxp_default_size(struct gk20a *g);
u32 gv100_gr_init_get_attrib_cb_gfxp_size(struct gk20a *g);

u32 gv100_gr_init_get_ctx_spill_size(struct gk20a *g);
u32 gv100_gr_init_get_ctx_betacb_size(struct gk20a *g);

#endif /* NVGPU_GR_INIT_GV100_H */
