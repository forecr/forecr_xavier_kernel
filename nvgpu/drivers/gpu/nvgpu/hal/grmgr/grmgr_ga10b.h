/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GRMGR_GA10B_H
#define NVGPU_GRMGR_GA10B_H

struct gk20a;

#ifdef CONFIG_NVGPU_MIG
const struct nvgpu_mig_gpu_instance_config *ga10b_grmgr_get_mig_config_ptr(
		struct gk20a *g);
u32 ga10b_grmgr_get_allowed_swizzid_size(struct gk20a *g);
int ga10b_grmgr_init_gr_manager(struct gk20a *g);
u32 ga10b_grmgr_get_max_sys_pipes(struct gk20a *g);
int ga10b_grmgr_get_gpc_instance_gpcgrp_id(struct gk20a *g,
		u32 gpu_instance_id, u32 gr_syspipe_id, u32 *gpcgrp_id);
int ga10b_grmgr_remove_gr_manager(struct gk20a *g);
int ga10b_grmgr_get_mig_gpu_instance_config(struct gk20a *g,
		const char **config_name,
		u32 *num_config_supported);
void ga10b_grmgr_get_gpcgrp_count(struct gk20a *g);
#endif

void ga10b_grmgr_load_smc_arb_timestamp_prod(struct gk20a *g);
int ga10b_grmgr_discover_gpc_ids(struct gk20a *g,
		u32 num_gpc, struct nvgpu_gpc *gpcs);

#endif /* NVGPU_GRMGR_GA10B_H */
