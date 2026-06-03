// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/runlist.h>

#include "runlist_fifo_gv11b.h"

#include <nvgpu/hw/gv11b/hw_fifo_gv11b.h>

#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
int gv11b_runlist_reschedule(struct nvgpu_channel *ch, bool preempt_next)
{
	(void)preempt_next;
	/*
	 * gv11b allows multiple outstanding preempts,
	 * so always preempt next for best reschedule effect
	 */
	return nvgpu_runlist_reschedule(ch, true, false);
}
#endif
