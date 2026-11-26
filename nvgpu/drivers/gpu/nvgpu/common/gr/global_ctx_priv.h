/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_GLOBAL_CTX_PRIV_H
#define NVGPU_GR_GLOBAL_CTX_PRIV_H

/**
 * Global context buffer descriptor structure.
 *
 * This structure stores properties applicable to each global
 * context buffer.
 */
struct nvgpu_gr_global_ctx_buffer_desc {
	/**
	 * Memory to hold global context buffer.
	 */
	struct nvgpu_mem mem;

	/**
	 * Size of global context buffer.
	 */
	size_t size;

	/**
	 * Cacheability flags for mapping this context buffer.
	 */
	u32 mapping_flags;

	/**
	 * Function pointer to free global context buffer.
	 */
	global_ctx_mem_destroy_fn destroy;
};

/**
 * Local Golden context image descriptor structure.
 *
 * This structure stores details of a local Golden context image.
 * Pointer to this struct is maintained in
 * #nvgpu_gr_obj_ctx_golden_image structure.
 */
struct nvgpu_gr_global_ctx_local_golden_image {
	/**
	 * Pointer to local Golden context image memory.
	 */
	u32 *context;

	/**
	 * Size of local Golden context image.
	 */
	size_t size;
};

#endif /* NVGPU_GR_GLOBAL_CTX_PRIV_H */
