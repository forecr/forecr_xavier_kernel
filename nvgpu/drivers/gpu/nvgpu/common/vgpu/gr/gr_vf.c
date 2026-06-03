// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/tsg.h>
#include <nvgpu/debugger.h>
#include <nvgpu/string.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/vgpu_ivc.h>

#include "common/vgpu/ivc/comm_vgpu.h"
#include "gr_vf.h"

bool vgpu_vf_gr_set_mmu_nack_pending(struct gk20a *g, u32 gr_instance_id,
		u32 gr_ctx_id)
{
	int err = 0;
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_mmu_nack_pending_params *p =
		&msg.params.vf_mmu_nack_pending;

	msg.cmd = TEGRA_VGPU_CMD_VF_SET_MMU_NACK_PENDING;
	msg.handle = vgpu_get_handle(g);
	p->gr_instance_id = gr_instance_id;
	p->gr_ctx_id = gr_ctx_id;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err == 0 ? msg.ret : err;
	if (err != 0) {
		nvgpu_err(g, "cmd failed err=%d", err);
		return false;
	}

	return p->handled;
}

int vgpu_vf_gr_clear_sm_error_state(struct gk20a *g,
		struct nvgpu_channel *ch, u32 sm_id)
{
	struct nvgpu_tsg *tsg;

	tsg = nvgpu_tsg_from_ch(ch);
	if (!tsg) {
		return -EINVAL;
	}

	nvgpu_mutex_acquire(&g->dbg_sessions_lock);
	(void)memset(&tsg->sm_error_states[sm_id], 0,
			sizeof(*tsg->sm_error_states));
	nvgpu_mutex_release(&g->dbg_sessions_lock);

	return 0;
}

int vgpu_vf_gr_suspend_contexts(struct gk20a *g,
		struct dbg_session_gk20a *dbg_s,
		int *ctx_resident_ch_fd)
{
	struct dbg_session_channel_data *ch_data;
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_suspend_contexts *p;
	size_t i = 0;
	size_t n = 0;
	int channel_fd = -1;
	int ivc = -1;
	int err = 0;
	u16 *oob;
	size_t oob_size;
	u16 *tsg_buffer = NULL;

	nvgpu_mutex_acquire(&g->dbg_sessions_lock);
	nvgpu_mutex_acquire(&dbg_s->ch_list_lock);

	nvgpu_list_for_each_entry(ch_data, &dbg_s->ch_list,
			dbg_session_channel_data, ch_entry) {
		n++;
	}

	tsg_buffer = nvgpu_kzalloc(g, n * sizeof(u16));
	if (tsg_buffer == NULL) {
		err = -ENOMEM;
		goto done;
	}

	n = 0;
	nvgpu_list_for_each_entry(ch_data, &dbg_s->ch_list,
			dbg_session_channel_data, ch_entry) {
		struct nvgpu_channel *ch = &g->fifo.channel[ch_data->chid];
		bool found = false;

		for (i = 0; i < n; i++) {
			if ((u32)tsg_buffer[i] == ch->tsgid) {
				found = true;
				break;
			}
		}
		if (found) {
			continue;
		}

		nvgpu_channel_disable_tsg(g, ch);
		tsg_buffer[n++] = (u16)ch->tsgid;
	}

	msg.cmd = TEGRA_VGPU_CMD_VF_SUSPEND_CONTEXTS;
	msg.handle = vgpu_get_handle(g);
	p = &msg.params.vf_suspend_contexts;
	p->num_tsgs = n;

	ivc = vgpu_comm_alloc_ivc_oob(g, (void **)&oob, &oob_size);
	if (ivc < 0) {
		err = ivc;
		goto done;
	}

	if (oob_size < n * sizeof(u16)) {
		err = -ENOMEM;
		goto done;
	}

	nvgpu_memcpy((u8 *)oob, (u8 *)tsg_buffer, n * sizeof(u16));

	err = vgpu_comm_sendrecv_locked(g, ivc, &msg);
	err = err == 0 ? msg.ret : err;
	if (err != 0) {
		nvgpu_err(g, "ivc failed err=%d", err);
		goto done;
	}

	if (p->resident_tsgid != (u16)~0) {
		nvgpu_list_for_each_entry(ch_data, &dbg_s->ch_list,
				dbg_session_channel_data, ch_entry) {
			struct nvgpu_channel *ch = &g->fifo.channel[ch_data->chid];
			if (ch->tsgid == p->resident_tsgid) {
				channel_fd = ch_data->channel_fd;
				break;
			}
		}
	}

done:
	if (ivc >= 0) {
		vgpu_comm_free_ivc(g, ivc);
	}
	nvgpu_kfree(g, tsg_buffer);
	nvgpu_mutex_release(&dbg_s->ch_list_lock);
	nvgpu_mutex_release(&g->dbg_sessions_lock);
	*ctx_resident_ch_fd = channel_fd;
	return err;
}

int vgpu_vf_gr_resume_contexts(struct gk20a *g,
		struct dbg_session_gk20a *dbg_s,
		int *ctx_resident_ch_fd)
{
	struct dbg_session_channel_data *ch_data;

	nvgpu_mutex_acquire(&g->dbg_sessions_lock);
	nvgpu_mutex_acquire(&dbg_s->ch_list_lock);

	nvgpu_list_for_each_entry(ch_data, &dbg_s->ch_list,
			dbg_session_channel_data, ch_entry) {
		struct nvgpu_channel *ch = &g->fifo.channel[ch_data->chid];

		nvgpu_channel_enable_tsg(g, ch);
	}

	nvgpu_mutex_release(&dbg_s->ch_list_lock);
	nvgpu_mutex_release(&g->dbg_sessions_lock);

	*ctx_resident_ch_fd = -1;

	return 0;
}
