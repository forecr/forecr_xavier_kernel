// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/soc.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/errata.h>
#include <nvgpu/runlist.h>
#include <nvgpu/types.h>
#include <nvgpu/channel.h>
#include <nvgpu/tsg.h>
#include <nvgpu/preempt.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/cic_rm.h>
#include <nvgpu/rc.h>
#ifdef CONFIG_NVGPU_LS_PMU
#include <nvgpu/pmu/mutex.h>
#endif
#ifdef CONFIG_NVGPU_GSP_SCHEDULER_VM
#include <nvgpu/gsp_vm_scheduler.h>
#endif

u32 nvgpu_preempt_get_timeout(struct gk20a *g)
{
	return g->ctxsw_timeout_period_ms;
}

int nvgpu_fifo_preempt_tsg(struct gk20a *g, u32 runlist_id, u32 tsgid, u32 gfid)
{
	struct nvgpu_runlist *runlist;
	int ret = 0;
	u32 preempt_retry_count = 10U;
	u32 preempt_retry_timeout =
			nvgpu_preempt_get_timeout(g) / preempt_retry_count;
#ifdef CONFIG_NVGPU_LS_PMU
	u32 token = PMU_INVALID_MUTEX_OWNER_ID;
	int mutex_ret = 0;
#endif
	(void)gfid;

	nvgpu_log_fn(g, "tsgid: %d", tsgid);

	if (runlist_id == INVAL_ID) {
		return 0;
	}

	/* WAR for bug 4984159: GSP scheduler tsg preemption cuasing BAR2 MMU fault
	 * on nvrm_gpu_tests, so directly use native path. Scheduler will add back the
	 * scheduler path.
	 */

	runlist = g->fifo.runlists[runlist_id];

	do {
		nvgpu_mutex_acquire(&runlist->runlist_lock);

		if (nvgpu_is_errata_present(g, NVGPU_ERRATA_2016608)) {
			nvgpu_runlist_set_state(g, BIT32(runlist_id),
						RUNLIST_DISABLED);
		}

#ifdef CONFIG_NVGPU_LS_PMU
		mutex_ret = nvgpu_pmu_lock_acquire(g, g->pmu,
						   PMU_MUTEX_ID_FIFO, &token);
#endif
		g->ops.fifo.preempt_trigger(g, runlist_id, tsgid, ID_TYPE_TSG);

		/*
		 * Poll for preempt done. if stalling interrupts are pending
		 * while preempt is in progress we poll for stalling interrupts
		 * to finish based on return value from this function and
		 * retry preempt again.
		 * If HW is hung, on the last retry instance we try to identify
		 * the engines hung and set the runlist reset_eng_bitmask
		 * and mark preemption completion.
		 */
		ret = g->ops.fifo.is_preempt_pending(g, runlist_id, tsgid,
					ID_TYPE_TSG, preempt_retry_count > 1U);

#ifdef CONFIG_NVGPU_LS_PMU
		if (mutex_ret == 0) {
			int err = nvgpu_pmu_lock_release(g, g->pmu,
						PMU_MUTEX_ID_FIFO, &token);
			if (err != 0) {
				nvgpu_err(g, "PMU_MUTEX_ID_FIFO not released err=%d", err);
			}
		}
#endif
		if (nvgpu_is_errata_present(g, NVGPU_ERRATA_2016608)) {
			nvgpu_runlist_set_state(g, BIT32(runlist_id),
						RUNLIST_ENABLED);
		}

		nvgpu_mutex_release(&runlist->runlist_lock);

		if (ret != -EAGAIN) {
			break;
		}

		ret = nvgpu_cic_rm_wait_for_stall_interrupts(g, preempt_retry_timeout);
		if (ret != 0) {
			nvgpu_log_info(g, "wait for stall interrupts failed %d", ret);
		}
	} while (--preempt_retry_count != 0U);

	if (ret != 0) {
		if (nvgpu_platform_is_silicon(g)) {
			nvgpu_err(g, "preempt timed out for tsgid: %u, "
			"ctxsw timeout will trigger recovery if needed",
			tsgid);
		} else {
			nvgpu_rc_preempt_timeout(g, runlist_id, tsgid);
		}
	}
	return ret;
}

int nvgpu_preempt_channel(struct gk20a *g, struct nvgpu_channel *ch)
{
	int err;
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);

	if (tsg != NULL) {
		err = nvgpu_tsg_preempt(ch->g, tsg);
	} else {
		err = g->ops.fifo.preempt_channel(ch->g, ch);
	}

	return err;
}

#ifdef CONFIG_NVGPU_RECOVERY
/* called from rc */
int nvgpu_preempt_poll_tsg_on_pbdma(struct gk20a *g,
		u32 runlist_id, u32 tsgid)
{
	unsigned long runlist_served_pbdmas;
	unsigned long pbdma_id_bit;
	u32 pbdma_id;
	struct nvgpu_runlist *runlist = g->fifo.runlists[runlist_id];

	if (g->ops.fifo.preempt_poll_pbdma == NULL) {
		return 0;
	}

	runlist_served_pbdmas = runlist->pbdma_bitmask;

	for_each_set_bit(pbdma_id_bit, &runlist_served_pbdmas,
			 nvgpu_get_litter_value(g, GPU_LIT_HOST_NUM_PBDMA)) {
		pbdma_id = U32(pbdma_id_bit);
		/*
		 * If pbdma preempt fails the only option is to reset
		 * GPU. Any sort of hang indicates the entire GPU’s
		 * memory system would be blocked.
		 */
		if (g->ops.fifo.preempt_poll_pbdma(g, tsgid, pbdma_id) != 0) {
			nvgpu_err(g, "PBDMA preempt failed");
			return -EBUSY;
		}
	}
	return 0;
}
#endif

/*
 * This should be called with runlist_lock held for all the
 * runlists set in runlists_mask
 */
void nvgpu_fifo_preempt_runlists_for_rc(struct gk20a *g, u32 runlists_bitmask)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 i;
#ifdef CONFIG_NVGPU_LS_PMU
	u32 token = PMU_INVALID_MUTEX_OWNER_ID;
	int mutex_ret = 0;
#endif

	/* runlist_lock are locked by teardown and sched are disabled too */
	nvgpu_log_fn(g, "preempt runlists_bitmask:0x%08x", runlists_bitmask);
#ifdef CONFIG_NVGPU_LS_PMU
	mutex_ret = nvgpu_pmu_lock_acquire(g, g->pmu,
			PMU_MUTEX_ID_FIFO, &token);
#endif

	for (i = 0U; i < f->num_runlists; i++) {
		struct nvgpu_runlist *runlist;

		runlist = &f->active_runlists[i];

		if ((BIT32(runlist->id) & runlists_bitmask) == 0U) {
			continue;
		}
		/* issue runlist preempt */
		g->ops.fifo.preempt_trigger(g, runlist->id, INVAL_ID,
					ID_TYPE_RUNLIST);
#ifdef CONFIG_NVGPU_RECOVERY
		/*
		 * Preemption will never complete in RC due to some
		 * fatal condition. Do not poll for preemption to
		 * complete. Reset engines served by runlists.
		 */
		runlist->reset_eng_bitmask = runlist->eng_bitmask;
#endif
	}

#ifdef CONFIG_NVGPU_LS_PMU
	if (mutex_ret == 0) {
		int err = nvgpu_pmu_lock_release(g, g->pmu, PMU_MUTEX_ID_FIFO,
				&token);
		if (err != 0) {
			nvgpu_err(g, "PMU_MUTEX_ID_FIFO not released err=%d",
					err);
		}
	}
#endif
}
