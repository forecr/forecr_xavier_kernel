/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_VM_SCHEDULER_H
#define NVGPU_GSP_VM_SCHEDULER_H

#include <nvgpu/types.h>

#define NV_GSP_UNIT_VM_SCHED				0x13U

enum {
	NV_GSP_VM_SCHED_CONFIG_CMD_ID = 0,
	NV_GSP_VM_SCHED_START_CMD_ID,
	NV_GSP_VM_SCHED_STOP_CMD_ID,
	NV_GSP_VM_SCHED_DISABLE_ENGINE_RL_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_UPDATE_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_UPDATE_AND_SUBMIT_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_STOP_SCHEDULE_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_DELETE_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_DELETE_ALL_CMD_ID,
	NV_GSP_VM_SCHED_TSG_PREEMPT_CMD_ID,
};

struct nvgpu_gsp_vm_sched_cmd_runlist_update {
	u8 cmd_type;
	u8 runlistEnable;
	u8 runlistEvent;
	u32 runlistId;
	u32 runlistGfid;
	u32 runlistIovaLo;
	u32 runlistIovaHi;
	u32 runlistEntries;
	u32 runlistAperture;
	u32 runlistOffset;
};

struct nvgpu_gsp_vm_sched_cmd_config {
	u8 cmd_type;
	u32 timeslice;
};

struct nvgpu_gsp_vm_sched_cmd_tsg_preempt {
	u8 cmd_type;
	u32 runlistId;
	u32 runlistGfid;
	u32 tsgId;
};

union nvgpu_gsp_vm_sched_cmd {
	u8 cmd_type;
	struct nvgpu_gsp_vm_sched_cmd_config config;
	struct nvgpu_gsp_vm_sched_cmd_runlist_update runlist_update;
	struct nvgpu_gsp_vm_sched_cmd_tsg_preempt tsg_preempt;
};

#endif // NVGPU_GSP_VM_SCHEDULER_H
