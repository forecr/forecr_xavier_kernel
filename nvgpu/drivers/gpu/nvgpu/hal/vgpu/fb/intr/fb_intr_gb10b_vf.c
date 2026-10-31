// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/gin.h>

#include "fb_intr_gb10b_vf.h"

static void vf_gb10b_fb_intr_stall_handler(struct gk20a *g, u64 cookie)
{
	u32 intr_unit = nvgpu_safe_cast_u64_to_u32(cookie);
	g->ops.fb.intr.isr(g, BIT32(intr_unit));
}

void vf_gb10b_fb_intr_vectorid_init(struct gk20a *g)
{
	u32 vectorid = 0U;

	vectorid = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_INFO_FAULT);
	nvgpu_gin_set_stall_handler(g, vectorid, &vf_gb10b_fb_intr_stall_handler,
			NVGPU_CIC_INTR_UNIT_MMU_INFO_FAULT);

	vectorid = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT);
	nvgpu_gin_set_stall_handler(g, vectorid, &vf_gb10b_fb_intr_stall_handler,
			NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT);

	vectorid = nvgpu_gin_get_unit_stall_vector(g,
			NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT_ERROR);
	nvgpu_gin_set_stall_handler(g, vectorid, &vf_gb10b_fb_intr_stall_handler,
			NVGPU_CIC_INTR_UNIT_MMU_NON_REPLAYABLE_FAULT_ERROR);
}

void vf_gb10b_fb_intr_enable(struct gk20a *g)
{
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_INFO_FAULT,
		NVGPU_GIN_INTR_ENABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT,
		NVGPU_GIN_INTR_ENABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT_ERROR,
		NVGPU_GIN_INTR_ENABLE);
}

void vf_gb10b_fb_intr_disable(struct gk20a *g)
{
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_INFO_FAULT,
		NVGPU_GIN_INTR_DISABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT,
		NVGPU_GIN_INTR_DISABLE);
	nvgpu_gin_stall_unit_config(g,
		NVGPU_GIN_INTR_UNIT_MMU, NVGPU_GIN_INTR_UNIT_MMU_SUBUNIT_NON_REPLAYABLE_FAULT_ERROR,
		NVGPU_GIN_INTR_DISABLE);
}
