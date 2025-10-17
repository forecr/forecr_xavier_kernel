// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/timers.h>
#include <nvgpu/mm.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include "bus_gp10b.h"

#include <nvgpu/hw/gp10b/hw_bus_gp10b.h>

int gp10b_bus_bar2_bind(struct gk20a *g, struct nvgpu_mem *bar2_inst)
{
	struct nvgpu_timeout timeout;
	int err = 0;
	u64 iova = nvgpu_inst_block_addr(g, bar2_inst);
	u32 ptr_v = nvgpu_safe_cast_u64_to_u32(iova
					>> bus_bar2_block_ptr_shift_v());

	nvgpu_log_info(g, "bar2 inst block ptr: 0x%08x", ptr_v);

	gk20a_writel(g, bus_bar2_block_r(),
		     nvgpu_aperture_mask(g, bar2_inst,
					 bus_bar2_block_target_sys_mem_ncoh_f(),
					 bus_bar2_block_target_sys_mem_coh_f(),
					 bus_bar2_block_target_vid_mem_f()) |
		     bus_bar2_block_mode_virtual_f() |
		     bus_bar2_block_ptr_f(ptr_v));
	nvgpu_timeout_init_retry(g, &timeout, 1000);

	do {
		u32 val = gk20a_readl(g, bus_bind_status_r());
		u32 pending = bus_bind_status_bar2_pending_v(val);
		u32 outstanding = bus_bind_status_bar2_outstanding_v(val);
		if ((pending == 0U) && (outstanding == 0U)) {
			break;
		}

		nvgpu_udelay(5);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	if (nvgpu_timeout_peek_expired(&timeout)) {
		err = -EINVAL;
	}

	return err;
}
