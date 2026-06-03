// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/channel.h>
#include <nvgpu/log.h>
#include <nvgpu/atomic.h>
#include <nvgpu/io.h>
#include <nvgpu/barrier.h>
#include <nvgpu/mm.h>
#include <nvgpu/gk20a.h>

#include "channel_gm20b.h"

#include <nvgpu/hw/gm20b/hw_ccsr_gm20b.h>
#include <nvgpu/hw/gm20b/hw_ram_gm20b.h>

void gm20b_channel_bind(struct nvgpu_channel *c)
{
	struct gk20a *g = c->g;

	u32 inst_ptr = nvgpu_inst_block_ptr(g, &c->inst_block);

	nvgpu_log_info(g, "bind channel %d inst ptr 0x%08x",
		c->chid, inst_ptr);

	nvgpu_writel(g, ccsr_channel_inst_r(c->chid),
		     ccsr_channel_inst_ptr_f(inst_ptr) |
		     nvgpu_aperture_mask(g, &c->inst_block,
				ccsr_channel_inst_target_sys_mem_ncoh_f(),
				ccsr_channel_inst_target_sys_mem_coh_f(),
				ccsr_channel_inst_target_vid_mem_f()) |
		     ccsr_channel_inst_bind_true_f());

	nvgpu_atomic_set(&c->bound, 1);
}

void gm20b_channel_force_ctx_reload(struct gk20a *g, u32 runlist_id, u32 chid)
{
	u32 reg = nvgpu_readl(g, ccsr_channel_r(chid));

	(void)runlist_id;
	nvgpu_writel(g, ccsr_channel_r(chid),
		reg | ccsr_channel_force_ctx_reload_true_f());
}
