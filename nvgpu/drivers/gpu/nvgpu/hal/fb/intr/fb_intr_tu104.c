// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "hal/fb/fb_tu104.h"
#include "hal/fb/fb_mmu_fault_tu104.h"
#include "hal/mc/mc_tu104.h"

#include "fb_intr_tu104.h"

#include "nvgpu/hw/tu104/hw_fb_tu104.h"

void tu104_fb_intr_enable(struct gk20a *g)
{
	u32 info_fault = nvgpu_readl(g, fb_mmu_int_vector_info_fault_r());
	u32 nonreplay_fault = nvgpu_readl(g,
		fb_mmu_int_vector_fault_r(
				NVGPU_MMU_FAULT_NONREPLAY_REG_INDX));
	u32 replay_fault = nvgpu_readl(g,
		fb_mmu_int_vector_fault_r(NVGPU_MMU_FAULT_REPLAY_REG_INDX));
	u32 ecc_error = nvgpu_readl(g, fb_mmu_int_vector_ecc_error_r());

	intr_tu104_vector_en_set(g,
		fb_mmu_int_vector_info_fault_vector_v(info_fault));
	intr_tu104_vector_en_set(g,
		fb_mmu_int_vector_fault_notify_v(nonreplay_fault));
	intr_tu104_vector_en_set(g,
		fb_mmu_int_vector_fault_error_v(nonreplay_fault));
	intr_tu104_vector_en_set(g,
		fb_mmu_int_vector_fault_notify_v(replay_fault));
	intr_tu104_vector_en_set(g,
		fb_mmu_int_vector_fault_error_v(replay_fault));
	intr_tu104_vector_en_set(g,
		fb_mmu_int_vector_ecc_error_vector_v(ecc_error));
}

void tu104_fb_intr_disable(struct gk20a *g)
{
	u32 info_fault = nvgpu_readl(g, fb_mmu_int_vector_info_fault_r());
	u32 nonreplay_fault = nvgpu_readl(g,
		fb_mmu_int_vector_fault_r(
				NVGPU_MMU_FAULT_NONREPLAY_REG_INDX));
	u32 replay_fault = nvgpu_readl(g,
		fb_mmu_int_vector_fault_r(NVGPU_MMU_FAULT_REPLAY_REG_INDX));
	u32 ecc_error = nvgpu_readl(g, fb_mmu_int_vector_ecc_error_r());

	intr_tu104_vector_en_clear(g,
		fb_mmu_int_vector_info_fault_vector_v(info_fault));
	intr_tu104_vector_en_clear(g,
		fb_mmu_int_vector_fault_notify_v(nonreplay_fault));
	intr_tu104_vector_en_clear(g,
		fb_mmu_int_vector_fault_error_v(nonreplay_fault));
	intr_tu104_vector_en_clear(g,
		fb_mmu_int_vector_fault_notify_v(replay_fault));
	intr_tu104_vector_en_clear(g,
		fb_mmu_int_vector_fault_error_v(replay_fault));
	intr_tu104_vector_en_clear(g,
		fb_mmu_int_vector_ecc_error_vector_v(ecc_error));
}

void tu104_fb_intr_isr(struct gk20a *g, u32 intr_unit_bitmask)
{
	u32 info_fault = nvgpu_readl(g, fb_mmu_int_vector_info_fault_r());
	u32 nonreplay_fault = nvgpu_readl(g,
		fb_mmu_int_vector_fault_r(
				NVGPU_MMU_FAULT_NONREPLAY_REG_INDX));
	u32 replay_fault = nvgpu_readl(g,
		fb_mmu_int_vector_fault_r(NVGPU_MMU_FAULT_REPLAY_REG_INDX));
	u32 ecc_error = nvgpu_readl(g, fb_mmu_int_vector_ecc_error_r());

	nvgpu_mutex_acquire(&g->mm.hub_isr_mutex);

	if (intr_tu104_vector_intr_pending(g,
			fb_mmu_int_vector_ecc_error_vector_v(ecc_error))) {
		g->ops.fb.intr.handle_ecc(g);
	}

	if (intr_tu104_vector_intr_pending(g,
			fb_mmu_int_vector_fault_notify_v(replay_fault)) ||
	    intr_tu104_vector_intr_pending(g,
			fb_mmu_int_vector_fault_error_v(replay_fault)) ||
	    intr_tu104_vector_intr_pending(g,
			fb_mmu_int_vector_fault_notify_v(nonreplay_fault)) ||
	    intr_tu104_vector_intr_pending(g,
			fb_mmu_int_vector_fault_error_v(nonreplay_fault)) ||
	    intr_tu104_vector_intr_pending(g,
			fb_mmu_int_vector_info_fault_vector_v(info_fault))) {
		nvgpu_log(g, gpu_dbg_intr, "MMU Fault");
		tu104_fb_handle_mmu_fault(g);
	}

	nvgpu_mutex_release(&g->mm.hub_isr_mutex);
}

bool tu104_fb_intr_is_mmu_fault_pending(struct gk20a *g)
{
	u32 info_fault = nvgpu_readl(g, fb_mmu_int_vector_info_fault_r());
	u32 nonreplay_fault = nvgpu_readl(g,
		fb_mmu_int_vector_fault_r(NVGPU_MMU_FAULT_NONREPLAY_REG_INDX));
	u32 replay_fault = nvgpu_readl(g,
		fb_mmu_int_vector_fault_r(NVGPU_MMU_FAULT_REPLAY_REG_INDX));
	u32 ecc_error = nvgpu_readl(g, fb_mmu_int_vector_ecc_error_r());

	if (intr_tu104_vector_intr_pending(g,
		fb_mmu_int_vector_fault_notify_v(replay_fault)) ||
	    intr_tu104_vector_intr_pending(g,
		fb_mmu_int_vector_fault_error_v(replay_fault)) ||
	    intr_tu104_vector_intr_pending(g,
		fb_mmu_int_vector_fault_notify_v(nonreplay_fault)) ||
	    intr_tu104_vector_intr_pending(g,
		fb_mmu_int_vector_fault_error_v(nonreplay_fault)) ||
	    intr_tu104_vector_intr_pending(g,
		fb_mmu_int_vector_info_fault_vector_v(info_fault)) ||
	    intr_tu104_vector_intr_pending(g,
		fb_mmu_int_vector_ecc_error_vector_v(ecc_error))) {
		return true;
	}

	return false;
}
