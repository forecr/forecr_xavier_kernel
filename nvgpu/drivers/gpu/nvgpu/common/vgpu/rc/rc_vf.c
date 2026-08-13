// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/fifo.h>
#include <nvgpu/engines.h>
#include <nvgpu/rc.h>
#include <nvgpu/runlist.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu.h>

#include "rc_vf.h"
#include "common/vgpu/ivc/comm_vgpu.h"

void vgpu_vf_fifo_recover(struct gk20a *g, u32 gfid, u32 runlist_id,
			u32 id, unsigned int id_type, unsigned int rc_type,
			bool should_defer_reset)
{
	int err;
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_fault_params *p = &msg.params.vf_fault;

	rec_dbg(g, "Recovery starting");
	rec_dbg(g, "  ID      = %u", id);
	rec_dbg(g, "  id_type = %s", nvgpu_id_type_to_str(id_type));
	rec_dbg(g, "  rc_type = %s", nvgpu_rc_type_to_str(rc_type));
	rec_dbg(g, "  Runlist = %u", runlist_id);

	/*
	 * We currently only expect recovery to be triggered in the VF as a
	 * result of receiving an MMU fault.
	 */
	nvgpu_assert(rc_type == RC_TYPE_MMU_FAULT);

	/*
	 * We currently do not support deferred reset in virtualization.
	 */
	nvgpu_assert(!should_defer_reset);

	msg.cmd = TEGRA_VGPU_CMD_VF_HANDLE_FAULT;
	msg.handle = vgpu_get_handle(g);
	p->virt_runlist_id = runlist_id;
	p->tsgid = id;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err == 0 ? msg.ret : err;
	if (err != 0) {
		nvgpu_err(g, "cmd failed err=%d", err);
	}
}
