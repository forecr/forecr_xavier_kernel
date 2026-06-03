// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/runlist.h>
#include <nvgpu/gk20a.h>

#include "runlist_fifo_tu104.h"

#include <nvgpu/hw/tu104/hw_fifo_tu104.h>
struct gk20a;

u32 tu104_runlist_count_max(struct gk20a *g)
{
	return fifo_runlist_base_lo__size_1_v();
}

void tu104_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid)
{
	u32 runlist_iova_lo, runlist_iova_hi;
	(void)gfid;

	runlist_iova_lo = u64_lo32(runlist_iova) >>
				fifo_runlist_base_lo_ptr_align_shift_v();
	runlist_iova_hi = u64_hi32(runlist_iova);

	if (count != 0U) {
		nvgpu_writel(g, fifo_runlist_base_lo_r(runlist_id),
			fifo_runlist_base_lo_ptr_lo_f(runlist_iova_lo) |
			nvgpu_aperture_mask_raw(g, aperture,
				fifo_runlist_base_lo_target_sys_mem_ncoh_f(),
				fifo_runlist_base_lo_target_sys_mem_coh_f(),
				fifo_runlist_base_lo_target_vid_mem_f()));

		nvgpu_writel(g, fifo_runlist_base_hi_r(runlist_id),
			fifo_runlist_base_hi_ptr_hi_f(runlist_iova_hi));
	}

	nvgpu_writel(g, fifo_runlist_submit_r(runlist_id),
		fifo_runlist_submit_length_f(count));
}

int tu104_runlist_check_pending(struct gk20a *g, struct nvgpu_runlist *runlist)
{
	int ret = 1;

	if ((nvgpu_readl(g, fifo_runlist_submit_info_r(runlist->id)) &
		fifo_runlist_submit_info_pending_true_f()) == 0U) {
		ret = 0;
	}

	return ret;
}
