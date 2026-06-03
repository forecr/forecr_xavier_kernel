// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/runlist.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu.h>

#include "common/vgpu/ivc/comm_vgpu.h"
#include "common/vgpu/fifo/runlist_vf.h"

void vgpu_vf_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_runlist_params *p = &msg.params.vf_runlist;
	int err;

	(void)aperture;
	(void)gfid;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_VF_SUBMIT_RUNLIST;
	msg.handle = vgpu_get_handle(g);
	p->virt_runlist_id = runlist_id;
	p->iova = runlist_iova;
	p->length = count;
	p->offset = 0U;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err == 0 ? msg.ret : err;
	if (err != 0) {
		nvgpu_err(g, "cmd failed err=%d", err);
	}
}

int vgpu_vf_runlist_check_pending(struct gk20a *g,
		struct nvgpu_runlist *runlist)
{
	(void)g;
	(void)runlist;

	/* .hw_submit for VF already waited for finish */
	return 0;
}
