// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/debugger.h>

#include "debugger_vgpu.h"

int vgpu_dbg_set_powergate(struct dbg_session_gk20a *dbg_s,
			bool disable_powergate)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_set_powergate_params *p = &msg.params.set_powergate;
	int err = 0;
	u32 mode;
	struct gk20a *g = dbg_s->g;

	nvgpu_log_fn(g, " ");

	/* Just return if requested mode is the same as the session's mode */
	if (disable_powergate) {
		if (dbg_s->is_pg_disabled) {
			return 0;
		}
		dbg_s->is_pg_disabled = true;
		mode = TEGRA_VGPU_POWERGATE_MODE_DISABLE;
	} else {
		if (!dbg_s->is_pg_disabled) {
			return 0;
		}
		dbg_s->is_pg_disabled = false;
		mode = TEGRA_VGPU_POWERGATE_MODE_ENABLE;
	}

	msg.cmd = TEGRA_VGPU_CMD_SET_POWERGATE;
	msg.handle = vgpu_get_handle(dbg_s->g);
	p->mode = mode;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}
