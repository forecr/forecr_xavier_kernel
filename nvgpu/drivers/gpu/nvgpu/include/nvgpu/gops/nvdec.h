/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_NVDEC_H
#define NVGPU_GOPS_NVDEC_H

struct nvgpu_multimedia_ctx;
struct nvgpu_firmware;

struct gops_nvdec {

	/**
	 * @brief Base address of NVDEC apperture.
	 *
	 * @param inst_id [in]		NVDEC engine's instance id
	 *
	 * This function gets the base address of NVDEC aperture.
	 */
	u32 (*base_addr)(u32 inst_id);

	/**
	 * @brief falcon2 base address of NVDEC aperture.
	 *
	 * @param inst_id [in]		NVDEC engine's instance id
	 *
	 * This function gets the falcon2 base address of NVDEC aperture.
	 */
	u32 (*falcon2_base_addr)(u32 inst_id);

	/**
	 * @brief Initialize NVDEC support.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * Initializes the private data struct for NVDEC unit in the GPU driver
	 * according to the current chip.
	 */
	int (*init)(struct gk20a *g);

	/**
	 * @brief Deinitialize NVDEC support.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * This function frees the memory allocated during initialization.
	 */
	int (*deinit)(struct gk20a *g);

	/**
	 * @brief Reset and boot NVDEC engine.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param dev [in]		NVDEC engine's device struct.
	 *
	 * This function resets the NVDEC HW unit and loads the firmware again.
	 */
	int (*reset_and_boot)(struct gk20a *g, const struct nvgpu_device *dev);

	/**
	 * Load and bootstrap ucode on NVDEC falcon.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_id [in]		NVDEC engine's instance id
	 *
	 * This function takes care of loading the firmware. It kick-starts
	 * the core and polls for the successful initialization.
	 */
	int (*bootstrap)(struct gk20a *g, u32 inst_id);

	/**
	 * @brief Set-up boot configuration registers.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_id [in]		NVDEC engine's instance id
	 *
	 * This function programs the configuration registers needed before boot.
	 */
	void (*setup_boot_config)(struct gk20a *g, u32 inst_id);

	/**
	 * @brief Halt the engine.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_id [in]		NVDEC engine's instance id
	 *
	 * This function halts the engine.
	 */
	void (*halt_engine)(struct gk20a *g, u32 inst_id);

	/**
	 * @brief Reset the engine.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_id [in]		NVDEC engine's instance id
	 *
	 * This function resets the engine.
	 */
	int (*reset_eng)(struct gk20a *g, u32 inst_id);

	/**
	 * @brief Sets stall and non-stall intr ctrl vectors for NVDEC.
	 *
	 * @param g [in]                Pointer to GPU driver struct.
	 * @param inst_id [in]          NVDEC engine's instance id
	 * @param enable [in]           Parameter to enable/disable interrupt.
	 * @param intr_ctrl [in]        Stall interrupt
	 * @param intr_notify_ctrl [in] Non Stall interrupt
	 *
	 * This function gets the interrupt ctrl message for NVDEC as requested.
	 */
	void (*get_intr_ctrl_msg)(struct gk20a *g, u32 inst_id, bool enable,
			u32 *intr_ctrl, u32 *intr_notify_ctrl);

	/**
	 * @brief Enable interrupts for NVDEC.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_id [in]		NVDEC engine's instance id
	 * @param enable [in]		Parameter to enable/disable interrupt.
	 *
	 * This function enables/disables the interrupts for NVDEC as requested.
	 */
	void (*enable_irq)(struct gk20a *g, u32 inst_id, bool enable);

	/**
	 * @brief Interrupt handler for NVDEC interrupts.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_id [in]		NVDEC engine's instance id
	 *
	 * This function handles the interrupts from NVDEC and takes the necessary
	 * actions.
	 */
	void (*nvdec_isr)(struct gk20a *g, u32 inst_id);

	bool (*nvdec_is_swgen0_set)(struct gk20a *g, u32 inst_id);

	/**
	 * @brief Allocate and setup engine context for GPU channel.
	 *
	 * @param ch [in]		Pointer to GPU channel.
	 * @param class_num [in]	GPU class ID.
	 * @param flags [in]		Flags for context allocation.
	 *
	 * This HAL allocates and sets up engine context for
	 * a GPU channel.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 * @retval -ENOMEM if memory allocation for context buffer fails.
	 * @retval -EINVAL if invalid GPU class ID is provided.
	 */
	int (*multimedia_alloc_ctx)(struct nvgpu_channel *ch,
			     u32 class_num, u32 flags);

	/**
	 * @brief Free engine context buffer.
	 *
	 * @param g [in]	Pointer to GPU driver struct.
	 * @param eng_ctx [in]	Pointer to engine context data.
	 *
	 * This function frees the memory allocated for engine
	 * context buffer.
	 */
	void (*multimedia_free_ctx)(struct gk20a *g,
			    struct nvgpu_multimedia_ctx *eng_ctx);

	/**
	 * @brief Dump nvdec status and debug registers.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_id [in]		NVDEC engine's instance id
	 * @param dbg_ctx [in]		Debug context info
	 *
	 * This function prints the contents of debug and status registers
	 * to help in debug process.
	 */
	void (*dump_engine_status)(struct gk20a *g, u32 inst_id,
			    struct nvgpu_debug_context *dbg_ctx);

	/**
	 * @brief Get NVDEC firmware.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param desc_fw [out]	Pointer to descriptor firmware that contains
	 *                      firmware metadata.
	 * @param image_fw [out]	Pointer to image firmware that contains the
	 *                      actual firmware binary.
	 * @param pkc_sig [out]	Pointer to PKC signature used for firmware
	 *                      authentication. For LSB v3, this contains the
	 *                      LSB header.
	 *
	 * This function gets the appropriate firmware names based on debug/production
	 * mode and LSB version, then loads the firmware files into memory.
	 */
	void (*get_nvdec_fw)(struct gk20a *g, struct nvgpu_firmware **desc_fw,
			struct nvgpu_firmware **image_fw,
			struct nvgpu_firmware **pkc_sig);
};

#endif /* NVGPU_GOPS_NVDEC_H */
