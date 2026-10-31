/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FUSE_GB10B_H
#define NVGPU_FUSE_GB10B_H

struct gk20a;

int gb10b_fuse_read_gcplex_config_fuse(struct gk20a *g, u32 *val);
int gb10b_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi);

u32 gb10b_fuse_status_opt_cpc_gpc(struct gk20a *g, u32 gpc);
int gb10b_fuse_init_hw(struct gk20a *g);
void gb10b_feature_override_ofa_err_contain_en(struct gk20a *g);
void gb10b_feature_override_nvenc_err_contain_en(struct gk20a *g);

#endif /* NVGPU_FUSE_GB10B_H */
