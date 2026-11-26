/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef MULTIMEDIA_TRACE_H
#define MULTIMEDIA_TRACE_H

/* Default number of trace entries for userspace buffer */
#define MULTIMEDIA_TRACE_NUM_RECORDS		(1 << 10)
/* Polling period for fetching trace data */
#define MULTIMEDIA_TRACE_FRAME_PERIOD_NS	(1000000000ULL/60ULL)

/**
 * Types of multimedia events supported.
 */
enum nvgpu_multimedia_event_id {
	/** Event ID for session (frame processing) start */
	MULTIMEDIA_EVENT_ID_SESSION_START = 0,
	/** Event ID for session end */
	MULTIMEDIA_EVENT_ID_SESSION_END,
	/** Event ID for power state change (reset, recovery) */
	MULTIMEDIA_EVENT_ID_POWER_STATE_CHANGE,
	/** Event ID for log data */
	MULTIMEDIA_EVENT_ID_LOG_DATA,
	/** Event ID for context start */
	MULTIMEDIA_EVENT_ID_CONTEXT_START,
	/** Event ID for context end or save complete */
	MULTIMEDIA_EVENT_ID_SAVE_END,
	/** Event ID for context save request sent by host */
	MULTIMEDIA_EVENT_ID_CTXSW_REQ_BY_HOST,
	/** Event ID for context switch restore operation start */
	MULTIMEDIA_EVENT_ID_RESTORE_START,
	/** Max or invalid event ID */
	MULTIMEDIA_EVENT_ID_NUM
};

/**
 * Multimedia trace entry format of 'struct nvgpu_multimedia_trace_entry_internal'
 * introduced here should match that of 'struct nvgpu_multimedia_trace_entry' defined
 * in uapi header, since this struct is intended to be a mirror copy of the uapi
 * struct.
 */
struct nvgpu_multimedia_trace_entry_internal {
	/** Tag represents the trace event type */
	u8 tag;
	/** Virtual Machine ID of the OS */
	u8 vmid;
	/** Sequence number to detect drops */
	u16 seqno;
	/** Context ID as allocated by nvgpu multimedia */
	u32 context_id;
	/** 64-bit Process-ID as allocated by OS */
	u64 pid;
	/* 64-bit timestamp */
	u64 timestamp;
};

#ifdef CONFIG_NVGPU_MULTIMEDIA_TRACE
/*
 * Below functions are defined in OS-specific code.
 * Declare them here in common header since they are called from common code.
 */
void nvgpu_multimedia_trace_add_tsg_reset(struct gk20a *g, struct nvgpu_tsg *tsg,
					u32 multimedia_id);
int nvgpu_multimedia_trace_write_entry(struct gk20a *g, u32 multimedia_id,
		struct nvgpu_multimedia_trace_entry_internal *entry);
u8 nvgpu_multimedia_event_id_to_tag(u8 event_id);
void nvgpu_multimedia_trace_wake_up(struct gk20a *g, u32 multimedia_id);
#endif
#endif /* MULTIMEDIA_TRACE_H */
