/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_HAL_INIT_H
#define NVGPU_HAL_INIT_H

struct gk20a;

/**
 * @brief Detect GPU and initialize the HAL.
 *
 * @param g [in] The GPU
 *
 * Initializes GPU units in the GPU driver. Each sub-unit is responsible for HW
 * initialization.
 *
 * Note: Requires the GPU is already powered on.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_detect_chip(struct gk20a *g);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
int nvgpu_init_hal(struct gk20a *g);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

#endif /* NVGPU_HAL_INIT_H */
