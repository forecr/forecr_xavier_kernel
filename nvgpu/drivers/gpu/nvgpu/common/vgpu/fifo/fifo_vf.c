// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/tsg.h>
#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/vgpu.h>

#include "fifo_vf.h"
#include "common/vgpu/ivc/comm_vgpu.h"

static void vf_fifo_reenable_runlist(struct gk20a *g, u32 runlist_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_enable_runlist_params *p =
		&msg.params.vf_enable_runlist;
	int err = 0;

	msg.cmd = TEGRA_VGPU_CMD_VF_ENABLE_RUNLIST;
	msg.handle = vgpu_get_handle(g);
	p->virt_runlist_id = runlist_id;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err == 0 ? msg.ret : err;
	if (err != 0) {
		nvgpu_err(g, "cmd failed err=%d", err);
	}
}

void vf_fifo_handle_runlist_disabled_for_rc(struct gk20a *g,
		struct tegra_vf_runlist_disabled *p)
{
	u32 runlist_id = p->runlist_id;
	u32 tsgid = p->tsgid;
	struct nvgpu_tsg *tsg = nvgpu_tsg_check_and_get_from_id(g, tsgid);

	if (tsg == NULL) {
		nvgpu_err(g, "invalid tsgid %u", tsgid);
		return;
	}

	nvgpu_tsg_disable(tsg);

	vf_fifo_reenable_runlist(g, runlist_id);

	nvgpu_tsg_wakeup_wqs(g, tsg);
	nvgpu_tsg_abort(g, tsg, false);
}

void vf_fifo_handle_runlist_disabled_for_cilp(struct gk20a *g,
		struct tegra_vf_runlist_disabled *p)
{
	u32 runlist_id = p->runlist_id;
	u32 tsgid = p->tsgid;
	struct nvgpu_tsg *tsg = nvgpu_tsg_check_and_get_from_id(g, tsgid);

	if (tsg == NULL) {
		nvgpu_err(g, "invalid tsgid %u", tsgid);
		return;
	}

	nvgpu_tsg_disable(tsg);

	vf_fifo_reenable_runlist(g, runlist_id);
}
