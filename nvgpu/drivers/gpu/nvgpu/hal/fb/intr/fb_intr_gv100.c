// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "fb_intr_gv100.h"

#include <nvgpu/hw/gv100/hw_fb_gv100.h>

void gv100_fb_intr_enable(struct gk20a *g)
{
	u32 mask;

	mask = fb_niso_intr_en_set_mmu_other_fault_notify_m() |
		fb_niso_intr_en_set_mmu_nonreplayable_fault_notify_m() |
		fb_niso_intr_en_set_mmu_nonreplayable_fault_overflow_m() |
		fb_niso_intr_en_set_mmu_replayable_fault_notify_m() |
		fb_niso_intr_en_set_mmu_replayable_fault_overflow_m();

	nvgpu_writel(g, fb_niso_intr_en_set_r(0), mask);
}

void gv100_fb_intr_disable(struct gk20a *g)
{
	u32 mask;

	mask = fb_niso_intr_en_set_mmu_other_fault_notify_m() |
		fb_niso_intr_en_set_mmu_nonreplayable_fault_notify_m() |
		fb_niso_intr_en_set_mmu_nonreplayable_fault_overflow_m() |
		fb_niso_intr_en_set_mmu_replayable_fault_notify_m() |
		fb_niso_intr_en_set_mmu_replayable_fault_overflow_m();

	nvgpu_writel(g, fb_niso_intr_en_clr_r(0), mask);
}
