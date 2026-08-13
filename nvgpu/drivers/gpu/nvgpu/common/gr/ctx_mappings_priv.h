/*
 * Copyright (c) 2022-2023, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_GR_CTX_MAPPINGS_PRIV_H
#define NVGPU_GR_CTX_MAPPINGS_PRIV_H

#include <nvgpu/types.h>

struct nvgpu_tsg;
struct vm_gk20a;

struct nvgpu_gr_ctx_mappings {

	/** TSG whose gr ctx mappings are tracked in this object */
	struct nvgpu_tsg *tsg;

	/** GPU virtual address space to which gr ctx buffers are mapped */
	struct vm_gk20a *vm;

	/**
	 * Array to store GPU virtual addresses of all TSG context
	 * buffers.
	 */
	u64	ctx_buffer_va[NVGPU_GR_CTX_COUNT];

	/**
	 * Array to store GPU virtual addresses of all global context
	 * buffers.
	 */
	u64	global_ctx_buffer_va[NVGPU_GR_GLOBAL_CTX_VA_COUNT];

	/**
	 * Array to store indexes of global context buffers
	 * corresponding to GPU virtual addresses above.
	 */
	u32	global_ctx_buffer_index[NVGPU_GR_GLOBAL_CTX_VA_COUNT];

	/**
	 * GR ctx mappings' entry in TSG's (#nvgpu_tsg) mappings list
	 * #gr_ctx_mappings_list.
	 */
	struct nvgpu_list_node tsg_entry;

	/**
	 * List of GR subcontexts (#nvgpu_gr_subctx) using this mapping.
	 */
	struct nvgpu_list_node subctx_list;
};

int nvgpu_gr_ctx_mappings_map_global_ctx_buffers(struct gk20a *g,
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer,
	bool support_gfx,
	struct nvgpu_gr_ctx_mappings *mappings,
	bool vpr);
void nvgpu_gr_ctx_mappings_unmap_global_ctx_buffers(
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer,
	struct nvgpu_gr_ctx_mappings *mappings);
void nvgpu_gr_ctx_mappings_unmap_ctx_buffer(struct nvgpu_gr_ctx *ctx,
	u32 index, struct nvgpu_gr_ctx_mappings *mappings);

#endif /* NVGPU_GR_CTX_MAPPINGS_PRIV_H */
