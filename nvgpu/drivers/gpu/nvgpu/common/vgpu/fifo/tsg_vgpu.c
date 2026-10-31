// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/runlist.h>
#include <nvgpu/tsg.h>
#include <nvgpu/bug.h>

#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/vgpu.h>

#include "tsg_vgpu.h"
#include "channel_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"

int vgpu_tsg_open(struct nvgpu_tsg *tsg)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_tsg_open_rel_params *p =
				&msg.params.tsg_open;
	int err;
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_TSG_OPEN;
	msg.handle = vgpu_get_handle(tsg->g);
	p->tsg_id = tsg->tsgid;
	p->pid = tsg->tgid;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (err) {
		nvgpu_err(tsg->g,
			"vgpu_tsg_open failed, tsgid %d", tsg->tsgid);
	}

	return err;
}

void vgpu_tsg_release(struct nvgpu_tsg *tsg)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_tsg_open_rel_params *p =
				&msg.params.tsg_release;
	int err;
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_TSG_RELEASE;
	msg.handle = vgpu_get_handle(tsg->g);
	p->tsg_id = tsg->tsgid;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (err) {
		nvgpu_err(tsg->g,
			"vgpu_tsg_release failed, tsgid %d", tsg->tsgid);
	}
}

/*
 * Must always be called with tsg->ch_list_lock held.
 */
void vgpu_tsg_enable(struct nvgpu_tsg *tsg)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_tsg_enable_disable *p =
				&msg.params.tsg_enable;
	int err;
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_TSG_ENABLE;
	msg.handle = vgpu_get_handle(tsg->g);
	p->tsg_id = tsg->tsgid;
	err = vgpu_comm_sendrecv(g, &msg);
	err = (err != 0) ? (err) : (msg.ret);
	if (err != 0) {
		nvgpu_err(tsg->g,
			"vgpu_tsg_enable failed, tsgid %d", tsg->tsgid);
	}
	nvgpu_log_fn(tsg->g, "done");
}

/*
 * Must always be called with tsg->ch_list_lock held.
 */
void vgpu_tsg_disable(struct nvgpu_tsg *tsg)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_tsg_enable_disable *p =
				&msg.params.tsg_disable;
	int err;
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_TSG_DISABLE;
	msg.handle = vgpu_get_handle(tsg->g);
	p->tsg_id = tsg->tsgid;
	err = vgpu_comm_sendrecv(g, &msg);
	err = (err != 0) ? (err) : (msg.ret);
	if (err != 0) {
		nvgpu_err(tsg->g,
			"vgpu_tsg_disable failed, tsgid %d", tsg->tsgid);
	}
	nvgpu_log_fn(tsg->g, "done");
}

int vgpu_tsg_bind_channel(struct nvgpu_tsg *tsg, struct nvgpu_channel *ch)
{
	struct tegra_vgpu_cmd_msg msg = {};
	int err;
	struct gk20a *g = ch->g;

	nvgpu_log_fn(g, " ");

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		struct tegra_vgpu_tsg_bind_unbind_channel_params *p =
				&msg.params.tsg_bind_unbind_channel;

		msg.cmd = TEGRA_VGPU_CMD_TSG_BIND_CHANNEL;
		p->tsg_id = tsg->tsgid;
		p->ch_handle = ch->virt_ctx;
		p->runlist_id = ch->runlist->id;
	} else {
		struct tegra_vgpu_tsg_bind_channel_ex_params *p =
				&msg.params.tsg_bind_channel_ex;

		msg.cmd = TEGRA_VGPU_CMD_TSG_BIND_CHANNEL_EX;
		p->tsg_id = tsg->tsgid;
		p->ch_handle = ch->virt_ctx;
		p->runlist_id = ch->runlist->id;
		p->subctx_id = ch->subctx_id;
		p->runqueue_sel = ch->runqueue_sel;
	}

	msg.handle = vgpu_get_handle(g);

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (err) {
		nvgpu_err(g, "vgpu_tsg_bind_channel failed, ch %d tsgid %d",
			ch->chid, tsg->tsgid);
	}

	return err;
}

int vgpu_tsg_unbind_channel(struct nvgpu_tsg *tsg, struct nvgpu_channel *ch)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_tsg_bind_unbind_channel_params *p =
				&msg.params.tsg_bind_unbind_channel;
	int err;
	struct gk20a *g = ch->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_TSG_UNBIND_CHANNEL;
	msg.handle = vgpu_get_handle(g);
	p->ch_handle = ch->virt_ctx;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	WARN_ON(err);

	return err;
}

int vgpu_tsg_set_timeslice(struct nvgpu_tsg *tsg, u32 timeslice)
{
	struct tegra_vgpu_cmd_msg msg = {0};
	struct tegra_vgpu_tsg_timeslice_params *p =
				&msg.params.tsg_timeslice;
	int err;
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_TSG_SET_TIMESLICE;
	msg.handle = vgpu_get_handle(g);
	p->tsg_id = tsg->tsgid;
	p->timeslice_us = timeslice;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	WARN_ON(err);
	if (!err) {
		tsg->timeslice_us = timeslice;
	}

	return err;
}

int vgpu_set_sm_exception_type_mask(struct nvgpu_channel *ch,
		u32 exception_mask)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_set_sm_exception_type_mask_params *p =
		&msg.params.set_sm_exception_mask;
	int err = 0;
	struct gk20a *g = ch->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_SET_SM_EXCEPTION_TYPE_MASK;
	msg.handle = vgpu_get_handle(g);
	p->handle = ch->virt_ctx;
	p->mask = exception_mask;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	WARN_ON(err);

	return err;
}

int vgpu_tsg_set_interleave(struct nvgpu_tsg *tsg, u32 new_level)
{
	struct tegra_vgpu_cmd_msg msg = {0};
	struct tegra_vgpu_tsg_runlist_interleave_params *p =
			&msg.params.tsg_interleave;
	int err;
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_TSG_SET_RUNLIST_INTERLEAVE;
	msg.handle = vgpu_get_handle(g);
	p->tsg_id = tsg->tsgid;
	p->level = new_level;
	err = vgpu_comm_sendrecv(g, &msg);
	WARN_ON(err || msg.ret);
	return err ? err : msg.ret;
}

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

u32 vgpu_tsg_default_timeslice_us(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	return priv->constants.default_timeslice_us;
}

void vgpu_tsg_set_ctx_mmu_error(struct gk20a *g, u32 chid)
{
	struct nvgpu_channel *ch = nvgpu_channel_from_id(g, chid);
	struct nvgpu_tsg *tsg = NULL;

	if (ch == NULL) {
		return;
	}

	tsg = nvgpu_tsg_from_ch(ch);
	if (tsg != NULL) {
		struct nvgpu_channel *ch_tsg = NULL;

		nvgpu_rwsem_down_read(&tsg->ch_list_lock);

		nvgpu_list_for_each_entry(ch_tsg, &tsg->ch_list,
				nvgpu_channel, ch_entry) {
			if (nvgpu_channel_get(ch_tsg)) {
				vgpu_channel_set_ctx_mmu_error(g, ch_tsg);
				nvgpu_channel_put(ch_tsg);
			}
		}

		nvgpu_rwsem_up_read(&tsg->ch_list_lock);
	} else {
		nvgpu_err(g, "chid: %d is not bound to tsg", ch->chid);
	}
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
