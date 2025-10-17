/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_ACR_H
#define NVGPU_GOPS_ACR_H

/**
 * @brief acr gops.
 *
 * The structure contains function pointers to acr init and execute operations.
 * The details of these callbacks are described in the assigned functions to
 * these pointers.
 */
struct gops_acr {
	/**
	 * Initializes ACR unit private data struct in the GPU driver based on
	 * current chip.
	 */
	int (*acr_init)(struct gk20a *g);
	/**
	 * Construct blob of LS ucode's in non-wpr memory. Load and bootstrap HS
	 * ACR ucode on specified engine Falcon.
	 */
	int (*acr_construct_execute)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_ACR_H */
