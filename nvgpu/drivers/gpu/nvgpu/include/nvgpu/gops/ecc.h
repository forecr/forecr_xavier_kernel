/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_ECC_H
#define NVGPU_GOPS_ECC_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * ECC HAL interface.
 */
struct gk20a;

/**
 * ECC unit hal operations.
 *
 * This structure stores the ECC unit hal pointers.
 *
 * @see gops
 */
struct gops_ecc {
	/**
	 * @brief Initialize ECC support.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * This function initializes the list head for tracking the list
	 * of ecc error counts for all units (like GR/LTC/FB/PMU) and
	 * subunits of GR (like falcon/sm/gpccs/etc).
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*ecc_init_support)(struct gk20a *g);

	/**
	 * @brief Remove ECC support.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * This function frees all the memory allocated for keeping
	 * track of ecc error counts for each GR engine units.
	 */
	void (*ecc_remove_support)(struct gk20a *g);

	/**
	 * @brief Finish ECC support initialization.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * This function initializes the sysfs nodes for ECC counters and
	 * marks ECC as initialized.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*ecc_finalize_support)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_ECC_H */
