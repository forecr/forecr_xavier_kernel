// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/channel.h>
#include <nvgpu/tsg.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/runlist.h>

#include "hal/fifo/tsg_gk20a.h"

/*
 * Must always be called with tsg->ch_list_lock held.
 */
void gk20a_tsg_enable(struct nvgpu_tsg *tsg)
{
	struct gk20a *g = tsg->g;
	struct nvgpu_channel *ch;

	nvgpu_log_fn(g, " ");

	if (tsg->runlist == NULL) {
		/*
		 * Enabling a TSG that has no runlist (implies no channels)
		 * is just a noop.
		 */
		nvgpu_log_fn(g, "done");
		return;
	}

	nvgpu_runlist_set_state(g, BIT32(tsg->runlist->id),
				RUNLIST_DISABLED);

	/*
	 * Due to h/w bug that exists in Maxwell and Pascal,
	 * we first need to enable all channels with NEXT and CTX_RELOAD set,
	 * and then rest of the channels should be enabled
	 */
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		struct nvgpu_channel_hw_state hw_state;

		g->ops.channel.read_state(g, ch->runlist->id, ch->chid,
					&hw_state);

		if (hw_state.next || hw_state.ctx_reload) {
			nvgpu_channel_enable(ch);
		}
	}

	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		struct nvgpu_channel_hw_state hw_state;

		g->ops.channel.read_state(g, ch->runlist->id, ch->chid,
					&hw_state);

		if (hw_state.next || hw_state.ctx_reload) {
			continue;
		}

		nvgpu_channel_enable(ch);
	}

	nvgpu_runlist_set_state(g, BIT32(tsg->runlist->id),
				RUNLIST_ENABLED);

	nvgpu_log_fn(g, "done");
}
