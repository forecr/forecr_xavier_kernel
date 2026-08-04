// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/channel.h>
#include <nvgpu/dma.h>
#include <nvgpu/gk20a.h>

#include "hal/fifo/tsg_ga10b.h"

int ga10b_tsg_unbind_channel_check_hw_next(struct nvgpu_channel *ch,
		struct nvgpu_channel_hw_state *hw_state)
{
	if (hw_state->next) {
		nvgpu_log_info(ch->g, "Channel %d to be removed "
			"from TSG %d has NEXT set!",
			ch->chid, ch->tsgid);
		return -EAGAIN;
	}

	return 0;
}
