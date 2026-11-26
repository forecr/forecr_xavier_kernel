/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GRMGR_GA100_H
#define NVGPU_GRMGR_GA100_H

struct gk20a;

const struct nvgpu_mig_gpu_instance_config *ga100_grmgr_get_mig_config_ptr(
		struct gk20a *g);
u32 ga100_grmgr_get_max_sys_pipes(struct gk20a *g);
u32 ga100_grmgr_get_allowed_swizzid_size(struct gk20a *g);
int ga100_grmgr_get_gpc_instance_gpcgrp_id(struct gk20a *g,
		u32 gpu_instance_id, u32 gr_syspipe_id, u32 *gpcgrp_id);

#endif /* NVGPU_GRMGR_GA100_H */
