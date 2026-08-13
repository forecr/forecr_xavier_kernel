/*
 * Copyright (c) 2019-2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NVGPU_GR_CTX_PRIV_H
#define NVGPU_GR_CTX_PRIV_H

struct nvgpu_mem;

/**
 * Patch context buffer descriptor structure.
 *
 * Pointer to this structure is maintained in #nvgpu_gr_ctx structure.
 */
struct patch_desc {
	/**
	 * Count of entries written into patch context buffer.
	 */
	u32 data_count;
};

#ifdef CONFIG_NVGPU_GRAPHICS
struct zcull_ctx_desc {
	u64 gpu_va;
	u32 ctx_sw_mode;
};
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
struct pm_ctx_desc {
	u32 pm_mode;
	bool mapped;
};
#endif

/**
 * GR context descriptor structure.
 *
 * This structure stores various properties of all GR context buffers.
 */
struct nvgpu_gr_ctx_desc {
	/**
	 * Array to store all GR context buffer sizes.
	 */
	u32 size[NVGPU_GR_CTX_COUNT];

#ifdef CONFIG_NVGPU_GRAPHICS
	bool force_preemption_gfxp;
#endif

#ifdef CONFIG_NVGPU_CILP
	bool force_preemption_cilp;
#endif

#ifdef CONFIG_DEBUG_FS
	bool dump_ctxsw_stats_on_channel_close;
#endif
};

/**
 * Graphics context buffer structure.
 *
 * This structure stores all the properties of a graphics context
 * buffer. One graphics context is allocated per GPU Time Slice
 * Group (TSG).
 */
struct nvgpu_gr_ctx {
	/**
	 * Context ID read from graphics context buffer.
	 */
	u32 ctx_id;

	/**
	 * Flag to indicate if above context ID is valid or not.
	 */
	bool ctx_id_valid;

	/**
	 * Array to store all GR context buffers.
	 */
	struct nvgpu_mem mem[NVGPU_GR_CTX_COUNT];

	/**
	 * Cacheability flags for mapping the context buffers.
	 */
	u32 mapping_flags[NVGPU_GR_CTX_COUNT];

	/**
	 * Pointer to structure that holds GPU mapping of context buffers.
	 * These mappings will exist for the lifetime of TSG when the
	 * subcontexts are not enabled.
	 */
	struct nvgpu_gr_ctx_mappings *mappings;

	/**
	 * Patch context buffer descriptor struct.
	 */
	struct patch_desc	patch_ctx;

#ifdef CONFIG_NVGPU_GRAPHICS
	struct zcull_ctx_desc	zcull_ctx;
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	struct pm_ctx_desc	pm_ctx;
#endif

	/**
	 * Graphics preemption mode of the graphics context.
	 */
	u32 graphics_preempt_mode;

	/**
	 * Compute preemption mode of the graphics context.
	 */
	u32 compute_preempt_mode;

#ifdef CONFIG_NVGPU_NON_FUSA
	bool golden_img_loaded;
#endif

#ifdef CONFIG_NVGPU_CILP
	bool cilp_preempt_pending;
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
	bool boosted_ctx;
#endif

	/**
	 * TSG identifier corresponding to the graphics context.
	 */
	u32 tsgid;

	bool ctx_initialized;

#ifdef CONFIG_NVGPU_SM_DIVERSITY
	/** SM diversity configuration offset.
	 * It is valid only if NVGPU_SUPPORT_SM_DIVERSITY support is true.
	 * else input param is just ignored.
	 * A valid offset starts from 0 to
	 * (#gk20a.max_sm_diversity_config_count - 1).
	 */
	u32 sm_diversity_config;
#endif

	bool global_ctx_buffers_patched;
	bool preempt_buffers_patched;
	bool default_compute_regs_patched;
	bool default_gfx_regs_patched;
};

#endif /* NVGPU_GR_CTX_PRIV_H */
