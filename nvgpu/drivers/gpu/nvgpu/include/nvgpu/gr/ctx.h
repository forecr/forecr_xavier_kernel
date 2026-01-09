/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_CTX_H
#define NVGPU_GR_CTX_H

#include <nvgpu/types.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/gr/global_ctx.h>
#include <nvgpu/gr/hwpm_map.h>

#define NVGPU_INVALID_SM_CONFIG_ID (U32_MAX)

/** Supports only mission (default) context. */
#define NVGPU_DEFAULT_SM_DIVERSITY_CONFIG 0U

/**
 * @file
 *
 * common.gr.ctx unit interface
 */
struct gk20a;
struct vm_gk20a;
struct nvgpu_tsg;
struct nvgpu_gr_ctx;
struct nvgpu_channel;
struct nvgpu_gr_ctx_mappings;
struct nvgpu_gr_global_ctx_buffer_desc;
struct nvgpu_gr_global_ctx_local_golden_image;
struct patch_desc;
struct pm_ctx_desc;
struct nvgpu_gr_ctx_desc;
#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
struct zcull_ctx_desc;
#endif

/*
 * allocate a minimum of 1 page (4KB) worth of patch space, this is 512 entries
 * of address and data pairs.
 */
/** Number of slots required in patch buffer per entry. */
#define PATCH_CTX_SLOTS_REQUIRED_PER_ENTRY	2U
/** Number of slots per NVGPU_CPU_PAGE_SIZE. */
#define PATCH_CTX_SLOTS_PER_PAGE \
	(NVGPU_CPU_PAGE_SIZE/(PATCH_CTX_SLOTS_REQUIRED_PER_ENTRY * (u32)sizeof(u32)))
/** Get number of entries in patch buffer given the size of buffer. */
#define PATCH_CTX_ENTRIES_FROM_SIZE(size) ((size)/sizeof(u32))

/** S/W defined value for preemption mode with no input. */
#define NVGPU_PREEMPTION_MODE_NONE		0U
/** S/W defined value for invalid preemption mode. */
#define NVGPU_PREEMPTION_MODE_INVALID		BIT32(31)

/** S/W defined value for WFI graphics preemption mode. */
#define NVGPU_PREEMPTION_MODE_GRAPHICS_WFI	BIT32(0)
#ifdef CONFIG_NVGPU_GFXP
/** S/W defined value for GFxP graphics preemption mode. */
#define NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP	BIT32(1)
#endif

/** S/W defined value for WFI compute preemption mode. */
#define NVGPU_PREEMPTION_MODE_COMPUTE_WFI	BIT32(0)
/** S/W defined value for CTA compute preemption mode. */
#define NVGPU_PREEMPTION_MODE_COMPUTE_CTA	BIT32(1)
#ifdef CONFIG_NVGPU_CILP
/** S/W defined value for CILP compute preemption mode. */
#define NVGPU_PREEMPTION_MODE_COMPUTE_CILP	BIT32(2)
#endif

/** S/W defined index for Graphics context buffer. */
#define NVGPU_GR_CTX_CTX		0U
#ifdef CONFIG_NVGPU_DEBUGGER
/** S/W defined index for PM context buffer. */
#define NVGPU_GR_CTX_PM_CTX		1U
#endif
/** S/W defined index for patch context buffer. */
#define NVGPU_GR_CTX_PATCH_CTX		2U
#ifdef CONFIG_NVGPU_GFXP
/** S/W defined index for preempt ctxsw buffer. */
#define NVGPU_GR_CTX_PREEMPT_CTXSW	3U
/** S/W defined index for spill ctxsw buffer. */
#define NVGPU_GR_CTX_SPILL_CTXSW	4U
/** S/W defined index for betacb ctxsw buffer. */
#define NVGPU_GR_CTX_BETACB_CTXSW	5U
/** S/W defined index for pagepool ctxsw buffer. */
#define NVGPU_GR_CTX_PAGEPOOL_CTXSW	6U
/** S/W defined index for rtvcb ctxsw buffer. */
#define NVGPU_GR_CTX_GFXP_RTVCB_CTXSW	7U
/** Number of context buffers. */
#define NVGPU_GR_CTX_COUNT		8U
#else
/** Number of context buffers. */
#define NVGPU_GR_CTX_COUNT		3U
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
/* PM Context Switch Modes */
/** This mode says that the pms are not to be context switched. */
#define NVGPU_GR_CTX_HWPM_CTXSW_MODE_NO_CTXSW               (0x00000000U)
/** This mode says that the pms in Mode-B are to be context switched. */
#define NVGPU_GR_CTX_HWPM_CTXSW_MODE_CTXSW                  (0x00000001U)
/** This mode says that the pms in Mode-E (stream out) are to be context switched. */
#define NVGPU_GR_CTX_HWPM_CTXSW_MODE_STREAM_OUT_CTXSW       (0x00000002U)
#endif

/**
 * @brief Initialize GR context descriptor structure.
 *
 * @param g [in]		Pointer to GPU driver struct.
 *
 * This function allocates memory for #nvgpu_gr_ctx_desc structure.
 *
 * @return pointer to #nvgpu_gr_ctx_desc struct in case of success,
 *         NULL in case of failure.
 */
struct nvgpu_gr_ctx_desc *nvgpu_gr_ctx_desc_alloc(struct gk20a *g);

/**
 * @brief Free GR context descriptor structure.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param desc [in]		Pointer to context descriptor struct.
 *
 * This function will free memory allocated for #nvgpu_gr_ctx_desc structure.
 */
void nvgpu_gr_ctx_desc_free(struct gk20a *g,
	struct nvgpu_gr_ctx_desc *desc);

/**
 * @brief Set size of GR context buffer with given index.
 *
 * @param desc [in]		Pointer to context descriptor struct.
 * @param index [in]		Index of GR context buffer.
 * @param size [in]		Size of buffer to be set.
 *
 * This function sets size of GR context buffer with given buffer
 * index. \a index must be less than NVGPU_GR_CTX_COUNT otherwise
 * an assert is raised.
 */
void nvgpu_gr_ctx_set_size(struct nvgpu_gr_ctx_desc *gr_ctx_desc,
	u32 index, u32 size);

/**
 * @brief Get size of GR context buffer with given index.
 *
 * @param desc [in]		Pointer to context descriptor struct.
 * @param index [in]		Index of GR context buffer.
 *
 * @return size of the buffer.
 *
 * This function returns the size of GR context buffer with given buffer
 * index. \a index must be less than NVGPU_GR_CTX_COUNT otherwise
 * an assert is raised.
 */
#ifdef CONFIG_NVGPU_GFXP
u32 nvgpu_gr_ctx_get_size(struct nvgpu_gr_ctx_desc *gr_ctx_desc,
	u32 index);
#endif

/**
 * @brief Free graphics context and associated resources.
 *
 * This function is responsible for freeing the graphics context and any
 * associated resources such as mappings and buffers. It also resets the
 * graphics context structure to zero. It ensures that all the lists in the TSG
 * structure are empty if #NVGPU_SUPPORT_TSG_SUBCONTEXTS is enabled.
 *
 * The steps performed by the function are as follows:
 * -# Check if the graphics context pointer is not NULL.
 * -# Retrieve the TSG structure associated with the graphics context using its
 *    TSG ID.
 * -# Acquire the mutex lock on the TSG context initialization lock.
 * -# If #NVGPU_SUPPORT_TSG_SUBCONTEXTS is enabled, assert that the channel list,
 *    subcontext list, and graphics context mappings list in the TSG structure
 *    are empty.
 * -# If #NVGPU_SUPPORT_TSG_SUBCONTEXTS is not enabled and the graphics context
 *    has valid mappings, unmap the buffers and free the mappings.
 * -# Reset the patch context data count in the graphics context to zero.
 * -# Free the graphics context buffers.
 * -# Reset the graphics context structure to zero.
 * -# Release the mutex lock on the TSG context initialization lock.
 *
 * @param [in]  g                       Pointer to the GPU driver structure.
 * @param [in]  gr_ctx                  Pointer to the graphics context structure.
 * @param [in]  global_ctx_buffer       Pointer to the global context buffer descriptor.
 *
 * @return None.
 */
void nvgpu_gr_ctx_free(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer);


/**
 * @brief Set data count in patch context buffer.
 *
 * @param gr_ctx [in]		Pointer to graphics context struct.
 * @param data_count [in]	Value to be set.
 *
 * This function sets data count of patch context buffer. Data count
 * indicates how many entries have been written into patch context.
 */
void nvgpu_gr_ctx_set_patch_ctx_data_count(struct nvgpu_gr_ctx *gr_ctx,
	u32 data_count);

/**
 * @brief Get context buffer mem struct of the given graphics context.
 *
 * @param gr_ctx [in]		Pointer to graphics context struct.
 * @param index [in]		Value from (NVGPU_GR_CTX_CTX, NVGPU_GR_CTX_GFXP_RTVCB_CTXSW)
 *
 * This function returns #mem of graphics context struct.
 *
 * @return context buffer mem of the given graphics context.
 */
struct nvgpu_mem *nvgpu_gr_ctx_get_ctx_mem(struct nvgpu_gr_ctx *gr_ctx, u32 index);

/**
 * @brief Get mapping flags of a context buffer of the given graphics context.
 *
 * @param gr_ctx [in]		Pointer to graphics context struct.
 * @param index [in]		Value from (NVGPU_GR_CTX_CTX, NVGPU_GR_CTX_GFXP_RTVCB_CTXSW)
 *
 * This function returns #mapping_flags of graphics context struct.
 *
 * @return context buffer mapping flags of the given graphics context.
 */
u32 nvgpu_gr_ctx_get_ctx_mapping_flags(struct nvgpu_gr_ctx *gr_ctx, u32 index);

#ifdef CONFIG_NVGPU_SM_DIVERSITY
/**
 * @brief Set sm diversity config in the given graphics context struct.
 *
 * @param gr_ctx [in]		Pointer to graphics context struct.
 * @param sm_diversity_config [in]	Value to be set.
 *
 * This function sets sm diversity config of the given graphics context struct.
 */
void nvgpu_gr_ctx_set_sm_diversity_config(struct nvgpu_gr_ctx *gr_ctx,
	u32 sm_diversity_config);

/**
 * @brief Get pointer of graphics context buffer memory struct.
 *
 * @param gr_ctx [in]		Pointer to graphics context struct.
 *
 * @return sm diversity config of the given graphics context struct.
 */
u32 nvgpu_gr_ctx_get_sm_diversity_config(struct nvgpu_gr_ctx *gr_ctx);
#endif

/**
 * @brief Load the local golden context image into the graphics context.
 *
 * This function loads the local golden context image into the specified graphics
 * context memory. It also initializes context switch program header data, sets
 * various context switch program configurations, and updates the patch count
 * and address. Depending on the build configuration, it may also set up
 * conditional dynamic evaluation (CDE) and performance monitoring unit (PMU)
 * options.
 *
 * The steps performed by the function are as follows:
 * -# Ignore the 'cde' parameter by casting it to void.
 * -# Get the pointer to the graphics context memory.
 * -# Load the local golden context image into the graphics context memory using
 *    nvgpu_gr_global_ctx_load_local_golden_image().
 * -# If #CONFIG_NVGPU_HAL_NON_FUSA is defined, initialize the context switch
 *    program header data.
 * -# If #CONFIG_NVGPU_DEBUGGER is defined and 'cde' is true, enable CDE for the
 *    context switch program.
 * -# If #CONFIG_NVGPU_SET_FALCON_ACCESS_MAP is defined, set the private access
 *    map configuration mode and address for the context switch program.
 * -# If #CONFIG_NVGPU_HAL_NON_FUSA is defined, disable verification features for
 *    the context switch program.
 * -# If #CONFIG_NVGPU_DEBUGGER is defined, set the PMU options for boost clock
 *    frequencies if the corresponding operation is available.
 * -# Set the patch count and address for the context switch program.
 * -# If #CONFIG_NVGPU_DEBUGGER is defined, set the performance monitoring mode
 *    and pointer for the context switch program.
 *
 * @param [in]  g                       Pointer to the GPU driver structure.
 * @param [in]  gr_ctx                  Pointer to the graphics context structure.
 * @param [in]  mappings                Pointer to the graphics context mappings.
 * @param [in]  local_golden_image      Pointer to the local golden context image.
 * @param [in]  cde                     Boolean flag for conditional dynamic
 *                                      evaluation (ignored in this function).
 *
 * @return None.
 */
void nvgpu_gr_ctx_load_golden_ctx_image(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	struct nvgpu_gr_ctx_mappings *mappings,
	struct nvgpu_gr_global_ctx_local_golden_image *local_golden_image,
	bool cde);

/**
 * @brief Prepare patch context buffer for writes.
 *
 * @param g [in]			Pointer to GPU driver struct.
 * @param gr_ctx [in]			Pointer to graphics context struct.
 * @param update_patch_count [in]	Boolean flag to update data count
 *                                      in patch context.
 *
 * This function will prepare patch context buffer for writes. This
 * function must be called before calling #nvgpu_gr_ctx_patch_write()
 * to add entries into the buffer.
 */
void nvgpu_gr_ctx_patch_write_begin(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	bool update_patch_count);

/**
 * @brief Finish patch context buffer writes.
 *
 * @param g [in]			Pointer to GPU driver struct.
 * @param gr_ctx [in]			Pointer to graphics context struct.
 * @param update_patch_count [in]	Boolean flag to update data count
 *                                      in patch context.
 *
 * This function will write final data count into patch context image
 * after all entries have been written by calling
 * #nvgpu_gr_ctx_patch_write().
 */
void nvgpu_gr_ctx_patch_write_end(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	bool update_patch_count);

/**
 * @brief Edit the context state.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param gr_ctx [in]		Pointer to graphics context struct.
 * @param addr [in]		Address entry to be added.
 * @param data [in]		Data entry to be added.
 * @param patch [in]		Boolean flag.
 *
 * This function will write given addr and data entries into patch
 * context buffer if boolean flag patch is set. In this case, the
 * context state will be updated on next context restore to h/w.
 *
 * If patch is not set, context is resident on h/w already and this
 * function will write data value into h/w register with address addr
 * directly.
 *
 * This function must be preceded by #nvgpu_gr_ctx_patch_write_begin()
 * and succeeded by #nvgpu_gr_ctx_patch_write_end().
 *
 * The steps performed by the function are as follows:
 * -# Check if patching is enabled.
 * -# If patching is enabled, proceed with the following steps:
 *    -# Verify that the graphics context pointer is not NULL.
 *    -# Get the pointer to the patch context memory from the graphics context.
 *    -# Calculate the current patch slot based on the data count and the
 *       number of slots required per entry.
 *    -# Calculate the maximum patch slot based on the size of the patch context
 *       memory and the number of slots required per entry.
 *    -# Verify that the current patch slot does not exceed the maximum patch
 *       slot.
 *    -# Write the address to the calculated patch slot in the patch context
 *       memory.
 *    -# Write the data to the next patch slot in the patch context memory.
 *    -# Increment the data count in the patch context.
 * -# If patching is not enabled, write the data directly to the hardware
 *    register at the given address.
 */
void nvgpu_gr_ctx_patch_write(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	u32 addr, u32 data, bool patch);

/**
 * @brief Set given compute preemption mode in graphics context struct.
 *
 * @param gr_ctx [in]			Pointer to graphics context struct.
 * @param compute_preempt_mode [in]	Compute preemption mode to be set.
 *
 * This function will set given compute preemption mode into #nvgpu_gr_ctx
 * structure.
 *
 * @see nvgpu_gr_ctx_check_valid_preemption_mode for valid compute preemption
 *      modes.
 */
void nvgpu_gr_ctx_init_compute_preemption_mode(struct nvgpu_gr_ctx *gr_ctx,
	u32 compute_preempt_mode);

/**
 * @brief Get compute preemption mode stored in graphics context struct.
 *
 * @param gr_ctx [in]		Pointer to graphics context struct.
 *
 * This function returns compute preemption mode stored in #nvgpu_gr_ctx
 * struct.
 *
 * @return compute preemption mode.
 */
u32 nvgpu_gr_ctx_get_compute_preemption_mode(struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Check if given preemption modes are valid.
 *
 * @param g [in]			Pointer to GPU driver struct.
 * @param ch [in]			Pointer to Channel struct.
 * @param gr_ctx [in]			Pointer to graphics context struct.
 * @param graphics_preempt_mode		Requested graphics preemption mode.
 * @param compute_preempt_mode		Requested compute preemption mode.
 *
 * This function checks if requested graphics/compute preemption modes are
 * valid or not.
 *
 * The function supports NVGPU_PREEMTION_MODE_GRAPHICS_WFI graphics
 * preemption mode and NVGPU_PREEMTION_MODE_COMPUTE_WFI,
 * NVGPU_PREEMTION_MODE_COMPUTE_CTA compute preemption modes as
 * valid preemption modes. Any other preemption mode is invalid.
 *
 * Requesting both CILP compute and GFxP graphics preemption modes at
 * the same time is not allowed.
 *
 * It is also not allowed to downgrade the preemption modes to lower
 * type.
 *
 * @return true if requested preemption modes are valid, false otherwise.
 */
bool nvgpu_gr_ctx_check_valid_preemption_mode(struct gk20a *g,
		struct nvgpu_channel *ch,
		struct nvgpu_gr_ctx *gr_ctx,
		u32 graphics_preempt_mode, u32 compute_preempt_mode);

/**
 * @brief Set the preemption modes in the graphics context.
 *
 * This function configures the graphics and compute preemption modes based on
 * the settings in the provided graphics context structure. It writes the
 * preemption mode settings to the graphics context memory.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the pointer to the graphics context memory.
 * -# If GFxP graphics preemption is enabled, set the graphics preemption mode
 *    to GFxP using set_graphics_preemption_mode_gfxp().
 * -# If CILP compute preemption is enabled, set the compute preemption mode
 *    to CILP using set_compute_preemption_mode_cilp().
 * -# If CTA compute preemption is enabled, set the compute preemption mode
 *    to CTA using set_compute_preemption_mode_cta().
 *
 * @Param [in] g - Pointer to the GPU driver structure.
 * @Param [in] gr_ctx - Pointer to the graphics context structure.
 *
 * There is no return value for this function.
 */
void nvgpu_gr_ctx_set_preemption_modes(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Initialize GR context structure.
 *
 * @param g [in]		Pointer to GPU driver struct.
 *
 * This function allocates memory for #nvgpu_gr_ctx structure.
 *
 * @return pointer to #nvgpu_gr_ctx struct in case of success,
 *         NULL in case of failure.
 */
struct nvgpu_gr_ctx *nvgpu_alloc_gr_ctx_struct(struct gk20a *g);

/**
 * @brief Free GR context structure.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param gr_ctx [in]		Pointer to graphics context struct.
 *
 * This function will free memory allocated for #nvgpu_gr_ctx structure.
 */
void nvgpu_free_gr_ctx_struct(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Free TSG specific GR context buffers.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param ctx [in]		Pointer to graphics context struct.
 *
 * This function frees all TSG specific GR context buffers.
 */
void nvgpu_gr_ctx_free_ctx_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx);

/**
 * @brief Allocate TSG specific GR context buffers.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param desc [in]		Pointer to context descriptor struct.
 * @param ctx [in]		Pointer to graphics context struct.
 *
 * This function allocates all TSG specific GR context buffers based on the
 * sizes specified in the graphics context descriptor. It also handles the
 * allocation of preemption buffers for GFXP mode if required. If the
 * allocation fails for any buffer, it frees all previously allocated context
 * buffers.
 *
 * The steps performed by the function are as follows:
 * -# Check if the size of the main graphics context buffer is set, return
 *    error if not set.
 * -# Iterate over each context buffer type defined by #NVGPU_GR_CTX_COUNT.
 * -# For each buffer type, check if the size is non-zero and the memory is not
 *    already valid.
 * -# If the above condition is true, allocate system memory for the context
 *    buffer.
 * -# If memory allocation fails, free all allocated context buffers and return
 *    error.
 * -# If the graphics context is not marked as initialized, set the context ID
 *    as invalid.
 * -# Return 0 to indicate success.
 *
 * @return 0 if all context buffers are allocated successfully.
 * @return -EINVAL if the main graphics context buffer size is not set.
 * @return Non-zero error code if memory allocation fails for any buffer.
 *
 */
int nvgpu_gr_ctx_alloc_ctx_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx_desc *desc,
	struct nvgpu_gr_ctx *ctx);

#ifdef CONFIG_NVGPU_GFXP
/**
 * @brief Allocate TSG specific GR preemption context buffers.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param desc [in]		Pointer to context descriptor struct.
 * @param ctx [in]		Pointer to graphics context struct.
 *
 * This function allocates all TSG specific GR preemption context buffers.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_gr_ctx_alloc_ctx_preemption_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx_desc *desc,
	struct nvgpu_gr_ctx *ctx);
#endif

/**
 * @brief Initialize mapping flags for graphics context buffers.
 *
 * Sets the mapping flags for all graphics context buffers to cacheable, except
 * for the main graphics context and patch context buffers, which are set to
 * non-cacheable.
 *
 * The steps performed by the function are as follows:
 * -# Iterate over each context buffer type defined by #NVGPU_GR_CTX_COUNT.
 * -# Set the mapping flag for each context buffer to cacheable.
 * -# Set the mapping flag for the main graphics context buffer to non-cacheable.
 * -# Set the mapping flag for the patch context buffer to non-cacheable.
 *
 * @param [in] g    Pointer to the GPU driver struct.
 * @param [in] ctx Pointer to the graphics context struct.
 *
 * @note This function does not return any value.
 */
void nvgpu_gr_ctx_init_ctx_buffers_mapping_flags(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx);

/**
 * @brief Allocate or retrieve existing GR context mappings for a TSG or channel.
 *
 * This function either allocates a new GR context mappings structure or retrieves
 * an existing one for a given TSG and channel. If subcontexts are supported, it
 * delegates the operation to nvgpu_tsg_subctx_alloc_or_get_mappings(). Otherwise,
 * it checks if the mappings already exist and returns them, or creates new mappings
 * using nvgpu_gr_ctx_mappings_create().
 *
 * The steps performed by the function are as follows:
 * -# Check if subcontexts are supported by querying #NVGPU_SUPPORT_TSG_SUBCONTEXTS.
 *    If supported, delegate to nvgpu_tsg_subctx_alloc_or_get_mappings() and return
 *    its result.
 * -# If mappings already exist in the TSG's GR context, return the existing mappings.
 * -# If no mappings exist, create a new GR context mappings structure using
 *    nvgpu_gr_ctx_mappings_create().
 * -# If the creation of the new mappings structure fails, log an error message and
 *    return NULL.
 * -# If the creation is successful, assign the new mappings to the TSG's GR context
 *    and return the new mappings structure.
 *
 * @Param [in] g            Pointer to the GPU driver structure.
 * @Param [in] tsg          Pointer to the TSG structure.
 * @Param [in] ch           Pointer to the channel structure.
 *
 * @return Pointer to the GR context mappings structure on success, or NULL on failure.
 */
struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_alloc_or_get_mappings(struct gk20a *g,
				struct nvgpu_tsg *tsg, struct nvgpu_channel *ch);

/**
 * @brief Retrieve the mappings structure for a TSG or its subcontext.
 *
 * This function returns the graphics context mappings associated with a TSG.
 * If subcontexts are supported and enabled, it retrieves the mappings for the
 * subcontext corresponding to the given channel's VM. Otherwise, it returns
 * the mappings directly associated with the TSG.
 *
 * The steps performed by the function are as follows:
 * -# Check if subcontexts are supported and enabled for the GPU associated
 *    with the TSG.
 * -# If subcontexts are supported, retrieve the mappings for the subcontext
 *    corresponding to the channel's VM using nvgpu_gr_ctx_mappings_get_subctx_mappings().
 * -# If subcontexts are not supported, return the mappings directly associated
 *    with the TSG.
 *
 * @param [in] tsg        Pointer to the TSG structure.
 * @param [in] ch         Pointer to the channel structure.
 *
 * @return Pointer to the graphics context mappings structure.
 */
struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_get_mappings(struct nvgpu_tsg *tsg,
					struct nvgpu_channel *ch);

/**
 * @brief Free the gr ctx mapping struct.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param ctx [in]		Pointer to graphics context struct.
 *
 * This function deletes the gr ctx mapping struct. This is to be
 * called when freeing the gr context or in error cases.
 */
void nvgpu_gr_ctx_free_mappings(struct gk20a *g,
				struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Set TSG id in graphics context structure.
 *
 * @param gr_ctx [in]		Pointer to graphics context struct.
 * @param tsgid [in]		TSG identifier.
 *
 * This function will set given TSG identifier into #nvgpu_gr_ctx
 * structure.
 */
void nvgpu_gr_ctx_set_tsgid(struct nvgpu_gr_ctx *gr_ctx, u32 tsgid);

/**
 * @brief Get TSG id stored in graphics context structure.
 *
 * @param gr_ctx [in]		Pointer to graphics context struct.
 *
 * This function will return TSG identifier stored in #nvgpu_gr_ctx
 * structure.
 *
 * @return TSG identifier.
 */
u32 nvgpu_gr_ctx_get_tsgid(struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Mark graphics context initialized.
 *
 * @param gr_ctx [in]          Pointer to graphics context struct.
 *
 * This function will mark obj_ctx initialized for the gr_ctx by
 * setting ctx_initialized to true.
 */
void nvgpu_gr_ctx_mark_ctx_initialized(struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Get ctx_initialized stored in graphics context structure.
 *
 * @param gr_ctx [in]          Pointer to graphics context struct.
 *
 * This function will return ctx_initialized stored in #nvgpu_gr_ctx
 * structure.
 *
 * @return ctx initialization status.
 */
bool nvgpu_gr_ctx_get_ctx_initialized(struct nvgpu_gr_ctx *gr_ctx);

#ifdef CONFIG_NVGPU_CILP
bool nvgpu_gr_ctx_desc_force_preemption_cilp(
		struct nvgpu_gr_ctx_desc *gr_ctx_desc);
#endif /* CONFIG_NVGPU_CILP */

#ifdef CONFIG_NVGPU_GFXP
void nvgpu_gr_ctx_set_preemption_buffer_va(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx);

bool nvgpu_gr_ctx_desc_force_preemption_gfxp(
		struct nvgpu_gr_ctx_desc *gr_ctx_desc);
#endif /* CONFIG_NVGPU_GFXP */

#ifdef CONFIG_NVGPU_GRAPHICS
/**
 * @brief Set the graphics preemption mode in the graphics context structure.
 *
 * The function sets the graphics preemption mode for a given graphics context.
 * The graphics preemption mode determines how graphics workloads can be
 * preempted to allow for better multitasking and resource sharing on the GPU.
 *
 * The steps performed by the function are as follows:
 * -# Assign the input graphics preemption mode to the graphics preemption mode
 *    field in the graphics context structure.
 *
 * @param [in] gr_ctx               Pointer to the graphics context structure.
 * @param [in] graphics_preempt_mode The graphics preemption mode to be set.
 *                                   Allowed values are defined by preemption
 *                                   mode macros such as
 *                                   #NVGPU_PREEMPTION_MODE_GRAPHICS_WFI or
 *                                   #NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP.
 *
 * There is no return value for this function.
 */
void nvgpu_gr_ctx_init_graphics_preemption_mode(struct nvgpu_gr_ctx *gr_ctx,
	u32 graphics_preempt_mode);

/**
 * @brief Retrieve the graphics preemption mode from the graphics context.
 *
 * This function returns the current graphics preemption mode set for a given
 * graphics context. The graphics preemption mode is used to determine how
 * graphics workloads are managed and preempted on the GPU.
 *
 * The steps performed by the function are as follows:
 * -# Return the value of the graphics preemption mode from the graphics
 *    context structure.
 *
 * @param [in] gr_ctx  Pointer to the graphics context structure from which the
 *                     graphics preemption mode is retrieved.
 *
 * @return The graphics preemption mode currently set in the graphics context.
 *         The return value is a 32-bit unsigned integer.
 */
u32 nvgpu_gr_ctx_get_graphics_preemption_mode(struct nvgpu_gr_ctx *gr_ctx);

#ifdef CONFIG_NVGPU_ZCULL
void nvgpu_gr_ctx_set_zcull_ctx(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
	u32 mode, u64 gpu_va);

u64 nvgpu_gr_ctx_get_zcull_ctx_va(struct nvgpu_gr_ctx *gr_ctx);

int nvgpu_gr_ctx_init_zcull(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx);

int nvgpu_gr_ctx_zcull_setup(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
	bool set_zcull_ptr);
#endif
#endif /* CONFIG_NVGPU_GRAPHICS */

u32 nvgpu_gr_ctx_get_ctx_id(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx);
u32 nvgpu_gr_ctx_read_ctx_id(struct nvgpu_gr_ctx *gr_ctx);

#ifdef CONFIG_NVGPU_DEBUGGER
int nvgpu_gr_ctx_alloc_map_pm_ctx(struct gk20a *g,
	struct nvgpu_tsg *tsg,
	struct nvgpu_gr_ctx_desc *gr_ctx_desc,
	struct nvgpu_gr_hwpm_map *hwpm_map);

void nvgpu_gr_ctx_reset_patch_count(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx);
void nvgpu_gr_ctx_set_patch_ctx(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx);

void nvgpu_gr_ctx_set_pm_ctx_pm_mode(struct nvgpu_gr_ctx *gr_ctx, u32 pm_mode);
u32 nvgpu_gr_ctx_get_pm_ctx_pm_mode(struct nvgpu_gr_ctx *gr_ctx);

int nvgpu_gr_ctx_set_smpc_mode(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
	bool enable);

int nvgpu_gr_ctx_prepare_hwpm_mode(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	u32 mode, bool *set_pm_ctx_gpu_va, bool *skip_update);
void nvgpu_gr_ctx_set_hwpm_pm_mode(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx);
void nvgpu_gr_ctx_set_hwpm_ptr(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
			       bool set_pm_ctx_gpu_va);
void nvgpu_gr_ctx_set_pm_ctx_mapped(struct nvgpu_gr_ctx *ctx, bool mapped);

#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
void nvgpu_gr_ctx_set_boosted_ctx(struct nvgpu_gr_ctx *gr_ctx, bool boost);
bool nvgpu_gr_ctx_get_boosted_ctx(struct nvgpu_gr_ctx *gr_ctx);
#endif
#endif /* CONFIG_NVGPU_DEBUGGER */

#ifdef CONFIG_DEBUG_FS
bool nvgpu_gr_ctx_desc_dump_ctxsw_stats_on_channel_close(
		struct nvgpu_gr_ctx_desc *gr_ctx_desc);
#endif

/**
 * @brief Check if global context buffers are patched.
 *
 * This function returns the status indicating whether the global context buffers
 * have been patched or not.
 *
 * The steps performed by the function are as follows:
 * -# Return the status of global context buffers being patched.
 *
 * @param [in] gr_ctx  Pointer to graphics context structure.
 *                      Range: Valid pointer to nvgpu_gr_ctx.
 *
 * @return Boolean status indicating if global context buffers are patched.
 *         'true' if patched, 'false' otherwise.
 */
bool nvgpu_gr_obj_ctx_global_ctx_buffers_patched(struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Set the status of global context buffers being patched.
 *
 * This function updates the status indicating whether the global context buffers
 * have been patched or not in the graphics context structure.
 *
 * The steps performed by the function are as follows:
 * -# Update the status of global context buffers being patched in the graphics
 *    context structure.
 *
 * @param [in] gr_ctx   Pointer to graphics context structure.
 *                      Range: Valid pointer to nvgpu_gr_ctx.
 * @param [in] patched  Boolean status to set.
 *                      Range: 'true' if global context buffers are patched,
 *                             'false' otherwise.
 */
void nvgpu_gr_obj_ctx_set_global_ctx_buffers_patched(
			struct nvgpu_gr_ctx *gr_ctx, bool patched);

/**
 * @brief Checks if preemption buffers have been patched.
 *
 * This function provides a boolean status indicating whether the preemption
 * buffers within a graphics context structure have been patched. It is a
 * simple accessor function that returns the value of a member variable within
 * the graphics context structure.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the value of the 'preempt_buffers_patched' member from the
 *    graphics context structure.
 * -# Return the retrieved value.
 *
 * @param [in] gr_ctx  Pointer to the graphics context structure.
 *                      Range: Valid pointer to a nvgpu_gr_ctx structure.
 *
 * @return Boolean status indicating if preemption buffers are patched.
 *         true - if preemption buffers are patched.
 *         false - if preemption buffers are not patched.
 */
bool nvgpu_gr_obj_ctx_preempt_buffers_patched(struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Sets the status of preemption buffers being patched.
 *
 * This function updates the graphics context structure to reflect whether the
 * preemption buffers have been patched. It sets the value of the member
 * variable indicating the patched status of the preemption buffers.
 *
 * The steps performed by the function are as follows:
 * -# Assign the provided patched status to the 'preempt_buffers_patched'
 *    member of the graphics context structure.
 *
 * @param [in] gr_ctx   Pointer to the graphics context structure.
 *                      Range: Valid pointer to a nvgpu_gr_ctx structure.
 * @param [in] patched  Boolean status to set for preemption buffers.
 *                      Range: true or false.
 */
void nvgpu_gr_obj_ctx_set_preempt_buffers_patched(
			struct nvgpu_gr_ctx *gr_ctx, bool patched);

/**
 * @brief Checks if default compute registers are patched.
 *
 * This function returns a boolean indicating whether the default compute
 * registers within the graphics context structure have been patched.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the value of the 'default_compute_regs_patched' member from the
 *    graphics context structure.
 * -# Return the retrieved value.
 *
 * @param [in] gr_ctx  Pointer to the graphics context structure.
 *                     Range: Valid pointer to a nvgpu_gr_ctx structure.
 *
 * @return Boolean status indicating if default compute registers are patched.
 *         true - if default compute registers are patched.
 *         false - if default compute registers are not patched.
 */
bool nvgpu_gr_obj_ctx_default_compute_regs_patched(struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Sets the status of default compute registers being patched.
 *
 * This function updates the graphics context structure to reflect whether the
 * default compute registers have been patched. It sets the value of the member
 * variable indicating the patched status of the default compute registers.
 *
 * The steps performed by the function are as follows:
 * -# Assign the provided patched status to the 'default_compute_regs_patched'
 *    member of the graphics context structure.
 *
 * @param [in] gr_ctx   Pointer to the graphics context structure.
 *                      Range: Valid pointer to a nvgpu_gr_ctx structure.
 * @param [in] patched  Boolean status to set for default compute registers.
 *                      Range: true or false.
 */
void nvgpu_gr_obj_ctx_set_default_compute_regs_patched(
			struct nvgpu_gr_ctx *gr_ctx, bool patched);

/**
 * @brief Checks if default graphics registers are patched.
 *
 * This function returns a boolean indicating whether the default graphics
 * registers within the graphics context structure have been patched.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the value of the 'default_gfx_regs_patched' member from the
 *    graphics context structure.
 * -# Return the retrieved value.
 *
 * @param [in] gr_ctx  Pointer to the graphics context structure.
 *                     Range: Valid pointer to a nvgpu_gr_ctx structure.
 *
 * @return Boolean status indicating if default graphics registers are patched.
 *         true - if default graphics registers are patched.
 *         false - if default graphics registers are not patched.
 */
bool nvgpu_gr_obj_ctx_default_gfx_regs_patched(struct nvgpu_gr_ctx *gr_ctx);

/**
 * @brief Sets the status of default graphics registers being patched.
 *
 * This function updates the graphics context structure to reflect whether the
 * default graphics registers have been patched. It sets the value of the member
 * variable indicating the patched status of the default graphics registers.
 *
 * The steps performed by the function are as follows:
 * -# Assign the provided patched status to the 'default_gfx_regs_patched'
 *    member of the graphics context structure.
 *
 * @param [in] gr_ctx   Pointer to the graphics context structure.
 *                      Range: Valid pointer to a nvgpu_gr_ctx structure.
 * @param [in] patched  Boolean status to set for default graphics registers.
 *                      Range: true or false.
 */
void nvgpu_gr_obj_ctx_set_default_gfx_regs_patched(
			struct nvgpu_gr_ctx *gr_ctx, bool patched);

#endif /* NVGPU_GR_CTX_H */
