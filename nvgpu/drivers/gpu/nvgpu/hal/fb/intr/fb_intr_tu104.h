/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_INTR_TU104_H
#define NVGPU_FB_INTR_TU104_H

#include <nvgpu/types.h>

struct gk20a;

void tu104_fb_intr_enable(struct gk20a *g);
void tu104_fb_intr_disable(struct gk20a *g);
void tu104_fb_intr_isr(struct gk20a *g, u32 intr_unit_bitmask);
bool tu104_fb_intr_is_mmu_fault_pending(struct gk20a *g);

#endif /* NVGPU_FB_INTR_TU104_H */
