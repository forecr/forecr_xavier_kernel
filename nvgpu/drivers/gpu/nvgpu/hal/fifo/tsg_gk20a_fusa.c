// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/channel.h>

#include "hal/fifo/tsg_gk20a.h"

int gk20a_tsg_unbind_channel_check_hw_next(struct nvgpu_channel *ch,
		struct nvgpu_channel_hw_state *hw_state)
{
	if (hw_state->next) {
		/*
		 * There is a possibility that the user sees the channel
		 * has finished all the work and invokes channel removal
		 * before the scheduler marks it idle (clears NEXT bit).
		 * Scheduler can miss marking the channel idle if the
		 * timeslice expires just after the work finishes.
		 *
		 * nvgpu will then see NEXT bit set even though the
		 * channel has no work left. To catch this case,
		 * reenable the tsg and check the hw state again
		 * to see if the channel is truly idle.
		 */
		nvgpu_log_info(ch->g, "Channel %d to be removed "
			"from TSG %d has NEXT set!",
			ch->chid, ch->tsgid);
		return -EAGAIN;
	}

	return 0;
}
