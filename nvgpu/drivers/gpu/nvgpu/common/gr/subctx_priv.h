/*
 * Copyright (c) 2019-2023, NVIDIA CORPORATION.  All rights reserved.
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
