// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "pmu_tu104.h"

#include <nvgpu/hw/tu104/hw_pwr_tu104.h>

bool tu104_is_pmu_supported(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_SIM
	/* PMU not supported in dGpu Simulation */
	if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)) {
		return false;
	} else
#endif
	{
		return true;
	}
}

u32 tu104_pmu_falcon_base_addr(void)
{
	return pwr_falcon_irqsset_r();
}

u32 tu104_pmu_queue_head_r(u32 i)
{
	return pwr_pmu_queue_head_r(i);
}

u32 tu104_pmu_queue_head__size_1_v(void)
{
	return pwr_pmu_queue_head__size_1_v();
}

u32 tu104_pmu_queue_tail_r(u32 i)
{
	return pwr_pmu_queue_tail_r(i);
}

u32 tu104_pmu_queue_tail__size_1_v(void)
{
	return pwr_pmu_queue_tail__size_1_v();
}

u32 tu104_pmu_mutex__size_1_v(void)
{
	return pwr_pmu_mutex__size_1_v();
}

void tu104_pmu_setup_apertures(struct gk20a *g)
{
	struct mm_gk20a *mm = &g->mm;
	u32 inst_block_ptr;

	/* PMU TRANSCFG */
	/* setup apertures - virtual */
	gk20a_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_UCODE),
			pwr_fbif_transcfg_mem_type_physical_f() |
			pwr_fbif_transcfg_target_local_fb_f());
	gk20a_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_VIRT),
			pwr_fbif_transcfg_mem_type_virtual_f());
	/* setup apertures - physical */
	gk20a_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_PHYS_VID),
			pwr_fbif_transcfg_mem_type_physical_f() |
			pwr_fbif_transcfg_target_local_fb_f());
	gk20a_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_PHYS_SYS_COH),
			pwr_fbif_transcfg_mem_type_physical_f() |
			pwr_fbif_transcfg_target_coherent_sysmem_f());
	gk20a_writel(g, pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_PHYS_SYS_NCOH),
			pwr_fbif_transcfg_mem_type_physical_f() |
			pwr_fbif_transcfg_target_noncoherent_sysmem_f());

	/* PMU Config */
	gk20a_writel(g, pwr_falcon_itfen_r(),
				gk20a_readl(g, pwr_falcon_itfen_r()) |
				pwr_falcon_itfen_ctxen_enable_f());
	inst_block_ptr = nvgpu_inst_block_ptr(g, &mm->pmu.inst_block);
	g->ops.pmu.set_pmu_new_instblk(g,
		pwr_pmu_new_instblk_ptr_f(inst_block_ptr) |
		pwr_pmu_new_instblk_valid_f(1) |
		nvgpu_aperture_mask(g, &mm->pmu.inst_block,
			pwr_pmu_new_instblk_target_sys_ncoh_f(),
			pwr_pmu_new_instblk_target_sys_coh_f(),
			pwr_pmu_new_instblk_target_fb_f()));
}
