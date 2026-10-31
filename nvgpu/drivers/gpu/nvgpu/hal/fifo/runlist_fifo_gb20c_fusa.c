// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/runlist.h>
#ifdef CONFIG_NVGPU_GSP_SCHEDULER_VM
#include <nvgpu/gsp_vm_scheduler.h>
#endif

#include <nvgpu/hw/gb20c/hw_runlist_gb20c.h>

#include "hal/fifo/fifo_utils_ga10b.h"
#include "runlist_fifo_gb20c.h"

u32 gb20c_virtual_channel_cfg_off(u32 gfid)
{
	return runlist_virtual_channel_cfg_r(gfid);
}

int gb20c_runlist_check_pending(struct gk20a *g, struct nvgpu_runlist *runlist)
{
	int ret = 1;

	if ((nvgpu_runlist_readl(g, runlist, runlist_submit_info_r()) &
			runlist_submit_info_pending_true_f()) == 0U) {
		ret = 0;
	}

	return ret;
}

void gb20c_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid)
{
#ifdef CONFIG_NVGPU_GSP_SCHEDULER_VM
	if (nvgpu_is_enabled(g, (u32)NVGPU_GSP_SCHEDULER_VM)) {
			int err = 0;
			u32 runlistId = runlist_id;
			u32 runlistIovaLo = u64_lo32(runlist_iova);
			u32 runlistIovaHi = u64_hi32(runlist_iova);
			u32 runlistEntries = count;
			u32 runlistAperture = aperture;

			rl_dbg(g, "Submitting runlist[%d], mem=0x%16llx", runlist_id,
					runlist_iova);

			err = nvgpu_gsp_vm_sched_runlist_update(g,
					runlistId,
					gfid,
					runlistIovaLo,
					runlistIovaHi,
					runlistEntries,
					runlistAperture);
			if (err != 0) {
			nvgpu_err(g, "nvgpu_gsp_vm_sched_runlist_update failed with: %d",
					err);
			}
	} else {
#endif /* CONFIG_NVGPU_GSP_SCHEDULER_VM */
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
		runlist_iova);

	/* TODO offset in runlist support */
	nvgpu_runlist_writel(g, runlist, runlist_submit_r(),
			runlist_submit_offset_f(0U) |
			runlist_submit_length_f(count));
#ifdef CONFIG_NVGPU_GSP_SCHEDULER_VM
	}
#endif /* CONFIG_NVGPU_GSP_SCHEDULER_VM */
}

u32 gb20c_sched_disable_reg_off(void)
{
	return runlist_sched_disable_r();
}
