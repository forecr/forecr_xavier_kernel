// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/gin.h>

#include <nvgpu/hw/gb10b/hw_fb_gb10b.h>

#include "fb_intr_gb10b.h"

static void gb10b_fb_intr_stall_handler(struct gk20a *g, u64 cookie)
{
	u32 intr_unit = nvgpu_safe_cast_u64_to_u32(cookie);
	g->ops.fb.intr.isr(g, BIT32(intr_unit));
}

void gb10b_fb_intr_vectorid_init(struct gk20a *g)
{
	u32 vectorid = 0U;
	u32 intr_ctrl_reg = 0U;
	u32 intr_ctrl_msg = 0U;

	vectorid = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_ECC_ERROR);
	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, vectorid,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	intr_ctrl_reg = fb_mmu_intr_ctrl_r(fb_mmu_intr_ctrl_ecc_error_index_v());
	nvgpu_writel(g, intr_ctrl_reg, intr_ctrl_msg);
	nvgpu_gin_set_stall_handler(g, vectorid, &gb10b_fb_intr_stall_handler,
			NVGPU_CIC_INTR_UNIT_MMU_FAULT_ECC_ERROR);

	vectorid = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_INFO_FAULT);
	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, vectorid,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	intr_ctrl_reg = fb_mmu_intr_ctrl_r(fb_mmu_intr_ctrl_info_fault_index_v());
	nvgpu_writel(g, intr_ctrl_reg, intr_ctrl_msg);
	nvgpu_gin_set_stall_handler(g, vectorid, &gb10b_fb_intr_stall_handler,
			NVGPU_CIC_INTR_UNIT_MMU_INFO_FAULT);

	vectorid = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT);
	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, vectorid,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	intr_ctrl_reg = fb_mmu_intr_ctrl_r(fb_mmu_intr_ctrl_non_replayable_fault_index_v());
	nvgpu_writel(g, intr_ctrl_reg, intr_ctrl_msg);
	nvgpu_gin_set_stall_handler(g, vectorid, &gb10b_fb_intr_stall_handler,
			NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT);

	vectorid = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT_ERROR);
	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, vectorid,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	intr_ctrl_reg = fb_mmu_intr_ctrl_r(fb_mmu_intr_ctrl_non_replayable_fault_error_index_v());
	nvgpu_writel(g, intr_ctrl_reg, intr_ctrl_msg);
	nvgpu_gin_set_stall_handler(g, vectorid, &gb10b_fb_intr_stall_handler,
			NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT_ERROR);

#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
	vectorid = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT);
	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, vectorid,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	intr_ctrl_reg = fb_mmu_intr_ctrl_r(fb_mmu_intr_ctrl_replayable_fault_index_v());
	nvgpu_writel(g, intr_ctrl_reg, intr_ctrl_msg);
	nvgpu_gin_set_stall_handler(g, vectorid, &gb10b_fb_intr_stall_handler,
			NVGPU_CIC_INTR_UNIT_MMU_REPLAYABLE_FAULT);

	vectorid = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT_ERROR);
	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, vectorid,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	intr_ctrl_reg = fb_mmu_intr_ctrl_r(fb_mmu_intr_ctrl_replayable_fault_error_index_v());
	nvgpu_writel(g, intr_ctrl_reg, intr_ctrl_msg);
	nvgpu_gin_set_stall_handler(g, vectorid, &gb10b_fb_intr_stall_handler,
			NVGPU_CIC_INTR_UNIT_MMU_REPLAYABLE_FAULT_ERROR);
#endif
}

void gb10b_fb_intr_enable(struct gk20a *g)
{
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_ECC_ERROR,
		NVGPU_GIN_INTR_ENABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_INFO_FAULT,
		NVGPU_GIN_INTR_ENABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT,
		NVGPU_GIN_INTR_ENABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT_ERROR,
		NVGPU_GIN_INTR_ENABLE);
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT,
		NVGPU_GIN_INTR_ENABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT_ERROR,
		NVGPU_GIN_INTR_ENABLE);
#endif
}

void gb10b_fb_intr_disable(struct gk20a *g)
{
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_ECC_ERROR,
		NVGPU_GIN_INTR_DISABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_INFO_FAULT,
		NVGPU_GIN_INTR_DISABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT,
		NVGPU_GIN_INTR_DISABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT_ERROR,
		NVGPU_GIN_INTR_DISABLE);
#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT,
		NVGPU_GIN_INTR_DISABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_REPLAYABLE_FAULT_ERROR,
		NVGPU_GIN_INTR_DISABLE);
#endif
}
