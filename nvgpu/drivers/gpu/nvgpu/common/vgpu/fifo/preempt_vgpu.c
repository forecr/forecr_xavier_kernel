// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/atomic.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/fifo.h>
#include <nvgpu/tsg.h>
#include <nvgpu/preempt.h>

#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/vgpu.h>

#include "preempt_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"

int vgpu_fifo_preempt_channel(struct gk20a *g, struct nvgpu_channel *ch)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_channel_config_params *p =
			&msg.params.channel_config;
	int err;

	nvgpu_log_fn(g, " ");

	if (!nvgpu_atomic_read(&ch->bound)) {
		return 0;
	}

	msg.cmd = TEGRA_VGPU_CMD_CHANNEL_PREEMPT;
	msg.handle = vgpu_get_handle(g);
	p->handle = ch->virt_ctx;
	err = vgpu_comm_sendrecv(g, &msg);

	if (err || msg.ret) {
		nvgpu_err(g,
			"preempt channel %d failed", ch->chid);
		err = -ENOMEM;
	}

	return err;
}

int vgpu_fifo_preempt_tsg(struct gk20a *g, u32 runlist_id, u32 tsgid, u32 gfid)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_tsg_params *p = &msg.params.tsg;
	int err;
	(void)gfid;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_TSG_PREEMPT;
	msg.handle = vgpu_get_handle(g);
	p->runlist_id = runlist_id;
	p->tsg_id = tsgid;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	if (err) {
		nvgpu_err(g,
			"preempt tsg %u failed", tsgid);
	}

	return err;
}
