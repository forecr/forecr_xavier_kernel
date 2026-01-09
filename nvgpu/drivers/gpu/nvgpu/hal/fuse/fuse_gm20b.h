/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FUSE_GM20B_H
#define NVGPU_FUSE_GM20B_H

struct gk20a;

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) || defined(CONFIG_NVGPU_DGPU)
/* FIXME: below functions are needed for dGPU safety build. */
int gm20b_fuse_check_priv_security(struct gk20a *g);
u32 gm20b_fuse_status_opt_gpc(struct gk20a *g);
#endif
u32 gm20b_fuse_status_opt_fbio(struct gk20a *g);
u32 gm20b_fuse_status_opt_fbp(struct gk20a *g);
u32 gm20b_fuse_status_opt_l2_fbp(struct gk20a *g, u32 fbp);
u32 gm20b_fuse_status_opt_tpc_gpc(struct gk20a *g, u32 gpc);
void gm20b_fuse_ctrl_opt_tpc_gpc(struct gk20a *g, u32 gpc, u32 val);
u32 gm20b_fuse_opt_sec_debug_en(struct gk20a *g);
u32 gm20b_fuse_opt_priv_sec_en(struct gk20a *g);

#endif /* NVGPU_FUSE_GM20B_H */
