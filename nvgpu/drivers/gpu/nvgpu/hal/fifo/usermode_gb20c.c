// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/runlist.h>

#include "usermode_gb20c.h"

#include <nvgpu/hw/gb20c/hw_ctrl_gb20c.h>

u32 gb20c_usermode_doorbell_token(struct gk20a *g, u32 runlist_id, u32 chid)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 hw_chid = f->channel_base + chid;

	/*
	 * From gb20x, HW requires SW to set enable bit for ESCHED to
	 * ring internal doorbell
	 */
	return ctrl_doorbell_vector_f(hw_chid) |
			ctrl_doorbell_runlist_id_f(runlist_id) |
			ctrl_doorbell_runlist_doorbell_enable_f();
}
