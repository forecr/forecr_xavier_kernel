/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FUSE_GA10B_H
#define NVGPU_FUSE_GA10B_H

#define GA10B_FUSE_READ_DEVICE_IDENTIFIER_RETRIES 100000U

struct gk20a;
struct nvgpu_fuse_feature_override_ecc;

int ga10b_fuse_read_gcplex_config_fuse(struct gk20a *g, u32 *val);
bool ga10b_fuse_is_opt_ecc_enable(struct gk20a *g);
bool ga10b_fuse_is_opt_feature_override_disable(struct gk20a *g);
u32 ga10b_fuse_status_opt_gpc(struct gk20a *g);
u32 ga10b_fuse_status_opt_fbio(struct gk20a *g);
u32 ga10b_fuse_status_opt_fbp(struct gk20a *g);
u32 ga10b_fuse_status_opt_l2_fbp(struct gk20a *g, u32 fbp);
u32 ga10b_fuse_status_opt_tpc_gpc(struct gk20a *g, u32 gpc);
#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
void ga10b_fuse_ctrl_opt_tpc_gpc(struct gk20a *g, u32 gpc, u32 val);
#endif
u32 ga10b_fuse_opt_priv_sec_en(struct gk20a *g);
u32 ga10b_fuse_opt_sm_ttu_en(struct gk20a *g);
#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
void ga10b_fuse_write_feature_override_ecc(struct gk20a *g, u32 val);
void ga10b_fuse_write_feature_override_ecc_1(struct gk20a *g, u32 val);
#endif
void ga10b_fuse_read_feature_override_ecc(struct gk20a *g,
		struct nvgpu_fuse_feature_override_ecc *ecc_feature);
int ga10b_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi);
u32 ga10b_fuse_opt_sec_debug_en(struct gk20a *g);
#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
u32 ga10b_fuse_status_opt_emc(struct gk20a *g);
#endif
u32 ga10b_fuse_opt_secure_source_isolation_en(struct gk20a *g);
int ga10b_fuse_check_priv_security(struct gk20a *g);
int ga10b_fetch_falcon_fuse_settings(struct gk20a *g, u32 falcon_id,
		unsigned long *fuse_settings);
u32 ga10b_fuse_status_opt_pes_gpc(struct gk20a *g, u32 gpc);
u32 ga10b_fuse_status_opt_rop_gpc(struct gk20a *g, u32 gpc);

#endif /* NVGPU_FUSE_GA10B_H */
