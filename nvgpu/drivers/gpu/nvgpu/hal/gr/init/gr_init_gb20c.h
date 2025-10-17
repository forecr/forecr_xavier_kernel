/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INIT_GB20C_H
#define NVGPU_GR_INIT_GB20C_H

#include <nvgpu/types.h>

struct gk20a;

u32 gb20c_gr_init_get_attrib_cb_default_size(struct gk20a *g);
u32 gb20c_gr_init_get_min_gpm_fifo_depth(struct gk20a *g);
u32 gb20c_gr_init_get_bundle_cb_token_limit(struct gk20a *g);
bool gb20c_is_cwd_gpc_tpc_id_removed(void);

#ifdef CONFIG_NVGPU_GRAPHICS
u32 gb20c_gr_init_get_attrib_cb_gfxp_size(struct gk20a *g);
u32 gb20c_gr_init_get_ctx_spill_size(struct gk20a *g);
u32 gb20c_gpcs_rops_crop_debug1_off(void);
#endif /* CONFIG_NVGPU_GRAPHICS */

#endif /* NVGPU_GR_INIT_GB20C_H */
