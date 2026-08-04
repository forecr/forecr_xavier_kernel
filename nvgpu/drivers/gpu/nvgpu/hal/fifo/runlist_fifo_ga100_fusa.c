// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/soc.h>
#include <nvgpu/fifo.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engines.h>
#include <nvgpu/runlist.h>

#include <nvgpu/hw/ga100/hw_runlist_ga100.h>

#include "fifo_utils_ga10b.h"
#include "runlist_fifo_ga100.h"

u32 ga100_runlist_count_max(struct gk20a *g)
{
	return nvgpu_get_litter_value(g, GPU_LIT_MAX_RUNLISTS_SUPPORTED);
}

void ga100_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
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

	/* TODO offset in runlist support */
	nvgpu_runlist_writel(g, runlist, runlist_submit_r(),
			runlist_submit_offset_f(0U) |
			runlist_submit_length_f(count));
}
