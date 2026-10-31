// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/bug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gin.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/nvgpu_err.h>

#include "sysctrl_gb10b.h"

#include <nvgpu/hw/gb10b/hw_pmc_gb10b.h>

static void gb10b_sysctrl_ue_handler(struct gk20a *g, u64 cookie)
{
	u32 pmc_register_parity_status = 0U;

	(void)cookie;

	pmc_register_parity_status =
			nvgpu_readl(g, pmc_register_parity_status_r());
	if (pmc_register_parity_status == 0U) {
		return;
	}
	nvgpu_err(g, "SYSCTRL register parity uncorrected error detected "
			"pmc_register_parity_status: 0x%08x",
			pmc_register_parity_status);

	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_SYSCTRL,
			GPU_SYSCTRL_UNCORRECTED_ERR);

	/* No need to clear the interrupt since GPU reset automatically resets all registers */
	/* Trigger GPU quiesce for fatal errors */
	nvgpu_sw_quiesce(g);

}

int gb10b_sysctrl_init_hw(struct gk20a *g)
{
	u32 ue_vector = 0;
	u32 intr_ctrl_msg_ue = 0;

    /*
     * Register Parity check is enabled by default in HW.
     * Program INTR_MASK register to allow error reporting.
     */
	nvgpu_writel(g, pmc_register_parity_intr_mask_r(),
			pmc_register_parity_intr_mask_en_m());

	ue_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_SYSCTRL,
				NVGPU_SYSCTRL_UE_VECTOR_OFFSET);

    /* Report SYSCTRL reg parity errors via the CPU tree*/
	intr_ctrl_msg_ue = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);
	nvgpu_writel(g, pmc_register_parity_intr_ctrl_r(), intr_ctrl_msg_ue);
	nvgpu_gin_set_stall_handler(g, ue_vector, &gb10b_sysctrl_ue_handler, 0);
	nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_SYSCTRL,
			NVGPU_SYSCTRL_UE_VECTOR_OFFSET,
			NVGPU_GIN_INTR_ENABLE);

	return 0;
}
