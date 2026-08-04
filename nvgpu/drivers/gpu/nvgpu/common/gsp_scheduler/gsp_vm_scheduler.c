// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/acr.h>
#include <nvgpu/gsp_vm_scheduler.h>
#include "../acr/acr_priv.h"
#include "ipc/gsp_cmd.h"
#include "gsp_vm_scheduler.h"

#define NVGPU_GSP_VM_SCHED_TIMESLICE_US (10000U)

static int nvgpu_gsp_vm_sched_send_scheduler_config(
		struct gk20a *g, u32 timeslice)
{
	struct nv_flcn_cmd_gsp cmd = {0};
	int err = 0;

	cmd.cmd.sched.config.cmd_type = NV_GSP_VM_SCHED_CONFIG_CMD_ID;
	cmd.cmd.sched.config.timeslice = timeslice;

	err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_VM_SCHED,
			sizeof(union nvgpu_gsp_vm_sched_cmd));
	if (err != 0) {
		nvgpu_err(g, "vm sched config to GSP failed");
	}

	return err;
}

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

	/*
	 * We use NV_GSP_VM_SCHED_RUNLIST_UPDATE_AND_SUBMIT_CMD_ID
	 * Instead of NV_GSP_VM_SCHED_RUNLIST_UPDATE_CMD_ID
	 * because it NV_GSP_VM_SCHED_RUNLIST_UPDATE_CMD_ID does not apply the
	 * command immediately which can lead to mmu faults if a channel from
	 * the old runlist is freed and the GSP scheduler has not applied the new
	 * runlist yet.
	 *
	 * Workaround for Bug 5272109
	 */
	cmd.cmd.sched.runlist_update.cmd_type =
					NV_GSP_VM_SCHED_RUNLIST_UPDATE_AND_SUBMIT_CMD_ID;
	cmd.cmd.sched.runlist_update.runlistEnable = 1U;
	cmd.cmd.sched.runlist_update.runlistId = runlistId;
	cmd.cmd.sched.runlist_update.runlistGfid = runlistGfid;
	cmd.cmd.sched.runlist_update.runlistIovaLo = runlistIovaLo;
	cmd.cmd.sched.runlist_update.runlistIovaHi = runlistIovaHi;
	cmd.cmd.sched.runlist_update.runlistEntries = runlistEntries;
	cmd.cmd.sched.runlist_update.runlistAperture = runlistAperture;

	err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_VM_SCHED,
			sizeof(union nvgpu_gsp_vm_sched_cmd));
	if (err != 0) {
		nvgpu_err(g, "vm sched update and submit to GSP failed");
	}

	return err;
}

int nvgpu_gsp_vm_sched_tsg_preempt(struct gk20a *g,
		u32 runlistId,
		u32 runlistGfid,
		u32 tsgId)
{
	struct nv_flcn_cmd_gsp cmd = {0};
	int err = 0;

	cmd.cmd.sched.tsg_preempt.cmd_type = NV_GSP_VM_SCHED_TSG_PREEMPT_CMD_ID;
	cmd.cmd.sched.tsg_preempt.runlistId = runlistId;
	cmd.cmd.sched.tsg_preempt.runlistGfid = runlistGfid;
	cmd.cmd.sched.tsg_preempt.tsgId = tsgId;
	err = nvgpu_acr_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_VM_SCHED,
			sizeof(union nvgpu_gsp_vm_sched_cmd));
	if (err != 0) {
		nvgpu_err(g, "vm sched tsg preempt to GSP failed");
	}

	return err;
}

int nvgpu_gsp_vm_sched_sw_init(struct gk20a *g)
{
	int err = 0;

	err = nvgpu_gsp_vm_sched_send_scheduler_config(g,
			NVGPU_GSP_VM_SCHED_TIMESLICE_US);
	if (err != 0) {
		nvgpu_err(g, "vm sched config to GSP failed");
		goto done;
	}

	err = nvgpu_gsp_vm_sched_start_scheduler(g);
	if (err != 0) {
		nvgpu_err(g, "vm sched start to GSP failed");
		goto done;
	}

done:
	return err;
}
