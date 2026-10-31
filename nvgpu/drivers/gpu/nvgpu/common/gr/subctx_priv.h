/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_SUBCTX_PRIV_H
#define NVGPU_GR_SUBCTX_PRIV_H

struct nvgpu_mem;

/**
 * GR subcontext data structure.
 */
struct nvgpu_gr_subctx {
	/**
	 * Memory to hold subcontext header image.
	 */
	struct nvgpu_mem ctx_header;

	/**
	 * GPU mappings of the GR ctx buffers for this subcontext.
	 */
	struct nvgpu_gr_ctx_mappings *mappings;

	/**
	 * GR subcontext's entry in gr ctx mappings' (#nvgpu_gr_ctx_mappings)
	 * subcontexts list #subctx_list.
	 */
	struct nvgpu_list_node gr_ctx_mappings_entry;
};

void nvgpu_golden_ctx_gr_subctx_free(struct gk20a *g,
		struct nvgpu_gr_subctx *gr_subctx, struct vm_gk20a *vm);

#endif /* NVGPU_GR_SUBCTX_PRIV_H */
