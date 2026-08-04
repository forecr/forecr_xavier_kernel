// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/io.h>
#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/io_usermode.h>
#include <nvgpu/runlist.h>

#include "usermode_tu104.h"

#include <nvgpu/hw/tu104/hw_usermode_tu104.h>
#include <nvgpu/hw/tu104/hw_ctrl_tu104.h>
#include <nvgpu/hw/tu104/hw_func_tu104.h>

u64 tu104_usermode_base(struct gk20a *g)
{
	(void)g;
	return func_cfg0_r();
}

u64 tu104_usermode_bus_base(struct gk20a *g)
{
	(void)g;
	return (u64)func_full_phys_offset_v() + func_cfg0_r();
}

void tu104_usermode_setup_hw(struct gk20a *g)
{
	u32 val;

	val = nvgpu_readl(g, ctrl_virtual_channel_cfg_r(0));
	val |= ctrl_virtual_channel_cfg_pending_enable_true_f();
	nvgpu_writel(g, ctrl_virtual_channel_cfg_r(0), val);
}

u32 tu104_usermode_doorbell_token(struct gk20a *g, u32 runlist_id, u32 chid)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 hw_chid = nvgpu_safe_add_u32(f->channel_base, chid);

	return ctrl_doorbell_vector_f(hw_chid) |
			ctrl_doorbell_runlist_id_f(runlist_id);
}

void tu104_usermode_ring_doorbell(struct gk20a *g, u32 runlist_id, u32 chid)
{
	nvgpu_log_info(g, "channel ring door bell %d, runlist %d",
			chid, runlist_id);

	nvgpu_usermode_writel(g, func_doorbell_r(),
				g->ops.usermode.doorbell_token(g, runlist_id, chid));
}
