/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_ENGINE_H
#define NVGPU_GOPS_ENGINE_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * Engine HAL interface.
 */
struct gk20a;
struct nvgpu_engine_status_info;
struct nvgpu_debug_context;

/**
 * Engine status HAL operations.
 *
 * @see gpu_ops
 */
struct gops_engine_status {
	/**
	 * @brief Read engine status info.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param engine_id [in]	H/w engine id.
	 * @param status [in,out]	Pointer to struct abstracting engine
	 * 				status.
	 *
	 * Read engine status information from GPU h/w, and:
	 * - Determine if engine is busy.
	 * - Determine if engine if faulted.
	 * - Determine current context status
	 *   (see #nvgpu_engine_status_ctx_status).
	 * - Determine current context id and type. Valid when status is
	 *   #NVGPU_CTX_STATUS_VALID, #NVGPU_CTX_STATUS_CTXSW_SAVE or
	 *   #NVGPU_CTX_STATUS_CTXSW_SWITCH.
	 * - Determine next context id and type. Valid when status is
	 *   #NVGPU_CTX_STATUS_CTXSW_LOAD or #NVGPU_CTX_STATUS_CTXSW_SWITCH.
	 *
	 * @see nvgpu_engine_status_info
	 * @see nvgpu_engine_status_ctx_status
	 * @see nvgpu_engine_get_ids
	 */
	void (*read_engine_status_info)(struct gk20a *g,
		u32 engine_id, struct nvgpu_engine_status_info *status);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	void (*dump_engine_status)(struct gk20a *g,
			struct nvgpu_debug_context *o);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

	u32 (*get_rleng_status0_reg_off)(u32 rleng_id);
	u32 (*get_rleng_status1_reg_off)(u32 rleng_id);
};

struct gops_engine {
	/**
	 * @brief Initializes the Copy Engine (CE) information for a GP10B GPU.
	 *
	 * This function iterates over all the devices of type #NVGPU_DEVTYPE_LCE (Light Copy Engine)
	 * and initializes the PBDMA (Push Buffer DMA) mapping for each device. It updates the
	 * host_engines and active_engines arrays within the nvgpu_fifo structure with the devices
	 * found and increments the number of engines.
	 *
	 * The steps performed by the function are as follows:
	 * -# Retrieve the GPU structure from the FIFO structure.
	 * -# Iterate over each device of type #NVGPU_DEVTYPE_LCE.
	 * -# For each device, check if the function pointer gops_fifo.find_pbdma_for_runlist() is
	 *    not NULL.
	 * -# If the function pointer is valid, call gops_fifo.find_pbdma_for_runlist() with the
	 *    runlist ID of the device to find the corresponding PBDMA ID.
	 * -# If the PBDMA ID is not found, log an error and return -EINVAL.
	 * -# Store the device pointer in the 'host_engines' array using the engine ID as the index.
	 * -# Add the device pointer to the next available slot in the 'active_engines' array.
	 * -# Increment the 'num_engines' count in the FIFO structure.
	 * -# Return 0 to indicate successful initialization.
	 *
	 * @param [in] f  Pointer to the FIFO structure that contains information about all FIFOs.
	 *
	 * @return 0 if the initialization is successful.
	 * @return -EINVAL if there is an error in finding the PBDMA ID for any device.
	 */
	int (*init_ce_info)(struct nvgpu_fifo *f);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	bool (*is_fault_engine_subid_gpc)(struct gk20a *g,
				 u32 engine_subid);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
};

#endif
