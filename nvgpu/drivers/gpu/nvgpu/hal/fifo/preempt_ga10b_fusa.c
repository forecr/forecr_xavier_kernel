/*
 * Copyright (c) 2020-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <nvgpu/fifo.h>
#include <nvgpu/runlist.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/preempt.h>
#include <nvgpu/nvgpu_err.h>

#include "fifo_utils_ga10b.h"
#include "preempt_ga10b.h"

#include <nvgpu/hw/ga10b/hw_runlist_ga10b.h>

void ga10b_fifo_preempt_trigger(struct gk20a *g,
		u32 runlist_id, u32 tsgid, unsigned int id_type)
{
	struct nvgpu_runlist *runlist;

	if (runlist_id == INVAL_ID ||
			(tsgid == INVAL_ID && id_type == ID_TYPE_TSG)) {
		nvgpu_log(g, gpu_dbg_info, "Invalid id, cannot preempt");
		return;
	}

	runlist = g->fifo.runlists[runlist_id];

	if (id_type == ID_TYPE_TSG) {
		nvgpu_runlist_writel(g, runlist, runlist_preempt_r(),
					runlist_preempt_id_f(tsgid) |
					runlist_preempt_type_tsg_f());
	} else if (id_type == ID_TYPE_RUNLIST) {
		nvgpu_runlist_writel(g, runlist, runlist_preempt_r(),
				runlist_preempt_type_runlist_f());
	} else {
		nvgpu_log_info(g, "id_type=%u preempt is noop", id_type);
	}
}
