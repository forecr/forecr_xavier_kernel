// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/gr/ctx.h>
#include <common/gr/gr_priv.h>
#include <nvgpu/tsg_subctx.h>
#include <nvgpu/pm_reservation.h>
#include <nvgpu/profiler.h>
#include <nvgpu/perfbuf.h>

#include "hal/gr/gr/gr_gk20a.h"

#include "common/vgpu/ivc/comm_vgpu.h"

#include "common/vgpu/gr/gr_vf.h"
#include "profiler_vf.h"

int vf_profiler_bind_hwpm(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg, bool streamout,
		u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	u32 mode = streamout ? NVGPU_GR_CTX_HWPM_CTXSW_MODE_STREAM_OUT_CTXSW :
				NVGPU_GR_CTX_HWPM_CTXSW_MODE_CTXSW;
	int err = 0;

	nvgpu_assert(gr_instance_id == 0U);

	if (is_ctxsw) {
		err = gr_gk20a_update_hwpm_ctxsw_mode(g, gr_instance_id, tsg, mode);
		if (err != 0) {
			nvgpu_err(g, "hwpm ctxsw mode update failed with err - %d", err);
			goto out;
		}
	}

	msg.cmd = TEGRA_VGPU_CMD_VF_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_BIND_HWPM;
	p->is_ctxsw = is_ctxsw;
	p->reservation_id = reservation_id;
	p->gr_instance_id = gr_instance_id;
	p->mode = mode;

	nvgpu_log(g, gpu_dbg_prof, "VM attr - %u, - %u",
					mode, reservation_id);
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

out:
	return err;
}

int vf_profiler_unbind_hwpm(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg)
{
	u32 mode = NVGPU_GR_CTX_HWPM_CTXSW_MODE_NO_CTXSW;
	int err = 0;

	nvgpu_assert(gr_instance_id == 0U);

	if (is_ctxsw) {
		err = gr_gk20a_update_hwpm_ctxsw_mode(g, gr_instance_id, tsg, mode);
		if (err != 0) {
			nvgpu_err(g, "hwpm ctxsw mode update failed with err - %d", err);
		}
	}

	return err;
}

int vf_gb10b_profiler_bind_smpc(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	int err = 0;

	nvgpu_assert(gr_instance_id == 0U);

	if (is_ctxsw) {
		err = g->ops.gr.update_smpc_ctxsw_mode(g, tsg, true);
		if (err != 0) {
			nvgpu_err(g, "smpc ctxsw update failed with err - %d", err);
			goto out;
		}
	}

	msg.cmd = TEGRA_VGPU_CMD_VF_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_BIND_SMPC;
	p->is_ctxsw = is_ctxsw;
	p->reservation_id = reservation_id;
	p->gr_instance_id = gr_instance_id;

	nvgpu_log(g, gpu_dbg_prof, "VM attr - %u", reservation_id);
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

out:
	return err;
}

int vf_gb10b_profiler_unbind_smpc(struct gk20a *g,
		u32 gr_instance_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	int err;

	nvgpu_assert(gr_instance_id == 0U);

	if (is_ctxsw) {
		err = g->ops.gr.update_smpc_ctxsw_mode(g, tsg, false);
		if (err != 0) {
			nvgpu_err(g, "smpc ctxsw mode update failed with err - %d", err);
		}
		return err;
	}

	msg.cmd = TEGRA_VGPU_CMD_VF_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_UNBIND_SMPC;
	p->is_ctxsw = is_ctxsw;
	p->reservation_id = reservation_id;
	p->gr_instance_id = gr_instance_id;

	nvgpu_log(g, gpu_dbg_prof, "VM attr - %u", reservation_id);
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	return err;
}

void vf_profiler_bind_hes_cwd(struct gk20a *g,
		u32 gr_instance_id,
		u32 reservation_id,
		u32 pma_channel_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_prof_bind_unbind_params *p =
			&msg.params.prof_bind_unbind;
	int err;

	nvgpu_assert(gr_instance_id == 0U);

	msg.cmd = TEGRA_VGPU_CMD_VF_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_BIND_HES_CWD;
	p->reservation_id = reservation_id;
	p->gr_instance_id = gr_instance_id;
	p->pma_channel_id = pma_channel_id;

	nvgpu_log(g, gpu_dbg_prof, "VM attr - %u", reservation_id);
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	if (err != 0)
		nvgpu_err(g, "HES bind/unbind failed with err - %d", err);

	return ;
}

void vf_profiler_set_hs_credit_per_chiplet(struct gk20a *g, u32 gr_instance_id,
			u32 pma_channel_id, u32 chiplet_type,
			u32 chiplet_local_index, u32 num_of_credits)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_set_hs_credits *p =
			&msg.params.set_hs_credit;
	int err;

	nvgpu_assert(gr_instance_id == 0U);

	msg.cmd = TEGRA_VGPU_CMD_VF_SET_HS_CREDITS;
	msg.handle = vgpu_get_handle(g);

	p->gr_instance_id = gr_instance_id;
	p->pma_channel_id = pma_channel_id;
	p->chiplet_type = chiplet_type;
	p->chiplet_local_index = chiplet_local_index;
	p->num_credits = num_of_credits;


	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	if (err != 0)
		nvgpu_err(g, "Set HS credit cmd err - %d", err);
}

int vf_profiler_bind_pma_streamout(struct gk20a *g,
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

	nvgpu_assert(gr_instance_id == 0U);

	msg.cmd = TEGRA_VGPU_CMD_VF_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_BIND_STREAMOUT;
	p->pma_buffer_va = pma_buffer_va;
	p->pma_buffer_size = pma_buffer_size;
	p->pma_bytes_available_buffer_va = pma_bytes_available_buffer_va;
	p->reservation_id = reservation_id;
	p->pma_channel_id = pma_channel_id;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}

static int vf_profiler_reset_mem_bytes_disable_perfbuf(struct gk20a *g,
		u32 pma_channel_id)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_reset_streaming *p =
		&msg.params.reset_streaming;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_VF_RESET_STREAMING;
	msg.handle = vgpu_get_handle(g);

	p->pma_channel_id = pma_channel_id;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	return err;
}

int vf_profiler_unbind_pma_streamout(struct gk20a *g,
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
	u64 bytes_available = 0;
	int err;

	nvgpu_assert(gr_instance_id == 0U);

	if (tsg != NULL)
		nvgpu_tsg_disable(tsg);

	msg.cmd = TEGRA_VGPU_CMD_VF_PROF_BIND_UNBIND;
	msg.handle = vgpu_get_handle(g);

	p->subcmd = TEGRA_VGPU_PROF_UNBIND_HWPM_STREAMOUT;
	p->reservation_id = reservation_id;
	p->pma_channel_id = pma_channel_id;
	p->tsg_id = tsg ? tsg->tsgid : U32_MAX;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (err != 0) {
		goto fail;
	}

	if (!is_ctxsw) {
		/* wait for all the inflight records from fb-hub to stream out */
		err = nvgpu_perfbuf_update_get_put(g, pma_channel_id, 0U,
				&bytes_available,
				pma_bytes_available_buffer_cpuva, true,
				NULL, NULL);
	} else {
		if (p->is_ctx_resident == false) {
			err = nvgpu_profiler_quiesce_hwpm_streamout_non_resident(g, tsg);
		} else {
			/* wait for all the inflight records from fb-hub to stream out */
			err = nvgpu_perfbuf_update_get_put(g, pma_channel_id, 0U,
					&bytes_available,
					pma_bytes_available_buffer_cpuva, true,
					NULL, NULL);
			if (err != 0) {
				nvgpu_err(g, "update get put ptrs failed");
			}
		}
	}

	if (tsg != NULL)
		nvgpu_tsg_enable(tsg);

	err = vf_profiler_reset_mem_bytes_disable_perfbuf(g, pma_channel_id);
	if (err != 0)
		nvgpu_err(g, "Streaming reset failed for pma channel - %u with err - %d",
				pma_channel_id, err);

fail:
	return err;
}
