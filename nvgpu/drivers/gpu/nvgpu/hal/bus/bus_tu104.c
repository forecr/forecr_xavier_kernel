// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/timers.h>
#include <nvgpu/soc.h>
#include <nvgpu/io.h>
#include <nvgpu/mm.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/gk20a.h>

#include "bus_tu104.h"

#include <nvgpu/hw/tu104/hw_bus_tu104.h>
#include <nvgpu/hw/tu104/hw_func_tu104.h>

int tu104_bus_init_hw(struct gk20a *g)
{
	u32 intr_en_mask = 0U;

	nvgpu_cic_mon_intr_stall_unit_config(g, NVGPU_CIC_INTR_UNIT_BUS, NVGPU_CIC_INTR_ENABLE);

	/*
	 * Note: bus_intr_en_0 is for routing intr to stall tree (mc_intr_0)
	 * bus_intr_en_1 is for routing bus intr to nostall tree (mc_intr_1)
	 */
	if (nvgpu_platform_is_silicon(g) || nvgpu_platform_is_fpga(g)) {
		intr_en_mask = bus_intr_en_0_pri_squash_m() |
				bus_intr_en_0_pri_fecserr_m() |
				bus_intr_en_0_pri_timeout_m();
	}

	nvgpu_writel(g, bus_intr_en_0_r(), intr_en_mask);

	if (g->ops.bus.configure_debug_bus != NULL) {
		g->ops.bus.configure_debug_bus(g);
	}

	return 0;
}

int bus_tu104_bar2_bind(struct gk20a *g, struct nvgpu_mem *bar2_inst)
{
	struct nvgpu_timeout timeout;
	int err = 0;
	u64 iova = nvgpu_inst_block_addr(g, bar2_inst);
	u32 ptr_v = (u32)(iova >> bus_bar2_block_ptr_shift_v());

	nvgpu_log_info(g, "bar2 inst block ptr: 0x%08x", ptr_v);

	nvgpu_timeout_init_retry(g, &timeout, 1000);

	nvgpu_func_writel(g, func_priv_bar2_block_r(),
		     nvgpu_aperture_mask(g, bar2_inst,
					 bus_bar2_block_target_sys_mem_ncoh_f(),
					 bus_bar2_block_target_sys_mem_coh_f(),
					 bus_bar2_block_target_vid_mem_f()) |
		     bus_bar2_block_mode_virtual_f() |
		     bus_bar2_block_ptr_f(ptr_v));

	do {
		u32 val = nvgpu_func_readl(g,
				func_priv_bind_status_r());
		bool pending = (bus_bind_status_bar2_pending_v(val) ==
				bus_bind_status_bar2_pending_busy_v());
		bool outstanding = (bus_bind_status_bar2_outstanding_v(val) ==
				  bus_bind_status_bar2_outstanding_true_v());
		if (!pending && !outstanding) {
			break;
		}

		nvgpu_udelay(5);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	if (nvgpu_timeout_peek_expired(&timeout)) {
		err = -EINVAL;
	}

	return err;
}
