// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/runlist.h>
#ifdef CONFIG_NVGPU_GSP_SCHEDULER_VM
#include <nvgpu/gsp_vm_scheduler.h>
#endif

#include <nvgpu/hw/gb10b/hw_runlist_gb10b.h>

#include "hal/fifo/fifo_utils_ga10b.h"
#include "runlist_fifo_gb10b.h"

u32 gb10b_runlist_count_max(struct gk20a *g)
{
	return nvgpu_get_litter_value(g, GPU_LIT_MAX_RUNLISTS_SUPPORTED);
}

u32 gb10b_virtual_channel_cfg_off(u32 gfid)
{
	return runlist_virtual_channel_cfg_r(gfid);
}

void gb10b_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
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

void gb10b_runlist_wait_for_preempt_before_reset(struct gk20a *g,
		u32 runlist_id)
{
	struct nvgpu_timeout timeout;
	struct nvgpu_runlist *runlist = g->fifo.runlists[runlist_id];
	u32 delay = POLL_DELAY_MIN_US;
	u32 reg_val = 0U;

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	do {
		reg_val = nvgpu_runlist_readl(g, runlist,
				g->ops.fifo.get_runlist_preempt_reg());

		if (!(reg_val & runlist_preempt_runlist_preempt_pending_true_f())) {
			return;
		}

		nvgpu_usleep_range(delay, nvgpu_safe_mult_u32(delay, 2U));
		delay = min_t(u32, delay << 1U, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	nvgpu_err(g, "timed out waiting for runlist %u preempt", runlist_id);
}

int gb10b_fifo_check_tsg_preempt_is_complete(struct gk20a *g,
		struct nvgpu_runlist *runlist)
{
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;
	u32 regval = 0U, runlist_preempt_reg;

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	runlist_preempt_reg = g->ops.fifo.get_runlist_preempt_reg();

	/*
	* In GB10B, the functionality of NV_RUNLIST_PREEMPT_TSG_PREEMPT_PENDING
	* bit in PREEMPT register has changed. In prior chips, this bit doesn't
	* wait to be cleared for all the TSG preempts to finish but from GB10B
	* this bit will be cleared only if there are no outstanding TSG preempts.
	*
	* As GB10B has TSC (TSG State Cache) and GAP (Gang Acquire Processor)
	* wait for this bit to be cleared to make sure that TSG is preempted from
	* TSC.
	*/
	do {
		regval = nvgpu_runlist_readl(g, runlist, runlist_preempt_reg);

		if ((regval & runlist_preempt_tsg_preempt_pending_true_f()) == 0U) {
			return 0;
		}

		nvgpu_usleep_range(delay, delay * 2U);
		delay = min_t(u32, delay << 1U, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	/*
	 * Check one more time because the last wake up from sleep might take
	 * take long time if some high priority cpu thread hogs up CPU
	 */
	regval = nvgpu_runlist_readl(g, runlist, runlist_preempt_reg);

	if ((regval & runlist_preempt_tsg_preempt_pending_true_f()) == 0U) {
		return 0;
	}

	nvgpu_err(g, "Waiting for all TSG preempts to complete timedout");
	return -EBUSY;
}
