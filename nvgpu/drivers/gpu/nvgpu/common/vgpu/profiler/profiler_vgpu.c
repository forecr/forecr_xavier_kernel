// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/tsg.h>
#include <nvgpu/pm_reservation.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gk20a.h>

#include "common/vgpu/ivc/comm_vgpu.h"
#include "profiler_vgpu.h"

int vgpu_profiler_bind_hwpm(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg, bool streamout,
		u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	int err;
	(void) reservation_id;

	nvgpu_assert(gr_instance_id == 0U);

	msg.cmd = TEGRA_VGPU_CMD_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_BIND_HWPM;
	p->is_ctxsw = is_ctxsw;
	p->tsg_id = tsg != NULL ? tsg->tsgid : NVGPU_INVALID_TSG_ID;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}

int vgpu_profiler_unbind_hwpm(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	int err;

	nvgpu_assert(gr_instance_id == 0U);

	msg.cmd = TEGRA_VGPU_CMD_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_UNBIND_HWPM;
	p->is_ctxsw = is_ctxsw;
	p->tsg_id = tsg != NULL ? tsg->tsgid : NVGPU_INVALID_TSG_ID;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}

int vgpu_profiler_bind_pma_streamout(struct gk20a *g,
		u32 gr_instance_id,
		u32 pma_channel_id,
		u64 pma_buffer_va,
		u32 pma_buffer_size,
		u64 pma_bytes_available_buffer_va,
		u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	int err;
	(void)reservation_id;
	(void)pma_channel_id;

	nvgpu_assert(gr_instance_id == 0U);

	msg.cmd = TEGRA_VGPU_CMD_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_BIND_STREAMOUT;
	p->pma_buffer_va = pma_buffer_va;
	p->pma_buffer_size = pma_buffer_size;
	p->pma_bytes_available_buffer_va = pma_bytes_available_buffer_va;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}

int vgpu_profiler_unbind_pma_streamout(struct gk20a *g,
		u32 gr_instance_id,
		u32 pma_channel_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		void *pma_bytes_available_buffer_cpuva,
		u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	int err;

	(void)reservation_id;

	nvgpu_assert(gr_instance_id == 0U);

	msg.cmd = TEGRA_VGPU_CMD_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_UNBIND_HWPM_STREAMOUT;
	p->is_ctxsw = is_ctxsw;
	p->tsg_id = tsg != NULL ? tsg->tsgid : NVGPU_INVALID_TSG_ID;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}

int vgpu_profiler_bind_smpc(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	int err;
	(void) reservation_id;

	nvgpu_assert(gr_instance_id == 0U);

	msg.cmd = TEGRA_VGPU_CMD_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_BIND_SMPC;
	p->is_ctxsw = is_ctxsw;
	p->tsg_id = tsg != NULL ? tsg->tsgid : NVGPU_INVALID_TSG_ID;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}

int vgpu_profiler_unbind_smpc(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	int err;
	(void) reservation_id;

	msg.cmd = TEGRA_VGPU_CMD_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_UNBIND_SMPC;
	p->is_ctxsw = is_ctxsw;
	p->tsg_id = tsg != NULL ? tsg->tsgid : NVGPU_INVALID_TSG_ID;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}
