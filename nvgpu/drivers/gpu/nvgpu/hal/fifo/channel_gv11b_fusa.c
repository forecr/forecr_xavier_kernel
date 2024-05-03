/*
 * Copyright (c) 2016-2023, NVIDIA CORPORATION.  All rights reserved.
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


#include <nvgpu/channel.h>
#include <nvgpu/log.h>
#include <nvgpu/atomic.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/runlist.h>

#include "channel_gk20a.h"
#include "channel_gv11b.h"

#include <nvgpu/hw/gv11b/hw_ccsr_gv11b.h>

void gv11b_channel_bind(struct nvgpu_channel *ch)
{
	struct gk20a *g = ch->g;
	int err;

	u32 inst_ptr = nvgpu_inst_block_ptr(g, &ch->inst_block);

	nvgpu_log_info(g, "bind channel %d inst ptr 0x%08x",
		ch->chid, inst_ptr);

	/* Enable subcontext */
	if ((g->ops.tsg.add_subctx_channel_hw != NULL) &&
		(!nvgpu_engine_is_multimedia_runlist_id(g, ch->runlist->id))) {
		err = g->ops.tsg.add_subctx_channel_hw(ch, ch->replayable);
		if (err != 0) {
			nvgpu_err(g, "Subcontext addition failed %d", err);
			return;
		}
	}

	/* Enable channel */
	nvgpu_writel(g, ccsr_channel_inst_r(ch->chid),
		     ccsr_channel_inst_ptr_f(inst_ptr) |
		     nvgpu_aperture_mask(g, &ch->inst_block,
				ccsr_channel_inst_target_sys_mem_ncoh_f(),
				ccsr_channel_inst_target_sys_mem_coh_f(),
				ccsr_channel_inst_target_vid_mem_f()) |
		     ccsr_channel_inst_bind_true_f());

	nvgpu_writel(g, ccsr_channel_r(ch->chid),
		(nvgpu_readl(g, ccsr_channel_r(ch->chid)) &
		 ~ccsr_channel_enable_set_f(~U32(0U))) |
		 ccsr_channel_enable_set_true_f());

	nvgpu_atomic_set(&ch->bound, 1);
}

void gv11b_channel_unbind(struct nvgpu_channel *ch)
{
	struct gk20a *g = ch->g;

	nvgpu_log_fn(g, " ");

	if (nvgpu_atomic_cmpxchg(&ch->bound, 1, 0) != 0) {
		nvgpu_writel(g, ccsr_channel_inst_r(ch->chid),
			ccsr_channel_inst_ptr_f(0U) |
			ccsr_channel_inst_bind_false_f());

		nvgpu_writel(g, ccsr_channel_r(ch->chid),
			ccsr_channel_enable_clr_true_f() |
			ccsr_channel_pbdma_faulted_reset_f() |
			ccsr_channel_eng_faulted_reset_f());
	}
}

u32 gv11b_channel_count(struct gk20a *g)
{
	(void)g;
	return ccsr_channel__size_1_v();
}

void gv11b_channel_read_state(struct gk20a *g, u32 runlist_id, u32 chid,
		struct nvgpu_channel_hw_state *state)
{
	u32 reg = nvgpu_readl(g, ccsr_channel_r(chid));

	gk20a_channel_read_state(g, runlist_id, chid, state);

	state->eng_faulted = ccsr_channel_eng_faulted_v(reg) ==
		ccsr_channel_eng_faulted_true_v();
}

void gv11b_channel_reset_faulted(struct gk20a *g, struct nvgpu_channel *ch,
		bool eng, bool pbdma)
{
	u32 reg = nvgpu_readl(g, ccsr_channel_r(ch->chid));

	if (eng) {
		reg |= ccsr_channel_eng_faulted_reset_f();
	}
	if (pbdma) {
		reg |= ccsr_channel_pbdma_faulted_reset_f();
	}

	nvgpu_writel(g, ccsr_channel_r(ch->chid), reg);
}
