/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CBC_TU104_H
#define CBC_TU104_H

#ifdef CONFIG_NVGPU_COMPRESSION

#include <nvgpu/types.h>

enum nvgpu_cbc_op;
struct gk20a;
struct nvgpu_cbc;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
int tu104_cbc_alloc_comptags(struct gk20a *g, struct nvgpu_cbc *cbc);
#endif
/**
 * @brief This function is used to for different compbit cache operations.
 *
 * Get the cache operation from #op.
 * Configure the required register with required values #min and
 *  #max and perform the cache operation.
 *
 * @param g    [in]    Pointer to the gk20a structure.
 * @param op   [in]    Cache operation in #nvgpu_cbc_op structure.
 * @param min  [in]    Min value where the cache operation starts.
 * @param max  [in]    Max value where the cache operation ends.
 *
 * @return 0 for success, < 0 for failures.
 */

int tu104_cbc_ctrl(struct gk20a *g, enum nvgpu_cbc_op op,
		       u32 min, u32 max);

#endif
#endif
