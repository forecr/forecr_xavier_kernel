// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/timers.h>
#include <nvgpu/mm.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include "bus_gm20b.h"

#include <nvgpu/hw/gm20b/hw_bus_gm20b.h>

int gm20b_bus_bar1_bind(struct gk20a *g, struct nvgpu_mem *bar1_inst)
{
	struct nvgpu_timeout timeout;
	int err = 0;
	u64 iova = nvgpu_inst_block_addr(g, bar1_inst);
	u32 ptr_v = nvgpu_safe_cast_u64_to_u32(iova
					>> bus_bar1_block_ptr_shift_v());

	nvgpu_log_info(g, "bar1 inst block ptr: 0x%08x", ptr_v);

	gk20a_writel(g, bus_bar1_block_r(),
		     nvgpu_aperture_mask(g, bar1_inst,
					 bus_bar1_block_target_sys_mem_ncoh_f(),
					 bus_bar1_block_target_sys_mem_coh_f(),
					 bus_bar1_block_target_vid_mem_f()) |
		     bus_bar1_block_mode_virtual_f() |
		     bus_bar1_block_ptr_f(ptr_v));
	nvgpu_timeout_init_retry(g, &timeout, 1000);

	do {
		u32 val = gk20a_readl(g, bus_bind_status_r());
		u32 pending = bus_bind_status_bar1_pending_v(val);
		u32 outstanding = bus_bind_status_bar1_outstanding_v(val);
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
