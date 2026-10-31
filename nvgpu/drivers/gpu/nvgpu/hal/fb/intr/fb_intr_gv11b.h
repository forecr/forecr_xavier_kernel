/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_INTR_GV11B_H
#define NVGPU_FB_INTR_GV11B_H

struct gk20a;

void gv11b_fb_intr_enable(struct gk20a *g);
void gv11b_fb_intr_disable(struct gk20a *g);
void gv11b_fb_intr_isr(struct gk20a *g, u32 intr_unit_bitmask);
bool gv11b_fb_intr_is_mmu_fault_pending(struct gk20a *g);

#endif /* NVGPU_FB_INTR_GV11B_H */
