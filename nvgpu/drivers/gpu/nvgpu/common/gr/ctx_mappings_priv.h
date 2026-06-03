/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
