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
#ifndef NVGPU_TSG_SUBCTX_H
#define NVGPU_TSG_SUBCTX_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_tsg;
struct nvgpu_tsg_subctx;
struct nvgpu_channel;

/**
 * @brief Check if the TSG subcontext has channels bound to it.
 *
 * @param tsg [in]		Pointer to TSG struct.
 * @param subctx_id [in]	Subcontext ID.
 *
 * - Loop through the #subctx_list in #tsg to check if the subctx
 *   exists for the provided subctx_id.
 * - If it exists, check if the channels list is empty or not and set
 *   chs_bound accordingly.
 * - Else return false.
 *
 * @return false if subcontext does not exist for supplied subctx_id.
 * @return false if subcontext exists for supplied subctx_id and ch list
 *		 is empty.
 * @return true  if subcontext exists for supplied subctx_id and ch list
 *		 is not empty.
 */
bool nvgpu_tsg_subctx_has_channels_bound(struct nvgpu_tsg *tsg, u32 subctx_id);

/**
 * @brief Bind a channel to the TSG subcontext.
 *
 * @param tsg [in]		Pointer to TSG struct.
 * @param ch [in]		Pointer to Channel struct.
 *
 * - Loop through the #subctx_list in #tsg to check if the subctx
 *   exists for the provided channel.
 * - If it exists, validate the channel VM with subctx VM.
 * - If validated, add the channel to the subctx #ch_list and exit.
 * - Else allocate and initialize new subctx structure.
 * - Add the channel to the subctx #ch_list and add subctx to the
 *   TSG #subctx_list.
 *
 * @return 0 for successful bind or if subctx support is disabled,
 *         < 0 for failure.
 * @retval -EINVAL if channel VM doesn't match with subctx VM for provided
 *                 subctx_id.
 * @retval -ENOMEM if subctx allocation fails.
 */
int nvgpu_tsg_subctx_bind_channel(struct nvgpu_tsg *tsg,
				  struct nvgpu_channel *ch);

/**
 * @brief Unbind a channel from the TSG subcontext.
 *
 * @param tsg [in]		Pointer to TSG struct.
 * @param ch [in]		Pointer to Channel struct.
 * @param force [in]		Free the VEID if force is true.
 *
 * - Validate that #subctx is allocated for the channel #ch.
 * - Remove the channel from the subctx #ch_list.
 * - If the subctx #ch_list is empty
 *   - Free the VEID corresponding to the channel if force is true.
 *   - Update the instance blocks of all channels to remove the
 *     subctx pdb.
 *   - Invoke g->ops.gr.setup.free_subctx to free the GR subcontext
 *     struct (and GR subcontext mappings struct).
 *   - Remove the subctx from the TSG #subctx_list.
 *   - Free the subctx memory. If this was the only active channel
 *     in the TSG this function will delete the objects in the
 *     sequence: mappings -> gr_subctx -> tsg_subctx
 */
void nvgpu_tsg_subctx_unbind_channel(struct nvgpu_tsg *tsg,
				     struct nvgpu_channel *ch, bool force);

/**
 * @brief Allocate GR subcontext for a TSG subcontext.
 *
 * @param g [in]		Pointer to gk20a struct.
 * @param ch [in]		Pointer to Channel struct.
 *
 * - Check if TSG subctx is allocated for the channel.
 * - If not allocated, return error.
 * - If allocated, and if GR subcontext is not allocated call
 *   #nvgpu_gr_subctx_alloc.
 *
 * @return 0 for successful allocation, < 0 for failure.
 */
int nvgpu_tsg_subctx_alloc_gr_subctx(struct gk20a *g, struct nvgpu_channel *ch);

/**
 * @brief Allocate and map GR subcontext header for a TSG subcontext.
 *
 * @param g [in]		Pointer to gk20a struct.
 * @param ch [in]		Pointer to Channel struct.
 *
 * - Check if TSG and GR subctx is allocated for the channel.
 * - If not allocated, return error.
 * - If allocated, setup subcontext header by calling
 *   #nvgpu_gr_subctx_setup_header.
 *
 * @return 0 for successful allocation, < 0 for failure.
 */
int nvgpu_tsg_subctx_setup_subctx_header(struct gk20a *g,
					 struct nvgpu_channel *ch);

/**
 * @brief Get GR subcontext for a TSG subcontext.
 *
 * @param tsg_subctx [in]	Pointer to TSG Subcontext struct.
 *
 * - Return #gr_subctx from #nvgpu_tsg_subctx.
 */
struct nvgpu_gr_subctx *nvgpu_tsg_subctx_get_gr_subctx(
				struct nvgpu_tsg_subctx *tsg_subctx);

/**
 * @brief Get id of a TSG subcontext.
 *
 * @param tsg_subctx [in]	Pointer to TSG Subcontext struct.
 *
 * - Return #subctx_id from #nvgpu_tsg_subctx.
 */
u32 nvgpu_tsg_subctx_get_id(struct nvgpu_tsg_subctx *tsg_subctx);

/**
 * @brief Set replayable state for a TSG subcontext.
 *
 * @param subctx [in]		Pointer to TSG subcontext.
 * @param Replayable [in]	replayable state for the subcontext.
 *
 * - Set #replayable in #nvgpu_tsg_subctx.
 */
void nvgpu_tsg_subctx_set_replayable(struct nvgpu_tsg_subctx *subctx,
				     bool replayable);

/**
 * @brief Get replayable state for a TSG subcontext.
 *
 * @param subctx [in]		Pointer to TSG subcontext.
 *
 * - Return #replayable from #nvgpu_tsg_subctx.
 */
bool nvgpu_tsg_subctx_get_replayable(struct nvgpu_tsg_subctx *subctx);

/**
 * @brief Get VM for a TSG subcontext.
 *
 * @param subctx [in]		Pointer to TSG subcontext.
 *
 * - Return #vm from #nvgpu_tsg_subctx.
 */
struct vm_gk20a *nvgpu_tsg_subctx_get_vm(struct nvgpu_tsg_subctx *subctx);

/**
 * @brief Allocate or get the mappings struct for the TSG subcontext.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param tsg [in]		Pointer to TSG struct.
 * @param ch [in]		Pointer to Channel struct.
 *
 * This function allocates the mappings struct for subcontext corresponding
 * to given Channel's VM if not available already else returns the same.
 * It adds the gr_subctx corresponding to the channel the mapping object's
 * subctx_list.
 *
 * @return mappings struct in case of success, null in case of failure.
 */
struct nvgpu_gr_ctx_mappings *nvgpu_tsg_subctx_alloc_or_get_mappings(
				struct gk20a *g,
				struct nvgpu_tsg *tsg,
				struct nvgpu_channel *ch);

#ifdef CONFIG_NVGPU_GFXP
/**
 * @brief Program preemption buffer virtual addresses for all subcontexts.
 *
 * @param tsg_subctx [in]		Pointer to TSG subcontext struct.
 *
 * - Checks if VEID0 mappings are available.
 * - If available, program the preemption buffer virtual addresses
 *   (VEID0 VA and VA in subcontext VM) for all GR subcontexts'
 *   headers.
 */
void nvgpu_tsg_subctxs_set_preemption_buffer_va(
			struct nvgpu_tsg_subctx *tsg_subctx);

/**
 * @brief Clear preemption buffer virtual addresses for all subcontexts.
 *
 * @param tsg_subctx [in]		Pointer to TSG subcontext struct.
 *
 * - Program the preemption buffer virtual addresses
 *   (VEID0 VA and VA in subcontext VM) for all GR subcontexts'
 *   headers to 0.
 */
void nvgpu_tsg_subctxs_clear_preemption_buffer_va(
			struct nvgpu_tsg_subctx *tsg_subctx);
#endif /* CONFIG_NVGPU_GFXP */

#ifdef CONFIG_NVGPU_DEBUGGER
/**
 * @brief Program PM buffer virtual addresses for all subcontexts.
 *
 * @param tsg [in]			Pointer to TSG struct.
 * @param set_pm_ctx_gpu_va [in]	Indicates if PM ctx buffer GPU VA
 *					is to be programmed.
 *
 * - Program the PM buffer virtual address for all GR subcontexts' headers.
 */
void nvgpu_tsg_subctxs_set_pm_buffer_va(struct nvgpu_tsg *tsg,
					bool set_pm_ctx_gpu_va);
#endif /* CONFIG_NVGPU_DEBUGGER */

bool nvgpu_tsg_channel_type_active(struct nvgpu_tsg *tsg,
				   bool match_subctx, u32 subctx_id,
				   bool match_pbdma, u32 pbdma_id,
				   bool (*is_valid_class)(u32 class_num));

#endif /* NVGPU_TSG_SUBCTX_H */
