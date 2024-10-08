/*
 * Copyright (c) 2011-2023, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/io.h>
#include <nvgpu/channel.h>
#include <nvgpu/runlist.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/fifo.h>
#include <nvgpu/engine_status.h>
#include <nvgpu/engines.h>
#include <nvgpu/gr/gr_falcon.h>

#include "runlist_fifo_gk20a.h"

#include <nvgpu/hw/gk20a/hw_fifo_gk20a.h>

u32 gk20a_runlist_length_max(struct gk20a *g)
{
	(void)g;
	return fifo_eng_runlist_length_max_v();
}

void gk20a_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count)
{
	nvgpu_spinlock_acquire(&g->fifo.runlist_submit_lock);

	if (count != 0U) {
		nvgpu_writel(g, fifo_runlist_base_r(),
			     fifo_runlist_base_ptr_f(u64_lo32(runlist_iova >> 12U)) |
			     nvgpu_aperture_mask_raw(g, aperture,
				fifo_runlist_base_target_sys_mem_ncoh_f(),
				fifo_runlist_base_target_sys_mem_coh_f(),
				fifo_runlist_base_target_vid_mem_f()));
	}

	nvgpu_writel(g, fifo_runlist_r(),
		     fifo_runlist_engine_f(runlist_id) |
		     fifo_eng_runlist_length_f(count));

	nvgpu_spinlock_release(&g->fifo.runlist_submit_lock);
}

int gk20a_runlist_check_pending(struct gk20a *g, struct nvgpu_runlist *runlist)
{
	int ret = 1;

	if ((nvgpu_readl(g, fifo_eng_runlist_r(runlist->id)) &
			fifo_eng_runlist_pending_true_f()) == 0U) {
		ret = 0;
	}

	return ret;
}

void gk20a_runlist_write_state(struct gk20a *g, u32 runlists_mask,
					 u32 runlist_state)
{
	u32 reg_val;
	u32 reg_mask = 0U;
	u32 i = 0U;

	while (runlists_mask != 0U && (i < g->fifo.max_runlists)) {
		if ((runlists_mask & BIT32(i)) != 0U) {
			reg_mask |= fifo_sched_disable_runlist_m(i);
		}
		runlists_mask &= ~BIT32(i);
		i++;
	}

	reg_val = nvgpu_readl(g, fifo_sched_disable_r());

	if (runlist_state == RUNLIST_DISABLED) {
		reg_val |= reg_mask;
	} else {
		reg_val &= ~reg_mask;
	}

	nvgpu_writel(g, fifo_sched_disable_r(), reg_val);
}
