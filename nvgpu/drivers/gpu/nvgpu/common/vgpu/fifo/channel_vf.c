// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/runlist.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu.h>

#include "common/vgpu/fifo/channel_vf.h"
#include "common/vgpu/ivc/comm_vgpu.h"

void vgpu_vf_channel_read_state(struct gk20a *g, u32 runlist_id, u32 chid,
			struct nvgpu_channel_hw_state *state)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_ch_state_params *p = &msg.params.vf_ch_state;
	int err;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_VF_CHANNEL_READ_STATE;
	msg.handle = vgpu_get_handle(g);
	p->virt_runlist_id = runlist_id;
	p->virt_chid = chid;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err == 0 ? msg.ret : err;
	if (err == 0) {
		*state = p->state;
	} else {
		nvgpu_err(g, "cmd failed err=%d", err);
	}
}

static int vgpu_vf_ch_cmd(struct gk20a *g, u32 cmd, u32 runlist_id, u32 chid)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_ch_params *p = &msg.params.vf_ch;
	int err;

	nvgpu_log_fn(g, "cmd %u", cmd);

	msg.cmd = cmd;
	msg.handle = vgpu_get_handle(g);
	p->virt_runlist_id = runlist_id;
	p->virt_chid = chid;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err == 0 ? msg.ret : err;
	if (err != 0) {
		nvgpu_err(g, "cmd %u failed err=%d", cmd, err);
	}

	return err;
}

void vgpu_vf_channel_enable(struct gk20a *g, u32 runlist_id, u32 chid)
{
	vgpu_vf_ch_cmd(g, TEGRA_VGPU_CMD_VF_CHANNEL_ENABLE,
			runlist_id, chid);
}

void vgpu_vf_channel_disable(struct gk20a *g, u32 runlist_id, u32 chid)
{
	vgpu_vf_ch_cmd(g, TEGRA_VGPU_CMD_VF_CHANNEL_DISABLE,
			runlist_id, chid);
}

void vgpu_vf_channel_clear(struct gk20a *g, u32 runlist_id, u32 chid)
{
	vgpu_vf_ch_cmd(g, TEGRA_VGPU_CMD_VF_CHANNEL_CLEAR,
			runlist_id, chid);
}

void vgpu_vf_channel_force_ctx_reload(struct gk20a *g, u32 runlist_id, u32 chid)
{
	vgpu_vf_ch_cmd(g, TEGRA_VGPU_CMD_VF_CHANNEL_FORCE_CTX_RELOAD,
			runlist_id, chid);
}

void vgpu_vf_channel_reset_faulted(struct gk20a *g, u32 runlist_id, u32 chid)
{
	vgpu_vf_ch_cmd(g, TEGRA_VGPU_CMD_VF_CHANNEL_RESET_FAULTED,
			runlist_id, chid);
}
