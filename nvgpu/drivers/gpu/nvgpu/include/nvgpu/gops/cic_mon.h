/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_CIC_MON_H
#define NVGPU_GOPS_CIC_MON_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * Central Interrupt Controller unit HAL interface
 *
 */
struct gk20a;
struct nvgpu_cic_mon;

/**
 * CIC-MON unit HAL operations
 *
 * @see gpu_ops
 */
struct gops_cic_mon {
	/**
	 * @brief Chip specific CIC unit initialization.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param cic [in] 		Pointer to CIC private struct.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*init)(struct gk20a *g, struct nvgpu_cic_mon *cic_mon);

	/**
	 * @brief Report error to safety services.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param err_id [in]		Error ID.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*report_err)(struct gk20a *g, u32 err_id);

#ifdef CONFIG_NVGPU_FSI_ERR_INJECTION
	/**
	 * @brief Register error injection callback with error
	 *        injection utility.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*reg_errinj_cb)(struct gk20a *g);

	/**
	 * @brief Unregister error injection callback from error
	 *        injection utility.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*dereg_errinj_cb)(void);
#endif
};

#endif/*NVGPU_GOPS_CIC_MON_H*/
