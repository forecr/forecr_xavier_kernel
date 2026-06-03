/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2013-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_DEBUGGER_H
#define NVGPU_DEBUGGER_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/cond.h>
#include <nvgpu/lock.h>
#include <nvgpu/list.h>

struct gk20a;
struct nvgpu_channel;
struct dbg_session_gk20a;
struct nvgpu_profiler_object;

struct nvgpu_channel *
nvgpu_dbg_gpu_get_session_channel(struct dbg_session_gk20a *dbg_s);

struct dbg_gpu_session_events {
	struct nvgpu_cond wait_queue;
	bool events_enabled;
	int num_pending_events;
};

struct dbg_session_gk20a {
	/* dbg session id used for trace/prints */
	int id;

	/* profiler session, if any */
	bool is_profiler;

	/* power enabled or disabled */
	bool is_pg_disabled;

	/* timeouts enabled or disabled */
	bool is_timeout_disabled;

	struct gk20a              *g;

	/* list of bound channels, if any */
	struct nvgpu_list_node ch_list;
	struct nvgpu_mutex ch_list_lock;

	/* event support */
	struct dbg_gpu_session_events dbg_events;

	struct nvgpu_mutex ioctl_lock;

	/*
	 * Dummy profiler object for debug session to synchronize PMA
	 * reservation and HWPM system reset with new context/device
	 * profilers.
	 */
	struct nvgpu_profiler_object *prof;

	/** GPU instance Id */
	u32 gpu_instance_id;

	/*
	 * Issue suspend_contexts() once, do nothing if it is already
	 * suspended. Issue resume_contexts() once, do nothing if it
	 * is already resumed.
	 */
	bool all_ctx_suspended;
};

struct dbg_session_data {
	struct dbg_session_gk20a *dbg_s;
	struct nvgpu_list_node dbg_s_entry;
};

static inline struct dbg_session_data *
dbg_session_data_from_dbg_s_entry(struct nvgpu_list_node *node)
{
	return (struct dbg_session_data *)
	     ((uintptr_t)node - offsetof(struct dbg_session_data, dbg_s_entry));
};

struct dbg_session_channel_data {
	int channel_fd;
	u32 chid;
	struct nvgpu_list_node ch_entry;
	struct dbg_session_data *session_data;
	int (*unbind_single_channel)(struct dbg_session_gk20a *dbg_s,
			struct dbg_session_channel_data *ch_data);
};

static inline struct dbg_session_channel_data *
dbg_session_channel_data_from_ch_entry(struct nvgpu_list_node *node)
{
	return (struct dbg_session_channel_data *)
	((uintptr_t)node - offsetof(struct dbg_session_channel_data, ch_entry));
};

/* used by the interrupt handler to post events */
void nvgpu_dbg_gpu_post_events(struct gk20a *g, u32 gfid, u32 tsgid);

int nvgpu_dbg_set_powergate(struct dbg_session_gk20a *dbg_s, bool disable_powergate);

void nvgpu_dbg_session_post_event(struct dbg_session_gk20a *dbg_s);
u32 nvgpu_set_powergate_locked(struct dbg_session_gk20a *dbg_s,
				bool mode);

int nvgpu_dbg_gpu_set_sm_dbgr_ctrl_single_step_mode(
		struct dbg_session_gk20a *dbg_s, u32 sm_id, bool enable);


#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_DEBUGGER_H */
