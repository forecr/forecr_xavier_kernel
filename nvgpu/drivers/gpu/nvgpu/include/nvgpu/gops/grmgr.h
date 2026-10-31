/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GOPS_GRMGR_H
#define NVGPU_GOPS_GRMGR_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * GR MANAGER unit HAL interface
 *
 */
struct gk20a;

/**
 * GR MANAGER unit HAL operations
 *
 * @see gpu_ops
 */
struct gops_grmgr {
	/**
	 * @brief Initialize GR Manager unit.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*init_gr_manager)(struct gk20a *g);

	/**
	 * @brief Query GPU physical->logical gpc ids.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param num_gpc [in]		Number of GPCs.
	 * @param gpcs [out]		Pointer to GPC Id information struct.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*discover_gpc_ids)(struct gk20a *g, u32 num_gpc,
		struct nvgpu_gpc *gpcs);

	/**
	 * @brief Remove GR Manager unit.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*remove_gr_manager)(struct gk20a *g);

	/**
	 * @brief Get gpc group information.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 */
	void (*get_gpcgrp_count)(struct gk20a *g);

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_MIG)
	u32 (*get_max_sys_pipes)(struct gk20a *g);
	const struct nvgpu_mig_gpu_instance_config* (*get_mig_config_ptr)(
			struct gk20a *g);
	u32 (*get_allowed_swizzid_size)(struct gk20a *g);
	int (*get_gpc_instance_gpcgrp_id)(struct gk20a *g,
			u32 gpu_instance_id, u32 gr_syspipe_id, u32 *gpcgrp_id);
	int (*get_mig_gpu_instance_config)(struct gk20a *g,
		const char **config_name, u32 *num_config_supported);
	bool (*is_gpc_gfx_capable)(struct gk20a *g, u32 physical_gpc_id);
	void (*set_syspipe_gfx_or_compute)(struct gk20a *g, u32 syspipe_id,
			bool is_gfx_supported);
	u32 (*get_runlist_config_reg_off)(void);
	void (*add_grce_syspipe_gfx)(struct gk20a *g, u32 syspipe_id,
			struct nvgpu_runlist *runlist,
			bool is_gfx_supported);
	void (*setup_veids_mask)(struct gk20a *g, u32 index);
	int (*choose_mig_vpr_setting)(struct gk20a *g, bool *out_swizzid_checks,
			u32 *out_gfx_swizzid);
	int (*setup_vpr)(struct gk20a *g);
#endif
	int (*setup_ltc_partition)(struct gk20a *g, u32 syspipe_id);
	void (*setup_fbp_id_mapping)(struct gk20a *g, struct nvgpu_gpu_instance *gpu_instance);
	void (*load_timestamp_prod)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_GRMGR_H */
