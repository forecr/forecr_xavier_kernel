// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/fifo.h>
#include <nvgpu/runlist.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/preempt.h>
#include <nvgpu/nvgpu_err.h>

#include "fifo_utils_ga10b.h"
#include "preempt_ga10b.h"

#include <nvgpu/hw/ga10b/hw_runlist_ga10b.h>

u32 ga10b_runlist_preempt_reg(void)
{
	return runlist_preempt_r();
}

void ga10b_fifo_preempt_trigger(struct gk20a *g,
		u32 runlist_id, u32 tsgid, unsigned int id_type)
{
	struct nvgpu_runlist *runlist;
	u32 runlist_preempt_reg;

	if (runlist_id == INVAL_ID ||
			(tsgid == INVAL_ID && id_type == ID_TYPE_TSG)) {
		nvgpu_log(g, gpu_dbg_info, "Invalid id, cannot preempt");
		return;
	}

	runlist = g->fifo.runlists[runlist_id];

	runlist_preempt_reg = g->ops.fifo.get_runlist_preempt_reg();

	if (id_type == ID_TYPE_TSG) {
		nvgpu_runlist_writel(g, runlist, runlist_preempt_reg,
					runlist_preempt_id_f(tsgid) |
					runlist_preempt_type_tsg_f());
	} else if (id_type == ID_TYPE_RUNLIST) {
		nvgpu_runlist_writel(g, runlist, runlist_preempt_reg,
				runlist_preempt_type_runlist_f());
	} else {
		nvgpu_log_info(g, "id_type=%u preempt is noop", id_type);
	}
}
