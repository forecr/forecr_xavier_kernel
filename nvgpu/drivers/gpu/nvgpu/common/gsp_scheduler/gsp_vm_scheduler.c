// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/acr.h>
#include <nvgpu/gsp_vm_scheduler.h>
#include "common/gsp/ipc/gsp_msg.h"
#include "../acr/acr_priv.h"
#include "ipc/gsp_cmd.h"
#include "ipc/gsp_msg.h"
#include "gsp_vm_scheduler.h"

#define NVGPU_GSP_VM_SCHED_TIMESLICE_US (10000U)

static int nvgpu_gsp_vm_sched_start_scheduler(struct gk20a *g)
{
	struct nv_flcn_cmd_gsp cmd = {0};
	int err = 0;

	cmd.cmd.sched.cmd_type = NV_GSP_VM_SCHED_START_CMD_ID;

	err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_VM_SCHED,
			sizeof(union nvgpu_gsp_vm_sched_cmd));
	if (err != 0) {
		nvgpu_err(g, "vm sched start to GSP failed");
	}

	return err;
}

static void gsp_dump_runlist_event_info(struct gk20a *g)
{
	u32 gfrl_index, gfid, rl_id;
	bool ack, use;

	for (gfrl_index = 0U; gfrl_index < VM_SCHED_MAX_GFIDS_RL_COMBO_COUNT; gfrl_index++) {
		gfid = g->gsp_vm_sched->rl_event[gfrl_index].gfid;
		rl_id = g->gsp_vm_sched->rl_event[gfrl_index].runlist_id;
		ack = g->gsp_vm_sched->rl_event[gfrl_index].runlist_event_ack;
		use = g->gsp_vm_sched->rl_event[gfrl_index].in_use;
		nvgpu_gsp_dbg(g, "gfrl id: 0x%x gfid: 0x%x, rl_id: 0x%x", gfrl_index, gfid, rl_id);
		nvgpu_gsp_dbg(g, "gfrl id: 0x%x ack: 0x%x, in_use: 0x%x", gfrl_index, ack, use);
	}
}

static int gsp_vm_sched_update_event_ack_var(struct gk20a *g,
			struct nvgpu_gsp_vm_sched_msg_rl_event rl_event)
{
	u32 gfrl_index = 0U;
	u32 gfid = 0U;
	u32 rl_id = 0U;

	for (gfrl_index = 0U; gfrl_index < VM_SCHED_MAX_GFIDS_RL_COMBO_COUNT; gfrl_index++) {
		if (!g->gsp_vm_sched->rl_event[gfrl_index].in_use) {
			continue;
		}

		gfid = g->gsp_vm_sched->rl_event[gfrl_index].gfid;
		rl_id = g->gsp_vm_sched->rl_event[gfrl_index].runlist_id;
		if ((rl_id == rl_event.runlist_id) && (gfid == rl_event.gfid)) {
			g->gsp_vm_sched->rl_event[gfrl_index].runlist_event_ack = true;
			g->gsp_vm_sched->rl_event[gfrl_index].in_use = false;
			g->gsp_vm_sched->rl_event[gfrl_index].runlist_id = 0x0U;
			g->gsp_vm_sched->rl_event[gfrl_index].gfid = 0x0U;
			return 0;
		}
	}

	gsp_dump_runlist_event_info(g);
	nvgpu_err(g, " gsp vm sched event var not in sync ");
	return -EINVAL;
}

static bool *gsp_vm_sched_get_event_var(struct gk20a *g, u32 gfid, u32 rl_id)
{
	u32 gfrl_index = 0U;

	for (gfrl_index = 0U; gfrl_index < VM_SCHED_MAX_GFIDS_RL_COMBO_COUNT; gfrl_index++) {
		if (!g->gsp_vm_sched->rl_event[gfrl_index].in_use) {
			g->gsp_vm_sched->rl_event[gfrl_index].gfid = gfid;
			g->gsp_vm_sched->rl_event[gfrl_index].runlist_id = rl_id;
			g->gsp_vm_sched->rl_event[gfrl_index].in_use = true;
			g->gsp_vm_sched->rl_event[gfrl_index].runlist_event_ack = false;
			return &g->gsp_vm_sched->rl_event[gfrl_index].runlist_event_ack;
		}
	}

	gsp_dump_runlist_event_info(g);
	nvgpu_err(g, " gsp vm sched get event var failed ");
	return NULL;
}

int nvgpu_gsp_vm_sched_runlist_update(struct gk20a *g,
		u32 runlistId,
		u32 runlistGfid,
		u32 runlistIovaLo,
		u32 runlistIovaHi,
		u32 runlistEntries,
		u32 runlistAperture)
{
	struct nv_flcn_cmd_gsp cmd = {0};
	int err = 0;
	bool *runlist_event = NULL;
	struct nvgpu_gsp_vm_sched_runlist_submit_info *submitted_info;
	struct nvgpu_gsp_vm_sched_cmd_runlist_update *runlist_update
								= &cmd.cmd.sched.runlist_update;

	runlist_update->cmd_type = NV_GSP_VM_SCHED_RUNLIST_UPDATE_CMD_ID;
	runlist_update->runlistId = runlistId;
	runlist_update->runlistGfid = runlistGfid;
	runlist_update->runlistIovaLo = runlistIovaLo;
	runlist_update->runlistIovaHi = runlistIovaHi;
	runlist_update->runlistEntries = runlistEntries;
	runlist_update->runlistAperture = runlistAperture;
	runlist_update->runlistEnable = 1U;
	runlist_update->runlistEvent =
			NV_GSP_VM_SCHED_RUNLIST_EVENT_DECOUPLED_STATUS_CHECK_ACK_HOST;
	runlist_update->runlistOffset = VM_SCHED_OFFSET_TRACKING_ENABLED;

	// update the timeslice from pct
	if (g->ops.runlist.get_runlist_timeslice != NULL) {
		runlist_update->runlistTimeslice =
			g->ops.runlist.get_runlist_timeslice(g, runlistId, runlistGfid);
	} else {
		runlist_update->runlistTimeslice = NVGPU_GSP_VM_SCHED_TIMESLICE_US;
	}

	nvgpu_gsp_dbg(g, "gfid: 0x%x ::: rl: 0x%x, Hi: 0x%x, Lo: 0x%x, Ets: 0x%x timeslice: %u",
				runlistGfid, runlistId, runlistIovaHi, runlistIovaLo, runlistEntries,
				runlist_update->runlistTimeslice);

	/* Setup the rl event info before sending the cmd to gsp fw */
	if (runlist_update->runlistEvent >= NV_GSP_VM_SCHED_RL_WAIT_FOR_EVENT) {
		/* acquire the lock before modifing event status */
		nvgpu_mutex_acquire(&g->gsp_vm_sched->event_lock);
		runlist_event = gsp_vm_sched_get_event_var(g, runlistGfid, runlistId);
		if (runlist_event == NULL) {
			nvgpu_err(g, "rl event get var failed");
		}
		nvgpu_mutex_release(&g->gsp_vm_sched->event_lock);
	}

	err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_VM_SCHED,
			sizeof(union nvgpu_gsp_vm_sched_cmd));
	if (err != 0) {
		nvgpu_err(g, "vm sched update and submit to GSP failed");
	}

	if (runlist_update->runlistEvent >= NV_GSP_VM_SCHED_RL_WAIT_FOR_EVENT) {
		err = nvgpu_acr_wait_message_cond(g, U32_MAX,
				runlist_event, U8(true));
		if (err != 0) {
			nvgpu_err(g, "rl submit event receive failed");
		}
	}

	/*
	 * for a null runlist disable the runlist in the firmware, we want the firmware
	 * to submit the null runlist to hardware once which is done above and later disable
	 * it from submission.
	 */
	if (runlistEntries == 0) {
		runlist_update->runlistEnable = 0U;
		runlist_update->runlistEvent =
			NV_GSP_VM_SCHED_RUNLIST_EVENT_NO_STATUS_CHECK;

		err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_VM_SCHED,
				sizeof(union nvgpu_gsp_vm_sched_cmd));
		if (err != 0) {
			nvgpu_err(g, "vm sched update and submit to GSP failed");
			return err;
		}
	}

	if ((runlistGfid < VM_SCHED_MAX_GFIDS) && (runlistId < VM_SCHED_MAX_RUNLIST)) {
		submitted_info = &g->gsp_vm_sched->runlist_submit_info[runlistGfid][runlistId];
		submitted_info->iova_lo = runlist_update->runlistIovaLo;
		submitted_info->iova_hi = runlist_update->runlistIovaHi;
		submitted_info->entries = runlist_update->runlistEntries;
		submitted_info->aperture = runlist_update->runlistAperture;
		submitted_info->submitted = true;
	} else {
		nvgpu_err(g, "out of range parameters gfid %u runlist %u", runlistGfid, runlistId);
		err = -EINVAL;
	}

	return err;
}

int nvgpu_gsp_vm_sched_runlist_resubmit(struct gk20a *g, u32 runlistId, u32 runlistGfid)
{
	struct nvgpu_gsp_vm_sched_runlist_submit_info *resubmit_info;
	int err = 0;

	if ((runlistGfid < VM_SCHED_MAX_GFIDS) && (runlistId < VM_SCHED_MAX_RUNLIST)) {
		resubmit_info = &g->gsp_vm_sched->runlist_submit_info[runlistGfid][runlistId];
		if (resubmit_info->submitted) {
			err = nvgpu_gsp_vm_sched_runlist_update(g, runlistId, runlistGfid,
								resubmit_info->iova_lo,
								resubmit_info->iova_hi,
								resubmit_info->entries,
								resubmit_info->aperture);
		}
	} else {
		nvgpu_err(g, "out of range parameters gfid %u runlist %u", runlistGfid, runlistId);
		err = -EINVAL;
	}

	return err;
}
int nvgpu_gsp_vm_sched_tsg_preempt(struct gk20a *g,
		u32 runlistId,
		u32 runlistGfid,
		u32 tsgId,
		u32 rlPbdmaBitMask,
		u32 rlEngBitMask)
{
	struct nv_flcn_cmd_gsp cmd = {0};
	int err = 0;

	cmd.cmd.sched.tsg_preempt.cmd_type = NV_GSP_VM_SCHED_TSG_PREEMPT_CMD_ID;
	cmd.cmd.sched.tsg_preempt.runlistId = runlistId;
	cmd.cmd.sched.tsg_preempt.runlistGfid = runlistGfid;
	cmd.cmd.sched.tsg_preempt.tsgId = tsgId;
	cmd.cmd.sched.tsg_preempt.rlPbdmaBitMask = rlPbdmaBitMask;
	cmd.cmd.sched.tsg_preempt.rlEngBitMask = rlEngBitMask;

	nvgpu_gsp_dbg(g, "gfid: 0x%x ::: rl: 0x%x,tsg: 0x%x", runlistGfid, runlistId, tsgId);

	err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_VM_SCHED,
			sizeof(union nvgpu_gsp_vm_sched_cmd));
	if (err != 0) {
		nvgpu_err(g, "vm sched tsg preempt to GSP failed");
	}

	return err;
}

static int nvgpu_gsp_vm_sched_send_debug_config(struct gk20a *g)
{
	struct nv_flcn_cmd_gsp cmd = {0};
	int err = 0;

	cmd.cmd.sched.log.cmd_type = NV_GSP_VM_SCHED_DEBUG_CONTROL_CMD_ID;
	cmd.cmd.sched.log.debugFlags = VM_SCHED_DEBUG_FLAG_FAILURE;
	cmd.cmd.sched.log.disableLogs = false;
	cmd.cmd.sched.log.submitBatchSize = VM_SCHED_SUBMIT_BATCH_SIZE_MAX;

	err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_VM_SCHED,
			sizeof(union nvgpu_gsp_vm_sched_cmd));
	if (err != 0) {
		nvgpu_err(g, "vm sched tsg preempt to GSP failed");
	}

	return err;
}

void nvgpu_gsp_vm_sched_sw_deinit(struct gk20a *g)
{
	struct nvgpu_gsp_vm_sched *gsp_vm_sched = g->gsp_vm_sched;

	nvgpu_gsp_dbg(g, " ");

	if (gsp_vm_sched != NULL) {
#ifdef CONFIG_NVGPU_FALCON_DEBUG
		nvgpu_gsp_debug_buf_deinit(g);
#endif
		nvgpu_mutex_destroy(&g->gsp_vm_sched->event_lock);
		nvgpu_kfree(g, gsp_vm_sched);
		g->gsp_vm_sched = NULL;
	}
}

int nvgpu_gsp_vm_sched_sw_init(struct gk20a *g)
{
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	if (g->gsp_vm_sched == NULL) {
		/* Init struct holding the gsp sched software state */
		g->gsp_vm_sched = (struct nvgpu_gsp_vm_sched *)
						nvgpu_kzalloc(g, sizeof(struct nvgpu_gsp_vm_sched));
		if (g->gsp_vm_sched == NULL) {
			err = -ENOMEM;
			goto done;
		}

		g->gsp_vm_sched->gsp = g->gsp;
		/* init the lock for rl event */
		nvgpu_mutex_init(&g->gsp_vm_sched->event_lock);
	}

#ifdef CONFIG_NVGPU_FALCON_DEBUG
	err = nvgpu_gsp_debug_buf_init(g, NV_RISCV_DEBUG_BUFFER_QUEUE,
			NV_RISCV_DMESG_BUFFER_SIZE);
	if (err != 0) {
		nvgpu_err(g, "GSP sched debug buf init failed");
		goto done;
	}
#endif

	err = nvgpu_gsp_vm_sched_start_scheduler(g);
	if (err != 0) {
		nvgpu_err(g, "vm sched start to GSP failed");
		goto done;
	}

#ifdef CONFIG_NVGPU_FALCON_DEBUG
	err = nvgpu_gsp_vm_sched_send_debug_config(g);
	if (err != 0) {
		nvgpu_err(g, "vm sched debug config to GSP failed");
		goto done;
	}
#endif

	return err;

done:
	nvgpu_gsp_vm_sched_sw_deinit(g);
	return err;
}

int gsp_vm_sched_event_handler(struct gk20a *g, struct nv_flcn_msg_gsp *msg)
{
	int err = 0;
	struct nvgpu_gsp_vm_sched_msg_rl_event rl_event = msg->msg.rl_event;

	nvgpu_gsp_dbg(g, "msg:gfid: 0x%x, rl_id: 0x%x", rl_event.gfid, rl_event.runlist_id);

	/* acquire the lock before modifing event status */
	nvgpu_mutex_acquire(&g->gsp_vm_sched->event_lock);
	/* make the runlist event var to true */
	err = gsp_vm_sched_update_event_ack_var(g, rl_event);
	if (err != 0) {
		nvgpu_err(g, "gsp vm event handler failed");
	}
	nvgpu_mutex_release(&g->gsp_vm_sched->event_lock);

	return err;
}
