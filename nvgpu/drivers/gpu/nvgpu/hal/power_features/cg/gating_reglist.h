/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CG_GATING_REGLIST_H
#define NVGPU_CG_GATING_REGLIST_H

#include <nvgpu/types.h>

struct gating_desc {
	u32 addr;
	u32 prod;
	u32 disable;
};

#endif /* NVGPU_CG_GATING_REGLIST_H */
