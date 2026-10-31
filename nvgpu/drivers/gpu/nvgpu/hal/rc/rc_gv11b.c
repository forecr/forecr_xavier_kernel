// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/soc.h>
#include <nvgpu/debug.h>
#include <nvgpu/mm.h>
#include <nvgpu/log2.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/fifo.h>
#include <nvgpu/engines.h>
#include <nvgpu/rc.h>
#include <nvgpu/runlist.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/swprofile.h>
#include <nvgpu/fifo/swprofile.h>
#include <nvgpu/power_features/power_features.h>
#include <nvgpu/gr/fecs_trace.h>
#include <nvgpu/preempt.h>
#ifdef CONFIG_NVGPU_LS_PMU
#include <nvgpu/pmu/mutex.h>
#endif
#include <nvgpu/nvgpu_init.h>

#include "rc_gv11b.h"

#include <nvgpu/hw/gv11b/hw_fifo_gv11b.h>

static void gv11b_fifo_locked_abort_runlist_active_tsgs(struct gk20a *g,
			unsigned int rc_type,
			u32 runlists_mask)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_tsg *tsg = NULL;
	unsigned long tsgid;
	struct nvgpu_runlist *runlist = NULL;
#ifdef CONFIG_NVGPU_LS_PMU
	u32 token = PMU_INVALID_MUTEX_OWNER_ID;
	int mutex_ret = 0;
#endif
	int err;
	u32 i;

	nvgpu_err(g, "abort active tsgs of runlists set in "
			"runlists_mask: 0x%08x", runlists_mask);
#ifdef CONFIG_NVGPU_LS_PMU
	/* runlist_lock  are locked by teardown */
	mutex_ret = nvgpu_pmu_lock_acquire(g, g->pmu,
			PMU_MUTEX_ID_FIFO, &token);
#endif
	for (i = 0U; i < f->num_runlists; i++) {
		runlist = &f->active_runlists[i];

		if ((runlists_mask & BIT32(runlist->id)) == 0U) {
			continue;
		}
		nvgpu_log(g, gpu_dbg_info, "abort runlist id %d",
				runlist->id);

		for_each_set_bit(tsgid, runlist->domain->active_tsgs, f->num_channels) {
			tsg = &g->fifo.tsg[tsgid];

			if (!tsg->abortable) {
				nvgpu_log(g, gpu_dbg_info,
					  "tsg %lu is not abortable, skipping",
					  tsgid);
				continue;
			}
			nvgpu_log(g, gpu_dbg_info, "abort tsg id %lu", tsgid);

			nvgpu_tsg_disable(tsg);

			nvgpu_tsg_reset_faulted_eng_pbdma(g, tsg);

#ifdef CONFIG_NVGPU_FECS_TRACE
			nvgpu_gr_fecs_trace_add_tsg_reset(g, tsg);
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
			if (!g->fifo.deferred_reset_pending) {
#endif
				if (rc_type == RC_TYPE_MMU_FAULT) {
					/* gpu server code does not reach here, use gfid 0 */
					nvgpu_tsg_set_ctx_mmu_error(g, 0U, tsg->tsgid);
					/*
					 * Mark error (returned verbose flag is
					 * ignored since it is not needed here)
					 */
					(void) nvgpu_tsg_mark_error(g, tsg);
				}
#ifdef CONFIG_NVGPU_DEBUGGER
			}
#endif

			/*
			 * remove all entries from this runlist; don't wait for
			 * the update to finish on hw.
			 */
			err = nvgpu_runlist_update_locked(g,
				runlist, runlist->domain, NULL, false, false);
			if (err != 0) {
				nvgpu_err(g, "runlist id %d is not cleaned up", runlist->id);
			}
#if defined(CONFIG_NVS_PRESENT)
			/*
			 * This path(CONFIG_KMD_SCHEDULING_WORKER_THREAD) contains the CPU based
			 * Manual mode scheduler. With GSP enabled, this will be no longer required
			 * and can be disabled.
			 */
#if defined(CONFIG_KMD_SCHEDULING_WORKER_THREAD)
			if (nvgpu_is_enabled(g, NVGPU_SUPPORT_KMD_SCHEDULING_WORKER_THREAD)) {
				/* Special case. Submit the recovery runlist now */
				err = g->nvs_worker_submit(g, runlist, runlist->domain, false);
				if (err == 1) {
					err = 0;
				} else if (err != 0) {
					nvgpu_err(g, "runlist id %d is not cleaned up", runlist->id);
				}
			}
#endif /*CONFIG_KMD_SCHEDULING_WORKER_THREAD*/
			/*
			 * The else path is for some traditional platforms that doesn't itself support
			 * NVS. They take the traditional submit path used before NVS.
			 */
#else
			err = nvgpu_rl_domain_sync_submit(g, runlist, runlist->domain, false);
			if (err != 0) {
				nvgpu_err(g, "runlist id %d is not cleaned up", runlist->id);
			}
#endif
			nvgpu_tsg_abort(g, tsg, false);

			nvgpu_log(g, gpu_dbg_info, "aborted tsg id %lu", tsgid);
		}
	}
#ifdef CONFIG_NVGPU_LS_PMU
	if (mutex_ret == 0) {
		err = nvgpu_pmu_lock_release(g, g->pmu, PMU_MUTEX_ID_FIFO,
				&token);
		if (err != 0) {
			nvgpu_err(g, "PMU_MUTEX_ID_FIFO not released err=%d",
					err);
		}
	}
#endif
}

void gv11b_fifo_rc_cleanup_and_reenable_ctxsw(struct gk20a *g, u32 gfid,
	u32 runlists_mask, u32 tsgid, bool deferred_reset_pending,
	unsigned int rc_type)
{
	struct nvgpu_tsg *tsg = nvgpu_tsg_check_and_get_from_id(g, tsgid);
	struct nvgpu_swprofiler *prof = &g->fifo.recovery_profiler;

	(void)gfid;

	if (tsg != NULL) {
		if (rc_type == RC_TYPE_MMU_FAULT) {
			if (!nvgpu_swprofile_is_enabled(prof)) {
				gk20a_debug_dump(g);
			}
			rec_dbg(g, "Clearing PBDMA_FAULTED, ENG_FAULTED in CCSR register");
			nvgpu_tsg_reset_faulted_eng_pbdma(g, tsg);
		}

		rec_dbg(g, "Disabling TSG");
		nvgpu_tsg_disable(tsg);

		nvgpu_swprofile_snapshot(prof, PROF_RECOVERY_DISABLE_TSG);

#ifdef CONFIG_NVGPU_FECS_TRACE
		nvgpu_gr_fecs_trace_add_tsg_reset(g, tsg);
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
		if (!deferred_reset_pending) {
#endif
			nvgpu_tsg_wakeup_wqs(g, tsg);
			nvgpu_tsg_abort(g, tsg, false);
#ifdef CONFIG_NVGPU_DEBUGGER
		}
#endif
	} else {
		gv11b_fifo_locked_abort_runlist_active_tsgs(g, rc_type,
			runlists_mask);
	}

	rec_dbg(g, "Re-enabling runlists");
	nvgpu_runlist_set_state(g, runlists_mask, RUNLIST_ENABLED);

	nvgpu_swprofile_snapshot(prof, PROF_RECOVERY_ENABLE_RL);

}

void gv11b_fifo_recover(struct gk20a *g, u32 gfid, u32 runlist_id,
			u32 id, unsigned int id_type, unsigned int rc_type,
			bool should_defer_reset)
{
	u32 tsgid = (id_type == ID_TYPE_TSG) ? id : INVAL_ID;
	u32 runlists_mask, i;
	unsigned long bit;
	unsigned long bitmask;
	struct nvgpu_runlist *runlist = NULL;
	u32 engine_id;
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_swprofiler *prof = &f->recovery_profiler;
#ifdef CONFIG_NVGPU_DEBUGGER
	bool deferred_reset_pending = false;
#endif
	const struct nvgpu_device *dev;

	(void)gfid;

	rec_dbg(g, "Recovery starting");
	rec_dbg(g, "  ID      = %u", id);
	rec_dbg(g, "  id_type = %s", nvgpu_id_type_to_str(id_type));
	rec_dbg(g, "  rc_type = %s", nvgpu_rc_type_to_str(rc_type));
	rec_dbg(g, "  Runlist = %u", runlist_id);

	/*
	 * Recovery path accesses many GR registers.
	 * Any access to GR registers with CG/PG enabled
	 * in recovery path will cause errors like pri timeout
	 * idle snap etc. So disable CG/PG before we start
	 * the recovery process to avoid such errors.
	 */
#ifdef CONFIG_NVGPU_NON_FUSA
	rec_dbg(g, "Disabling CG/PG now");
	if (nvgpu_cg_pg_disable(g) != 0) {
		nvgpu_warn(g, "fail to disable power mgmt");
	}
#endif

	nvgpu_swprofile_begin_sample(prof);

	rec_dbg(g, "Acquiring engines_reset_mutex");
	nvgpu_mutex_acquire(&f->engines_reset_mutex);

	/* acquire runlist_lock for num_runlists */
	rec_dbg(g, "Acquiring runlist_lock for active runlists");
	nvgpu_runlist_lock_active_runlists(g);

	nvgpu_swprofile_snapshot(prof, PROF_RECOVERY_ACQ_ACTIVE_RL);

	g->ops.fifo.intr_set_recover_mask(g);

	/* Set unserviceable flag right at start of recovery to reduce
	 * the window of race between job submit and recovery on same
	 * TSG.
	 * The unserviceable flag is checked during job submit and
	 * prevent new jobs from being submitted to TSG which is headed
	 * for teardown.
	 */
	if (tsgid != INVAL_ID) {
		/* Set error notifier before letting userspace
		 * know about faulty channel
		 * The unserviceable flag is moved early to
		 * disallow submits on the broken channel. If
		 * userspace checks the notifier code when a
		 * submit fails, we need it set to convey to
		 * userspace that channel is no longer usable.
		 */
		if (rc_type == RC_TYPE_MMU_FAULT) {
			/*
			 * If a debugger is attached and debugging is enabled,
			 * then do not set error notifier as it will cause the
			 * application to teardown the channels and debugger will
			 * not be able to collect any data.
			 */
			if (!should_defer_reset) {
				nvgpu_tsg_set_ctx_mmu_error(g, gfid, tsgid);
			}
		}
		nvgpu_tsg_set_unserviceable(g, gfid, tsgid);
	}

	/* get runlists mask */
	if (runlist_id == INVAL_ID) {
		runlists_mask = 0U;
		for (i = 0U; i < f->num_runlists; i++) {
			runlist = &f->active_runlists[i];
			runlists_mask |= BIT32(runlist->id);
		}
	} else {
		runlists_mask = BIT32(runlist_id);
	}

	rec_dbg(g, "Runlist Bitmask: 0x%x", runlists_mask);

	nvgpu_swprofile_snapshot(prof, PROF_RECOVERY_GET_RL_MASK);

	/*
	 * release runlist lock for the runlists that are not
	 * being recovered
	 */
	nvgpu_runlist_unlock_runlists(g, ~runlists_mask);

	/* Disable runlist scheduler */
	rec_dbg(g, "Disabling RL scheduler now");
	nvgpu_runlist_set_state(g, runlists_mask, RUNLIST_DISABLED);

	nvgpu_swprofile_snapshot(prof, PROF_RECOVERY_DISABLE_RL);

	/*
	 * Even though TSG preempt timed out, the RC sequence would by design
	 * require s/w to issue another preempt.
	 * If recovery includes an ENGINE_RESET, to not have race conditions,
	 * use RUNLIST_PREEMPT to kick all work off, and cancel any context
	 * load which may be pending. This is also needed to make sure
	 * that all PBDMAs serving the engine are not loaded when engine is
	 * reset.
	 */
	rec_dbg(g, "Preempting runlists for RC");
	nvgpu_fifo_preempt_runlists_for_rc(g, runlists_mask);

	nvgpu_swprofile_snapshot(prof, PROF_RECOVERY_PREEMPT_RL);

	/*
	 * For each PBDMA which serves the runlist, poll to verify the TSG is no
	 * longer on the PBDMA and the engine phase of the preempt has started.
	 */
	rec_dbg(g, "Polling for TSG to be off PBDMA");
	if (tsgid != INVAL_ID && runlist_id != INVAL_ID) {
		if (nvgpu_preempt_poll_tsg_on_pbdma(g, runlist_id, tsgid) != 0) {
			nvgpu_err(g, "TSG preemption on PBDMA failed; "
					"PBDMA seems stuck; cannot recover stuck PBDMA.");
			/* Trigger Quiesce as recovery failed on hung PBDMA. */
			nvgpu_sw_quiesce(g);
			return;
		}
	}
	rec_dbg(g, "  Done!");

	nvgpu_swprofile_snapshot(prof, PROF_RECOVERY_POLL_TSG_ON_PBDMA);

#ifdef CONFIG_NVGPU_DEBUGGER
	nvgpu_mutex_acquire(&f->deferred_reset_mutex);
	g->fifo.deferred_reset_pending = false;
	nvgpu_mutex_release(&f->deferred_reset_mutex);
#endif

	rec_dbg(g, "Resetting relevant engines");
	/* check if engine reset should be deferred */
	for (i = 0U; i < f->num_runlists; i++) {
		runlist = &f->active_runlists[i];

		if (((runlists_mask & BIT32(runlist->id)) == 0U) ||
		    (runlist->reset_eng_bitmask == 0U)) {
			continue;
		}

		bitmask = runlist->reset_eng_bitmask;
		rec_dbg(g, "  Engine bitmask for RL %u: 0x%lx",
		       runlist->id, bitmask);

		for_each_set_bit(bit, &bitmask, f->max_engines) {

			engine_id = U32(bit);

			dev = nvgpu_engine_get_active_eng_info(g, engine_id);
			if (dev != NULL) {
				if (nvgpu_device_is_multimedia(g, dev)) {
					nvgpu_multimedia_debug_dump(g, dev);
				}
			} else {
				nvgpu_warn(g, "Multimedia debug dump skipped");
			}

			rec_dbg(g, "  > Restting engine: ID=%u", engine_id);

#ifdef CONFIG_NVGPU_DEBUGGER
			if ((tsgid != INVAL_ID) && (rc_type == RC_TYPE_MMU_FAULT) &&
					should_defer_reset) {
				rec_dbg(g, "    (deferred)");

				f->deferred_fault_engines |= BIT64(engine_id);

				/* handled during channel free */
				nvgpu_mutex_acquire(&f->deferred_reset_mutex);
				f->deferred_reset_pending = true;
				nvgpu_mutex_release(&f->deferred_reset_mutex);

				deferred_reset_pending = true;

				nvgpu_log(g, gpu_dbg_intr | gpu_dbg_gpu_dbg,
					"sm debugger attached, deferring "
					"channel recovery to channel free");
			} else {
#endif
#ifdef CONFIG_NVGPU_ENGINE_RESET
				nvgpu_engine_reset(g, engine_id);
				rec_dbg(g, "    Done!");
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
			}
#endif
		}
	}

	nvgpu_swprofile_snapshot(prof, PROF_RECOVERY_ENGINES_RESET);

	g->ops.fifo.rc_cleanup_and_reenable_ctxsw(g, gfid, runlists_mask,
			tsgid, deferred_reset_pending, rc_type);

#ifdef CONFIG_NVGPU_NON_FUSA
	rec_dbg(g, "Re-enabling CG/PG");
	if (nvgpu_cg_pg_enable(g) != 0) {
		nvgpu_warn(g, "fail to enable power mgmt");
	}
#endif

	g->ops.fifo.intr_unset_recover_mask(g);

	/* release runlist_lock for the recovered runlists */
	nvgpu_runlist_unlock_runlists(g, runlists_mask);

	rec_dbg(g, "Releasing engines reset mutex");
	nvgpu_mutex_release(&f->engines_reset_mutex);

	nvgpu_swprofile_snapshot(prof, PROF_RECOVERY_DONE);
}
