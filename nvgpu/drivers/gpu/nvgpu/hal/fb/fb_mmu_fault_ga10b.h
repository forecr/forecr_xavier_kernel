/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_MMU_FAULT_GA10B_H
#define NVGPU_FB_MMU_FAULT_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

void ga10b_fb_handle_mmu_fault(struct gk20a *g, u32 intr_unit_bitmask);

#endif /* NVGPU_FB_MMU_FAULT_GA10B_H */

