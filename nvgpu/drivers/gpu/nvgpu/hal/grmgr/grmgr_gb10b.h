/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

/* GB10B GR MANAGER */

#ifndef NVGPU_GRMGR_GB10B_H
#define NVGPU_GRMGR_GB10B_H

struct gk20a;

#ifdef CONFIG_NVGPU_MIG
const struct nvgpu_mig_gpu_instance_config *gb10b_grmgr_get_mig_config_ptr(
		                struct gk20a *g);
u32 gb10b_grmgr_get_allowed_swizzid_size(struct gk20a *g);
bool gb10b_grmgr_is_gpc_gfx_capable(struct gk20a *g, u32 logical_gpc_id);
void gb10b_grmgr_set_syspipe_gfx_or_compute(struct gk20a *g, u32 syspipe_id,
				bool is_gfx_supported);
u32 gb10b_runlist_config_reg_off(void);
void gb10b_grmgr_add_grce_syspipe_gfx(struct gk20a *g, u32 syspipe_id,
				struct nvgpu_runlist *runlist,
				bool is_gfx_supported);
void gb10b_grmgr_setup_veids_mask(struct gk20a *g, u32 index);
int gb10b_grmgr_choose_mig_vpr_setting(struct gk20a *g, bool *out_swizzid_checks,
				u32 *out_gfx_swizzid);
int gb10b_grmgr_setup_vpr(struct gk20a *g);
#endif

int gb10b_grmgr_setup_ltc_partition(struct gk20a *g, u32 syspipe_id);
void gb10b_grmgr_setup_fbp_id_mapping(struct gk20a *g, struct nvgpu_gpu_instance *gpu_instance);

#endif /* NVGPU_GRMGR_GB10B_H */
