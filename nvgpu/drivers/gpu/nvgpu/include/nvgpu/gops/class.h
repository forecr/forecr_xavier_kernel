/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_CLASS_H
#define NVGPU_GOPS_CLASS_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * class unit HAL interface
 * @page class class
 *
 * Overview
 * ========
 *
 * The class unit of |nvgpu-rm| provides:
 *
 * - List of valid class numbers supported in our GPU architecture.
 * - API to query if the given class number is supported.
 * - API to query if the given class number belongs to compute class.
 *
 * Data Structures
 * ===============
 *
 * The class unit exposes following list of supported class numbers:
 *
 *  + @ref NVGPU_CLASS_VALID_NUM
 *
 * Static Design
 * =============
 *
 * External APIs
 * -------------
 *
 *   + nvgpu_falcon_sw_init()
 *   + gops_class.is_valid
 *   + gops_class.is_valid_compute
 */

/**
 * class unit HAL operations
 *
 * @see gpu_ops
 */
struct gops_class {
	/**
	 * @brief Checks if given class number is valid as per our GPU
	 *        architechture. This API is used by common.gr unit to
	 *        validate the class associated with the channel.
	 *
	 * List of valid class numbers:
	 * 1. Compute class:
	 * 	- \ref #VOLTA_COMPUTE_A           -> 0xC3C0U
	 * 2. DMA copy class:
	 *	- \ref #VOLTA_DMA_COPY_A          -> 0xC3B5U
	 * 3. Channel Gpfifo class:
	 *	- \ref #VOLTA_CHANNEL_GPFIFO_A    -> 0xC36FU
	 * 4. Graphics class:
	 *	- \ref #VOLTA_A                   -> 0xC397U
	 * 5. NVENC class:
	 *	- \ref #TURING_NVENC_A            -> 0xC4B7U
	 *
	 * @param class_num [in]	Class number to be validated based on
	 *                              GPU architecture.
	 *				- No validation is performed on this
	 * 				  parameter
	 *
	 * @return true when \a class_num is one of the numbers in above list or
	 *	   false otherwise.
	 */
	bool (*is_valid)(u32 class_num);

	/**
	 * @brief Checks if given class number is valid compute class number
	 * 	  as per our GPU architechture. This API is used by common.gr
	 *        unit to set apart the compute class from other classes.
	 *        This is needed when the preemption mode is selected based
	 *        on the class type.
	 *
	 * List of valid compute class numbers:
	 * 	- \ref #VOLTA_COMPUTE_A           -> 0xC3C0U
	 *
	 * @param class_num [in]	Class number to be validated based on
	 * 				GPU architecture.
	 *				- No validation is performed on this
	 * 				  parameter
	 *
	 * @return true when \a class_num is one of the numbers in above list or
	 *	   false otherwise.
	 */
	bool (*is_valid_compute)(u32 class_num);

	/** @cond DOXYGEN_SHOULD_SKIP_THIS */
#ifdef CONFIG_NVGPU_GRAPHICS
	bool (*is_valid_gfx)(u32 class_num);
#endif
	/**
	 * @brief Checks if given class number is valid multimedia class number
	 *        as per our GPU architecture.
	 *
	 * List of valid multimedia class numbers:
	 *	- \ref #TURING_NVENC_A           -> 0xC4B7U
	 *
	 * @param class_num [in]	Class number to be validated based on
	 *				GPU architecture.
	 *				- No validation is performed on this
	 *				  parameter
	 *
	 * @return true when \a class_num is one of the numbers in above list or
	 *	   false otherwise.
	 */
	bool (*is_valid_multimedia)(u32 class_num);

	/**
	 * @brief Checks if given class number is valid nvenc class number
	 *        as per our GPU architecture.
	 *
	 * List of valid nvenc class numbers:
	 *	- \ref #TURING_NVENC_A           -> 0xC4B7U
	 *
	 * @param class_num [in]	Class number to be validated based on
	 *				GPU architecture.
	 *				- No validation is performed on this
	 *				  parameter
	 *
	 * @return true when \a class_num is one of the numbers in above list or
	 *	   false otherwise.
	 */
	bool (*is_valid_nvenc)(u32 class_num);

	/**
	 * @brief Checks if given class number is valid ofa class number
	 *        as per our GPU architecture.
	 *
	 * @param class_num [in]	Class number to be validated based on
	 *				GPU architecture.
	 *				- No validation is performed on this
	 *				  parameter
	 *
	 * @return true when \a class_num is matching with one of the ofa class,
	 *	   false otherwise.
	 */
	bool (*is_valid_ofa)(u32 class_num);

	/**
	 * @brief Checks if given class number is valid nvdec class number
	 *        as per our GPU architecture.
	 *
	 * @param class_num [in]	Class number to be validated based on
	 *				GPU architecture.
	 *				- No validation is performed on this
	 *				  parameter
	 *
	 * @return true when \a class_num is matching with one of the nvdec class,
	 *	   false otherwise.
	 */
	bool (*is_valid_nvdec)(u32 class_num);

	/**
	 * @brief Checks if given class number is valid nvjpg class number
	 *        as per our GPU architecture.
	 *
	 * @param class_num [in]	Class number to be validated based on
	 *				GPU architecture.
	 *				- No validation is performed on this
	 *				  parameter
	 *
	 * @return true when \a class_num is matching with one of the nvjpg class,
	 *	   false otherwise.
	 */
	bool (*is_valid_nvjpg)(u32 class_num);
	/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
};

#endif /* NVGPU_GOPS_CLASS_H */
