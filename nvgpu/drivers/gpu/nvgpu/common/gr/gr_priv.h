/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_PRIV_H
#define NVGPU_GR_PRIV_H

#include <nvgpu/types.h>
#include <nvgpu/cond.h>

struct nvgpu_gr_ctx_desc;
struct nvgpu_gr_global_ctx_buffer_desc;
struct nvgpu_gr_obj_ctx_golden_image;
struct nvgpu_gr_config;
#ifdef CONFIG_NVGPU_GRAPHICS
struct nvgpu_gr_zbc;
struct nvgpu_gr_zcull;
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
struct nvgpu_gr_hwpm_map;
#endif

/**
 * GR engine data structure.
 *
 * This is the parent structure to all other GR engine data structures,
 * and holds a pointer to all of them. This structure also stores
 * various fields to track GR engine initialization state.
 *
 * Pointer to this structure is maintained in GPU driver structure.
 */
struct nvgpu_gr {
	/**
	 * Pointer to GPU driver struct.
	 */
	struct gk20a *g;

	/**
	 * Instance ID of GR engine.
	 */
	u32 instance_id;

	/**
	 * GR falcons instances
	 */
	struct nvgpu_falcon *fecs_flcn;
	struct nvgpu_falcon *gpccs_flcn;

	/**
	 * Condition variable for GR initialization.
	 * Waiters shall wait on this condition to ensure GR engine
	 * is initialized.
	 */
	struct nvgpu_cond init_wq;

	/**
	 * Flag to indicate if GR engine is initialized.
	 */
	bool initialized;

	/**
	 * Syspipe ID of the GR instance.
	 */
	u32 syspipe_id;

	/**
	 * Pointer to global context buffer descriptor structure.
	 */
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer;

	/**
	 * Pointer to Golden context image structure.
	 */
	struct nvgpu_gr_obj_ctx_golden_image *golden_image;

	/**
	 * Pointer to GR context descriptor structure.
	 */
	struct nvgpu_gr_ctx_desc *gr_ctx_desc;

	/**
	 * Pointer to GR configuration structure.
	 */
	struct nvgpu_gr_config *config;

	/**
	 * Pointer to GR falcon data structure.
	 */
	struct nvgpu_gr_falcon *falcon;

	/**
	 * Pointer to GR interrupt data structure.
	 */
	struct nvgpu_gr_intr *intr;

	/**
	 * Function pointer to remove GR s/w support.
	 */
	void (*remove_support)(struct gk20a *g);

	/**
	 * Flag to indicate GR s/w has been initialized.
	 */
	bool sw_ready;

#ifdef CONFIG_NVGPU_DEBUGGER
	struct nvgpu_gr_hwpm_map *hwpm_map;
	struct nvgpu_priv_addr_map *pri_addr_map;
#endif

#ifdef CONFIG_NVGPU_GRAPHICS
	struct nvgpu_gr_zcull *zcull;

	struct nvgpu_gr_zbc *zbc;
#endif

#ifdef CONFIG_NVGPU_NON_FUSA
	u32 fecs_feature_override_ecc_val;
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
	bool can_broadcast_stop_trigger;
#endif

#ifdef CONFIG_NVGPU_CILP
	bool cilp_preempt_pending;
	u32 cilp_preempt_pending_gfid;
	u32 cilp_preempt_pending_tsgid;
#endif

#if defined(CONFIG_NVGPU_RECOVERY) || defined(CONFIG_NVGPU_DEBUGGER)
	struct nvgpu_mutex ctxsw_disable_mutex;
	int ctxsw_disable_count;
#endif
	unsigned long *enabled_gr_class_flags;
	bool is_support_gfx;

	struct nvgpu_mutex pending_mmu_nack_mutex;
	u32 pending_mmu_nack_ctx_id;

	bool dump_gr_hw_state_on_ctxsw_timeout;
};

#endif /* NVGPU_GR_PRIV_H */

