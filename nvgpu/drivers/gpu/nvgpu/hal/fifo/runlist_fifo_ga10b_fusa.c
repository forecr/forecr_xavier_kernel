// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/soc.h>
#include <nvgpu/fifo.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/mc.h>
#include <nvgpu/engines.h>
#include <nvgpu/runlist.h>

#include <nvgpu/hw/ga10b/hw_runlist_ga10b.h>

#include "fifo_utils_ga10b.h"
#include "runlist_fifo_ga10b.h"

u32 ga10b_runlist_count_max(struct gk20a *g)
{
	(void)g;
	/* TODO Needs to be read from litter values */
	return 4U;
}

u32 ga10b_runlist_length_max(struct gk20a *g)
{
	(void)g;
	return runlist_submit_length_max_v();
}

void ga10b_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid)
{
	struct nvgpu_runlist *runlist = g->fifo.runlists[runlist_id];
	u32 runlist_iova_lo, runlist_iova_hi;
	(void)gfid;

	runlist_iova_lo = u64_lo32(runlist_iova) >>
			runlist_submit_base_lo_ptr_align_shift_v();
	runlist_iova_hi = u64_hi32(runlist_iova);

	if (count != 0U) {
		nvgpu_runlist_writel(g, runlist, runlist_submit_base_lo_r(),
			runlist_submit_base_lo_ptr_lo_f(runlist_iova_lo) |
			nvgpu_aperture_mask_raw(g, aperture,
			runlist_submit_base_lo_target_sys_mem_noncoherent_f(),
			runlist_submit_base_lo_target_sys_mem_coherent_f(),
			runlist_submit_base_lo_target_vid_mem_f()));

		nvgpu_runlist_writel(g, runlist, runlist_submit_base_hi_r(),
			runlist_submit_base_hi_ptr_hi_f(runlist_iova_hi));
	}

	rl_dbg(g, "Submitting runlist[%d], mem=0x%16llx", runlist_id,
		(u64)runlist_iova);

	/* TODO offset in runlist support */
	nvgpu_runlist_writel(g, runlist, runlist_submit_r(),
			runlist_submit_offset_f(0U) |
			runlist_submit_length_f(count));
}

int ga10b_runlist_check_pending(struct gk20a *g, struct nvgpu_runlist *runlist)
{
	int ret = 1;

	if ((nvgpu_runlist_readl(g, runlist, runlist_submit_info_r()) &
			runlist_submit_info_pending_true_f()) == 0U) {
		ret = 0;
	}

	return ret;
}

u32 ga10b_get_runlist_aperture(struct gk20a *g, struct nvgpu_mem *mem)
{
	return nvgpu_aperture_mask(g, mem,
			runlist_submit_base_lo_target_sys_mem_noncoherent_f(),
			runlist_submit_base_lo_target_sys_mem_coherent_f(),
			runlist_submit_base_lo_target_vid_mem_f());
}

u32 ga10b_sched_disable_reg_off(void)
{
	return runlist_sched_disable_r();
}

void ga10b_runlist_write_state(struct gk20a *g, u32 runlists_mask,
						u32 runlist_state)
{
	u32 reg_val;
	u32 runlist_id = 0U;
	struct nvgpu_runlist *runlist = NULL;

	if (runlist_state == RUNLIST_DISABLED) {
		reg_val = runlist_sched_disable_runlist_disabled_v();
	} else {
		reg_val = runlist_sched_disable_runlist_enabled_v();
	}

	while (runlists_mask != 0U && (runlist_id < g->fifo.max_runlists)) {
		if ((runlists_mask & BIT32(runlist_id)) != 0U) {
			runlist = g->fifo.runlists[runlist_id];
			/*
			 * Its possible that some of the engines might be
			 * FSed, in which case the entry in fifo.runlists will
			 * be NULL, hence perform a NULL check first.
			 */
			if (runlist != NULL) {
				nvgpu_runlist_writel(g, runlist,
					g->ops.runlist.get_sched_disable_reg_off(), reg_val);
			}
		}
		runlists_mask &= ~BIT32(runlist_id);
		runlist_id++;
	}
}
