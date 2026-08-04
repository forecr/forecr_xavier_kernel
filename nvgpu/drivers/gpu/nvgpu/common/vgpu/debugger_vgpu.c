// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/debugger.h>
#include <nvgpu/bug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/string.h>
#include <nvgpu/regops.h>

#include "debugger_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"

int vgpu_exec_regops(struct gk20a *g,
		struct nvgpu_tsg *tsg,
		struct nvgpu_dbg_reg_op *ops,
		u32 num_ops,
		u32 ctx_wr_count,
		u32 ctx_rd_count,
		u32 *flags)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_reg_ops_params *p = &msg.params.reg_ops;
	void *oob;
	size_t oob_size, ops_size;
	int ivc;
	int err = 0;

	nvgpu_log_fn(g, " ");
	BUG_ON(sizeof(*ops) != sizeof(struct tegra_vgpu_reg_op));

	ivc = vgpu_comm_alloc_ivc_oob(g, &oob, &oob_size);
	if (ivc < 0) {
		return ivc;
	}

	ops_size = sizeof(*ops) * num_ops;
	if (oob_size < ops_size) {
		err = -ENOMEM;
		goto fail;
	}

	nvgpu_memcpy((u8 *)oob, (u8 *)ops, ops_size);

	msg.cmd = TEGRA_VGPU_CMD_REG_OPS;
	msg.handle = vgpu_get_handle(g);
	p->tsg_id = tsg ? tsg->tsgid : U32_MAX;
	p->num_ops = num_ops;
	p->flags = *flags;
	err = vgpu_comm_sendrecv_locked(g, ivc, &msg);
	err = err ? err : msg.ret;
	if (err == 0) {
		nvgpu_memcpy((u8 *)ops, (u8 *)oob, ops_size);
	}
	*flags = p->flags;

fail:
	vgpu_comm_free_ivc(g, ivc);
	return err;
}

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

int vgpu_tsg_set_long_timeslice(struct nvgpu_tsg *tsg, u32 timeslice_us)
{
	struct tegra_vgpu_cmd_msg msg = {0};
	struct tegra_vgpu_tsg_timeslice_params *p =
				&msg.params.tsg_timeslice;
	int err;
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_TSG_SET_LONG_TIMESLICE;
	msg.handle = vgpu_get_handle(g);
	p->tsg_id = tsg->tsgid;
	p->timeslice_us = timeslice_us;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	WARN_ON(err);
	if (!err) {
		tsg->timeslice_us = timeslice_us;
	}

	return err;
}
