/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
