/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_OBJ_CTX_PRIV_H
#define NVGPU_GR_OBJ_CTX_PRIV_H

#include <nvgpu/types.h>
#include <nvgpu/lock.h>

struct nvgpu_gr_global_ctx_local_golden_image;

/**
 * Graphics specific context register values structure.
 *
 * This structure stores init values for some of the registers that need to be
 * configured differently for Graphics contexts.
 */
struct nvgpu_gr_obj_ctx_gfx_regs {
	u32 reg_sm_disp_ctrl;
	u32 reg_gpcs_setup_debug;
	u32 reg_tex_lod_dbg;
	u32 reg_hww_warp_esr_report_mask;
};

/**
 * Golden context image descriptor structure.
 *
 * This structure stores details of the Golden context image.
 */
struct nvgpu_gr_obj_ctx_golden_image {
	/**
	 * Flag to indicate if Golden context image is ready or not.
	 */
	bool ready;

	/**
	 * Mutex to hold for accesses to Golden context image.
	 */
	struct nvgpu_mutex ctx_mutex;

	/**
	 * Size of Golden context image.
	 */
	size_t size;

	/**
	 * Pointer to local Golden context image struct.
	 */
	struct nvgpu_gr_global_ctx_local_golden_image *local_golden_image;

	/**
	 * Init values for graphics specific registers.
	 */
	struct nvgpu_gr_obj_ctx_gfx_regs gfx_regs;

#ifdef CONFIG_NVGPU_GR_GOLDEN_CTX_VERIFICATION
	/**
	 * Pointer to local Golden context image struct used for Golden
	 * context verification.
	 */
	struct nvgpu_gr_global_ctx_local_golden_image *local_golden_image_copy;
#endif
};

#ifdef CONFIG_NVGPU_DEBUGGER
struct ctxsw_map_entry {
	u32 addr;
	u32 offset;
};

struct nvgpu_priv_addr_map {
	u32 map_size;
	struct ctxsw_map_entry *map;
	u32 num_entries;
	bool init_done;
};
#endif
#endif /* NVGPU_GR_OBJ_CTX_PRIV_H */
