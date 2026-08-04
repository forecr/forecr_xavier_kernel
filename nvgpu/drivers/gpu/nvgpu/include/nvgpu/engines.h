/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_H
#define NVGPU_ENGINE_H
/**
 * @file
 *
 * Abstract interface for engine related functionality.
 */

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_device;

/**
 * PBDMA engine instance 0.
 */
#define ENGINE_PBDMA_INSTANCE0 0U

/**
 * @brief Initialize additional device information for a given device.
 *
 * This function is responsible for initializing the Parallel Data Mover (PBDMA)
 * information for a specific device. It checks if the platform is post-ga10b
 * and if so, it retrieves the PBDMA information using the device's runlist
 * priority base address. It validates that at least one PBDMA instance is
 * associated with the device and updates the device's PBDMA ID with this
 * information.
 *
 * The steps performed by the function are as follows:
 * -# Check if the platform is pre-ga10b by verifying if the function pointer
 *    gops_runlist.get_engine_id_from_rleng_id() is NULL. If it is NULL, the function
 *    returns success as no further initialization is required.
 * -# Retrieve the PBDMA information for the device using the runlist priority
 *    base address by calling gops_runlist.get_pbdma_info().
 * -# Validate that the first PBDMA instance (instance 0) is valid by checking
 *    if its ID is not equal to #NVGPU_INVALID_PBDMA_ID. If it is invalid,
 *    log an error and return -EINVAL.
 * -# Update the device's PBDMA ID with the ID of the first PBDMA instance.
 * -# Return success.
 *
 * @param [in]  g        The GPU driver struct.
 * @param [in]  dev      The device whose additional information is to be initialized.
 *
 * @return 0 upon success.
 * @return -EINVAL if the PBDMA information is invalid.
 */
int nvgpu_engine_init_one_dev_extra(struct gk20a *g,
		const struct nvgpu_device *dev);

/**
 * Invalid engine id value.
 */
#define NVGPU_INVALID_ENG_ID		(~U32(0U))

struct gk20a;
struct nvgpu_fifo;
struct nvgpu_device;

/**
 * Engine enum types used for s/w purpose. These enum values are
 * different as compared to engine enum types defined by h/w.
 * Refer device.h header file.
 */
enum nvgpu_fifo_engine {
	/** GR engine enum */
	NVGPU_ENGINE_GR        = 0U,
	/** GR CE engine enum */
	NVGPU_ENGINE_GRCE      = 1U,
	/** Async CE engine enum */
	NVGPU_ENGINE_ASYNC_CE  = 2U,
	/** NVENC engine enum */
	NVGPU_ENGINE_NVENC     = 3U,
	/** OFA engine enum */
	NVGPU_ENGINE_OFA       = 4U,
	/** NVDEC engine enum */
	NVGPU_ENGINE_NVDEC     = 5U,
	/** NVJPG engine enum */
	NVGPU_ENGINE_NVJPG     = 6U,
	/** Invalid engine enum */
	NVGPU_ENGINE_INVAL     = 7U,
};

/**
 * @brief Get s/w defined engine enum type for engine enum type defined by h/w.
 *        See device.h for engine enum types defined by h/w.
 *
 * @param g [in]		The GPU driver struct.
 * @param dev [in]		Device to check.
 *
 * This is used to map engine enum type defined by h/w to engine enum type
 * defined by s/w.
 *
 * @return S/w defined valid engine enum type < #NVGPU_ENGINE_INVAL.
 * @retval #NVGPU_ENGINE_INVAL in case h/w #engine_type
 *         does not support gr (graphics) and ce (copy engine) engine enum
 *         types or if #engine_type does not match with h/w defined engine enum
 *         types for gr and/or ce engines.
 */
enum nvgpu_fifo_engine nvgpu_engine_enum_from_dev(struct gk20a *g,
					const struct nvgpu_device *dev);
/**
 * @brief Get pointer to #nvgpu_device for the h/w engine id.
 *
 * @param g [in]		The GPU driver struct.
 * @param engine_id [in]	Active (h/w) Engine id.
 *
 * If #engine_id is one of the supported h/w engine ids, get pointer to
 * #nvgpu_device from an array of structures that is indexed by h/w
 * engine id.
 *
 * @return Pointer to #nvgpu_device.
 * @retval NULL if #g is NULL.
 * @retval NULL if engine_id is not less than max supported number of engines
 *         i.e. #nvgpu_fifo.max_engines or if #engine_id does not match with
 *         any engine id supported by h/w or number of available engines
 *         i.e. #nvgpu_fifo.num_engines is 0.
 */
const struct nvgpu_device *nvgpu_engine_get_active_eng_info(
		struct gk20a *g, u32 engine_id);

/**
 * @brief Check if engine id is one of the supported h/w engine ids.
 *
 * @param g [in]		The GPU driver struct.
 * @param engine_id [in]	Engine id.
 *
 * Check if #engine_id is one of the supported active engine ids.
 *
 * @return True if #engine_id is valid.
 * @retval False if engine_id is not less than maximum number of supported
 *         engines on the chip i.e. #nvgpu_fifo.max_engines or if engine id
 *         does not match with any of the engine ids supported by h/w.
 */
bool nvgpu_engine_check_valid_id(struct gk20a *g, u32 engine_id);
/**
 * @brief Get h/w engine id based on engine's instance identification number
 *        #NVGPU_ENGINE_GR engine enum type using nvgpu_device_get() with device type
 *        #NVGPU_DEVTYPE_GRAPHICS.
 *
 * @param g [in]		The GPU driver struct.
 * @param inst_id [in]		Engine's instance identification number.
 *
 * @return H/W engine id for #NVGPU_ENGINE_GR engine enum type.
 * @retval #NVGPU_INVALID_ENG_ID if #NVGPU_ENGINE_GR engine enum type could not
 *         be found in the set of available h/w engine ids.
 */
u32 nvgpu_engine_get_gr_id_for_inst(struct gk20a *g, u32 inst_id);
/**
 * @brief Get instance count and first available h/w engine id for
 *        #NVGPU_ENGINE_GR engine enum type.
 *
 * @param g [in]		The GPU driver struct.
 *
 * Call nvgpu_engine_get_gr_id_for_inst() to get first available #NVGPU_ENGINE_GR
 * engine enum type.
 *
 * @return First available h/w engine id for #NVGPU_ENGINE_GR engine enum type.
 * @retval #NVGPU_INVALID_ENG_ID if #NVGPU_ENGINE_GR engine enum type could not
 *         be found in the set of available h/w engine ids.
 */
u32 nvgpu_engine_get_gr_id(struct gk20a *g);
/**
 * @brief Get intr mask for the GR engine supported by the chip.
 *
 * @param g[in]			The GPU driver struct.
 *
 * Return bitmask of each GR engine's interrupt bit.
 *
 * @return Interrupt mask for GR engine.
 */
u32 nvgpu_gr_engine_interrupt_mask(struct gk20a *g);
/**
 * @brief Get intr mask for the CE engines supported by the chip.
 *
 * @param g [in]		The GPU driver struct.
 *
 * Query all types of copy engine devices and OR their interrupt bits into
 * a CE interrupt mask.
 *
 * @return 0U if there is no CE support in the system.
 * @return The logical OR of all interrupt bits for all CE devices present.
 */
u32 nvgpu_ce_engine_interrupt_mask(struct gk20a *g);
/**
 * @brief Get intr mask for the device corresponding the provided engine_id.
 *
 * @param g [in]		The GPU driver struct.
 * @param engine_id [in]	HW engine_id.
 *
 * Return the interrupt mask for the host device corresponding to \a engine_id.
 *
 * @return Intr mask for the #engine_id or 0 if the engine_id does not have a
 *         corresponding device.
 */
u32 nvgpu_engine_act_interrupt_mask(struct gk20a *g, u32 engine_id);
/**
 * @brief Allocate and initialize s/w context for engine related info.
 *
 * @param g [in]		The GPU driver struct.
 *
 * - Get max number of engines supported on the chip from h/w config register.
 * - Allocate kernel memory area for storing engine info for max number of
 *   engines supported on the chip. This allocated area of memory is set to 0
 *   and then filled with value read from device info h/w registers.
 *   Also this area of memory is indexed by h/w engine id.
 * - Allocate kernel memory area for max number of engines supported on the
 *   chip to map s/w defined engine ids starting from 0 to h/w (active) engine
 *   id read from device info h/w register. This allocated area of memory is
 *   set to 0xff and then filled with engine id read from device info h/w
 *   registers. This area of memory is indexed by s/w defined engine id starting
 *   from 0.
 * - Initialize engine info related s/w context by calling hal that will read
 *   device info h/w registers and also initialize s/w variable
 *   #nvgpu_fifo.num_engines that is used to count total number of valid h/w
 *   engine ids read from device info h/w registers.
 *
 * @return 0 upon success, < 0 otherwise.
 * @retval -ENOMEM upon failure to allocate memory for engine structure.
 * @retval -EINVAL upon failure to get engine info from device info h/w
 *         registers.
 */
int nvgpu_engine_setup_sw(struct gk20a *g);
/**
 * @brief Clean up s/w context for engine related info.
 *
 * @param g [in]		The GPU driver struct.
 *
 * - Free up kernel memory area that is used for storing engine info read
 *   from device info h/w registers.
 * - Free up kernel memory area to map s/w defined engine ids (starting with 0)
 *   to active (h/w) engine ids read from device info h/w register.
 */
void nvgpu_engine_cleanup_sw(struct gk20a *g);

#ifdef CONFIG_NVGPU_FIFO_ENGINE_ACTIVITY
void nvgpu_engine_enable_activity_all(struct gk20a *g);
int nvgpu_engine_disable_activity(struct gk20a *g,
			const struct nvgpu_device *dev,
			bool wait_for_idle);
int nvgpu_engine_disable_activity_all(struct gk20a *g,
				bool wait_for_idle);

int nvgpu_engine_wait_for_idle(struct gk20a *g);
#endif
#ifdef CONFIG_NVGPU_ENGINE_RESET
/**
 * Called from recovery. This will not be part of the safety build after
 * recovery is not supported in the safety build.
 */
void nvgpu_engine_reset(struct gk20a *g, u32 engine_id);
#endif
/**
 * @brief Get runlist id for the last available #NVGPU_ENGINE_ASYNC_CE
 *        engine enum type that is bound to the specified GPU instance
 *        id.
 *
 * @param g [in]		            The GPU driver struct.
 * @param gpu_instance_id [in]		The GPU instance id.
 * @param ce_runlist_id [out]		The CE runlist id.
 *
 * - Obtain gpu_instance object based on the gpu_instance_id.
 * - Loop in reverse through all Logical Copy Engine (LCE) that is
 *   assigned to the specified GPU instance.
 * - Find the first LCE with type #NVGPU_ENGINE_ASYNC_CE, write out
 *   its runlist id into ce_runlist_id and return 0 for success.
 * - If not LCE is found, return -ENOENT to indicate that no LCE
 *   is found.
 *
 * @retval Return value 0 on success.
 * @retval Return value -ENOENT on failure.
 */
int nvgpu_engine_get_fast_ce_runlist_id(struct gk20a *g, u32 gpu_instance_id, u32 *ce_runlist_id);

/**
 * @brief Retrieves the runlist ID for the graphics engine.
 *
 * This function obtains the device information for the graphics engine and returns
 * the associated runlist ID. If no graphics device is found, it returns an invalid
 * runlist ID.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the device information for the first graphics engine instance using
 *    nvgpu_device_get().
 * -# Check if the retrieved device information is valid.
 * -# If the device information is not valid, return an invalid runlist ID.
 * -# If the device information is valid, return the runlist ID associated with
 *    the graphics engine.
 *
 * @param [in] g  Pointer to the GPU driver struct.
 *
 * @return Runlist ID of the graphics engine.
 * @return #NVGPU_INVALID_RUNLIST_ID if no graphics device is found.
 */
u32 nvgpu_engine_get_gr_runlist_id(struct gk20a *g);
/**
 * @brief Get runlist id for the given instance of #NVGPU_ENGINE_NVENC engine enum
 *        type.
 *
 * @param g [in]		The GPU driver struct.
 * @param inst_id [in]		NVENC engine's instance id
 *
 * -- Get #nvgpu_device for the requested instance of nvenc engine id.
 * -- Return #nvgpu_device.runlist_id for the same.
 *
 * @return #nvgpu_device.runlist_id for the given instance of nvenc engine id.
 * @retval NVGPU_INVALID_RUNLIST_ID if #nvgpu_device is not available for the nvenc
 *         instance.
 */
u32 nvgpu_engine_get_nvenc_runlist_id(struct gk20a *g, u32 inst_id);
/**
 * @brief Get runlist id for the given instance of #NVGPU_ENGINE_OFA engine enum
 *        type.
 *
 * @param g [in]		The GPU driver struct.
 * @param inst_id [in]		OFA engine's instance id
 *
 * -- Get #nvgpu_device for the requested instance of ofa engine id.
 * -- Return #nvgpu_device.runlist_id for the same.
 *
 * @return #nvgpu_device.runlist_id for the given instance of ofa engine id.
 * @retval NVGPU_INVALID_RUNLIST_ID if #nvgpu_device is not available for the ofa
 *         instance.
 */
u32 nvgpu_engine_get_ofa_runlist_id(struct gk20a *g, u32 inst_id);
/**
 * @brief Get runlist id for the given instance of #NVGPU_ENGINE_NVDEC engine enum
 *        type.
 *
 * @param g [in]		The GPU driver struct.
 * @param inst_id [in]		NVDEC engine's instance id
 *
 * -- Get #nvgpu_device for the requested instance of nvdec engine id.
 * -- Return #nvgpu_device.runlist_id for the same.
 *
 * @return #nvgpu_device.runlist_id for the given instance of nvdec engine id.
 * @retval NVGPU_INVALID_RUNLIST_ID if #nvgpu_device is not available for the nvdec
 *         instance.
 */
u32 nvgpu_engine_get_nvdec_runlist_id(struct gk20a *g, u32 inst_id);
/**
 * @brief Get runlist id for the given instance of #NVGPU_ENGINE_NVJPG engine enum
 *        type.
 *
 * @param g [in]		The GPU driver struct.
 * @param inst_id [in]		NVJPG engine's instance id
 *
 * -- Get #nvgpu_device for the requested instance of nvjpg engine id.
 * -- Return #nvgpu_device.runlist_id for the same.
 *
 * @return #nvgpu_device.runlist_id for the given instance of nvjpg engine id.
 * @retval NVGPU_INVALID_RUNLIST_ID if #nvgpu_device is not available for the nvjpg
 *         instance.
 */
u32 nvgpu_engine_get_nvjpg_runlist_id(struct gk20a *g, u32 inst_id);
/**
 * @brief Check if runlist id corresponds to runlist id of one of the
 *        engine ids supported by h/w.
 *
 * @param g [in]		The GPU driver struct.
 * @param runlist_id [in]	Runlist id.
 *
 * Check if #runlist_id corresponds to runlist id of one of the engine
 * ids supported by h/w by checking #nvgpu_device for each of
 * #nvgpu_fifo.num_engines engines.
 *
 * @return True if #runlist_id is valid.
 * @return False if #nvgpu_device is NULL for all the engine ids starting
 *         with 0 upto #nvgpu_fifo.num_engines or #runlist_id did not match with
 *         any of the runlist ids of engine ids supported by h/w.
 */
bool nvgpu_engine_is_valid_runlist_id(struct gk20a *g, u32 runlist_id);
/**
 * @brief Check if the given runlist id belongs to one of the multimedia engines
 *        supported by the h/w.
 *
 * @param g [in]		The GPU driver struct.
 * @param runlist_id [in]	Runlist id.
 *
 * Check if #runlist_id corresponds to runlist id of one of the multimedia engines
 * supported by h/w, by checking #nvgpu_device for each of the supported
 * multimedia engines in #nvgpu_fifo.num_engines engines.
 *
 * @return True if #runlist_id is valid.
 * @return False if #nvgpu_device is NULL for all multimedia engines starting
 *         with 0 up to #nvgpu_fifo.num_engines or #runlist_id did not match with
 *         any of the runlist ids of multimedia engine ids supported by h/w.
 */
bool nvgpu_engine_is_multimedia_runlist_id(struct gk20a *g, u32 runlist_id);
/**
 * @brief Check if the given runlist id belongs to one of the multimedia engines
 *        supported by the h/w and return the respective multimedia id.
 *
 * @param g [in]		The GPU driver struct.
 * @param runlist_id [in]	Runlist id.
 *
 * Check if #runlist_id corresponds to runlist id of one of the multimedia engines
 * supported by h/w, by checking #nvgpu_device for each of the supported
 * multimedia engines in #nvgpu_fifo.num_engines engines and return the matching
 * multimedia id.
 *
 * @return Valid multimedia id corresponding to #runlist_id.
 * @retval Invalid multimedia id, #NVGPU_MULTIMEDIA_ENGINE_MAX,
 *         if #nvgpu_device is NULL for all multimedia engines starting
 *         with 0 up to #NVGPU_MULTIMEDIA_ENGINE_MAX-1 or #runlist_id did not match with
 *         any of the runlist ids of multimedia engine ids supported by h/w.
 */
u32 nvgpu_engine_multimedia_id_from_runlist(struct gk20a *g, u32 runlist_id);
/**
 * @brief Get mmu fault id for the engine id.
 *
 * @param g [in]		The GPU driver struct.
 * @param engine_id [in]	Engine id.
 *
 * Get pointer to #nvgpu_device for the #engine_id. Use this pointer to
 * get mmu fault id for the #engine_id.
 *
 * @return Mmu fault id for #engine_id.
 * @retval Invalid mmu fault id, #NVGPU_INVALID_ENG_ID.
 */
u32 nvgpu_engine_id_to_mmu_fault_id(struct gk20a *g, u32 engine_id);
/**
 * @brief Get engine id from mmu fault id.
 *
 * @param g [in]		The GPU driver struct.
 * @param fault_id [in]		Mmu fault id.
 *
 * Check if #fault_id corresponds to fault id of one of the active engine
 * ids supported by h/w by checking #nvgpu_device for each of
 * #nvgpu_fifo.num_engines engines.
 *
 * @return Valid engine id corresponding to #fault_id.
 * @retval Invalid engine id, #NVGPU_INVALID_ENG_ID if pointer to
 *         #nvgpu_device is NULL for the  engine ids starting with
 *         0 upto #nvgpu_fifo.num_engines or
 *         #fault_id did not match with any of the fault ids of h/w engine ids.
 */
u32 nvgpu_engine_mmu_fault_id_to_engine_id(struct gk20a *g, u32 fault_id);
/**
 * Called from recovery. This will not be part of the safety build after
 * recovery is not supported in the safety build.
 */
u32 nvgpu_engine_get_mask_on_id(struct gk20a *g, u32 id, bool is_tsg);

/**
 * @brief Initializes the engine information for the GPU.
 *
 * This function sets up the engine information by iterating over all graphics
 * devices and initializing each one. It also initializes the copy engine info
 * by calling a specific function from the GPU operations structure.
 *
 * The steps performed by the function are as follows:
 * -# Set the number of engines to zero.
 * -# Iterate over each graphics device:
 *    -# Call 'nvgpu_engine_init_one_dev()' to initialize the engine information
 *       for the current device.
 *    -# If an error occurs during the initialization of a device, return the
 *       error code immediately.
 * -# After all graphics devices are initialized, call gops_engine.init_ce_info() from the
 *    GPU operations structure to initialize the copy engine information.
 * -# Return zero to indicate success.
 *
 * @param [in] f  Pointer to the GPU FIFO struct containing engine information.
 *
 * @return Zero on success, or a negative error code on failure.
 */
int nvgpu_engine_init_info(struct nvgpu_fifo *f);

/**
 * Called from recovery handling for architectures before volta. This will
 * not be part of safety build after recovery is not supported in the safety
 * build.
 */
void nvgpu_engine_get_id_and_type(struct gk20a *g, u32 engine_id,
					  u32 *id, u32 *type);
/**
 * Called from ctxsw timeout intr handling. This function will not be part
 * of safety build after recovery is not supported in the safety build.
 */
u32 nvgpu_engine_find_busy_doing_ctxsw(struct gk20a *g,
		u32 *id_ptr, bool *is_tsg_ptr);
/**
 * Called from runlist update timeout handling. This function will not be part
 * of safety build after recovery is not supported in safety build.
 */
u32 nvgpu_engine_get_runlist_busy_engines(struct gk20a *g, u32 runlist_id);

#ifdef CONFIG_NVGPU_DEBUGGER
bool nvgpu_engine_should_defer_reset(struct gk20a *g, u32 engine_id,
			u32 engine_subid, bool fake_fault);
#endif
/**
 * @brief Get veid from mmu fault id.
 *
 * @param g [in]		The GPU driver struct.
 * @param mmu_fault_id [in]	Mmu fault id.
 * @param gr_eng_fault_id [in]	GR engine's mmu fault id.
 *
 * Get valid veid by subtracting #gr_eng_fault_id from #mmu_fault_id,
 * if #mmu_fault_id is greater than or equal to #gr_eng_fault_id and less
 * than #gr_eng_fault_id + #nvgpu_fifo.max_subctx_count.
 *
 * @return Veid.
 * @retval Invalid veid, #INVAL_ID.
 */
u32 nvgpu_engine_mmu_fault_id_to_veid(struct gk20a *g, u32 mmu_fault_id,
			u32 gr_eng_fault_id);
/**
 * @brief Get engine id, veid and pbdma id from mmu fault id.
 *
 * @param g [in]		The GPU driver struct.
 * @param mmu_fault_id [in]	Mmu fault id.
 * @param engine_id [in,out]	Pointer to store active engine id.
 * @param veid [in,out]		Pointer to store veid.
 * @param pbdma_id [in,out]	Pointer to store pbdma id.
 *
 * Calls function nvgpu_engine_mmu_fault_id_to_eng_id_and_veid() to get h/w engine id and veid for
 * given #mmu_fault_id.
 * If h/w (active) engine id is not #INVAL_ID, call function to get pbdma id for the engine having
 * fault id as #mmu_fault_id. Otherwise, set pbdma id as #INVAL_ID.
 *
 * @return Updated #engine_id, #veid and #pbdma_id pointers
 */
void nvgpu_engine_mmu_fault_id_to_eng_ve_pbdma_id(struct gk20a *g,
	u32 mmu_fault_id, u32 *engine_id, u32 *veid, u32 *pbdma_id);
/**
 * @brief Remove a device entry from engine list.
 *
 * @param g [in]		The GPU driver struct.
 * @param dev [in]		A device.
 *
 * Remove the device entry \a dev from fifo->host_engines, fifo->active_engines.
 * The device entry is retained in g->devs->devlist_heads list to ensure device
 * reset.
 */
void nvgpu_engine_remove_one_dev(struct nvgpu_fifo *f,
		const struct nvgpu_device *dev);

/**
 * @brief halt engines in the GPU.
 *
 * @param g [in]		The GPU driver struct.
 *
 * - For each supported FIFO engine, call respective hal to halt the
 *   engine.
 * - In case ctxsw fails to halt, error is returned to the caller.
 *   In that case, other engines are halted. On reloading the driver
 *   ctxsw reset will not work though.
 *
 * @return 0 upon success.
 * @retval < 0 if call to halt GR ctxsw fails.
 */
int nvgpu_halt_engines(struct gk20a *g);

#endif /*NVGPU_ENGINE_H*/
