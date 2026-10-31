// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io_usermode.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/fifo.h>
#include <nvgpu/static_analysis.h>

#include "usermode_gv11b.h"

#include <nvgpu/hw/gv11b/hw_usermode_gv11b.h>

u64 gv11b_usermode_base(struct gk20a *g)
{
	(void)g;
	return usermode_cfg0_r();
}

u64 gv11b_usermode_bus_base(struct gk20a *g)
{
	(void)g;
	return usermode_cfg0_r();
}

u32 gv11b_usermode_doorbell_token(struct gk20a *g, u32 runlist_id, u32 chid)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 hw_chid = nvgpu_safe_add_u32(f->channel_base, chid);

	(void)runlist_id;

	return usermode_notify_channel_pending_id_f(hw_chid);
}

void gv11b_usermode_ring_doorbell(struct gk20a *g, u32 runlist_id, u32 chid)
{
	nvgpu_log_info(g, "channel ring door bell %d", chid);

	nvgpu_usermode_writel(g, usermode_notify_channel_pending_r(),
		gv11b_usermode_doorbell_token(g, runlist_id, chid));
}
