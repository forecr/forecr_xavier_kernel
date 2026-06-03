/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_VM_SCHEDULER_H
#define NVGPU_GSP_VM_SCHEDULER_H

#include <nvgpu/types.h>

#define NV_GSP_UNIT_VM_SCHED			0x13U

/* Disable all debug output */
#define VM_SCHED_DEBUG_FLAG_NONE		0x00000000U
/* Enable timer debug messages */
#define VM_SCHED_DEBUG_FLAG_TIMERS		0x00000001U
/* Enable event debug messages */
#define VM_SCHED_DEBUG_FLAG_EVENTS		0x00000002U
/* Enable scheduler debug messages */
#define VM_SCHED_DEBUG_FLAG_SCHEDULER	0x00000004U
/* Enable runlist debug messages */
#define VM_SCHED_DEBUG_FLAG_RUNLIST		0x00000008U
/* Enable submit debug messages with batching */
#define VM_SCHED_DEBUG_FLAG_SUBMIT		0x00000010U
/* Enable failure/error debug messages from all modules */
#define VM_SCHED_DEBUG_FLAG_FAILURE		0x00000020U
/* Enable all debug subsystems */
#define VM_SCHED_DEBUG_FLAG_ALL			0xFFFFFFFFU

/* Submit logging configuration */
#define VM_SCHED_SUBMIT_BATCH_SIZE_MIN	1U
#define VM_SCHED_SUBMIT_BATCH_SIZE_MAX	100U

/* Runlist Offset Tracking Control Values */
/* Disable offset tracking */
#define VM_SCHED_OFFSET_TRACKING_DISABLED	0x0U
/* Enable offset tracking */
#define VM_SCHED_OFFSET_TRACKING_ENABLED	0x1U

/* Max runlist events supported by vm sched */
#define VM_SCHED_MAX_GFIDS_RL_COMBO_COUNT	24U

/* Max gfid supported in vm sched */
#define VM_SCHED_MAX_GFIDS	16U

/* Max runlists supported in vm sched */
#define VM_SCHED_MAX_RUNLIST	32U

/* RL Event Required Status*/
#define NV_GSP_VM_SCHED_RL_WAIT_FOR_EVENT	0x3U


enum {
	/* Submit with synchronous wait (legacy behavior) */
	NV_GSP_VM_SCHED_RUNLIST_EVENT_INLINE_STATUS_CHECK = 0,
	/* Submit without any status checking (fire-and-forget) */
	NV_GSP_VM_SCHED_RUNLIST_EVENT_NO_STATUS_CHECK = 1,
	/* Submit without wait, check status via event */
	NV_GSP_VM_SCHED_RUNLIST_EVENT_DECOUPLED_STATUS_CHECK = 2,
	/* Submit without wait, check status via event, send ack to host */
	NV_GSP_VM_SCHED_RUNLIST_EVENT_DECOUPLED_STATUS_CHECK_ACK_HOST = 3,
	/* Submit with synchronous wait and send ack to host */
	NV_GSP_VM_SCHED_RUNLIST_EVENT_INLINE_STATUS_CHECK_ACK_HOST = 4,
	NV_GSP_VM_SCHED_RUNLIST_EVENT_MAX = 5
};

enum {
	NV_GSP_VM_SCHED_START_CMD_ID = 0,
	NV_GSP_VM_SCHED_STOP_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_UPDATE_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_UPDATE_AND_SUBMIT_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_STOP_SCHEDULE_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_DELETE_CMD_ID,
	NV_GSP_VM_SCHED_RUNLIST_DELETE_ALL_CMD_ID,
	NV_GSP_VM_SCHED_TSG_PREEMPT_CMD_ID,
	NV_GSP_VM_SCHED_DEBUG_CONTROL_CMD_ID,
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
	u32 runlistTimeslice;
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
	u32 rlPbdmaBitMask;
	u32 rlEngBitMask;
};

struct nvgpu_gsp_vm_sched_debug_control {
    u8  cmd_type;
    u32 debugFlags;
    u32 submitBatchSize;
    u8 disableLogs;
};


union nvgpu_gsp_vm_sched_cmd {
	u8 cmd_type;
	struct nvgpu_gsp_vm_sched_cmd_config config;
	struct nvgpu_gsp_vm_sched_cmd_runlist_update runlist_update;
	struct nvgpu_gsp_vm_sched_cmd_tsg_preempt tsg_preempt;
	struct nvgpu_gsp_vm_sched_debug_control log;
};

struct nvgpu_gsp_vm_sched_rl_event {
	bool in_use;
	bool runlist_event_ack;
	u32 gfid;
	u32 runlist_id;
};

struct nvgpu_gsp_vm_sched_runlist_submit_info {
	u32 iova_lo;
	u32 iova_hi;
	u32 entries;
	u32 aperture;
	bool submitted;
};

struct nvgpu_gsp_vm_sched {
	struct nvgpu_gsp *gsp;

	/* rl event info */
	struct nvgpu_gsp_vm_sched_rl_event rl_event[VM_SCHED_MAX_GFIDS_RL_COMBO_COUNT];
	/* Mutex to lock event data when in use */
	struct nvgpu_mutex event_lock;

	/* runlist submitted info */
	struct nvgpu_gsp_vm_sched_runlist_submit_info runlist_submit_info[VM_SCHED_MAX_GFIDS][VM_SCHED_MAX_RUNLIST];
};

#endif // NVGPU_GSP_VM_SCHEDULER_H
