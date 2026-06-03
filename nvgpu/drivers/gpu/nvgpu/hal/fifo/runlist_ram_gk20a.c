// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/channel.h>
#include <nvgpu/runlist.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engine_status.h>
#include <nvgpu/engines.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/static_analysis.h>

#include "runlist_ram_gk20a.h"

#include <nvgpu/hw/gk20a/hw_ram_gk20a.h>

#define FECS_MAILBOX_0_ACK_RESTORE 0x4U

#define RL_MAX_TIMESLICE_TIMEOUT ram_rl_entry_timeslice_timeout_v(U32_MAX)
#define RL_MAX_TIMESLICE_SCALE ram_rl_entry_timeslice_scale_v(U32_MAX)

u32 gk20a_runlist_entry_size(struct gk20a *g)
{
	(void)g;
	return ram_rl_entry_size_v();
}

u32 gk20a_runlist_max_timeslice(void)
{
	return ((RL_MAX_TIMESLICE_TIMEOUT << RL_MAX_TIMESLICE_SCALE) / 1000) * 1024;
}

void gk20a_runlist_get_tsg_entry(struct nvgpu_tsg *tsg,
		u32 *runlist, u32 timeslice)
{
	struct gk20a *g = tsg->g;
	u32 timeout = timeslice;
	u32 scale = 0U;

	WARN_ON(timeslice == 0U);

	while (timeout > RL_MAX_TIMESLICE_TIMEOUT) {
		timeout >>= 1U;
		scale = nvgpu_safe_add_u32(scale, 1U);
	}

	if (scale > RL_MAX_TIMESLICE_SCALE) {
		nvgpu_err(g, "requested timeslice value is clamped");
		timeout = RL_MAX_TIMESLICE_TIMEOUT;
		scale = RL_MAX_TIMESLICE_SCALE;
	}

	runlist[0] = ram_rl_entry_id_f(tsg->tsgid) |
			ram_rl_entry_type_tsg_f() |
			ram_rl_entry_tsg_length_f(tsg->num_active_channels) |
			ram_rl_entry_timeslice_scale_f(scale) |
			ram_rl_entry_timeslice_timeout_f(timeout);
	runlist[1] = 0;
}

void gk20a_runlist_get_ch_entry(struct nvgpu_channel *ch, u32 *runlist)
{
	runlist[0] = ram_rl_entry_chid_f(ch->chid);
	runlist[1] = 0;
}

u32 gk20a_runlist_get_max_channels_per_tsg(void)
{
	return ram_rl_entry_tsg_length_max_v();
}
