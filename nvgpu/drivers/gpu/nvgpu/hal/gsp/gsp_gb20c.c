// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gsp.h>

#include <nvgpu/hw/gb20c/hw_pgsp_gb20c.h>

#include "gsp_gb20c.h"
#include "gsp_gb10b.h"

void gb20c_gsp_flcn_queue_type(void)
{
    /*HAL definition implies that the message buffer will br DMEM*/
	return;
}

void gb20c_gsp_enable_irq(struct gk20a *g, bool enable)
{
	u32 legacy_vector = 0U;
	u32 intr_ctrl_msg = 0U;
	u32 old_vector = 0U;
	u32 old_intr_ctrl_msg = 0U;

	nvgpu_log_fn(g, " ");

	if (enable) {
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_GSP,
					NVGPU_GSP_LEGACY_VECTOR_OFFSET,
					NVGPU_GIN_INTR_ENABLE);
	} else {
		nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_GSP,
					NVGPU_GSP_LEGACY_VECTOR_OFFSET,
					NVGPU_GIN_INTR_DISABLE);
	}
	legacy_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_GSP, NVGPU_GSP_LEGACY_VECTOR_OFFSET);

	nvgpu_gin_set_stall_handler(g, legacy_vector, &gb10b_gsp_legacy_handler, 0);

	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, legacy_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);

	old_intr_ctrl_msg = nvgpu_readl(g, pgsp_falcon_intr_ctrl_r(0));
	old_vector = nvgpu_gin_get_vector_from_intr_ctrl_msg(g, old_intr_ctrl_msg);
	nvgpu_writel(g, pgsp_falcon_intr_ctrl_r(0), intr_ctrl_msg);
	nvgpu_writel(g, pgsp_falcon_intr_retrigger_r(0),
			pgsp_falcon_intr_retrigger_trigger_true_f());
	if (old_vector != legacy_vector) {
		nvgpu_gin_clear_pending_intr(g, old_vector);
	}
}