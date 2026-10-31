// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/mc.h>
#include <nvgpu/cic_mon.h>

#include "hal/fb/intr/fb_intr_ecc_gv11b.h"
#include "hal/fb/fb_mmu_fault_ga10b.h"
#include "fb_intr_ga10b.h"

#include <nvgpu/hw/ga10b/hw_fb_ga10b.h>

void ga10b_fb_intr_vectorid_init(struct gk20a *g)
{
	u32 ecc_error, info_fault, nonreplay_fault;
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
	u32 replay_fault;
#endif
	u32 vectorid;

	ecc_error = nvgpu_readl(g, fb_mmu_int_vector_ecc_error_r());
	vectorid = fb_mmu_int_vector_ecc_error_vector_v(ecc_error);
	nvgpu_cic_mon_intr_unit_vectorid_init(g, NVGPU_CIC_INTR_UNIT_MMU_FAULT_ECC_ERROR,
			&vectorid, NVGPU_CIC_INTR_VECTORID_SIZE_ONE);

	info_fault = nvgpu_readl(g, fb_mmu_int_vector_info_fault_r());
	vectorid = fb_mmu_int_vector_info_fault_vector_v(info_fault);
	nvgpu_cic_mon_intr_unit_vectorid_init(g, NVGPU_CIC_INTR_UNIT_MMU_INFO_FAULT,
			&vectorid, NVGPU_CIC_INTR_VECTORID_SIZE_ONE);

	nonreplay_fault = nvgpu_readl(g, fb_mmu_int_vector_fault_r(
				NVGPU_MMU_FAULT_NONREPLAY_REG_INDX));
	vectorid = fb_mmu_int_vector_fault_notify_v(nonreplay_fault);
	nvgpu_cic_mon_intr_unit_vectorid_init(g, NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT,
			&vectorid, NVGPU_CIC_INTR_VECTORID_SIZE_ONE);

	vectorid = fb_mmu_int_vector_fault_error_v(nonreplay_fault);
	nvgpu_cic_mon_intr_unit_vectorid_init(g, NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT_ERROR,
			&vectorid, NVGPU_CIC_INTR_VECTORID_SIZE_ONE);

#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
	replay_fault = nvgpu_readl(g, fb_mmu_int_vector_fault_r(
				NVGPU_MMU_FAULT_REPLAY_REG_INDX));
	vectorid = fb_mmu_int_vector_fault_notify_v(replay_fault);
	nvgpu_cic_mon_intr_unit_vectorid_init(g, NVGPU_CIC_INTR_UNIT_MMU_REPLAYABLE_FAULT,
			&vectorid, NVGPU_CIC_INTR_VECTORID_SIZE_ONE);

	vectorid = fb_mmu_int_vector_fault_error_v(replay_fault);
	nvgpu_cic_mon_intr_unit_vectorid_init(g, NVGPU_CIC_INTR_UNIT_MMU_REPLAYABLE_FAULT_ERROR,
			&vectorid, NVGPU_CIC_INTR_VECTORID_SIZE_ONE);
#endif

	/* TBD hub_access_cntr_intr */
}

void ga10b_fb_intr_enable(struct gk20a *g)
{
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_FAULT_ECC_ERROR, NVGPU_CIC_INTR_ENABLE);
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_INFO_FAULT, NVGPU_CIC_INTR_ENABLE);
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT, NVGPU_CIC_INTR_ENABLE);
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT_ERROR, NVGPU_CIC_INTR_ENABLE);
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_REPLAYABLE_FAULT, NVGPU_CIC_INTR_ENABLE);
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_REPLAYABLE_FAULT_ERROR, NVGPU_CIC_INTR_ENABLE);
#endif
	/* TBD hub_access_cntr_intr */
}

void ga10b_fb_intr_disable(struct gk20a *g)
{
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_FAULT_ECC_ERROR, NVGPU_CIC_INTR_DISABLE);
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_INFO_FAULT, NVGPU_CIC_INTR_DISABLE);
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT, NVGPU_CIC_INTR_DISABLE);
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT_ERROR, NVGPU_CIC_INTR_DISABLE);
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_REPLAYABLE_FAULT, NVGPU_CIC_INTR_DISABLE);
	nvgpu_cic_mon_intr_stall_unit_config(g,
		NVGPU_CIC_INTR_UNIT_MMU_REPLAYABLE_FAULT_ERROR, NVGPU_CIC_INTR_DISABLE);
#endif
	/* TBD hub_access_cntr_intr */

}

void ga10b_fb_intr_isr(struct gk20a *g, u32 intr_unit_bitmask)
{
	nvgpu_mutex_acquire(&g->mm.hub_isr_mutex);

	nvgpu_log(g, gpu_dbg_intr, "MMU Fault");

	if ((intr_unit_bitmask & BIT32(NVGPU_CIC_INTR_UNIT_MMU_FAULT_ECC_ERROR)) !=
			0U) {
		g->ops.fb.intr.handle_ecc(g);
	}

	ga10b_fb_handle_mmu_fault(g, intr_unit_bitmask);

	nvgpu_mutex_release(&g->mm.hub_isr_mutex);
}
