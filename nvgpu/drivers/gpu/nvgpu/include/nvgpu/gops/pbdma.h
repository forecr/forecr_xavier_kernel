/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GOPS_PBDMA_H
#define NVGPU_GOPS_PBDMA_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gpfifo_entry;
struct nvgpu_pbdma_status_info;
struct nvgpu_device;

/** @cond DOXYGEN_SHOULD_SKIP_THIS */

/** NON FUSA */
struct nvgpu_debug_context;
struct nvgpu_channel_dump_info;

struct gops_pbdma_status {
	void (*read_pbdma_status_info)(struct gk20a *g,
		u32 pbdma_id, struct nvgpu_pbdma_status_info *status);
	u32 (*pbdma_status_sched_reg_addr)(struct gk20a *g, u32 pbdma_id);
	u32 (*pbdma_channel_reg_addr)(struct gk20a *g, u32 pbdma_id);
	u32 (*pbdma_next_channel_reg_addr)(struct gk20a *g, u32 pbdma_id);
};
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

struct gops_pbdma {
	/**
	 * @see nvgpu_pbdma_setup_sw().
	 */
	int (*setup_sw)(struct gk20a *g);

	/**
	 * @see nvgpu_pbdma_cleanup_sw().
	 */
	void (*cleanup_sw)(struct gk20a *g);

	/**
	 * @brief Enables or disables PBDMA interrupts.
	 *
	 * This function is responsible for enabling or disabling the interrupts for all
	 * PBDMA units. It iterates through all PBDMA units and either disables and clears
	 * all interrupts if the enable parameter is false, or enables the interrupts and
	 * routes them to tree_0 if the enable parameter is true.
	 *
	 * @param [in] g       Pointer to the GPU driver struct.
	 * @param [in] enable  Boolean flag to enable or disable interrupts.
	 *
	 * Steps:
	 * -# Initialize the PBDMA ID to 0.
	 * -# Initialize the tree variable to 0, which represents the interrupt tree to
	 *    route the interrupts.
	 * -# Retrieve the maximum number of PBDMA units by calling the function
	 *    gops_pbdma.set_clear_intr_offsets() with the INTR_SET_SIZE macro.
	 * -# If the enable parameter is false, call the function
	 *    ga10b_pbdma_disable_and_clear_all_intr() to disable and clear all interrupts
	 *    for all PBDMA units and return from the function.
	 * -# If the enable parameter is true, iterate over each PBDMA unit from 0 to the
	 *    maximum number of PBDMA units.
	 *    - For each PBDMA unit:
	 *      -# Call the function ga10b_pbdma_clear_all_intr() to clear any pending
	 *         interrupts for the current PBDMA unit.
	 *      -# Write to the PBDMA_INTR_0_EN_SET_TREE register for the current PBDMA
	 *         unit to enable interrupts and route them to tree_0 using the bitmask
	 *         provided by gops_pbdma.intr_0_en_set_tree_mask().
	 *      -# Write to the PBDMA_INTR_1_EN_SET_TREE register for the current PBDMA
	 *         unit to enable interrupts and route them to tree_0 using the bitmask
	 *         provided by gops_pbdma.intr_1_en_set_tree_mask().
	 */
	void (*intr_enable)(struct gk20a *g, bool enable);

	/**
	 * @brief Constructs a bitmask for enabling PBDMA interrupts on tree 0.
	 *
	 * This function creates a bitmask that is used to enable various PBDMA interrupts
	 * and route them to interrupt tree 0. The bitmask is a combination of multiple
	 * fields, each corresponding to a specific interrupt type.
	 *
	 * @return The constructed bitmask for enabling PBDMA interrupts on tree 0.
	 *
	 * Steps:
	 * -# Create an unsigned 32-bit integer 'mask' initialized to 0.
	 * -# Set bits in the 'mask' corresponding to the following interrupts by OR'ing
	 *    the respective field values:
	 *    -# GPFIFO interrupt enable field.
	 *    -# GPPTR interrupt enable field.
	 *    -# GPENTRY interrupt enable field.
	 *    -# GPCRC interrupt enable field.
	 *    -# PBPTR interrupt enable field.
	 *    -# PBENTRY interrupt enable field.
	 *    -# PBCRC interrupt enable field.
	 *    -# METHOD interrupt enable field.
	 *    -# DEVICE interrupt enable field.
	 *    -# ENG_RESET interrupt enable field.
	 *    -# SEMAPHORE interrupt enable field.
	 *    -# ACQUIRE interrupt enable field.
	 *    -# PRI interrupt enable field.
	 *    -# PBSEG interrupt enable field.
	 *    -# SIGNATURE interrupt enable field.
	 * -# Return the constructed bitmask.
	 */
	u32 (*intr_0_en_set_tree_mask)(void);

	/**
	 * @brief Constructs a bitmask for clearing PBDMA interrupts on tree 0.
	 *
	 * This function creates a bitmask that is used to clear various PBDMA interrupts
	 * and disable them from interrupt tree 0. The bitmask is a combination of multiple
	 * fields, each corresponding to a specific interrupt type that needs to be cleared.
	 *
	 * @return The constructed bitmask for clearing PBDMA interrupts on tree 0.
	 *
	 * Steps:
	 * -# Create an unsigned 32-bit integer 'mask' initialized to 0.
	 * -# Set bits in the 'mask' corresponding to the following interrupts by OR'ing
	 *    the respective field values:
	 *    -# GPFIFO interrupt clear field.
	 *    -# GPPTR interrupt clear field.
	 *    -# GPENTRY interrupt clear field.
	 *    -# GPCRC interrupt clear field.
	 *    -# PBPTR interrupt clear field.
	 *    -# PBENTRY interrupt clear field.
	 *    -# PBCRC interrupt clear field.
	 *    -# METHOD interrupt clear field.
	 *    -# DEVICE interrupt clear field.
	 *    -# ENG_RESET interrupt clear field.
	 *    -# SEMAPHORE interrupt clear field.
	 *    -# ACQUIRE interrupt clear field.
	 *    -# PRI interrupt clear field.
	 *    -# PBSEG interrupt clear field.
	 *    -# SIGNATURE interrupt clear field.
	 * -# Return the constructed bitmask.
	 */
	u32 (*intr_0_en_clear_tree_mask)(void);

	/**
	 * @brief Constructs a bitmask for enabling PBDMA interrupts on tree 1.
	 *
	 * This function creates a bitmask that is used to enable various PBDMA interrupts
	 * related to the host copy engine (HCE) and route them to interrupt tree 1. The
	 * bitmask is a combination of multiple fields, each corresponding to a specific
	 * interrupt type.
	 *
	 * @return The constructed bitmask for enabling PBDMA interrupts on tree 1.
	 *
	 * Steps:
	 * -# Create an unsigned 32-bit integer 'mask' initialized to 0.
	 * -# Set bits in the 'mask' corresponding to the following interrupts by OR'ing
	 *    the respective field values:
	 *    -# HCE_RE_ILLEGAL_OP interrupt enable field.
	 *    -# HCE_RE_ALIGNB interrupt enable field.
	 *    -# HCE_PRIV interrupt enable field.
	 *    -# HCE_ILLEGAL_MTHD interrupt enable field.
	 *    -# HCE_ILLEGAL_CLASS interrupt enable field.
	 *    -# CTXNOTVALID interrupt enable field.
	 * -# Return the constructed bitmask.
	 */
	u32 (*intr_1_en_set_tree_mask)(void);

	/**
	 * @brief Constructs a bitmask for clearing PBDMA interrupts on tree 1.
	 *
	 * This function creates a bitmask that is used to clear various PBDMA interrupts
	 * related to the host copy engine (HCE) and disable them from interrupt tree 1.
	 * The bitmask is a combination of multiple fields, each corresponding to a specific
	 * interrupt type that needs to be cleared.
	 *
	 * @return The constructed bitmask for clearing PBDMA interrupts on tree 1.
	 *
	 * Steps:
	 * -# Create an unsigned 32-bit integer 'mask' initialized to 0.
	 * -# Set bits in the 'mask' corresponding to the following interrupts by OR'ing
	 *    the respective field values:
	 *    -# HCE_RE_ILLEGAL_OP interrupt clear field.
	 *    -# HCE_RE_ALIGNB interrupt clear field.
	 *    -# HCE_PRIV interrupt clear field.
	 *    -# HCE_ILLEGAL_MTHD interrupt clear field.
	 *    -# HCE_ILLEGAL_CLASS interrupt clear field.
	 *    -# CTXNOTVALID interrupt clear field.
	 * -# Return the constructed bitmask.
	 */
	u32 (*intr_1_en_clear_tree_mask)(void);

	/**
	 * @brief Handles PBDMA interrupt 0 for GA10B.
	 *
	 * This function processes the PBDMA interrupt 0 and determines if recovery is
	 * needed. It checks for engine reset interrupts and logs them. It also calls
	 * the report_error function if it is available in the GPU operations.
	 *
	 * @param [in]  g               Pointer to the GPU driver struct.
	 * @param [in]  pbdma_id        The ID of the PBDMA unit where the interrupt occurred.
	 * @param [in]  pbdma_intr_0    The interrupt status bits for PBDMA interrupt 0.
	 * @param [out] error_notifier  Pointer to the error notifier to be updated.
	 * @return True if recovery is needed, false otherwise.
	 *
	 * Steps:
	 * -# Call the function ga10b_pbdma_handle_intr_0_legacy() with the GPU driver
	 *    struct, PBDMA ID, interrupt status bits, and the error notifier pointer.
	 *    Assign the return value to 'recover', indicating if recovery is needed.
	 * -# Check if the engine reset pending flag is set in the interrupt status bits
	 *    using the macro #pbdma_intr_0_eng_reset_pending_f().
	 *    - If the flag is set, log the occurrence using nvgpu_log() with the debug
	 *      level 'gpu_dbg_intr' and set 'recover' to true.
	 * -# Check if the report_error function is available in the GPU operations.
	 *    - If available, call gops_pbdma.report_error() with the GPU driver
	 *      struct, PBDMA ID, and interrupt status bits.
	 * -# Return the 'recover' flag indicating whether recovery is needed.
	 */
	bool (*handle_intr_0)(struct gk20a *g,
			u32 pbdma_id, u32 pbdma_intr_0,
			u32 *error_notifier);

	/**
	 * @brief Handles PBDMA interrupt 1 and reports errors if any.
	 *
	 * This function checks for PBDMA interrupt 1 status and handles any errors
	 * associated with the "host copy engine" (HCE). The steps are as follows:
	 * -# Initialize the recovery flag to false indicating no recovery action needed
	 *    initially.
	 * -# Read the current PBDMA interrupt 1 status from the hardware using
	 *    nvgpu_readl().
	 * -# Check if the CTXNOTVALID pending bit is not set in the current interrupt
	 *    status to avoid race conditions with GPU clearing the pending interrupt.
	 *    If the bit is not set, clear the CTXNOTVALID pending bit in the local
	 *    copy of the interrupt status.
	 * -# If the local copy of the interrupt status is zero, indicating no pending
	 *    interrupts, return the recovery flag as is.
	 * -# If there are pending interrupts, set the recovery flag to true indicating
	 *    that recovery is needed.
	 * -# Report the HCE error to the Safety Diagnostic Layer (SDL) using
	 *    nvgpu_report_err_to_sdl().
	 * -# If the CTXNOTVALID pending bit is set, log the context not valid interrupt
	 *    and the interrupt status using nvgpu_log() and nvgpu_err().
	 * -# If the CTXNOTVALID pending bit is not set, log that the rest of the
	 *    interrupts are HCE related and not supported, and treat them as channel
	 *    fatal using nvgpu_err().
	 * -# Return the recovery flag indicating whether recovery is needed.
	 *
	 * @param [in]  g                Pointer to the GPU driver struct.
	 * @param [in]  pbdma_id         ID of the PBDMA unit.
	 * @param [in]  pbdma_intr_1     Local copy of PBDMA interrupt 1 status.
	 * @param [out] error_notifier   Pointer to the error notifier.
	 *
	 * @return True if recovery is needed, false otherwise.
	 */
	bool (*handle_intr_1)(struct gk20a *g,
			u32 pbdma_id, u32 pbdma_intr_1,
			u32 *error_notifier);

	/**
	 * @brief Handles PBDMA interrupts and initiates recovery if necessary.
	 *
	 * This function processes PBDMA interrupts for both interrupt 0 and interrupt 1,
	 * logs the information, and attempts recovery if specified and needed. The steps
	 * are as follows:
	 * -# Initialize a PBDMA status information structure.
	 * -# Set the default error notifier value to #NVGPU_ERR_NOTIFIER_PBDMA_ERROR.
	 * -# Initialize the error variable to 0, indicating no error.
	 * -# Read the PBDMA interrupt 0 status from the hardware using nvgpu_readl().
	 * -# Read the PBDMA interrupt 1 status from the hardware using nvgpu_readl().
	 * -# If PBDMA interrupt 0 status is non-zero, log the pending interrupt using
	 *    nvgpu_log().
	 * -# Call the function to handle PBDMA interrupt 0. If it returns true, indicating
	 *    that recovery is needed:
	 *    -# Read the PBDMA status information using gops_pbdma_status.read_pbdma_status_info().
	 *    -# If recovery is requested, attempt to recover from the PBDMA fault using
	 *       nvgpu_rc_pbdma_fault(). If recovery fails, log an error using nvgpu_err().
	 *    -# Write the PBDMA interrupt 0 status back to the hardware to clear the
	 *       interrupt using nvgpu_writel().
	 * -# If PBDMA interrupt 1 status is non-zero, log the pending interrupt using
	 *    nvgpu_log().
	 * -# Call the function to handle PBDMA interrupt 1. If it returns true, indicating
	 *    that recovery is needed and no previous errors occurred:
	 *    -# Read the PBDMA status information using gops_pbdma_status.read_pbdma_status_info().
	 *    -# If recovery is requested, attempt to recover from the PBDMA fault using
	 *       nvgpu_rc_pbdma_fault(). If recovery fails, log an error using nvgpu_err().
	 *    -# Write the PBDMA interrupt 1 status back to the hardware to clear the
	 *       interrupt using nvgpu_writel().
	 * -# Return the error variable indicating the result of the recovery attempt or 0
	 *    if no errors occurred.
	 *
	 * @param [in]  g        Pointer to the GPU driver struct.
	 * @param [in]  pbdma_id ID of the PBDMA unit.
	 * @param [in]  recover  Flag indicating whether to attempt recovery.
	 *
	 * @return 0 if no errors occurred or if recovery was successful, non-zero error
	 *         code otherwise.
	 */
	int (*handle_intr)(struct gk20a *g, u32 pbdma_id, bool recover);

	/**
	 * @brief Dumps the PBDMA interrupt 0 status and associated registers for debugging.
	 *
	 * This function reads various PBDMA registers related to interrupt 0 and logs
	 * their values. The steps are as follows:
	 * -# Read the PBDMA push buffer header register using nvgpu_readl().
	 * -# Read the PBDMA data using the read_data() operation from the PBDMA HAL.
	 * -# Read the PBDMA GP shadow 0 register using nvgpu_readl().
	 * -# Read the PBDMA GP shadow 1 register using nvgpu_readl().
	 * -# Read the PBDMA method 0 register using nvgpu_readl().
	 * -# Read the PBDMA method 1 register using nvgpu_readl().
	 * -# Read the PBDMA method 2 register using nvgpu_readl().
	 * -# Read the PBDMA method 3 register using nvgpu_readl().
	 * -# Log all the read values along with the PBDMA ID and the interrupt 0 status
	 *    using nvgpu_err() to report the error and the register values for
	 *    debugging purposes.
	 *
	 * @param [in] g           Pointer to the GPU driver struct.
	 * @param [in] pbdma_id    ID of the PBDMA unit.
	 * @param [in] pbdma_intr_0 The value of PBDMA interrupt 0 status to be logged.
	 */
	void (*dump_intr_0)(struct gk20a *g, u32 pbdma_id,
				u32 pbdma_intr_0);

	/**
	 * @brief Retrieves the size of PBDMA interrupt set or clear registers.
	 *
	 * This function returns the size of the PBDMA interrupt set or clear registers
	 * based on the input parameter specifying which size to retrieve. The steps are
	 * as follows:
	 * -# Initialize the return value to 0.
	 * -# Use a switch statement to determine the action based on the input parameter
	 *    specifying the size type.
	 *    -# If the size type is #INTR_SIZE, set the return value to the size of the
	 *       PBDMA interrupt 0 register using pbdma_intr_0__size_1_v().
	 *    -# If the size type is #INTR_SET_SIZE, set the return value to the size of
	 *       the PBDMA interrupt 0 enable set tree register using
	 *       pbdma_intr_0_en_set_tree__size_1_v().
	 *    -# If the size type is #INTR_CLEAR_SIZE, set the return value to the size
	 *       of the PBDMA interrupt 0 enable clear tree register using
	 *       pbdma_intr_0_en_clear_tree__size_1_v().
	 *    -# If the input parameter does not match any case, log an error using
	 *       nvgpu_err() indicating an invalid input for set_clear_intr_offset.
	 * -# Return the size value determined by the switch statement.
	 *
	 * @param [in] g               Pointer to the GPU driver struct.
	 * @param [in] set_clear_size  Specifies the type of size to retrieve.
	 *
	 * @return The size of the specified PBDMA interrupt set or clear registers.
	 */
	u32 (*set_clear_intr_offsets) (struct gk20a *g,
			u32 set_clear_size);

	/**
	 * @brief Retrieves the PBDMA signature for GP10B.
	 *
	 * This function obtains the PBDMA signature value specific to the GP10B GPU
	 * architecture. The signature is a combination of the GPFIFO class litter value
	 * and a software-defined zero field.
	 *
	 * @param [in] g  Pointer to the GPU driver struct.
	 * @return The PBDMA signature value for GP10B.
	 *
	 * Steps:
	 * -# Call the function get_litter_value() with the GPU driver struct and the
	 *    macro #GPU_LIT_GPFIFO_CLASS to retrieve the GPFIFO class litter value.
	 * -# OR the retrieved litter value with the software-defined zero field using
	 *    the macro #pbdma_signature_sw_zero_f().
	 * -# Return the resulting PBDMA signature value.
	 */
	u32 (*get_signature)(struct gk20a *g);

	/**
	 * @brief Computes the PBDMA acquire timeout value.
	 *
	 * This function calculates the register value for setting the PBDMA acquire
	 * timeout based on the given timeout period. The timeout is adjusted to 80%
	 * of the given value and converted to nanoseconds (in units of 1024ns) before
	 * being encoded into the register format.
	 *
	 * @param [in] timeout  The timeout period in microseconds.
	 * @return The register value representing the PBDMA acquire timeout.
	 *
	 * Steps:
	 * -# Initialize a 32-bit integer 'val' with the default retry count and exponent
	 *    fields using the macros #pbdma_acquire_retry_man_2_f() and
	 *    #pbdma_acquire_retry_exp_2_f().
	 * -# If the input 'timeout' is zero, return 'val' as it is.
	 * -# Convert the input 'timeout' to 80% of its value in nanoseconds by
	 *    multiplying with (1000000UL	 * 80UL) / 100UL and then dividing by 1024 to
	 *    get the value in units of 1024ns.
	 * -# Initialize 'exponent' to 0. Start a loop to find the smallest exponent such
	 *    that the timeout value is less than or equal to the maximum mantissa value.
	 *    - In each iteration, right shift the 'timeout' by 1 (divide by 2) and
	 *      increment 'exponent' by 1.
	 *    - Continue this process until 'timeout' is less than or equal to the maximum
	 *      mantissa value #pbdma_acquire_timeout_man_max_v() or 'exponent' exceeds
	 *      the maximum exponent value #pbdma_acquire_timeout_exp_max_v().
	 * -# If 'exponent' exceeds the maximum allowed value, clamp 'exponent' to the
	 *    maximum value and set 'mantissa' to the maximum mantissa value.
	 *    Otherwise, cast 'timeout' to a 32-bit integer and assign it to 'mantissa'.
	 * -# Combine the calculated 'exponent' and 'mantissa' into 'val' using the
	 *    macros #pbdma_acquire_timeout_exp_f() and #pbdma_acquire_timeout_man_f().
	 * -# Enable the acquire timeout by OR'ing 'val' with the macro
	 *    #pbdma_acquire_timeout_en_enable_f().
	 * -# Return the updated 'val' which now represents the PBDMA acquire timeout
	 *    register value.
	 */
	u32 (*acquire_val)(u64 timeout);

	/**
	 * @brief Reads the PBDMA header shadow register data.
	 *
	 * This function performs a single operation:
	 * -# Return the value of the PBDMA header shadow register for the specified
	 *    PBDMA unit by calling nvgpu_readl() with the appropriate register offset
	 *    obtained from pbdma_hdr_shadow_r().
	 *
	 * @param [in] g         Pointer to the GPU driver struct.
	 * @param [in] pbdma_id  ID of the PBDMA unit from which to read the data.
	 *
	 * @return The value read from the PBDMA header shadow register.
	 */
	u32 (*read_data)(struct gk20a *g, u32 pbdma_id);

	/**
	 * @brief Resets the PBDMA push buffer header register.
	 *
	 * This function performs the following operation:
	 * -# Write to the PBDMA push buffer header register to reset it by setting the
	 *    FIRST bit and the TYPE to non-incrementing. This is done by calling
	 *    nvgpu_writel() with the register offset obtained from pbdma_pb_header_r()
	 *    and the value constructed by OR'ing #pbdma_pb_header_first_true_f() and
	 *    #pbdma_pb_header_type_non_inc_f().
	 *
	 * @param [in] g         Pointer to the GPU driver struct.
	 * @param [in] pbdma_id  ID of the PBDMA unit whose header is to be reset.
	 */
	void (*reset_header)(struct gk20a *g, u32 pbdma_id);

	/**
	 * @brief Resets a PBDMA method register to a NOP instruction.
	 *
	 * This function performs the following operations:
	 * -# Calculate the stride between method registers by subtracting the offset of
	 *    the first method register from the second using nvgpu_safe_sub_u32().
	 * -# Calculate the register offset for the specified method index by adding the
	 *    base offset of the first method register to the product of the method index
	 *    and the stride using nvgpu_safe_add_u32() and nvgpu_safe_mult_u32().
	 * -# Write a NOP instruction to the calculated method register offset by calling
	 *    nvgpu_writel(). The value written is a combination of flags indicating the
	 *    method is valid, it's the first method, and the address of the NOP
	 *    instruction, which is obtained by shifting the result of pbdma_udma_nop_r()
	 *    right by 2 bits to fit the register field.
	 *
	 * @param [in] g                  Pointer to the GPU driver struct.
	 * @param [in] pbdma_id           ID of the PBDMA unit whose method is to be reset.
	 * @param [in] pbdma_method_index Index of the method register to reset.
	 */
	void (*reset_method)(struct gk20a *g, u32 pbdma_id,
			u32 pbdma_method_index);

	/**
	 * @brief Returns the bitmask representing fatal device errors in PBDMA.
	 *
	 * This function constructs a bitmask that represents fatal errors in the PBDMA
	 * (Push Buffer DMA) unit of the device. These errors are considered critical and
	 * indicate serious issues with the device operation.
	 *
	 * Steps:
	 * -# Define a variable to hold the bitmask of fatal device errors.
	 * -# Assign the bitmask with the value representing a PRI (privileged) pending
	 *    fatal error using the macro #pbdma_intr_0_pri_pending_f().
	 * -# Return the constructed bitmask.
	 *
	 * @return The bitmask representing fatal device errors in PBDMA.
	 */
	u32 (*device_fatal_0_intr_descs)(void);

	/**
	 * @brief Returns the bitmask representing fatal channel errors in PBDMA.
	 *
	 * This function constructs a bitmask that represents various fatal errors related
	 * to data parsing, framing, and other errors that can potentially be recovered
	 * from by intervention or by resetting the channel in the PBDMA (Push Buffer DMA)
	 * unit of the device.
	 *
	 * Steps:
	 * -# Define a variable to hold the bitmask of fatal channel errors.
	 * -# Assign the bitmask with the combined values representing different types of
	 *    fatal channel errors using the bitwise OR operator '|'. Each error type is
	 *    represented by a specific macro:
	 *    -# #pbdma_intr_0_gpfifo_pending_f() for GP FIFO pending errors.
	 *    -# #pbdma_intr_0_gpptr_pending_f() for GP pointer pending errors.
	 *    -# #pbdma_intr_0_gpentry_pending_f() for GP entry pending errors.
	 *    -# #pbdma_intr_0_gpcrc_pending_f() for GP CRC mismatch errors.
	 *    -# #pbdma_intr_0_pbptr_pending_f() for PB pointer pending errors.
	 *    -# #pbdma_intr_0_pbentry_pending_f() for PB entry invalid errors.
	 *    -# #pbdma_intr_0_pbcrc_pending_f() for PB CRC mismatch errors.
	 *    -# #pbdma_intr_0_method_pending_f() for method pending errors.
	 *    -# #pbdma_intr_0_pbseg_pending_f() for PB segment errors.
	 *    -# #pbdma_intr_0_eng_reset_pending_f() for engine reset pending errors.
	 *    -# #pbdma_intr_0_semaphore_pending_f() for semaphore pending errors.
	 *    -# #pbdma_intr_0_signature_pending_f() for signature mismatch errors.
	 * -# Return the constructed bitmask.
	 *
	 * @return The bitmask representing fatal channel errors in PBDMA.
	 */
	u32 (*channel_fatal_0_intr_descs)(void);

	/**
	 * @brief Returns the bitmask representing restartable errors in PBDMA.
	 *
	 * This function constructs a bitmask that represents errors in the PBDMA
	 * (Push Buffer DMA) unit of the device that are considered restartable. These
	 * errors typically involve software methods or represent timeouts that can be
	 * recovered from without requiring a full device reset.
	 *
	 * Steps:
	 * -# Define a variable to hold the bitmask of restartable errors.
	 * -# Assign the bitmask with the value representing a device pending error that
	 *    is restartable using the macro #pbdma_intr_0_device_pending_f().
	 * -# Return the constructed bitmask.
	 *
	 * @return The bitmask representing restartable errors in PBDMA.
	 */
	u32 (*restartable_0_intr_descs)(void);

	/**
	 * @brief Computes the GP base register value from the given GP FIFO base address.
	 *
	 * This function calculates the value to be programmed into the GP base register
	 * of the PBDMA (Push Buffer DMA) unit, based on the base address of the GP FIFO.
	 * It shifts the base address to account for reserved bits and applies a mask to
	 * obtain the appropriate register value.
	 *
	 * Steps:
	 * -# Call #u64_lo32() to extract the lower 32 bits of the GP FIFO base address.
	 * -# Right shift the lower 32 bits of the GP FIFO base address by the number of
	 *    reserved bits using #pbdma_gp_base_rsvd_s().
	 * -# Call #pbdma_gp_base_offset_f() to apply the mask and shift necessary to
	 *    obtain the GP base register value.
	 * -# Return the computed GP base register value.
	 *
	 * @param [in]  gpfifo_base  The base address of the GP FIFO.
	 *
	 * @return The value to be programmed into the GP base register.
	 */
	u32 (*get_gp_base)(u64 gpfifo_base);

	/**
	 * @brief Computes the high part of the GP base register value from the given GP FIFO base address and entry size.
	 *
	 * This function calculates the high part of the value to be programmed into the GP base register
	 * of the PBDMA (Push Buffer DMA) unit, based on the high 32 bits of the GP FIFO base address and
	 * the size of the GP FIFO entry. It applies a mask to obtain the appropriate register value and
	 * calculates the limit field based on the log2 of the GP FIFO entry size.
	 *
	 * Steps:
	 * -# Call #u64_hi32() to extract the higher 32 bits of the GP FIFO base address.
	 * -# Call #pbdma_gp_base_hi_offset_f() to apply the mask necessary to obtain the
	 *    high offset part of the GP base register value.
	 * -# Call #nvgpu_ilog2() to compute the binary logarithm of the GP FIFO entry size.
	 * -# Cast the result of the binary logarithm to a 32-bit unsigned integer using
	 *    #nvgpu_safe_cast_u64_to_u32().
	 * -# Call #pbdma_gp_base_hi_limit2_f() to apply the mask and shift necessary to
	 *    obtain the high limit part of the GP base register value.
	 * -# Combine the high offset and high limit parts using the bitwise OR operator '|'.
	 * -# Return the computed high part of the GP base register value.
	 *
	 * @param [in]  gpfifo_base   The base address of the GP FIFO.
	 * @param [in]  gpfifo_entry  The size of the GP FIFO entry.
	 *
	 * @return The high part of the value to be programmed into the GP base register.
	 */
	u32 (*get_gp_base_hi)(u64 gpfifo_base, u32 gpfifo_entry);

	/**
	 * @brief Constructs the full control PB (Push Buffer) header value for flow control.
	 *
	 * This function constructs the PB header value that is used for flow control in the
	 * PBDMA (Push Buffer DMA) unit. The header includes fields for method, subchannel,
	 * level, first, and type, which are all set to specific values indicating an incremental
	 * method operation that is the first in a new PB segment.
	 *
	 * Steps:
	 * -# Combine the following macros using the bitwise OR operator '|':
	 *    -# #pbdma_pb_header_method_zero_f() to set the method field to zero.
	 *    -# #pbdma_pb_header_subchannel_zero_f() to set the subchannel field to zero.
	 *    -# #pbdma_pb_header_level_main_f() to set the level field to indicate the main level.
	 *    -# #pbdma_pb_header_first_true_f() to indicate this is the first PB header.
	 *    -# #pbdma_pb_header_type_inc_f() to set the type field to indicate an incremental method.
	 * -# Return the constructed PB header value.
	 *
	 * @return The PB header value for flow control.
	 */
	u32 (*get_fc_pb_header)(void);

	/**
	 * @brief Constructs the flow control subdevice register value for PBDMA.
	 *
	 * This function constructs the subdevice register value that is used for flow control
	 * in the PBDMA (Push Buffer DMA) unit. The register value includes the subdevice ID,
	 * the status indicating whether the subdevice is active, and whether channel DMA is
	 * enabled for the subdevice.
	 *
	 * Steps:
	 * -# Combine the following macros using the bitwise OR operator '|':
	 *    -# #pbdma_subdevice_id_f() with the argument #PBDMA_SUBDEVICE_ID to set the subdevice ID.
	 *    -# #pbdma_subdevice_status_active_f() to indicate the subdevice is active.
	 *    -# #pbdma_subdevice_channel_dma_enable_f() to enable channel DMA for the subdevice.
	 * -# Return the constructed subdevice register value.
	 *
	 * @return The subdevice register value for flow control.
	 */
	u32 (*get_fc_subdevice)(void);

	/**
	 * @brief Constructs the flow control target register value for PBDMA.
	 *
	 * This function constructs the target register value for flow control in the PBDMA
	 * (Push Buffer DMA) unit based on the given device's runlist engine ID. It sets the
	 * target engine field, and marks both the engine and copy engine context as valid.
	 *
	 * Steps:
	 * -# Combine the following macros using the bitwise OR operator '|':
	 *    -# #pbdma_target_engine_f() with the device's runlist engine ID to set the target engine.
	 *    -# #pbdma_target_eng_ctx_valid_true_f() to indicate that the engine context is valid.
	 *    -# #pbdma_target_ce_ctx_valid_true_f() to indicate that the copy engine context is valid.
	 * -# Return the constructed target register value.
	 *
	 * @param [in]  dev  A pointer to the device structure containing the runlist engine ID.
	 *
	 * @return The target register value for flow control.
	 */
	u32 (*get_fc_target)(const struct nvgpu_device *dev);

	/**
	 * @brief Retrieves the control value to enable HCE (Host Copy Engine) privileged mode.
	 *
	 * This function returns the register value that enables privileged mode for the HCE
	 * within the PBDMA (Push Buffer DMA) unit. Privileged mode is required for certain
	 * operations that the HCE performs.
	 *
	 * Steps:
	 * -# Call #pbdma_hce_ctrl_hce_priv_mode_yes_f() to retrieve the register value that
	 *    sets the HCE to operate in privileged mode.
	 * -# Return the register value that enables HCE privileged mode.
	 *
	 * @return The register value to enable HCE privileged mode.
	 */
	u32 (*get_ctrl_hce_priv_mode_yes)(void);

	/**
	 * @brief Retrieves the privileged authorization level configuration value.
	 *
	 * This function returns a constant value that represents the privileged
	 * authorization level for the PBDMA (Push Buffer DMA) unit. The value is
	 * defined in the hardware specification and is used to configure the PBDMA
	 * unit to operate in privileged mode.
	 *
	 * -# Calls the macro 'pbdma_config_auth_level_privileged_f()' to obtain the
	 *    predefined constant value that sets the authorization level of the PBDMA
	 *    to privileged.
	 *
	 * @return The constant value representing the privileged authorization level
	 *         for PBDMA configuration.
	 */
	u32 (*get_config_auth_level_privileged)(void);

	/**
	 * @brief Sets the VEID (Virtual Engine ID) in the PBDMA channel info register.
	 *
	 * This function encodes the subcontext ID into the format required by the
	 * PBDMA channel info register to set the VEID field. The VEID is used to
	 * identify the virtual engine associated with the channel.
	 *
	 * -# Calls the macro 'pbdma_set_channel_info_veid_f()' with the provided
	 *    subcontext ID to encode it into the register format for setting the VEID.
	 *
	 * @param [in] subctx_id The subcontext ID to be set as the VEID.
	 *
	 * @return The encoded value to be written to the PBDMA channel info register
	 *         to set the VEID.
	 */
	u32 (*set_channel_info_veid)(u32 subctx_id);

	/**
	 * @brief Enable USERD writeback configuration.
	 *
	 * This function enables the USERD writeback configuration by setting the
	 * appropriate field in the PBDMA configuration register. The steps are:
	 * -# Use 'set_field()' to modify the input value 'v' by enabling the USERD
	 *    writeback. This is done by masking the USERD writeback field using
	 *    '#pbdma_config_userd_writeback_m()' and then setting the field to the
	 *    enabled state using '#pbdma_config_userd_writeback_enable_f()'.
	 * -# Return the modified configuration value.
	 *
	 * @param [in] v The original PBDMA configuration register value.
	 *
	 * @return The modified PBDMA configuration register value with USERD writeback enabled.
	 */
	u32 (*config_userd_writeback_enable)(u32 v);

	/**
	 * @brief Encodes the channel ID into the PBDMA channel info register format.
	 *
	 * This function takes a channel ID and encodes it into the format required by
	 * the PBDMA channel info register. The encoded value is used to set the CHID
	 * (Channel ID) field in the register, which uniquely identifies a channel
	 * within the PBDMA.
	 *
	 * -# Calls the macro 'pbdma_set_channel_info_chid_f()' with the provided
	 *    channel ID to encode it into the register format for setting the CHID.
	 *
	 * @param [in] chid The channel ID to be encoded into the PBDMA channel info
	 *                  register format.
	 *
	 * @return The encoded value to be written to the PBDMA channel info register
	 *         to set the CHID.
	 */
	u32 (*set_channel_info_chid)(u32 chid);

	/**
	 * @brief Configures the interrupt notification settings for PBDMA.
	 *
	 * This function sets up the interrupt notification control for a PBDMA unit. It
	 * specifies the engine interrupt vector to use, disables notification to the
	 * GSP (Guest Signal Processor), and enables notification to the CPU.
	 *
	 * -# Encodes the engine interrupt vector using the macro
	 *    'pbdma_intr_notify_vector_f()' with the provided engine interrupt vector.
	 * -# Disables GSP interrupt notification by using the macro
	 *    'pbdma_intr_notify_ctrl_gsp_disable_f()' which sets the corresponding bit
	 *    in the interrupt notify control register to disable GSP notifications.
	 * -# Enables CPU interrupt notification by using the macro
	 *    'pbdma_intr_notify_ctrl_cpu_enable_f()' which sets the corresponding bit
	 *    in the interrupt notify control register to enable CPU notifications.
	 * -# Combines the above settings using bitwise OR to form the final value to be
	 *    written to the PBDMA interrupt notify control register.
	 *
	 * @param [in] eng_intr_vector The engine interrupt vector number to be used for
	 *                             PBDMA interrupt notifications.
	 *
	 * @return The combined encoded value of the engine interrupt vector and the
	 *         interrupt notification control settings.
	 */
	u32 (*set_intr_notify)(u32 eng_intr_vector);

	/**
	 * @brief Retrieve the MMU fault ID for a given PBDMA unit.
	 *
	 * This function reads the PBDMA configuration register to extract the MMU fault
	 * ID associated with the specified PBDMA unit. The steps are:
	 * -# Read the PBDMA configuration register 0 for the given PBDMA ID using
	 *    'nvgpu_readl()'.
	 * -# Extract the MMU fault ID from the read configuration value using
	 *    '#pbdma_cfg0_pbdma_fault_id_v()'.
	 * -# Return the extracted MMU fault ID.
	 *
	 * @param [in] g        Pointer to the GPU driver struct.
	 * @param [in] pbdma_id ID of the PBDMA unit for which to retrieve the MMU fault ID.
	 *
	 * @return The MMU fault ID associated with the specified PBDMA unit.
	 */
	u32 (*get_mmu_fault_id)(struct gk20a *g, u32 pbdma_id);

	/**
	 * @brief Get the number of PBDMA units available.
	 *
	 * This function returns the number of PBDMA units by retrieving the predefined
	 * constant value that represents the size of the PBDMA configuration register 0.
	 * The steps are:
	 * -# Return the value obtained from '#pbdma_cfg0__size_1_v()' which represents
	 *    the number of PBDMA units.
	 *
	 * @return The number of PBDMA units available.
	 */
	u32 (*get_num_of_pbdmas)(void);

	/**
	 * @brief Report PBDMA errors based on the interrupt status.
	 *
	 * This function analyzes the PBDMA interrupt status and reports the error type
	 * based on the bits set in the interrupt status. The steps are:
	 * -# Initialize the error type to #GPU_HOST_INVALID_ERROR.
	 * -# Check if any of the GPFIFO or PB (push buffer) related interrupt bits are set
	 *    by performing a bitwise AND operation with the corresponding pending function
	 *    masks. If any are set, update the error type to #GPU_HOST_PBDMA_GPFIFO_PB_ERROR.
	 * -# Check if any of the method or device related interrupt bits are set by
	 *    performing a similar bitwise AND operation. If any are set, update the error
	 *    type to #GPU_HOST_PBDMA_METHOD_ERROR.
	 * -# Check if the signature error interrupt bit is set. If it is, update the error
	 *    type to #GPU_HOST_PBDMA_SIGNATURE_ERROR.
	 * -# If the error type is not #GPU_HOST_INVALID_ERROR, log the error with the
	 *    PBDMA ID and interrupt status using 'nvgpu_err()' and report the error to
	 *    the Safety Diagnostic Layer (SDL) using 'nvgpu_report_err_to_sdl()'.
	 *
	 * @param [in] g           Pointer to the GPU driver struct.
	 * @param [in] pbdma_id    ID of the PBDMA unit where the error occurred.
	 * @param [in] pbdma_intr_0 Interrupt status register value for the PBDMA unit.
	 */
	void (*report_error)(struct gk20a *g, u32 pbdma_id,
		u32 pbdma_intr_0);

	/**
	 * @brief Handles the PBDMA interrupt for semaphore acquire timeout.
	 *
	 * This function checks if a semaphore acquire timeout has occurred and handles
	 * it by disabling the timeout and setting the error notifier if timeouts are
	 * enabled. It also triggers a GPU debug dump.
	 *
	 * @param [in]  g               Pointer to the GPU driver struct.
	 * @param [in]  pbdma_id        The ID of the PBDMA unit where the interrupt occurred.
	 * @param [in]  pbdma_intr_0    The interrupt status bits for PBDMA interrupt 0.
	 * @param [out] error_notifier  Pointer to the error notifier to be updated.
	 * @return True if recovery is needed due to a semaphore acquire timeout, false otherwise.
	 *
	 * Steps:
	 * -# Initialize a boolean 'recover' to false, indicating no recovery is needed by default.
	 * -# Check if the semaphore acquire pending flag is set in the interrupt status bits
	 *    using the macro #pbdma_intr_0_acquire_pending_f().
	 *    - If the flag is set:
	 *      -# Read the current value of the PBDMA_ACQUIRE register using nvgpu_readl().
	 *      -# Clear the acquire timeout enable bit in the register value using the bitwise
	 *         AND operator with the negated macro #pbdma_acquire_timeout_en_enable_f().
	 *      -# Write the updated value back to the PBDMA_ACQUIRE register using nvgpu_writel().
	 *      -# Check if timeouts are enabled using nvgpu_is_timeouts_enabled().
	 *         - If timeouts are enabled:
	 *           -# Set 'recover' to true, indicating recovery is needed.
	 *           -# Log an error message indicating a semaphore acquire timeout using nvgpu_err().
	 *           -# Trigger a GPU debug dump using gk20a_debug_dump().
	 *           -# Update the error notifier to indicate a semaphore acquire timeout using the
	 *              constant #NVGPU_ERR_NOTIFIER_GR_SEMAPHORE_TIMEOUT.
	 * -# Return the 'recover' flag indicating whether recovery is needed.
	 */
	bool (*handle_intr_0_acquire)(struct gk20a *g, u32 pbdma_id,
			u32 pbdma_intr_0, u32 *error_notifier);

	/**
	 * @brief Check if the method sub-channel is a software method sub-channel.
	 *
	 * This function determines if a given method index corresponds to a software
	 * method sub-channel for a particular PBDMA unit. The software method
	 * sub-channels are identified by specific sub-channel numbers. The steps are:
	 * -# Calculate the stride between method registers by subtracting the address
	 *    of the first method register from the address of the second method register
	 *    using 'pbdma_method1_r()' and 'pbdma_method0_r()' respectively.
	 * -# Calculate the register address for the method index by adding the base
	 *    address of the first method register to the product of the method index
	 *    and the stride calculated in the previous step. This uses 'pbdma_method0_r()',
	 *    'nvgpu_safe_mult_u32()', and 'nvgpu_safe_add_u32()'.
	 * -# Read the value from the calculated method register address using 'nvgpu_readl()'.
	 * -# Extract the sub-channel number from the method register value using
	 *    'pbdma_method0_subch_v()'.
	 * -# Check if the extracted sub-channel number is one of the software method
	 *    sub-channels (5, 6, or 7). If it is, return true, indicating that it is a
	 *    software method sub-channel. Otherwise, return false.
	 *
	 * @param [in]  g                Pointer to the GPU driver struct.
	 * @param [in]  pbdma_id         ID of the PBDMA unit being queried.
	 * @param [in]  pbdma_method_index Index of the method being checked.
	 *
	 * @return True if the method index corresponds to a software method sub-channel,
	 *         false otherwise.
	 */
	bool (*is_sw_method_subch)(struct gk20a *g, u32 pbdma_id,
						u32 pbdma_method_index);

	/**
	 * @brief Retrieves the bitmask for PBDMA interrupt 0 PBCRC pending status.
	 *
	 * This function returns the bitmask that represents the PBCRC pending status in
	 * the PBDMA interrupt 0 register.
	 *
	 * @return The bitmask for PBDMA interrupt 0 PBCRC pending status.
	 *
	 * Steps:
	 * -# Return the bitmask for the PBCRC pending status using the macro
	 *    #pbdma_intr_0_pbcrc_pending_f().
	 */
	u32 (*intr_0_pbcrc_pending)(void);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	u32 (*get_fc_formats)(void);
	u32 (*get_userd_aperture_mask)(struct gk20a *g,
			struct nvgpu_mem *mem);
	u32 (*get_userd_addr)(u32 addr_lo);
	u32 (*get_userd_hi_addr)(u32 addr_hi);
	u32 (*get_fc_runlist_timeslice)(void);
	u32 (*allowed_syncpoints_0_index_f)(u32 syncpt);
	u32 (*allowed_syncpoints_0_valid_f)(void);
	u32 (*allowed_syncpoints_0_index_v)(u32 offset);
	void (*format_gpfifo_entry)(struct gk20a *g,
			struct nvgpu_gpfifo_entry *gpfifo_entry,
			u64 pb_gpu_va, u32 method_size);
	void (*setup_hw)(struct gk20a *g);
	u32 (*intr_0_reg_addr)(struct gk20a *g, u32 pbdma_id);
	u32 (*intr_1_reg_addr)(struct gk20a *g, u32 pbdma_id);
	u32 (*hce_dbg0_reg_addr)(struct gk20a *g, u32 pbdma_id);
	u32 (*hce_dbg1_reg_addr)(struct gk20a *g, u32 pbdma_id);
	/** NON FUSA */
	void (*syncpt_debug_dump)(struct gk20a *g,
			struct nvgpu_debug_context *o,
			struct nvgpu_channel_dump_info *info);
	void (*dump_status)(struct gk20a *g,
			struct nvgpu_debug_context *o);
#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
	void (*pbdma_force_ce_split)(struct gk20a *g);
#endif
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
};


#endif
