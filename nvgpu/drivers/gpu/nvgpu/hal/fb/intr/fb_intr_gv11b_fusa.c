// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/mc.h>

#include "hal/fb/fb_gv11b.h"
#include "hal/fb/fb_mmu_fault_gv11b.h"

#include "fb_intr_gv11b.h"

#include <nvgpu/hw/gv11b/hw_fb_gv11b.h>

void gv11b_fb_intr_enable(struct gk20a *g)
{
	u32 mask;

	mask = fb_niso_intr_en_set_mmu_other_fault_notify_m() |
		fb_niso_intr_en_set_mmu_nonreplayable_fault_notify_m() |
		fb_niso_intr_en_set_mmu_nonreplayable_fault_overflow_m() |
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
		fb_niso_intr_en_set_mmu_replayable_fault_notify_m() |
		fb_niso_intr_en_set_mmu_replayable_fault_overflow_m() |
#endif
		fb_niso_intr_en_set_mmu_ecc_uncorrected_error_notify_m();

	nvgpu_cic_mon_intr_stall_unit_config(g, NVGPU_CIC_INTR_UNIT_HUB, NVGPU_CIC_INTR_ENABLE);

	nvgpu_writel(g, fb_niso_intr_en_set_r(0), mask);
}

void gv11b_fb_intr_disable(struct gk20a *g)
{
	u32 mask;

	mask = fb_niso_intr_en_set_mmu_other_fault_notify_m() |
		fb_niso_intr_en_set_mmu_nonreplayable_fault_notify_m() |
		fb_niso_intr_en_set_mmu_nonreplayable_fault_overflow_m() |
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
		fb_niso_intr_en_set_mmu_replayable_fault_notify_m() |
		fb_niso_intr_en_set_mmu_replayable_fault_overflow_m() |
#endif
		fb_niso_intr_en_set_mmu_ecc_uncorrected_error_notify_m();

	nvgpu_writel(g, fb_niso_intr_en_clr_r(0), mask);

	nvgpu_cic_mon_intr_stall_unit_config(g, NVGPU_CIC_INTR_UNIT_HUB, NVGPU_CIC_INTR_DISABLE);
}

void gv11b_fb_intr_isr(struct gk20a *g, u32 intr_unit_bitmask)
{
	u32 niso_intr;

	(void)intr_unit_bitmask;

	nvgpu_mutex_acquire(&g->mm.hub_isr_mutex);

	niso_intr = nvgpu_readl(g, fb_niso_intr_r());

	nvgpu_log(g, gpu_dbg_intr, "enter hub isr, niso_intr = 0x%08x",
			niso_intr);

	if ((niso_intr &
		 (fb_niso_intr_hub_access_counter_notify_m() |
		  fb_niso_intr_hub_access_counter_error_m())) != 0U) {

		nvgpu_info(g, "hub access counter notify/error");
	}
	if ((niso_intr &
	     fb_niso_intr_mmu_ecc_uncorrected_error_notify_pending_f()) != 0U) {
		g->ops.fb.intr.handle_ecc(g);
	}
	if ((niso_intr &
		(fb_niso_intr_mmu_other_fault_notify_m() |
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
		fb_niso_intr_mmu_replayable_fault_notify_m() |
		fb_niso_intr_mmu_replayable_fault_overflow_m() |
#endif
		fb_niso_intr_mmu_nonreplayable_fault_notify_m() |
		fb_niso_intr_mmu_nonreplayable_fault_overflow_m())) != 0U) {

		nvgpu_log(g, gpu_dbg_intr, "MMU Fault");
		gv11b_fb_handle_mmu_fault(g, niso_intr);
	}

	nvgpu_mutex_release(&g->mm.hub_isr_mutex);
}

bool gv11b_fb_intr_is_mmu_fault_pending(struct gk20a *g)
{
	if ((gk20a_readl(g, fb_niso_intr_r()) &
		(fb_niso_intr_mmu_other_fault_notify_m() |
		 fb_niso_intr_mmu_ecc_uncorrected_error_notify_m() |
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
		 fb_niso_intr_mmu_replayable_fault_notify_m() |
		 fb_niso_intr_mmu_replayable_fault_overflow_m() |
#endif
		 fb_niso_intr_mmu_nonreplayable_fault_notify_m() |
		 fb_niso_intr_mmu_nonreplayable_fault_overflow_m())) != 0U) {
		return true;
	}

	return false;
}
