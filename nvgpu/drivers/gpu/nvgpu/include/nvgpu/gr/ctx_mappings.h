/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_CTX_MAPPINGS_H
#define NVGPU_GR_CTX_MAPPINGS_H

struct gk20a;
struct nvgpu_tsg;
struct vm_gk20a;
struct nvgpu_gr_ctx;
struct nvgpu_gr_subctx;
struct nvgpu_gr_ctx_mappings;
struct nvgpu_gr_global_ctx_buffer_desc;

/**
 * @brief Create GR ctx buffers mappings for a TSG.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param tsg [in]		Pointer to TSG struct.
 * @param vm [in]		Pointer to vm struct.
 *
 * This function allocates the mappings struct for TSG corresponding to
 * given vm.
 *
 * @return mappings struct in case of success, null in case of failure.
 */
struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_mappings_create(struct gk20a *g,
				struct nvgpu_tsg *tsg, struct vm_gk20a *vm);

/**
 * @brief Free the GR ctx buffers mappings.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param mappings [in]		Pointer to GR ctx buffers mappings struct.
 *
 * This function frees the mappings struct.
 */
void nvgpu_gr_ctx_mappings_free(struct gk20a *g,
				struct nvgpu_gr_ctx_mappings *mappings);

/**
 * @brief Map GR context buffer and store in mappings struct.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param ctx [in]		Pointer to GR context struct.
 * @param index [in]		index of the buffer.
 * @param mappings [in]		Pointer to GR context buffer mappings struct.
 *
 * This function will map the GR context buffer at #index in #mappings->vm
 * and stores the mapped address.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_gr_ctx_mappings_map_ctx_buffer(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx, u32 index,
	struct nvgpu_gr_ctx_mappings *mappings);

/**
 * @brief Map GR context preemption buffers and store in mappings struct.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param ctx [in]		Pointer to GR context struct.
 * @param subctx [in]		Pointer to TSG subcontext struct.
 * @param mappings [in]		Pointer to GR context buffer mappings struct.
 *
 * This function will map the GR context preemption buffers in #mappings->vm
 * and stores the mapped address. For subcontext case NVGPU_GR_CTX_PREEMPT_CTXSW
 * buffer is mapped to all subcontexts.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
#ifdef CONFIG_NVGPU_GFXP
int nvgpu_gr_ctx_mappings_map_ctx_preemption_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx,
	struct nvgpu_tsg_subctx *subctx,
	struct nvgpu_gr_ctx_mappings *mappings);
#endif /* CONFIG_NVGPU_GFXP */
/**
 * @brief Map GR and global context buffers and store in mappings struct.
 *
 * @param g [in]			Pointer to GPU driver struct.
 * @param gr_ctx [in]			Pointer to GR context struct.
 * @param subctx [in]			Pointer to TSG subcontext struct.
 * @param global_ctx_buffer [in]	Pointer global context buffer desc.
 * @param mappings [in]			Pointer to GR context buffer
 *					mappings struct.
 * @param vpr [in]			Indicates if VPR buffer copy is to be
 *					mapped.
 *
 * This function will map the GR and global context buffers in #mappings->vm
 * and stores the mapped address.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_gr_ctx_mappings_map_gr_ctx_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, struct nvgpu_tsg_subctx *subctx,
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer,
	struct nvgpu_gr_ctx_mappings *mappings,
	bool vpr);

/**
 * @brief Unmap GR and global context buffers and store in mappings struct.
 *
 * @param g [in]			Pointer to GPU driver struct.
 * @param gr_ctx [in]			Pointer to GR context struct.
 * @param subctx [in]			Pointer to TSG subcontext struct.
 * @param global_ctx_buffer [in]	Pointer global context buffer desc.
 * @param mappings [in]			Pointer to GR context buffer
 *					mappings struct.
 *
 * This function will unmap the GR and global context buffers in #mappings->vm.
 */
void nvgpu_gr_ctx_unmap_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	struct nvgpu_tsg_subctx *subctx,
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer,
	struct nvgpu_gr_ctx_mappings *mappings);

/**
 * @brief Get global context buffer gpu virtual address.
 *
 * @param mappings [in]		Pointer to GR context buffer
 *				mappings struct.
 * @param index [in]		index of the buffer.
 *
 * This function will get the gpu virtual address of the global context buffer
 * in #mappings.
 *
 * @return gpu virtual address of global context buffer.
 */
u64 nvgpu_gr_ctx_mappings_get_global_ctx_va(struct nvgpu_gr_ctx_mappings *mappings,
	u32 index);

/**
 * @brief Get GR context buffer gpu virtual address.
 *
 * @param mappings [in]		Pointer to GR context buffer
 *				mappings struct.
 * @param index [in]		index of the buffer.
 *
 * This function will get the gpu virtual address of the GR context buffer
 * in #mappings.
 *
 * @return gpu virtual address of GR context buffer.
 */
u64 nvgpu_gr_ctx_mappings_get_ctx_va(struct nvgpu_gr_ctx_mappings *mappings,
	u32 index);

/**
 * @brief Get GR ctx buffers mappings for a TSG corresponding to VM.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param tsg [in]		Pointer to TSG struct.
 * @param vm [in]		Pointer to vm struct.
 *
 * This function retrieves the mappings struct for TSG corresponding to
 * given vm from #tsg->gr_ctx_mappings_list.
 *
 * @return mappings struct in case of success, null in case of failure.
 */
struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_mappings_get_subctx_mappings(
				struct gk20a *g,
				struct nvgpu_tsg *tsg,
				struct vm_gk20a *vm);

/**
 * @brief Allocate GR ctx buffers mappings for a TSG corresponding to VM.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param tsg [in]		Pointer to TSG struct.
 * @param vm [in]		Pointer to vm struct.
 *
 * This function allocates the mappings struct for TSG corresponding to
 * given vm and inserts in #tsg->gr_ctx_mappings_list.
 *
 * @return mappings struct in case of success, null in case of failure.
 */
struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_mappings_create_subctx_mappings(
				struct gk20a *g,
				struct nvgpu_tsg *tsg,
				struct vm_gk20a *vm);

/**
 * @brief Link GR subctx to mappings struct.
 *
 * @param mappings [in]		Pointer to GR context buffers mappings struct.
 * @param subctx [in]		Pointer to GR subcontext struct.
 *
 * This function checks and inserts the subctx in #mappings->subctx_list.
 */
void nvgpu_gr_ctx_mappings_add_gr_subctx(struct nvgpu_gr_ctx_mappings *mappings,
				   struct nvgpu_gr_subctx *subctx);

/**
 * @brief Free GR context buffers mappings struct for subcontexts.
 *
 * @param subctx [in]		Pointer to GR subcontext struct.
 * @param mappings [in]		Pointer to GR context buffers mappings struct.
 * @param unmap [in]		Indicates if the GR context buffers are to be
 *				unmapped. true in case of native nvgpu config,
 *				false in case of vgpu config. For vgpu case,
 *				this path is used to handle the VM references
 *				per subcontext.
 *
 * This function checks if the #mappings->subctx_list is empty and if empty,
 * unmaps the buffers and deletes the mappings.
 */
void nvgpu_gr_ctx_mappings_free_subctx_mappings(struct nvgpu_tsg_subctx *subctx,
			struct nvgpu_gr_ctx_mappings *mappings, bool unmap);

/**
 * @brief Map GR context buffer to all subcontext VMs.
 *
 * @param tsg [in]		Pointer to tsg struct.
 * @param index [in]		Index of the buffer to be mapped.
 *
 * This function maps the GR context buffer at #index to all VMs listed
 * in #tsg->gr_ctx_mappings_list.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
#if defined(CONFIG_NVGPU_GFXP) || defined(CONFIG_NVGPU_DEBUGGER)
int nvgpu_gr_ctx_mappings_map_buffer_all_subctx(
		struct nvgpu_tsg *tsg, u32 index);
#endif

#endif /* NVGPU_GR_CTX_MAPPINGS_H */
