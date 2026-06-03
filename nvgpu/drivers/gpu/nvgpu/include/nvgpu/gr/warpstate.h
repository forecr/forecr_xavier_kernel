/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_WARPSTATE_H
#define NVGPU_GR_WARPSTATE_H

#include <nvgpu/types.h>

struct nvgpu_warpstate {
	u64 valid_warps[2];
	u64 trapped_warps[2];
	u64 paused_warps[2];
};

#endif /* NVGPU_GR_WARPSTATE_H */
