// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/runlist.h>

#include <nvgpu/vgpu/vgpu.h>

#include "tsg_vgpu.h"

int vgpu_tsg_force_reset_ch(struct nvgpu_channel *ch,
					u32 err_code, bool verbose)
{
	struct nvgpu_tsg *tsg = NULL;
	struct nvgpu_channel *ch_tsg = NULL;
	struct gk20a *g = ch->g;
	struct tegra_vgpu_cmd_msg msg = {0};
	struct tegra_vgpu_tsg_params *p = &msg.params.tsg;
	int err;

	nvgpu_log_fn(g, " ");

	tsg = nvgpu_tsg_from_ch(ch);
	if (tsg != NULL) {
		nvgpu_rwsem_down_read(&tsg->ch_list_lock);

		nvgpu_list_for_each_entry(ch_tsg, &tsg->ch_list,
				nvgpu_channel, ch_entry) {
			if (nvgpu_channel_get(ch_tsg)) {
				nvgpu_channel_set_error_notifier(g, ch_tsg,
								err_code);
				nvgpu_channel_set_unserviceable(ch_tsg);
				nvgpu_channel_put(ch_tsg);
			}
		}

		nvgpu_rwsem_up_read(&tsg->ch_list_lock);
	} else {
		nvgpu_err(g, "chid: %d is not bound to tsg", ch->chid);
	}

	msg.cmd = TEGRA_VGPU_CMD_TSG_FORCE_RESET;
	msg.handle = vgpu_get_handle(ch->g);
	p->runlist_id = tsg->runlist->id;
	p->tsg_id = tsg->tsgid;
	err = vgpu_comm_sendrecv(g, &msg);
	WARN_ON(err || msg.ret);
	if (!err) {
		nvgpu_channel_abort(ch, false);
	}
	return err ? err : msg.ret;
}

void vgpu_tsg_handle_event(struct gk20a *g,
			struct tegra_vgpu_channel_event_info *info)
{
	if (info->tsgid >= g->fifo.num_channels ||
		info->event_id >= TEGRA_VGPU_CHANNEL_EVENT_ID_MAX) {
		nvgpu_err(g, "invalid tsg event");
		return;
	}

	g->ops.tsg.post_event_id(g, 0U, info->tsgid, info->event_id);
}

void vgpu_tsg_set_error_notifier(struct gk20a *g,
				struct tegra_vgpu_tsg_set_error_notifier *p)
{
	if (p->tsgid >= g->fifo.num_channels) {
		nvgpu_err(g, "invalid tsgid %u", p->tsgid);
		return;
	}

	nvgpu_tsg_set_error_notifier(g, 0U, p->tsgid, p->error);
}
