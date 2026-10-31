// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>

#include "ramfc_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"

int vgpu_ramfc_setup(struct nvgpu_channel *ch, u64 gpfifo_base,
		u32 gpfifo_entries, u64 pbdma_acquire_timeout, u32 flags)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_ramfc_params *p = &msg.params.ramfc;
	int err;
	struct gk20a *g = ch->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_CHANNEL_SETUP_RAMFC;
	msg.handle = vgpu_get_handle(ch->g);
	p->handle = ch->virt_ctx;
	p->gpfifo_va = gpfifo_base;
	p->num_entries = gpfifo_entries;
	p->userd_addr = ch->userd_iova;
	p->iova = 0;
	err = vgpu_comm_sendrecv(g, &msg);

	return (err || msg.ret) ? -ENOMEM : 0;
}
