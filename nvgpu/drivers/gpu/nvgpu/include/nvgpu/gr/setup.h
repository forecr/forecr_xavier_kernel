/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GR_SETUP_H
#define NVGPU_GR_SETUP_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * common.gr.setup unit interface
 */
struct gk20a;
struct nvgpu_channel;
struct vm_gk20a;
struct nvgpu_gr_ctx;

/** Supports only mission (default) context. */
#define NVGPU_DEFAULT_SM_DIVERSITY_CONFIG_COUNT 1U

/** Max SM diversity configuration count.
 * Offset 0 for mission (default) context.
 * Offset 1 for redundant context.
 */
#define NVGPU_MAX_SM_DIVERSITY_CONFIG_COUNT 2U

/**
 * @brief Allocate and setup object context s/w image for GPU channel.
 *        Application requires that a channel context is created and
 *        initialized before any work is submitted to GR HW.
 *
 * @param c [in]		Pointer to GPU channel.
 * @param class_num [in]	GPU class ID.
 * @param flags [in]		Flags for context allocation.
 *
 * This function allocates and sets up object context for a GPU channel.
 * The steps include:
 *
 * - If \ref CONFIG_NVGPU_VPR is set:
 *   - If args->flags has the \ref NVGPU_OBJ_CTX_FLAGS_SUPPORT_VPR bit set:
 *     set c->vpr to true.
 * - Validating GPU class ID in parameter list.
 * - Allocating GR engine context image.
 * - Allocating subcontext image.
 * - Allocating patch context image.
 * - Creating Golden context image upon first request to allocate
 *   object context.
 * - Capturing golden context image twice and ensuring contents match.
 * - Initializing context preemption mode.
 * - Initializing various other fields in context image.
 * - Mapping global context buffers into context image.
 * - Committing the context image into channel instance block.
 *
 * Note that if requested class is a valid class but not a COMPUTE class, this
 * function will not do anything but just return success. All graphics classes
 * are treated as invalid classes in safety.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ENOMEM if memory allocation fails for any context image.
 * @retval -ENOMEM if contents from two golden image captures do not match.
 * @retval -ETIMEDOUT if GR engine idle times out.
 * @retval -EINVAL if invalid GPU class ID is provided.
 *
 * @see gops_class for list of valid classes.
 * @see nvgpu_gr_obj_ctx_alloc.
 * @see nvgpu_gr_obj_ctx_alloc_golden_ctx_image.
 */
int nvgpu_gr_setup_alloc_obj_ctx(struct nvgpu_channel *c, u32 class_num,
		u32 flags);

/**
 * @brief Free GR engine context image.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param gr_ctx [in]		Pointer to GR engine context image.
 *
 * This function will free memory allocated for patch context image and
 * GR engine context image in #nvgpu_gr_setup_alloc_obj_ctx().
 */
void nvgpu_gr_setup_free_gr_ctx(struct gk20a *g,
		struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Free GR engine subcontext.
 *
 * @param c [in]		Pointer to GPU channel.
 *
 * This function will free memory allocated for GR engine subcontext
 * image in #nvgpu_gr_setup_alloc_obj_ctx().
 */
void nvgpu_gr_setup_free_subctx(struct nvgpu_channel *c);

/**
 * @brief Validate preemption mode in GR engine context image in case
 *        Application optionally wants to change default preemption mode.
 *
 * @param graphics_preempt_mode [in]	Requested graphics preemption mode.
 * @param compute_preempt_mode [in]	Requested compute preemption mode.
 * @param gr_ctx [in]			Pointer to GR engine context image.
 *
 * @return true in case of success, false in case of failure.
 */
bool nvgpu_gr_setup_validate_preemption_mode(u32 *graphics_preempt_mode,
				u32 *compute_preempt_mode,
				struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Setup preemption mode in GR engine context image in case
 *        Application optionally wants to change default preemption mode.
 *
 * @param ch [in]			Pointer to GPU channel.
 * @param graphics_preempt_mode [in]	Requested graphics preemption mode.
 * @param compute_preempt_mode [in]	Requested compute preemption mode.
 * @param gr_instance_id [in]		Instance ID of the graphics engine.
 *
 * This function will program newly requested preemption modes into
 * GR engine context image. This function is typically needed if user
 * application needs to change default preemption modes already set in
 * #nvgpu_gr_setup_alloc_obj_ctx().
 *
 * Note that if requested preemption modes are already set, this
 * function will return 0.
 *
 * The function supports NVGPU_PREEMTION_MODE_GRAPHICS_WFI graphics
 * preemption mode and NVGPU_PREEMTION_MODE_COMPUTE_WFI,
 * NVGPU_PREEMTION_MODE_COMPUTE_CTA compute preemption modes as
 * valid preemption modes.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -EINVAL if invalid preemption modes are provided.
 * @retval -EINVAL if invalid GPU channel pointer is provided.
 *
 * @see nvgpu_gr_ctx_check_valid_preemption_mode.
 * @see nvgpu_gr_obj_ctx_set_ctxsw_preemption_mode.
 * @see nvgpu_gr_obj_ctx_update_ctxsw_preemption_mode.
 */
int nvgpu_gr_setup_set_preemption_mode(struct nvgpu_channel *ch,
		u32 graphics_preempt_mode, u32 compute_preempt_mode,
		u32 gr_instance_id);

#ifdef CONFIG_NVGPU_DEBUGGER
/**
 * @brief Setup PRI address buffer to capture registers context
 *        switched over ramchain.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param inst_blcok [in]	Pointer to memory describing instance block.
 * @param gr_ctx [in]		Pointer to GR engine context image.
 * @param golden_image[in]	Pointer to the golden image context descriptor.
 *
 * This function will send a method to FECS ucode to save the pri
 * address map into a local buffer. The size of the buffer is
 * expected to be equal to golden context buffer. Content would
 * be a mirror of the context buffer but with addresses where there
 * is PRI data. The pri address map is stored onto a local datastructure
 * which is then consumed during regops.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ENOMEM if any memory allocation operations failed.
 */
int nvgpu_gr_setup_pri_address_buffer(struct gk20a *g,
		struct nvgpu_mem *inst_block, struct nvgpu_gr_ctx *gr_ctx,
		struct nvgpu_gr_obj_ctx_golden_image *golden_image);
#endif

#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
int nvgpu_gr_setup_bind_ctxsw_zcull(struct gk20a *g, struct nvgpu_channel *c,
			u64 zcull_va, u32 mode);
#endif

#endif /* NVGPU_GR_SETUP_H */
