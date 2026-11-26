// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/soc.h>
#include <nvgpu/mm.h>
#include <nvgpu/io.h>
#include <nvgpu/bug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_sgt.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/mc.h>

#include "bus_gk20a.h"

#include <nvgpu/hw/gk20a/hw_bus_gk20a.h>

int gk20a_bus_init_hw(struct gk20a *g)
{
	u32 intr_en_mask = 0U;

#ifdef CONFIG_NVGPU_NONSTALL_INTR
	nvgpu_cic_mon_intr_nonstall_unit_config(g, NVGPU_CIC_INTR_UNIT_BUS, NVGPU_CIC_INTR_ENABLE);
#endif

	/*
	 * Note: bus_intr_en_0 is for routing intr to stall tree (mc_intr_0)
	 * bus_intr_en_1 is for routing bus intr to nostall tree (mc_intr_1)
	 */
	if (nvgpu_platform_is_silicon(g) || nvgpu_platform_is_fpga(g)) {
		intr_en_mask = bus_intr_en_1_pri_squash_m() |
				bus_intr_en_1_pri_fecserr_m() |
				bus_intr_en_1_pri_timeout_m();
	}

	nvgpu_writel(g, bus_intr_en_1_r(), intr_en_mask);

	if (g->ops.bus.configure_debug_bus != NULL) {
		g->ops.bus.configure_debug_bus(g);
	}

	return 0;
}

void gk20a_bus_isr(struct gk20a *g)
{
	u32 val;
	u32 err_type = GPU_HOST_INVALID_ERROR;

	val = nvgpu_readl(g, bus_intr_0_r());

	if ((val & (bus_intr_0_pri_squash_m() |
			bus_intr_0_pri_fecserr_m() |
			bus_intr_0_pri_timeout_m())) != 0U) {
		if ((val & bus_intr_0_pri_squash_m()) != 0U) {
			err_type = GPU_HOST_PBUS_SQUASH_ERROR;
			nvgpu_err (g, "host pbus squash error");
		}
		if ((val & bus_intr_0_pri_fecserr_m()) != 0U) {
			err_type = GPU_HOST_PBUS_FECS_ERROR;
			nvgpu_err (g, "host pbus fecs error");
		}
		if ((val & bus_intr_0_pri_timeout_m()) != 0U) {
			err_type = GPU_HOST_PBUS_TIMEOUT_ERROR;
			nvgpu_err (g, "host pbus timeout error");
		}
		g->ops.ptimer.isr(g);
	} else {
		nvgpu_err(g, "Unhandled NV_PBUS_INTR_0: 0x%08x", val);
		/* We group following errors as part of PBUS_TIMEOUT_ERROR:
		 * FB_REQ_TIMEOUT, FB_ACK_TIMEOUT, FB_ACK_EXTRA,
		 * FB_RDATA_TIMEOUT, FB_RDATA_EXTRA, POSTED_DEADLOCK_TIMEOUT,
		 * ACCESS_TIMEOUT.
		 */
		err_type = GPU_HOST_PBUS_TIMEOUT_ERROR;
	}
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_HOST, err_type);
	nvgpu_writel(g, bus_intr_0_r(), val);
}
