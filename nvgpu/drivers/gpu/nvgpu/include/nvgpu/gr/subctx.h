/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_SUBCTX_H
#define NVGPU_GR_SUBCTX_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * common.gr.subctx unit interface
 */
struct gk20a;
struct vm_gk20a;
struct nvgpu_gr_ctx;
struct nvgpu_gr_subctx;
struct nvgpu_mem;
struct nvgpu_gr_ctx_mappings;

/**
 * @brief Allocate and map graphics subcontext context header buffer.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param subctx [in]		Pointer to graphics subcontext struct.
 * @param vm [in]		Pointer to virtual memory.
 *
 * This function allocates memory for subcontext header stored in
 * #nvgpu_gr_subctx structure.
 *
 * Subcontext header memory will be mapped to given virtual
 * memory.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_gr_subctx_setup_header(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx,
	struct vm_gk20a *vm);

/**
 * @brief Allocate graphics subcontext buffer.
 *
 * @param g [in]		Pointer to GPU driver struct.
 *
 * This function allocates memory for #nvgpu_gr_subctx structure.
 *
 * @return pointer to #nvgpu_gr_subctx struct in case of success,
 *         NULL in case of failure.
 */
struct nvgpu_gr_subctx *nvgpu_gr_subctx_alloc(struct gk20a *g);

/**
 * @brief Free graphics subcontext buffer.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param subctx [in]		Pointer to TSG subcontext struct.
 * @param vm [in]		Pointer to virtual memory.
 * @param unmap [in]		Indicates if GR context buffers and subctx
 *				buffer are to be unmapped.
 *				true in case of native nvgpu config and
 *				false in case of vgpu config. For vgpu case,
 *				this path is used to handle the VM references
 *				per subcontext.
 *
 * This function will free memory allocated for subcontext header and
 * #nvgpu_gr_subctx structure.
 */
void nvgpu_gr_subctx_free(struct gk20a *g,
	struct nvgpu_tsg_subctx *subctx,
	struct vm_gk20a *vm,
	bool unmap);

/**
 * @brief Initialize graphics subcontext buffer header.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param subctx [in]		Pointer to graphics subcontext struct.
 * @param gr_ctx [in]		Pointer to graphics context struct.
 * @param mappings [in]		GPU virtual address mappings of graphics
 *                              context buffers.
 *
 * This function will initialize graphics subcontext buffer header
 * by reading appropriate values from #nvgpu_gr_ctx structure and
 * then writing them into subcontext buffer header.
 *
 * This function will also program GPU virtual address of graphics
 * context buffer into subcontext buffer header.
 */
void nvgpu_gr_subctx_load_ctx_header(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx,
	struct nvgpu_gr_ctx *gr_ctx,
	struct nvgpu_gr_ctx_mappings *mappings);

/**
 * @brief Get pointer of subcontext header memory struct.
 *
 * @param subctx [in]		Pointer to graphics subcontext struct.
 *
 * This function returns #nvgpu_mem pointer of subcontext header stored
 * in #nvgpu_gr_subctx.
 *
 * @return pointer to subcontext header memory struct.
 */
struct nvgpu_mem *nvgpu_gr_subctx_get_ctx_header(struct nvgpu_gr_subctx *subctx);

/**
 * @brief Get pointer of GR context buffers mappings struct for a subcontext.
 *
 * @param subctx [in]		Pointer to graphics subcontext struct.
 *
 * This function returns #nvgpu_gr_ctx_mappings pointer of GR context buffers
 * mappings stored in #nvgpu_gr_subctx.
 *
 * @return pointer to subcontext GR context buffers mappings struct.
 */
#ifdef CONFIG_NVGPU_GFXP
struct nvgpu_gr_ctx_mappings *nvgpu_gr_subctx_get_mappings(
				struct nvgpu_gr_subctx *subctx);
#endif
#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
void nvgpu_gr_subctx_zcull_setup(struct gk20a *g, struct nvgpu_gr_subctx *subctx,
		struct nvgpu_gr_ctx *gr_ctx);
#endif /* CONFIG_NVGPU_GRAPHICS and CONFIG_NVGPU_ZCULL */

#ifdef CONFIG_NVGPU_GFXP
void nvgpu_gr_subctx_set_preemption_buffer_va(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx,
	struct nvgpu_gr_ctx_mappings *mappings);

void nvgpu_gr_subctx_clear_preemption_buffer_va(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx);
#endif /* CONFIG_NVGPU_GFXP */

#ifdef CONFIG_NVGPU_DEBUGGER
void nvgpu_gr_subctx_set_hwpm_ptr(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx,
	bool set_pm_ctx_gpu_va);
#endif
#endif /* NVGPU_GR_SUBCTX_H */
