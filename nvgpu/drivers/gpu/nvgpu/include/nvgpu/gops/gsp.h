/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_GSP_H
#define NVGPU_GOPS_GSP_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gsp;

#define GSP_WAIT_TIME_MS 10000

struct gops_gsp {
/**
 * @brief Retrieves the base address of the GSP Falcon interrupt set register.
 *
 * This function provides the base address for the GSP Falcon interrupt set
 * register by calling a hardware register read function.
 *
 * The steps performed by the function are as follows:
 * -# Call the 'pgsp_falcon_irqsset_r()' function to read the base address
 *    from the hardware register specific to the GSP Falcon interrupt set.
 *
 * @return The base address of the GSP Falcon interrupt set register.
 */
	u32 (*falcon_base_addr)(void);
/**
 * @brief Retrieves the base address of the second GSP Falcon controller.
 *
 * This function provides the base address for the second GSP Falcon controller
 * by calling a hardware register read function.
 *
 * The steps performed by the function are as follows:
 * -# Call the 'pgsp_falcon2_gsp_base_r()' function to read the base address
 *    from the hardware register specific to the second GSP Falcon controller.
 *
 * @return The base address of the second GSP Falcon controller.
 */
	u32 (*falcon2_base_addr)(void);
/**
 * @brief Sets up the boot configuration for the GSP Falcon.
 *
 * This function configures the next context for the GSP Falcon by writing the
 * instance block pointer to the FALCON NXTCTX register. It ensures that the
 * instance block is properly aligned and marks the context as valid.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the pointer to the memory management structure from the GPU driver struct.
 * -# Calculate the instance block pointer by calling nvgpu_inst_block_ptr(), which
 *    retrieves the lower 32-bits of the 4K-aligned physical instance block address.
 * -# Write the instance block pointer to the FALCON NXTCTX register with the context
 *    validity flag set to 1 and the appropriate aperture mask by calling gk20a_writel().
 *    The aperture mask is determined by calling nvgpu_aperture_mask() with the instance
 *    block and the corresponding aperture flags for non-coherent system memory, coherent
 *    system memory, and framebuffer memory.
 *
 * @param [in]  g  Pointer to the GPU driver struct.
 */
	void (*falcon_setup_boot_config)(struct gk20a *g);

/**
 * @brief Resets the GSP engine.
 *
 * This function performs a reset sequence on the GSP engine. It first asserts the
 * reset condition, waits for a brief period, and then deasserts the reset condition.
 * After the reset sequence, it enables the SLCG (Second Level Clock Gating) production
 * values for the GSP.
 *
 * The steps performed by the function are as follows:
 * -# Write to the GSP Falcon engine register to assert the reset condition using
 *    'gk20a_writel()' and 'pgsp_falcon_engine_reset_true_f()'.
 * -# Delay execution for a short period using 'nvgpu_udelay()'.
 * -# Write to the GSP Falcon engine register to deassert the reset condition using
 *    'gk20a_writel()' and 'pgsp_falcon_engine_reset_false_f()'.
 * -# Enable SLCG production values for the GSP using 'nvgpu_cg_slcg_gsp_load_enable()'.
 *
 * @param [in]  g  Pointer to the GPU driver struct.
 *
 * @return Zero, indicating success.
 */
	int (*gsp_reset)(struct gk20a *g, u32 inst_id);
/**
 * @brief Validates the memory integrity of the GSP Falcon.
 *
 * This function reads the ECC status register of the GSP Falcon and uses it to
 * determine if there have been any uncorrected ECC errors. It then returns a boolean
 * value indicating the result of the memory integrity validation.
 *
 * The steps performed by the function are as follows:
 * -# Read the ECC status register using 'nvgpu_readl()'.
 * -# Call 'ga10b_gsp_handle_ecc()' with the read ECC status and a predefined error
 *    module identifier '#NVGPU_ERR_MODULE_GSP_ACR'.
 * -# Return 'true' if 'ga10b_gsp_handle_ecc()' indicates no errors, otherwise return 'false'.
 *
 * @param [in]  g  Pointer to the GPU driver struct.
 *
 * @return 'true' if memory integrity is validated, 'false' otherwise.
 */
	bool (*validate_mem_integrity)(struct gk20a *g);
/**
 * @brief Checks if the GSP Falcon is in debug mode.
 *
 * This function reads the hardware configuration register to determine if the GSP
 * Falcon is operating in debug mode or production mode. It returns a boolean value
 * indicating the mode of operation.
 *
 * The steps performed by the function are as follows:
 * -# Read the GSP Falcon hardware configuration 2 register using 'nvgpu_readl()'.
 * -# Compare the debug mode value extracted from the read register using
 *    'pgsp_falcon_hwcfg2_dbgmode_v()' with the enable value using
 *    'pgsp_falcon_hwcfg2_dbgmode_enable_v()'.
 * -# If the values match, log "DEBUG MODE" using 'nvgpu_gsp_dbg()' and return 'true'.
 * -# If the values do not match, log "PROD MODE" using 'nvgpu_gsp_dbg()' and return 'false'.
 *
 * @param [in]  g  Pointer to the GPU driver struct.
 *
 * @return 'true' if the GSP Falcon is in debug mode, 'false' if in production mode.
 */
	bool (*is_debug_mode_enabled)(struct gk20a *g);
	bool is_gsp_supported;
/**
 * @brief Retrieves the register offset for the GSP queue head.
 *
 * This function provides the register offset for the head of a specified GSP queue.
 * The offset is used to access the head of the queue for operations such as reading
 * or updating the queue head pointer.
 *
 * The steps performed by the function are as follows:
 * -# Call the macro #pgsp_queue_head_r() with the provided queue index to get the
 *    register offset for the GSP queue head.
 *
 * @param [in]  i  Index of the GSP queue for which the head register offset is requested.
 *
 * @return The register offset for the head of the specified GSP queue.
 */
	u32 (*gsp_get_queue_head)(u32 i);

/**
 * @brief Returns the size of the GSP queue head register.
 *
 * This function provides the size of the head register for the GSP queues. It is
 * used to determine the size of the register area dedicated to the queue head.
 *
 * The steps performed by the function are as follows:
 * -# Call the macro #pgsp_queue_head__size_1_v() to get the size of the GSP queue
 *    head register.
 *
 * @return The size of the GSP queue head register.
 */
	u32 (*gsp_get_queue_head_size)(void);

/**
 * @brief Retrieves the register offset for the GSP queue tail.
 *
 * This function provides the register offset for the tail of a specified GSP queue.
 * The offset is used to access the tail of the queue for operations such as reading
 * or updating the queue tail pointer.
 *
 * The steps performed by the function are as follows:
 * -# Call the macro #pgsp_queue_tail_r() with the provided queue index to get the
 *    register offset for the GSP queue tail.
 *
 * @param [in]  i  Index of the GSP queue for which the tail register offset is requested.
 *
 * @return The register offset for the tail of the specified GSP queue.
 */
	u32 (*gsp_get_queue_tail_size)(void);

/**
 * @brief Returns the size of the GSP queue tail register.
 *
 * This function provides the size of the tail register for the GSP queues. It is
 * used to determine the size of the register area dedicated to the queue tail.
 *
 * The steps performed by the function are as follows:
 * -# Call the macro #pgsp_queue_tail__size_1_v() to get the size of the GSP queue
 *    tail register.
 *
 * @return The size of the GSP queue tail register.
 */
	u32 (*gsp_get_queue_tail)(u32 i);
/**
 * @brief Copies data from the source buffer to the GSP EMEM.
 *
 * This function initiates a transfer of data from a source buffer in system memory
 * to the GSP's External Memory (EMEM) at the specified destination offset and port.
 * It wraps the generic EMEM transfer function with the direction set to copy to EMEM.
 *
 * The steps performed by the function are as follows:
 * -# Call ga10b_gsp_emem_transfer() with the provided parameters and the direction
 *    set to false, indicating a copy to EMEM operation.
 *
 * @param [in]  g      Pointer to the GPU driver struct.
 * @param [in]  dst    The destination offset in GSP EMEM where data will be copied to.
 * @param [in]  src    Pointer to the source buffer containing the data to be copied.
 * @param [in]  size   The size of the data to be copied in bytes.
 * @param [in]  port   The port number to be used for the copy operation.
 *
 * @return The status of the memory transfer operation. A zero value indicates success,
 *         while a non-zero value indicates an error.
 */
	int (*gsp_copy_to_emem)(struct gk20a *g, u32 dst,
			u8 *src, u32 size, u8 port);
/**
 * @brief Copies data from the GSP EMEM to the destination buffer.
 *
 * This function initiates a transfer of data from the GSP's External Memory (EMEM)
 * at the specified source offset and port to a destination buffer in system memory.
 * It wraps the generic EMEM transfer function with the direction set to copy from EMEM.
 *
 * The steps performed by the function are as follows:
 * -# Call ga10b_gsp_emem_transfer() with the provided parameters and the direction
 *    set to true, indicating a copy from EMEM operation.
 *
 * @param [in]  g      Pointer to the GPU driver struct.
 * @param [in]  src    The source offset in GSP EMEM from where data will be copied.
 * @param [out] dst    Pointer to the destination buffer where the data will be stored.
 * @param [in]  size   The size of the data to be copied in bytes.
 * @param [in]  port   The port number to be used for the copy operation.
 *
 * @return The status of the memory transfer operation. A zero value indicates success,
 *         while a non-zero value indicates an error.
 */
	int (*gsp_copy_from_emem)(struct gk20a *g,
			u32 src, u8 *dst, u32 size, u8 port);
/**
 * @brief Gets or sets the head of a GSP queue.
 *
 * This function either retrieves or updates the head of a specified GSP queue based on the
 * 'set' flag. If 'set' is false, it reads the head of the queue; if 'set' is true, it updates
 * the head of the queue with the provided value.
 *
 * The steps performed by the function are as follows:
 * -# Define the queue head size as 8.
 * -# Check if the queue_id is less than or equal to the last command ID obtained by calling
 *    nvgpu_gsp_get_last_cmd_id().
 *    - If the queue_index is greater than or equal to the queue head size, return -EINVAL.
 *    - If 'set' is false, read the queue head address from the hardware using gk20a_readl()
 *      and pgsp_queue_head_r(), and store it in the provided 'head' variable.
 *    - If 'set' is true, write the new head address to the hardware using gk20a_writel()
 *      and pgsp_queue_head_r().
 * -# If the queue_id is greater than the last command ID:
 *    - If 'set' is false, read the message queue head value from the hardware using
 *      gk20a_readl() and pgsp_msgq_head_r(), and store it in the provided 'head' variable.
 *    - If 'set' is true, write the new message queue head value to the hardware using
 *      gk20a_writel() and pgsp_msgq_head_r().
 * -# Return 0 to indicate success.
 *
 * @param [in]      g            Pointer to the GPU driver struct.
 * @param [in]      queue_id     Identifier of the GSP queue.
 * @param [in]      queue_index  Index of the queue for which the head is to be retrieved or set.
 * @param [in, out] head         Pointer to the variable where the queue head address is stored
 *                               or from which the new head address is read.
 * @param [in]      set          Flag indicating whether to get (false) or set (true) the queue head.
 *
 * @return 0 on success, -EINVAL if the queue index is invalid.
 */
	int (*gsp_queue_head)(struct gk20a *g,
			u32 queue_id, u32 queue_index,
			u32 *head, bool set);
/**
 * @brief Gets or sets the tail of a GSP queue.
 *
 * This function either retrieves or updates the tail of a specified GSP queue based on the
 * 'set' flag. If 'set' is false, it reads the tail of the queue; if 'set' is true, it updates
 * the tail of the queue with the provided value.
 *
 * The steps performed by the function are as follows:
 * -# Define the queue tail size as 8.
 * -# Check if the queue_id is equal to the last command ID obtained by calling
 *    nvgpu_gsp_get_last_cmd_id().
 *    - If the queue_index is greater than or equal to the queue tail size, return -EINVAL.
 *    - If 'set' is false, read the queue tail address from the hardware using gk20a_readl()
 *      and pgsp_queue_tail_r(), and store it in the provided 'tail' variable.
 *    - If 'set' is true, write the new tail address to the hardware using gk20a_writel()
 *      and pgsp_queue_tail_r().
 * -# If the queue_id is not equal to the last command ID:
 *    - If 'set' is false, read the message queue tail value from the hardware using
 *      gk20a_readl() and pgsp_msgq_tail_r(), and store it in the provided 'tail' variable.
 *    - If 'set' is true, write the new message queue tail value to the hardware using
 *      gk20a_writel() and pgsp_msgq_tail_r().
 * -# Return 0 to indicate success.
 *
 * @param [in]      g            Pointer to the GPU driver struct.
 * @param [in]      queue_id     Identifier of the GSP queue.
 * @param [in]      queue_index  Index of the queue for which the tail is to be retrieved or set.
 * @param [in, out] tail         Pointer to the variable where the queue tail address is stored
 *                               or from which the new tail address is read.
 * @param [in]      set          Flag indicating whether to get (false) or set (true) the queue tail.
 *
 * @return 0 on success, -EINVAL if the queue index is invalid.
 */
	int (*gsp_queue_tail)(struct gk20a *g,
			u32 queue_id, u32 queue_index,
			u32 *tail, bool set);
/**
 * @brief Gets or sets the tail of the GSP message queue.
 *
 * This function either retrieves or updates the tail of the GSP message queue based on the
 * 'set' flag. If 'set' is false, it reads the tail of the message queue; if 'set' is true,
 * it updates the tail of the message queue with the provided value.
 *
 * The steps performed by the function are as follows:
 * -# If 'set' is false, read the message queue tail value from the hardware using
 *    gk20a_readl() and store it in the provided 'tail' variable.
 * -# If 'set' is true, write the new message queue tail value to the hardware using
 *    gk20a_writel().
 * -# The 'gsp' parameter is unused in this function.
 *
 * @param [in]      g     Pointer to the GPU driver struct.
 * @param [in]      gsp   Pointer to the GSP struct (unused).
 * @param [in, out] tail  Pointer to the variable where the message queue tail value is stored
 *                        or from which the new tail value is read.
 * @param [in]      set   Flag indicating whether to get (false) or set (true) the message queue tail.
 */
	void (*msgq_tail)(struct gk20a *g, struct nvgpu_gsp *gsp,
			u32 *tail, bool set);
/**
 * @brief Enables or disables the GSP interrupts.
 *
 * This function configures the interrupt controller to either enable or disable
 * interrupts for the GSP (Graphics System Processor). It ensures that the RISCV
 * interrupts are configured as expected by the firmware.
 *
 * The steps performed by the function are as follows:
 * -# Disable the GSP interrupts by calling nvgpu_cic_mon_intr_stall_unit_config()
 *    with NVGPU_CIC_INTR_DISABLE.
 * -# If the 'enable' parameter is true, enable the GSP interrupts by calling
 *    nvgpu_cic_mon_intr_stall_unit_config() with NVGPU_CIC_INTR_ENABLE.
 *    - Note: The actual configuration of RISCV interrupts is expected to be done
 *      by the firmware and is not handled by this function.
 *
 * @param [in]  g       Pointer to the GPU driver struct.
 * @param [in]  enable  Boolean flag to enable or disable GSP interrupts.
 */
	void (*enable_irq)(struct gk20a *g, bool enable);
/**
 * @brief Checks if any supported GSP interrupts are pending.
 *
 * This function reads the GSP interrupt status register and checks against a mask
 * of supported interrupts to determine if any relevant interrupts are pending. It
 * updates the provided interrupt status variable with the raw interrupt status.
 *
 * The steps performed by the function are as follows:
 * -# Read the GSP interrupt status register using the gk20a_readl() function.
 * -# Define a mask of supported GSP interrupts by combining the relevant interrupt
 *    status flags using bitwise OR operations.
 * -# Store the raw interrupt status in the provided interrupt status variable.
 * -# Check if any of the supported interrupts are pending by performing a bitwise
 *    AND operation between the interrupt status and the supported interrupts mask.
 *    If the result is non-zero, return true indicating that an interrupt is pending.
 * -# If no supported interrupts are pending, return false.
 *
 * @param [in]  g     Pointer to the GPU driver struct.
 * @param [out] intr  Pointer to a variable where the raw interrupt status will be stored.
 *
 * @return True if any supported interrupts are pending, false otherwise.
 */
	bool (*gsp_is_interrupted)(struct gk20a *g,  u32 *intr);
/**
 * @brief Interrupt Service Routine for GSP interrupts.
 *
 * This function is the main ISR for handling interrupts generated by the GSP
 * (Graphics System Processor). It checks if the interrupt is supported, acquires
 * the ISR mutex, verifies if the ISR is enabled, and then processes the interrupt
 * if it is within the mask. It also clears any unhandled interrupts.
 *
 * The steps performed by the function are as follows:
 * -# Check if the GSP is interrupted by calling ga10b_gsp_is_interrupted().
 *    - If not, log an error message and return.
 * -# Acquire the ISR mutex to ensure exclusive access to the interrupt handling
 *    by calling nvgpu_gsp_isr_mutex_acquire().
 * -# Check if the ISR is enabled by calling nvgpu_gsp_is_isr_enable().
 *    - If not, release the mutex and exit the function.
 * -# Obtain the mask of enabled GSP interrupts by calling ga10b_gsp_get_irqmask().
 * -# If the interrupt is not within the mask, log the information, clear the
 *    unhandled interrupt by writing to the RISCV IRQ clear register, and exit
 *    the function.
 * -# Perform a bitwise AND operation between the interrupt status and the mask.
 * -# Clear the interrupt by calling ga10b_gsp_clr_intr().
 * -# Handle the interrupts by calling ga10b_gsp_handle_interrupts().
 * -# Release the ISR mutex by calling nvgpu_gsp_isr_mutex_release().
 *
 * @param [in]  g    Pointer to the GPU driver struct.
 * @param [in]  gsp  Pointer to the GSP struct.
 */
    void (*gsp_isr)(struct gk20a *g);
/**
 * @brief Sets the message interrupt for the GSP.
 *
 * This function enables the SWGEN0 (software-generated interrupt 0) for the GSP
 * by writing to the RISCV IRQ mask set register. It is used to trigger an interrupt
 * that the GSP firmware can use to signal completion of a message processing or other
 * events.
 *
 * The steps performed by the function are as follows:
 * -# Write to the RISCV IRQ mask set register using gk20a_writel() to set the SWGEN0
 *    bit, enabling the corresponding interrupt.
 *
 * @param [in]  g  Pointer to the GPU driver struct.
 */
	void (*set_msg_intr)(struct gk20a *g);
/**
 * @brief Handles ECC errors reported by the GSP Falcon.
 *
 * This function checks for various uncorrected ECC errors in the GSP Falcon's
 * IMEM, DMEM, DCLS, REG, and EMEM. It reports these errors to the SDL (Safety
 * Diagnostics Layer) and logs them as errors. If any ECC error is detected, the
 * function returns an error code.
 *
 * The steps performed by the function are as follows:
 * -# Check for an uncorrected ECC error in the IMEM. If an error is detected,
 *    report it to the SDL with the corresponding error code for IMEM ECC uncorrected
 *    errors and set the return value to indicate a fault.
 * -# Check for an uncorrected ECC error in the DMEM. If an error is detected,
 *    report it to the SDL with the corresponding error code for DMEM ECC uncorrected
 *    errors and set the return value to indicate a fault.
 * -# Check for an uncorrected ECC error in the DCLS. If an error is detected,
 *    report it to the SDL with the corresponding error code for DCLS uncorrected
 *    errors and set the return value to indicate a fault.
 * -# Check for an uncorrected ECC error in the REG. If an error is detected,
 *    report it to the SDL with the corresponding error code for REG ECC uncorrected
 *    errors and set the return value to indicate a fault.
 * -# Check for an uncorrected ECC error in the EMEM. If an error is detected,
 *    report it to the SDL with the corresponding error code for EMEM ECC uncorrected
 *    errors and set the return value to indicate a fault.
 *
 * @param [in]  g           Pointer to the GPU driver struct.
 * @param [in]  err_module  Identifier for the module where the error occurred.
 *
 * @return Zero if no ECC errors are detected, otherwise '-EFAULT'.
 */
	int (*gsp_handle_ecc)(struct gk20a *g, u32 err_module);
/**
 * @brief Defines the GSP Falcon queue type for GB20C.
 *
 * This function is responsible for defining the queue type for the GSP Falcon
 * in GB20C. The HAL (Hardware Abstraction Layer) definition
 * implies that the message buffer used for the queue will be located in DMEM
 * (Data Memory). The function does not take any parameters and does not return
 * a value.
 *
 *
 * @return None
 */
	void (*gsp_queue_type)(void);
};

#endif /* NVGPU_GOPS_GSP_H */
