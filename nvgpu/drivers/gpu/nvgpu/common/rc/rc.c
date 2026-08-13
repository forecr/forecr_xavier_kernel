// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/fifo.h>
#ifdef CONFIG_NVGPU_RECOVERY
#include <nvgpu/engines.h>
#include <nvgpu/debug.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/gr_instances.h>
#endif
#include <nvgpu/channel.h>
#include <nvgpu/engine_status.h>
#include <nvgpu/tsg.h>
#include <nvgpu/error_notifier.h>
#include <nvgpu/pbdma_status.h>
#include <nvgpu/rc.h>
#include <nvgpu/runlist.h>
#include <nvgpu/nvgpu_init.h>

void nvgpu_rc_fifo_recover(struct gk20a *g, u32 gfid,
			u32 runlist_id, u32 hw_id, bool id_is_tsg,
			bool id_is_known, bool debug_dump, u32 rc_type)
{
#ifdef CONFIG_NVGPU_RECOVERY
	unsigned int id_type;

	if (debug_dump) {
		gk20a_debug_dump(g);
	}

	if (g->ops.ltc.flush != NULL) {
		g->ops.ltc.flush(g);
	}

	if (id_is_known) {
		id_type = id_is_tsg ? ID_TYPE_TSG : ID_TYPE_CHANNEL;
	} else {
		id_type = ID_TYPE_UNKNOWN;
	}

	g->ops.fifo.recover(g, gfid, runlist_id, hw_id, id_type,
				 rc_type, false);
#else
	WARN_ON(!g->sw_quiesce_pending);
	(void)gfid;
	(void)runlist_id;
	(void)hw_id;
	(void)id_is_tsg;
	(void)id_is_known;
	(void)debug_dump;
	(void)rc_type;
#endif
}

void nvgpu_rc_ctxsw_timeout(struct gk20a *g, u32 eng_bitmask,
				u32 gfid, u32 tsgid, bool debug_dump)
{
#ifdef CONFIG_NVGPU_RECOVERY
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_runlist *runlist = NULL;
	u32 runlist_id = INVAL_ID;
	u32 i;
#endif

	nvgpu_tsg_set_error_notifier(g, gfid, tsgid,
		NVGPU_ERR_NOTIFIER_FIFO_ERROR_IDLE_TIMEOUT);

#ifdef CONFIG_NVGPU_RECOVERY

	/*
	 * Cancel all channels' wdt since ctxsw timeout causes the runlist to
	 * stuck and might falsely trigger multiple watchdogs at a time. We
	 * won't detect proper wdt timeouts that would have happened, but if
	 * they're stuck, they will trigger the wdt soon enough again.
	 */
	nvgpu_channel_restart_all_wdts(g);

	for (i = 0U; i < f->num_runlists; i++) {
		runlist = &f->active_runlists[i];

		if (runlist->eng_bitmask & eng_bitmask) {
			runlist_id = runlist->id;
			break;
		}
	}
	nvgpu_assert(runlist_id != INVAL_ID);

	nvgpu_rc_fifo_recover(g, gfid, runlist_id, tsgid, true, true, debug_dump,
			RC_TYPE_CTXSW_TIMEOUT);
#else
	WARN_ON(!g->sw_quiesce_pending);
	(void)eng_bitmask;
	(void)debug_dump;
	(void)gfid;
#endif
}

int nvgpu_rc_pbdma_fault(struct gk20a *g, u32 pbdma_id, u32 error_notifier,
			struct nvgpu_pbdma_status_info *pbdma_status)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_runlist *runlist;
	u32 runlist_id = INVAL_ID;
	u32 id_type = PBDMA_STATUS_ID_TYPE_INVALID;
	int err = 0;
	u32 id;
	u32 gfid;
	u32 i;

	if (error_notifier >= NVGPU_ERR_NOTIFIER_INVAL) {
		nvgpu_err(g, "Invalid error notifier %u", error_notifier);
		err = -EINVAL;
		nvgpu_sw_quiesce(g);
		goto out;
	}

	nvgpu_log(g, gpu_dbg_info, "pbdma id %d error notifier %d",
			pbdma_id, error_notifier);

	if (nvgpu_pbdma_status_is_chsw_valid(pbdma_status) ||
			nvgpu_pbdma_status_is_chsw_save(pbdma_status)) {
		id = pbdma_status->id;
		id_type = pbdma_status->id_type;
		gfid = pbdma_status->gfid;
	} else if (nvgpu_pbdma_status_is_chsw_load(pbdma_status) ||
			nvgpu_pbdma_status_is_chsw_switch(pbdma_status)) {
		id = pbdma_status->next_id;
		id_type = pbdma_status->next_id_type;
		gfid = pbdma_status->next_gfid;
	} else if (nvgpu_pbdma_status_ch_not_loaded(pbdma_status)) {
		/* Nothing to do here */
		nvgpu_log_info(g, "no channel loaded on pbdma.");
		goto out;
	} else {
		nvgpu_err(g, "pbdma status not valid");
		err = -EINVAL;
		nvgpu_sw_quiesce(g);
		goto out;
	}

	for (i = 0U; i < f->num_runlists; i++) {
		runlist = &f->active_runlists[i];

		if (runlist->pbdma_bitmask & BIT32(pbdma_id)) {
			runlist_id = runlist->id;
			break;
		}
	}
	nvgpu_assert(runlist_id != INVAL_ID);

	switch (id_type) {
	case PBDMA_STATUS_ID_TYPE_TSGID:
	{
		u32 tsgid = id;

		nvgpu_tsg_set_error_notifier(g, gfid, tsgid, error_notifier);
		nvgpu_rc_fifo_recover(g, gfid, runlist_id, tsgid,
				true, true, true, RC_TYPE_PBDMA_FAULT);
		break;
	}
	default:
		nvgpu_err(g, "Invalid pbdma_status id_type or next_id_type");
		err = -EINVAL;
		nvgpu_sw_quiesce(g);
		break;
	}

out:
	return err;
}

void nvgpu_rc_runlist_update(struct gk20a *g, u32 runlist_id)
{
#ifdef CONFIG_NVGPU_RECOVERY
	u32 eng_bitmask = nvgpu_engine_get_runlist_busy_engines(g, runlist_id);

	if (eng_bitmask != 0U) {
		/* not used on sr-iov gpu server, so set gfid 0 */
		nvgpu_rc_fifo_recover(g, 0, runlist_id, INVAL_ID, false, false, true,
				RC_TYPE_RUNLIST_UPDATE_TIMEOUT);
	}
#else
	/*
	 * Runlist update occurs in non-mission mode, when
	 * adding/removing channel/TSGs. The pending bit
	 * is a debug only feature. As a result logging a
	 * warning is sufficient.
	 * We expect other HW safety mechanisms such as
	 * PBDMA timeout to detect issues that caused pending
	 * to not clear. It's possible bad base address could
	 * cause some MMU faults too.
	 * Worst case we rely on the application level task
	 * monitor to detect the GPU tasks are not completing
	 * on time.
	 */
	WARN_ON(!g->sw_quiesce_pending);
	(void)runlist_id;
#endif
}

void nvgpu_rc_preempt_timeout(struct gk20a *g, u32 runlist_id, u32 tsgid)
{
	/* gfid is 0 since the code not used on server */
	nvgpu_tsg_set_error_notifier(g, 0U, tsgid,
		NVGPU_ERR_NOTIFIER_FIFO_ERROR_IDLE_TIMEOUT);

#ifdef CONFIG_NVGPU_RECOVERY
	nvgpu_rc_fifo_recover(g, 0U, runlist_id, tsgid,
			true, true, true, RC_TYPE_PREEMPT_TIMEOUT);
#else
	(void)runlist_id;
	BUG_ON(!g->sw_quiesce_pending);
#endif
}

void nvgpu_rc_gr_fault(struct gk20a *g, u32 gfid, u32 tsgid)
{
#ifdef CONFIG_NVGPU_RECOVERY
	u32 cur_gr_instance_id = nvgpu_gr_get_cur_instance_id(g);
	u32 inst_id = nvgpu_gr_get_syspipe_id(g, cur_gr_instance_id);
	const struct nvgpu_device *dev = NULL;

	nvgpu_log(g, gpu_dbg_gr, "RC GR%u inst_id%u",
		cur_gr_instance_id, inst_id);

	dev = nvgpu_device_get(g, NVGPU_DEVTYPE_GRAPHICS, inst_id);
	nvgpu_assert(dev != NULL);

	nvgpu_rc_fifo_recover(g, gfid, dev->runlist_id, tsgid,
			true, true, true, RC_TYPE_GR_FAULT);
#else
	WARN_ON(!g->sw_quiesce_pending);
	(void)gfid;
	(void)tsgid;
#endif
	nvgpu_log(g, gpu_dbg_gr, "done");
}

void nvgpu_rc_ce_fault(struct gk20a *g, u32 inst_id)
{
	const struct nvgpu_device *dev = NULL;
	struct nvgpu_engine_status_info engine_status;
	u32 tsgid = NVGPU_INVALID_TSG_ID;

	dev = nvgpu_device_get(g, NVGPU_DEVTYPE_LCE, inst_id);
	nvgpu_assert(dev != NULL);

	g->ops.engine_status.read_engine_status_info(g,
			dev->engine_id, &engine_status);

	if (engine_status.ctx_id_type == ENGINE_STATUS_CTX_ID_TYPE_TSGID) {
		tsgid = engine_status.ctx_id;
	}

	if (tsgid == NVGPU_INVALID_TSG_ID) {
		nvgpu_err(g, "invalid tsgid %u", tsgid);
		/* ToDo: Trigger Quiesce? */
		return;
	}

	nvgpu_tsg_set_error_notifier(g, engine_status.gfid, tsgid, NVGPU_ERR_NOTIFIER_CE_ERROR);

#ifdef CONFIG_NVGPU_RECOVERY
	nvgpu_rc_fifo_recover(g, engine_status.gfid, dev->runlist_id, tsgid,
			true, true, true, RC_TYPE_CE_FAULT);
#else
	WARN_ON(!g->sw_quiesce_pending);
#endif
}

void nvgpu_rc_sched_error_bad_tsg(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_RECOVERY
	/* id is unknown, preempt all runlists and do recovery */
	nvgpu_rc_fifo_recover(g, 0U, INVAL_ID, INVAL_ID, false, false, false,
		RC_TYPE_SCHED_ERR);
#else
	WARN_ON(!g->sw_quiesce_pending);
#endif
}

void nvgpu_rc_tsg_and_related_engines(struct gk20a *g, struct nvgpu_tsg *tsg,
			 bool debug_dump, u32 rc_type)
{
#ifdef CONFIG_NVGPU_RECOVERY
	u32 eng_bitmask = 0U;
	int err = 0;

#ifdef CONFIG_NVGPU_DEBUGGER
	nvgpu_mutex_acquire(&g->dbg_sessions_lock);
#endif

	/* disable tsg so that it does not get scheduled again */
	nvgpu_tsg_disable(tsg);

	/*
	 * On hitting engine reset, h/w drops the ctxsw_status to INVALID in
	 * fifo_engine_status register. Also while the engine is held in reset
	 * h/w passes busy/idle straight through. fifo_engine_status registers
	 * are correct in that there is no context switch outstanding
	 * as the CTXSW is aborted when reset is asserted.
	 */
	nvgpu_log_info(g, "acquire engines_reset_mutex");
	nvgpu_mutex_acquire(&g->fifo.engines_reset_mutex);

	/*
	 * stop context switching to prevent engine assignments from
	 * changing until engine status is checked to make sure tsg
	 * being recovered is not loaded on the engines
	 */
	err = nvgpu_gr_disable_ctxsw(g);
	if (err != 0) {
		/* if failed to disable ctxsw, just abort tsg */
		nvgpu_err(g, "failed to disable ctxsw");
	} else {
		/* recover engines if tsg is loaded on the engines */
		eng_bitmask = nvgpu_engine_get_mask_on_id(g, tsg->tsgid, true);

		/*
		 * it is ok to enable ctxsw before tsg is recovered. If engines
		 * is 0, no engine recovery is needed and if it is  non zero,
		 * gk20a_fifo_recover will call get_mask_on_id again.
		 * By that time if tsg is not on the engine, engine need not
		 * be reset.
		 */
		err = nvgpu_gr_enable_ctxsw(g);
		if (err != 0) {
			nvgpu_err(g, "failed to enable ctxsw");
		}
	}
	nvgpu_log_info(g, "release engines_reset_mutex");
	nvgpu_mutex_release(&g->fifo.engines_reset_mutex);

	if (eng_bitmask != 0U) {
		nvgpu_rc_fifo_recover(g, 0U, tsg->runlist->id, tsg->tsgid, true, true,
			debug_dump, rc_type);
	} else {
		if (nvgpu_tsg_mark_error(g, tsg) && debug_dump) {
			gk20a_debug_dump(g);
		}

		nvgpu_tsg_abort(g, tsg, false);
	}

#ifdef CONFIG_NVGPU_DEBUGGER
	nvgpu_mutex_release(&g->dbg_sessions_lock);
#endif
#else
	WARN_ON(!g->sw_quiesce_pending);
	(void)tsg;
	(void)debug_dump;
	(void)rc_type;
#endif
}

#ifdef CONFIG_NVGPU_RECOVERY
static int nvgpu_rc_mmu_fault_recovery(struct gk20a *g, u32 act_eng_bitmask,
				       u32 id, unsigned int id_type,
				       unsigned int rc_type,
				       struct mmu_fault_info *mmufault)
{
	int err = 0;
	u32 pbdma_bitmask = 0U;
	u32 runlists_mask = 0U;
	u32 runlist_id = 0U;
	bool should_defer_reset = false;

	if (id >= g->fifo.num_channels && id != INVAL_ID) {
		nvgpu_err(g, "invalid id %u", id);
		err = -EINVAL;
		goto out;
	}

	if (id_type > ID_TYPE_TSG && id_type != ID_TYPE_UNKNOWN) {
		nvgpu_err(g, "invalid id type %u", id_type);
		err = -EINVAL;
		goto out;
	}

	nvgpu_err(g, "mmu fault id=%u id_type=%u act_eng_bitmask=%08x",
		id, id_type, act_eng_bitmask);

#ifdef CONFIG_NVGPU_DEBUGGER
	should_defer_reset = nvgpu_engine_should_defer_reset(g,
			mmufault->faulted_engine, mmufault->client_type, false);
#endif

	if (mmufault->faulted_pbdma != INVAL_ID) {
		pbdma_bitmask = BIT32(mmufault->faulted_pbdma);
	}
	runlists_mask = nvgpu_runlist_get_runlists_mask(g, id, id_type,
			act_eng_bitmask, pbdma_bitmask);

	runlist_id = nvgpu_safe_sub_u32(
			nvgpu_safe_cast_u64_to_u32(nvgpu_ffs(runlists_mask)), 1U);
	g->ops.fifo.recover(g, 0U, runlist_id,
		id, id_type, rc_type, should_defer_reset);

out:
	if (err != 0) {
		nvgpu_sw_quiesce(g);
	}

	return err;
}
#endif

int nvgpu_rc_mmu_fault(struct gk20a *g, u32 act_eng_bitmask,
		       u32 id, unsigned int id_type, unsigned int rc_type,
		       struct mmu_fault_info *mmufault)
{
#ifndef CONFIG_NVGPU_RECOVERY
	int err = 0;

	if (id >= g->fifo.num_channels) {
		nvgpu_err(g, "invalid id %u", id);
		err = -EINVAL;
		goto out;
	}

	if (id_type > ID_TYPE_TSG) {
		nvgpu_err(g, "invalid id type %u", id_type);
		err = -EINVAL;
		goto out;
	}

	nvgpu_err(g, "mmu fault id=%u id_type=%u act_eng_bitmask=%08x",
		id, id_type, act_eng_bitmask);

	if ((id != INVAL_ID) && (id_type == ID_TYPE_TSG)) {
		struct nvgpu_tsg *tsg = &g->fifo.tsg[id];
		/* not used by sr-iov server, use gfid 0 */
		nvgpu_tsg_set_ctx_mmu_error(g, 0U, tsg->tsgid);
		(void)nvgpu_tsg_mark_error(g, tsg);
	}

	WARN_ON(!g->sw_quiesce_pending);
	(void)rc_type;
	(void)mmufault;

out:
	if (err != 0) {
		nvgpu_sw_quiesce(g);
	}

	return err;
#else
	return nvgpu_rc_mmu_fault_recovery(g, act_eng_bitmask, id, id_type,
					   rc_type, mmufault);
#endif
}
