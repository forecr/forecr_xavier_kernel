/*
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_COMMON_FIFO_TSG_SUBCTX_PRIV_H
#define NVGPU_COMMON_FIFO_TSG_SUBCTX_PRIV_H

#include <nvgpu/types.h>
#include <nvgpu/list.h>

struct nvgpu_tsg;
struct vm_gk20a;
struct nvgpu_gr_subctx;

struct nvgpu_tsg_subctx {

	/** Subcontext Id (aka. veid). */
	u32 subctx_id;

	/** TSG to which this subcontext belongs. */
	struct nvgpu_tsg *tsg;

	/** Subcontext's address space. */
	struct vm_gk20a *vm;

	/** Subcontext's GR ctx header and GR ctx buffers mappings. */
	struct nvgpu_gr_subctx *gr_subctx;

	/** Replayable faults state for a subcontext. */
	bool replayable;

	/**
	 * Subcontext's entry in TSG's (#nvgpu_tsg) subcontexts list
	 * #subctx_list.
	 */
	struct nvgpu_list_node tsg_entry;

	/**
	 * List of channels (#nvgpu_channel) bound to this TSG subcontext.
	 * Accessed by holding #ch_list_lock from TSG.
	 */
	struct nvgpu_list_node ch_list;
};

#endif /* NVGPU_COMMON_FIFO_TSG_SUBCTX_PRIV_H */
