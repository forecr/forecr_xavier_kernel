/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CHANNEL_H
#define NVGPU_CHANNEL_H

#include <nvgpu/list.h>
#include <nvgpu/lock.h>
#include <nvgpu/timers.h>
#include <nvgpu/cond.h>
#include <nvgpu/atomic.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/allocator.h>
#include <nvgpu/debug.h>

/**
 * @file
 *
 * Channel interface.
 */
struct gk20a;
struct dbg_session_gk20a;
struct nvgpu_fence_type;
struct nvgpu_swprofiler;
struct nvgpu_channel_sync;
struct nvgpu_gpfifo_userdata;
struct nvgpu_gr_ctx;
struct nvgpu_debug_context;
struct priv_cmd_queue;
struct priv_cmd_entry;
struct nvgpu_channel_wdt;
struct nvgpu_user_fence;
struct nvgpu_runlist;

/**
 * Size of task name. Should strictly be equal to TASK_COMM_LEN
 */
#define TASK_NAME_LEN		(16U)
/**
 * S/W defined invalid channel identifier.
 */
#define NVGPU_INVALID_CHANNEL_ID	(~U32(0U))

/**
 * Enable VPR support.
 */
#define NVGPU_SETUP_BIND_FLAGS_SUPPORT_VPR				BIT32(0)
/**
 * Channel must have deterministic (and low) submit latency.
 * This flag is only valid for kernel mode submit.
 */
#define NVGPU_SETUP_BIND_FLAGS_SUPPORT_DETERMINISTIC			BIT32(1)
/**
 * Enable replayable faults.
 */
#define NVGPU_SETUP_BIND_FLAGS_REPLAYABLE_FAULTS_ENABLE			BIT32(2)
/**
 * Enable usermode submit (mutually exclusive with kernel_mode submit).
 */
#define NVGPU_SETUP_BIND_FLAGS_USERMODE_SUPPORT				BIT32(3)
/**
 * Enable GPU MMIO support
 */
#define NVGPU_SETUP_BIND_FLAGS_USERMODE_GPU_MAP_RESOURCES_SUPPORT	BIT32(4)
/**
 * Insert a wait on previous job's completion fence, before gpfifo entries.
 * See also #nvgpu_fence.
 */
#define NVGPU_SUBMIT_FLAGS_FENCE_WAIT			BIT32(0)
/**
 * Insert a job completion fence update after gpfifo entries, and return the
 * new fence for others to wait on.
 */
#define NVGPU_SUBMIT_FLAGS_FENCE_GET			BIT32(1)
/**
 * Use HW GPFIFO entry format.
 */
#define NVGPU_SUBMIT_FLAGS_HW_FORMAT			BIT32(2)
/**
 * Interpret fence as a sync fence fd instead of raw syncpoint fence.
 */
#define NVGPU_SUBMIT_FLAGS_SYNC_FENCE			BIT32(3)
/**
 * Suppress WFI before fence trigger.
 */
#define NVGPU_SUBMIT_FLAGS_SUPPRESS_WFI			BIT32(4)
/**
 * Skip buffer refcounting during submit.
 */
#define NVGPU_SUBMIT_FLAGS_SKIP_BUFFER_REFCOUNTING	BIT32(5)

/**
 * The binary format of 'struct nvgpu_channel_fence' introduced here
 * should match that of 'struct nvgpu_fence' defined in uapi header, since
 * this struct is intended to be a mirror copy of the uapi struct. This is
 * not a hard requirement though because of nvgpu_get_fence_args conversion
 * function.
 *
 * See also #nvgpu_fence.
 */
struct nvgpu_channel_fence {
	/** Syncpoint Id */
	u32 id;
	/** Syncpoint value to wait on, or for others to wait */
	u32 value;
};

/**
 * The binary format of 'struct nvgpu_gpfifo_entry' introduced here
 * should match that of 'struct nvgpu_gpfifo' defined in uapi header, since
 * this struct is intended to be a mirror copy of the uapi struct. This is
 * a rigid requirement because there's no conversion function and there are
 * memcpy's present between the user gpfifo (of type nvgpu_gpfifo) and the
 * kern gpfifo (of type nvgpu_gpfifo_entry).
 *
 * See also #nvgpu_gpfifo.
 */
struct nvgpu_gpfifo_entry {
	/** First word of gpfifo entry. */
	u32 entry0;
	/** Second word of gpfifo entry. */
	u32 entry1;
};

struct gpfifo_desc {
	/** Memory area containing gpfifo entries. */
	struct nvgpu_mem mem;
	/** Number of entries in gpfifo. */
	u32 entry_num;
	/** Index to last gpfifo entry read by H/W. */
	u32 get;
	/** Index to next gpfifo entry to write to. */
	u32 put;
#ifdef CONFIG_NVGPU_DGPU
	/**
	 * If gpfifo lives in vidmem or is forced to go via PRAMIN, first copy
	 * from userspace to pipe and then from pipe to gpu buffer.
	 */
	void *pipe;
#endif
};

#define NVGPU_CHANNEL_STATUS_STRING_LENGTH	120U

/**
 * Structure abstracting H/W state for channel.
 * Used when unbinding a channel from TSG.
 * See #nvgpu_tsg_unbind_channel_hw_state_check.
 */
struct nvgpu_channel_hw_state {
	/** Channel scheduling is enabled. */
	bool enabled;
	/** Channel is next to run when TSG is scheduled. */
	bool next;
	/** Channel context's was preempted and needs to be reloaded. */
	bool ctx_reload;
	/** Channel has work to do in its GPFIFO. */
	bool busy;
	/** Channel is pending on a semaphore/syncpoint acquire. */
	bool pending_acquire;
	/** Channel has encountered an engine page fault. */
	bool eng_faulted;
	/** Human-readable status string. */
	char status_string[NVGPU_CHANNEL_STATUS_STRING_LENGTH];
};

/**
 * Structure used to take a snapshot of channel status, in order
 * to dump it. See #nvgpu_channel_debug_dump_all.
 */
struct nvgpu_channel_dump_info {
	/** Channel Identifier. */
	u32 chid;
	/** TSG Identifier. */
	u32 tsgid;
	/** TSG Disable count. */
	u32 tsg_disable_count;
	/** Pid of the process that created this channel. */
	int pid;
	/**
	 * Name of the thread that created the channel.
	 * Same size as task_struct.comm[] on linux.
	 */
	char thread_name[TASK_NAME_LEN];
	/** Number of references to this channel. */
	int refs;
	/** Channel uses deterministic submit (kernel submit only). */
	bool deterministic;
	/** Channel H/W state */
	struct nvgpu_channel_hw_state hw_state;
	/** Snapshot of channel instance fields. */
	struct {
		u64 pb_top_level_get;
		u64 pb_put;
		u64 pb_get;
		u64 pb_fetch;
		u32 pb_header;
		u32 pb_count;
		u64 sem_addr;
		u64 sem_payload;
		u32 sem_execute;
		u32 syncpointa;
		u32 syncpointb;
		u32 semaphorea;
		u32 semaphoreb;
		u32 semaphorec;
		u32 semaphored;
	} inst;
	/** Semaphore status. */
	struct {
		u32 value;
		u32 next;
		u64 addr;
	} sema;
	char nvs_domain_name[32];
};

/**
 * The binary format of 'struct nvgpu_setup_bind_args' introduced here
 * should match that of 'struct nvgpu_channel_setup_bind_args' defined in
 * uapi header, since this struct is intended to be a mirror copy of the
 * uapi struct. This is not a hard requirement though because of
 * #nvgpu_get_setup_bind_args conversion function.
 *
 * See also #nvgpu_channel_setup_bind_args.
 */
struct nvgpu_setup_bind_args {
	u32 num_gpfifo_entries;
	u32 num_inflight_jobs;
	u64 userd_dmabuf_fd;
	u64 userd_dmabuf_offset;
	u64 gpfifo_dmabuf_fd;
	u64 gpfifo_dmabuf_offset;
	u32 work_submit_token;
	u64 gpfifo_gpu_va;
	u64 userd_gpu_va;
	u64 usermode_mmio_gpu_va;
	u32 flags;
};

/**
 * The binary format of 'struct notification' introduced here
 * should match that of 'struct nvgpu_notification' defined in uapi header,
 * since this struct is intended to be a mirror copy of the uapi struct.
 * This is hard requirement, because there is no conversion function.
 *
 * See also #nvgpu_notification.
 */
struct notification {
	struct {
		u32 nanoseconds[2];
	} timestamp;
	u32 info32;
	u16 info16;
	u16 status;
};

struct nvgpu_channel_joblist {
	struct {
		unsigned int length;
		unsigned int put;
		unsigned int get;
		struct nvgpu_channel_job *jobs;
		struct nvgpu_mutex read_lock;
	} pre_alloc;
};

/**
 * Track refcount actions, saving their stack traces. This number specifies how
 * many most recent actions are stored in a buffer. Set to 0 to disable. 128
 * should be enough to track moderately hard problems from the start.
 */
#define GK20A_CHANNEL_REFCOUNT_TRACKING 0

#if GK20A_CHANNEL_REFCOUNT_TRACKING

/**
 * Stack depth for the saved actions.
 */
#define GK20A_CHANNEL_REFCOUNT_TRACKING_STACKLEN 8

/**
 * Because the puts and gets are not linked together explicitly (although they
 * should always come in pairs), it's not possible to tell which ref holder to
 * delete from the list when doing a put. So, just store some number of most
 * recent gets and puts in a ring buffer, to obtain a history.
 *
 * These are zeroed when a channel is closed, so a new one starts fresh.
 */
enum nvgpu_channel_ref_action_type {
	channel_gk20a_ref_action_get,
	channel_gk20a_ref_action_put
};

#include <linux/stacktrace.h>

struct nvgpu_channel_ref_action {
	enum nvgpu_channel_ref_action_type type;
	s64 timestamp_ms;
	/*
	 * Many of these traces will be similar. Simpler to just capture
	 * duplicates than to have a separate database for the entries.
	 */
	struct stack_trace trace;
	unsigned long trace_entries[GK20A_CHANNEL_REFCOUNT_TRACKING_STACKLEN];
};
#endif

struct nvgpu_channel_user_syncpt;

/** Channel context */
struct nvgpu_channel {
	/** Pointer to GPU context. Set only when channel is active. */
	struct gk20a *g;
	/** Channel's entry in list of free channels. */
	struct nvgpu_list_node free_chs;
	/** Spinlock to acquire a reference on the channel. */
	struct nvgpu_spinlock ref_obtain_lock;
	/** Number of references to this channel. */
	nvgpu_atomic_t ref_count;
	/** Wait queue to wait on reference decrement. */
	struct nvgpu_cond ref_count_dec_wq;
#if GK20A_CHANNEL_REFCOUNT_TRACKING
	/**
	 * Ring buffer for most recent refcount gets and puts. Protected by
	 * #ref_actions_lock when getting or putting refs (i.e., adding
	 * entries), and when reading entries.
	 */
	struct nvgpu_channel_ref_action ref_actions[
		GK20A_CHANNEL_REFCOUNT_TRACKING];
	size_t ref_actions_put; /* index of next write */
	struct nvgpu_spinlock ref_actions_lock;
#endif
	/**
	 * Channel instance has been bound to hardware (i.e. instance block
	 * has been set up, and bound in CCSR).
	 */
	nvgpu_atomic_t bound;

	/** Channel Identifier. */
	u32 chid;
	/** TSG Identifier. */
	u32 tsgid;
	/**
	 * Thread identifier of the thread that created the channel.
	 * The naming is as per Linux kernel semantic, where each thread actually
	 * gets a distinct pid.
	 */
	pid_t pid;
	/**
	 * Process identifier of the thread that created the channel.
	 * The naming of this variable is as per Linux kernel semantic,
	 * where each thread in a process share the same Thread Group Id.
	 * Confusingly, at userspace level, this is what is seen as the "pid".
	 */
	pid_t tgid;
	/**
	 * Name of the thread that created the channel.
	 * Same size as task_struct.comm[] on linux.
	 */
	char thread_name[TASK_NAME_LEN];
	/** Lock to serialize ioctls for this channel. */
	struct nvgpu_mutex ioctl_lock;

	/** Channel's entry in TSG's channel list. */
	struct nvgpu_list_node ch_entry;

	/**
	 * Channel's entry in TSG Subcontext's (#nvgpu_tsg_subctx) channels list
	 * #ch_list.
	 */
	struct nvgpu_list_node subctx_entry;

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	struct nvgpu_channel_joblist joblist;
	struct gpfifo_desc gpfifo;
	struct priv_cmd_queue *priv_cmd_q;
	struct nvgpu_channel_sync *sync;
	struct nvgpu_channel_sync *gpfifo_sync;
	/* lock for gpfifo hw_sema access */
	struct nvgpu_mutex gpfifo_hw_sema_lock;
	/* for job cleanup handling in the background worker */
	struct nvgpu_list_node worker_item;
#endif /* CONFIG_NVGPU_KERNEL_MODE_SUBMIT */

	/* kernel watchdog to kill stuck jobs */
	struct nvgpu_channel_wdt *wdt;
	bool wdt_debug_dump;

	/** Fence allocator in case of deterministic submit. */
	struct nvgpu_allocator fence_allocator;

	/** Channel's virtual memory. */
	struct vm_gk20a *vm;

	/** USERD memory for usermode submit. */
	struct nvgpu_mem usermode_userd;
	/** GPFIFO memory for usermode submit. */
	struct nvgpu_mem usermode_gpfifo;
	/** Channel instance block memory. */
	struct nvgpu_mem inst_block;

	/**
	 * USERD address that will be programmed in H/W.
	 * It depends on whether usermode submit is enabled or not.
	 */
	u64 userd_iova;

	/**
	 * If kernel mode submit is enabled, userd_mem points to
	 * one userd slab, and userd_offset indicates the offset in bytes
	 * from the start of this slab.
	 * If user mode submit is enabled, userd_mem points to usermode_userd,
	 * and userd_offset is 0.
	 */
	struct nvgpu_mem *userd_mem;
	/** Offset from the start of userd_mem (in bytes). */
	u32 userd_offset;

	/** Notifier wait queue (see #NVGPU_WAIT_TYPE_NOTIFIER). */
	struct nvgpu_cond notifier_wq;
	/** Semaphore wait queue (see #NVPGU_WAIT_TYPE_SEMAPHORE). */
	struct nvgpu_cond semaphore_wq;

#if defined(CONFIG_NVGPU_CYCLESTATS)
	struct {
		void *cyclestate_buffer;
		u32 cyclestate_buffer_size;
		struct nvgpu_mutex cyclestate_buffer_mutex;
	} cyclestate;

	struct nvgpu_mutex cs_client_mutex;
	struct gk20a_cs_snapshot_client *cs_client;
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
	/** Channel's debugger session lock. */
	struct nvgpu_mutex dbg_s_lock;
	/** Channel entry in debugger session's list. */
	struct nvgpu_list_node dbg_s_list;
#endif

	/** Syncpoint lock to allocate fences. */
	struct nvgpu_mutex sync_lock;
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	/** Syncpoint for usermode submit case. */
	struct nvgpu_channel_user_syncpt *user_sync;
#endif

#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
	/** Channel handle for vgpu case. */
	u64 virt_ctx;
#endif

	/** Channel's subcontext. */
	struct nvgpu_tsg_subctx *subctx;

	/** Lock to access unserviceable state. */
	struct nvgpu_spinlock unserviceable_lock;
	/**
	 * An uncorrectable error has occurred on the channel.
	 * It is not possible to take more references on this channel,
	 * and only available option for userspace is to close the
	 * channel fd.
	 */
	bool unserviceable;

	/** Any operating system specific data. */
	void *os_priv;

	/** We support only one object per channel */
	u32 obj_class;

	/**
	 * Accumulated context switch timeouts in ms.
	 * On context switch timeout interrupt, if
	 * #ctxsw_timeout_max_ms is > 0, we check if there was any
	 * progress with pending work in gpfifo. If not, timeouts
	 * are accumulated in #ctxsw_timeout_accumulated_ms.
	 */
	u32 ctxsw_timeout_accumulated_ms;
	/**
	 * GP_GET value read at last context switch timeout. It is
	 * compared with current H/W value for GP_GET to determine
	 * if H/W made any progress processing the gpfifo.
	 */
	u32 ctxsw_timeout_gpfifo_get;
	/**
	 * Maximum accumulated context switch timeout in ms.
	 * Above this limit, the channel is considered to have
	 * timed out. If recovery is enabled, engine is reset.
	 */
	u32 ctxsw_timeout_max_ms;
	/**
	 * Indicates if detailed information shoud be dumped in case of
	 * ctxsw timeout.
	 */
	bool ctxsw_timeout_debug_dump;

	/** Subcontext Id (aka. veid). */
	u32 subctx_id;
	/**
	 * Selects which PBDMA should run this channel if more than
	 * one PBDMA is supported by the runlist.
	 */
	u32 runqueue_sel;

	/** Runlist the channel will run on. */
	struct nvgpu_runlist *runlist;

	/**
	 * Replayable fault state for the channel.
	 */
	bool replayable;

	/**
	 * Indicates if we can take more references on a given channel.
	 * Typically it is not possible to take more references on a
	 * free or unserviceable channel.
	 */
	bool referenceable;
	/** True if VPR support was requested during alloc obj context */
	bool vpr;
#ifdef CONFIG_NVGPU_DETERMINISTIC_CHANNELS
	/**
	 * Channel shall exhibit deterministic behavior in the submit path.
	 * Submit latency shall be consistent (and low). Submits that may cause
	 * nondeterministic behaviour are not allowed and may fail (for example,
	 * sync fds or mapped buffer refcounting are not deterministic).
	 */
	bool deterministic;
	/** Deterministic, but explicitly idle and submits disallowed. */
	bool deterministic_railgate_allowed;
#endif
	/** Channel uses Color Decompression Engine. */
	bool cde;
	/**
	 * If enabled, USERD and GPFIFO buffers are handled in userspace.
	 * Userspace writes a submit token to the doorbell register in the
	 * usermode region to notify the GPU of new work on this channel.
	 * Usermode and kernelmode submit modes are mutually exclusive.
	 * On usermode submit channels, the caller must keep track of GPFIFO
	 * usage. The recommended way for the current hardware (Maxwell..Turing)
	 * is to use semaphore releases to track the pushbuffer progress.
	 */
	bool usermode_submit_enabled;
	/**
	 * Channel is hooked to OS fence framework.
	 */
	bool has_os_fence_framework_support;
	/**
	 * Privileged channel will be able to execute privileged operations via
	 * Host methods on its pushbuffer.
	 */
	bool is_privileged_channel;

#ifdef CONFIG_NVGPU_DEBUGGER
	/**
	 * MMU Debugger Mode is enabled for this channel if mmu_debug_mode_refcnt > 0
	 */
	nvgpu_atomic_t mmu_debug_mode_refcnt;
	/**
	 * MMU Debugger GCC Mode is enabled for this channel if mmu_debug_gcc_mode_refcnt > 0
	 */
	nvgpu_atomic_t mmu_debug_gcc_mode_refcnt;
	/**
	 * ERRBAR is enabled for this channel if refcnt > 0
	 */
	nvgpu_atomic_t sched_exit_wait_for_errbar_refcnt;
#endif
	u64 userd_va;
	u64 gpfifo_va;
	u64 userd_va_mapsize;
	u64 gpfifo_va_mapsize;
};

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT

int nvgpu_channel_worker_init(struct gk20a *g);
void nvgpu_channel_worker_deinit(struct gk20a *g);
void nvgpu_channel_update(struct nvgpu_channel *c);
u32 nvgpu_channel_update_gpfifo_get_and_get_free_count(
		struct nvgpu_channel *ch);
u32 nvgpu_channel_get_gpfifo_free_count(struct nvgpu_channel *ch);
int nvgpu_channel_add_job(struct nvgpu_channel *c,
				 struct nvgpu_channel_job *job,
				 bool skip_buffer_refcounting);
void nvgpu_channel_clean_up_jobs(struct nvgpu_channel *c);
void nvgpu_channel_clean_up_deterministic_job(struct nvgpu_channel *c);
int nvgpu_submit_channel_gpfifo_user(struct nvgpu_channel *c,
				struct nvgpu_gpfifo_userdata userdata,
				u32 num_entries,
				u32 flags,
				struct nvgpu_channel_fence *fence,
				struct nvgpu_user_fence *fence_out,
				struct nvgpu_swprofiler *profiler);

int nvgpu_submit_channel_gpfifo_kernel(struct nvgpu_channel *c,
				struct nvgpu_gpfifo_entry *gpfifo,
				u32 num_entries,
				u32 flags,
				struct nvgpu_channel_fence *fence,
				struct nvgpu_fence_type **fence_out);
#ifdef CONFIG_TEGRA_GK20A_NVHOST
int nvgpu_channel_set_syncpt(struct nvgpu_channel *ch);
#endif

bool nvgpu_channel_update_and_check_ctxsw_timeout(struct nvgpu_channel *ch,
		u32 timeout_delta_ms, bool *progress);

static inline bool nvgpu_channel_is_deterministic(struct nvgpu_channel *c)
{
#ifdef CONFIG_NVGPU_DETERMINISTIC_CHANNELS
	return c->deterministic;
#else
	(void)c;
	return false;
#endif
}

#endif /* CONFIG_NVGPU_KERNEL_MODE_SUBMIT */

/**
 * @brief Get channel pointer from its node in free channels list.
 *
 * @param node [in]	Pointer to node entry in the list of free channels.
 *			Cannot be NULL, and must be valid.
 *
 * @return Channel pointer.
 */
static inline struct nvgpu_channel *
nvgpu_channel_from_free_chs(struct nvgpu_list_node *node)
{
       return (struct nvgpu_channel *)
                  ((uintptr_t)node - offsetof(struct nvgpu_channel, free_chs));
};

/**
 * @brief Get channel pointer from its node in TSG's channel list.
 *
 * @param node [in]	Pointer to node entry in TSG's channel list.
 *			Cannot be NULL, and must be valid.
 *
 * Computes channel pointer from #node pointer.
 *
 * @return Channel pointer.
 */
static inline struct nvgpu_channel *
nvgpu_channel_from_ch_entry(struct nvgpu_list_node *node)
{
       return (struct nvgpu_channel *)
          ((uintptr_t)node - offsetof(struct nvgpu_channel, ch_entry));
};

/**
 * @brief Check if channel is bound to an address space.
 *
 * @param ch [in]	Channel pointer.
 *
 * @return True if channel is bound to an address space, false otherwise.
 */
static inline bool nvgpu_channel_as_bound(struct nvgpu_channel *ch)
{
	return (ch->vm != NULL);
}

/**
 * @brief Commit channel's address space.
 *
 * @param c [in]Channel pointer.
 *
 * Once a channel is bound to an address space, this function applies
 * related settings to channel instance block (e.g. PDB and page size).
 */
void nvgpu_channel_commit_va(struct nvgpu_channel *c);

/**
 * @brief Initializes channel context.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *                      - No validation is performed on this parameter.
 * @param chid [in]	Channel H/W Identifier.
 *                      - No validation is performed on this parameter.
 *
 * Below, \a c is used as a shorthand for &g->fifo.channel[chid].
 *
 * - Assign c->g = NULL.
 * - Assign c->chid = chid.
 * - Initialize c->bound to zero by \ref nvgpu_atomic_set(&c->bound, 0).
 * - Initialize the channel reference lock by \ref nvgpu_spinlock_init(&c->ref_obtain_lock).
 * - Initialize the channel reference count by \ref nvgpu_atomic_set(&c->ref_count, 0).
 * - Assign c->referenceable = false.
 * - Initialize the channel refcount workqueue by \ref nvgpu_cond_init(&c->ref_count_dec_wq).
 *   In case of error, log a descriptive message using nvgpu_err() return the same error.
 * - Initialize the following spinlocks with nvgpu_spinlock_init():
 *   - c->unserviceable_lock
 *   - c->ref_actions_lock (if GK20A_CHANNEL_REFCOUNT_TRACKING is set)
 * - Initialize the following list nodes with nvgpu_init_list_node():
 *   - c->worker_item (if \ref CONFIG_NVGPU_KERNEL_MODE_SUBMIT is set)
 *   - c->dbg_s_list (if \ref CONFIG_NVGPU_DEBUGGER is set)
 *   - c->ch_entry
 * - Initialize the following mutexes with nvgpu_mutex_init():
 *   - c->joblist.pre_alloc.read_lock (if \ref CONFIG_NVGPU_KERNEL_MODE_SUBMIT is set)
 *   - c->ioctl_lock
 *   - c->sync_lock
 *   - c->cyclestate.cyclestate_buffer_mutex (if \ref CONFIG_NVGPU_CYCLESTATS is set)
 *   - c->cs_client_mutex (if \ref CONFIG_NVGPU_CYCLESTATS is set)
 *   - c->dbg_s_lock (if \ref CONFIG_NVGPU_DEBUGGER is set)
 * - Initialize channel's entry list and subctx_entry list using nvgpu_init_list_node().
 * - Add the channel to the free channels list by \ref nvgpu_list_add(&c->free_chs, &g->fifo.free_chs).
 *
 * @retval 0 in case of success.
 * @retval <0 Unspecified error.
 */
int nvgpu_channel_init_support(struct gk20a *g, u32 chid);

/**
 * @brief Initializes channel unit.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *                      No validation is performed on this parameter.
 *
 * Set up software resources used for channels during driver lifetime.
 *
 * - Get number of channels from hardware using \ref #gops_channel.count "g->ops.channel.count(g)",
 *   assign it to \ref #nvgpu_fifo.num_channels "g->fifo.num_channels".
 * - Initialize the mutex that guards free channel list access by
 *   \ref nvgpu_mutex_init(&g->fifo.free_chs_mutex).
 *     - Upon failure of any subsequent step, call \ref nvgpu_mutex_destroy(&g->fifo.free_chs_mutex)
 *       before returning to release the mutex resources.
 * - Allocate memory for g->fifo.channel by
 *   \ref nvgpu_vzalloc(g, g->fifo.num_channels * sizeof(*g->fifo.channel))
 *   - On failure, log a descriptive error with nvgpu_err() and return -ENOMEM.
 * - Initialize free channel list head by \ref nvgpu_init_list_node(&g->fifo.free_chs)
 * - Loop \a chid from 0 to g->fifo.num_channels - 1. On each iteration:
 *     - Call \ref nvgpu_channel_init_support "nvgpu_channel_init_support(g, chid)".
 *       If the call fails, do the following and return the obtained error code.
 *       - Log a descriptive error with nvgpu_err().
 *       - For each \a chid that initialized a channel successfully, call
 *         \ref nvgpu_channel_destroy(g->fifo.channel[chid]).
 *       - Release the allocated channel memory by \ref nvgpu_vfree(g, g->fifo.channel).
 *       - Assign g->fifo.channel = NULL.

 * @retval 0 success
 * @retval -ENOMEM insufficient memory
 * @retval <0 Other unspecified error
 */
int nvgpu_channel_setup_sw(struct gk20a *g);

/**
 * @brief De-initializes channel unit.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *                      - No validation is performed on this parameter.
 *
 * Undo the resource allocations performed in nvgpu_channel_setup_sw().
 *
 * - For each \a chid from 0 to g->fifo.num_channels:
 *     - Obtain &f->channel[chid] as \a ch
 *     - If ch->referenceable is true, use nvgpu_channel_kill() to mark the channel free by force.
 *     - Destroy remaining channel resources with nvgpu_channel_destroy().
 * - Release memory allocated for the channel list by \ref nvgpu_vfree(g, g->fifo.channel).
 * - Set g->fifo.channel to NULL.
 * - Destroy the free channels mutex by \ref nvgpu_mutex_destroy(&f->free_chs_mutex).
 */
void nvgpu_channel_cleanup_sw(struct gk20a *g);

/**
 * @brief Emergency quiescing of channels
 *
 * @param g [in]	Pointer to GPU driver struct.
 *              	- No validation is performed on this parameter.
 *
 * Driver has encountered uncorrectable error, and is entering SW Quiesce state. Mark all open
 * channels unusable and signal all threads waiting for such conditions.
 *
 * - For each channel \a ch in \ref #gk20a.fifo.channel "g->fifo.channel[chid]"
 *   (chid from 0 to \ref gk20a.fifo.num_channels "g->fifo.num_channels-1)".
 *   - Try to get a reference by \ref nvgpu_channel_get(ch). If returned non-NULL:
 *     - Call nvgpu_channel_set_error_notifier() with NVGPU_ERR_NOTIFIER_FIFO_ERROR_IDLE_TIMEOUT
 *       to mark the channel unusable
 *     - Call nvgpu_channel_set_unserviceable() to mark channel as unserviceable.
 *     - Call nvgpu_channel_wakeup_wqs() to signal on wait queues
 *       (notify_wq and semaphore_wq)
 *     - Release the reference: call \ref nvgpu_channel_put(ch)
 */
void nvgpu_channel_sw_quiesce(struct gk20a *g);

/**
 * @brief Close channel
 *
 * @param ch [in]	Channel pointer.
 *			- No validation is performed on this parameter.
 *
 * @note must be inside gk20a_busy()..gk20a_idle()
 *
 * This calls channel_free() internally with the \a force parameter set to false.
 */
void nvgpu_channel_close(struct nvgpu_channel *ch);

/**
 * @brief Forcibly close a channel
 *
 * @param ch [in]	Channel pointer.
 *
 * Forcibly close a channel by calling channel_free() with force as true. It is meant for
 * terminating channels when we know the driver is otherwise dying. Ref counts and the like
 * are ignored by this version of the cleanup.
 *
 * @note must be inside gk20a_busy()..gk20a_idle()
 */
void nvgpu_channel_kill(struct nvgpu_channel *ch);

/**
 * @brief Mark unrecoverable error for channel
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param ch [in]	Channel pointer.
 *
 * An unrecoverable error occurred for the channel. Mark the channel
 * as unserviceable, and unblock pending waits on this channel (semaphore
 * and error notifier wait queues).
 *
 * @return True if channel state should be dumped for debug.
 */
bool nvgpu_channel_mark_error(struct gk20a *g, struct nvgpu_channel *ch);

/**
 * @brief Abort channel's TSG
 *
 * @param ch [in]	Channel pointer.
 * @param preempt [in]	True if TSG should be pre-empted
 *
 * Disables and optionally preempts the channel's TSG.
 * Afterwards, all channels in the TSG are marked as unserviceable.
 *
 * @note: If channel is not bound to a TSG, the call is ignored.
 */
void nvgpu_channel_abort(struct nvgpu_channel *ch, bool channel_preempt);

void nvgpu_channel_abort_clean_up(struct nvgpu_channel *ch);

/**
 * @brief Wake up all threads waiting on semaphore wait
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param post_events [in]	When true, notify all threads waiting
 *				on TSG events.
 *
 * Goes through all channels, and wakes up semaphore wait queue.
 * If #post_events is true, it also wakes up TSG event wait queue.
 */
void nvgpu_channel_semaphore_wakeup(struct gk20a *g, bool post_events);

/**
 * @brief Enable all channels in channel's TSG
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param ch [in]	Channel pointer.
 *
 * Enables all channels that are in the same TSG as #ch.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -EINVAL if channel is not bound to TSG.
 */
int nvgpu_channel_enable_tsg(struct gk20a *g, struct nvgpu_channel *ch);

/**
 * @brief Disables all channels in channel's TSG
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param ch [in]	Channel pointer.
 *
 * Disables all channels that are in the same TSG as #ch.
 * A disable channel is never scheduled to run, even if it is
 * next in the runlist.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -EINVAL if channel is not bound to TSG.
 */
int nvgpu_channel_disable_tsg(struct gk20a *g, struct nvgpu_channel *ch);

/**
 * @brief Suspend all serviceable channels
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This function is typically called when preparing power off.
 * It disables and preempts all active TSGs, then unbinds all channels contexts
 * from hardware (CCSR). Pending channel notifications are cancelled.
 * Channels marked as unserviceable are ignored.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_channel_suspend_all_serviceable_ch(struct gk20a *g);

/**
 * @brief Resume all serviceable channels
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * Bind all serviceable channels contexts back to hardware.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_channel_resume_all_serviceable_ch(struct gk20a *g);

#ifdef CONFIG_NVGPU_DETERMINISTIC_CHANNELS
/**
 * @brief Stop deterministic channel activity for do_idle().
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * Stop deterministic channel activity for do_idle() when power needs to go off
 * momentarily but deterministic channels keep power refs for potentially a
 * long time.
 * Grabs exclusive access to block new deterministic submits, then walks through
 * deterministic channels to drop power refs.
 *
 * @note Must be paired with #nvgpu_channel_deterministic_unidle().
 */
void nvgpu_channel_deterministic_idle(struct gk20a *g);

/**
 * @brief Allow deterministic channel activity again for do_unidle().
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * Releases exclusive access to allow again new deterministic submits, then
 * walks through deterministic channels to take back power refs.
 *
 * @note Must be paired with #nvgpu_channel_deterministic_idle().
 */
void nvgpu_channel_deterministic_unidle(struct gk20a *g);
#endif

/**
 * @brief Get a reference to the channel.
 *
 * @param ch [in]	Channel pointer.
 * @param caller [in]	Caller function name (for reference tracking).
 *
 * Always when a nvgpu_channel pointer is seen and about to be used, a
 * reference must be held to it - either by you or the caller, which should be
 * documented well or otherwise clearly seen. This usually boils down to the
 * file from ioctls directly, or an explicit get in exception handlers when the
 * channel is found by a chid.
 * @note should always be paired with #nvgpu_channel_put.
 *
 * @return ch if reference to channel was obtained, NULL otherwise.
 * @retval NULL if the channel is dead or being freed elsewhere and you must
 *         not touch it.
 */
struct nvgpu_channel *nvgpu_channel_get__func(
		struct nvgpu_channel *ch, const char *caller);
#define nvgpu_channel_get(ch) nvgpu_channel_get__func(ch, __func__)

/**
 * @brief Drop a reference to the channel.
 *
 * @param ch [in]	Channel pointer.
 * @param caller [in]	Caller function name (for reference tracking).
 *
 * Drop reference to a channel, when nvgpu_channel pointer is not used anymore.
 */
void nvgpu_channel_put__func(struct nvgpu_channel *ch, const char *caller);
#define nvgpu_channel_put(ch) nvgpu_channel_put__func(ch, __func__)

/**
 * @brief Get a reference to the channel by id.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param chid [in]	Channel Identifier.
 * @param caller [in]	Caller function name (for reference tracking).
 *
 * Same as #nvgpu_channel_get, except that the channel is first retrieved
 * by chid.
 *
 * @return ch if reference to channel was obtained, NULL otherwise.
 * @retval NULL if #chid is invalid, or if the channel is dead or being freed
 *	   elsewhere and should not be used.
 */
struct nvgpu_channel *nvgpu_channel_from_id__func(
		struct gk20a *g, u32 chid, const char *caller);
#define nvgpu_channel_from_id(g, chid)	\
	nvgpu_channel_from_id__func(g, chid, __func__)

/**
 * @brief Open and initialize a new channel.
 *
 * @param g [in]		Pointer to GPU driver struct.
 *				- No validation is performed on this parameter.
 * @param runlist_id [in]	Runlist Identifer.
 *				- Invalid runlist ID is replaced by #NVGPU_ENGINE_GR.
 * @param is_privileged [in]	Privileged channel will be able to execute
 *				privileged operations via Host methods on its
 *				pushbuffer.
 * @param pid [in]		pid of current thread (for tracking).
 *				- No validation is performed on this parameter.
 * @param tid [in]		tid of current thread (for tracking).
 *				- No validation is performed on this parameter.
 *
 * Acquire a channel handle for use.
 *
 * This function only takes care or the basic channel context initialization. The channel still
 * needs an address space, as well as a gpfifo and userd to submit some work. It will also need to
 * be bound to a TSG, since standalone channels are not supported.
 *
 * @return Valid channel pointer if channel could be opened, NULL if there are not enough resources
 * to allocate and initialize the channel.
 *
 * - Obtain nvgpu_channel ch from \ref allocate_channel(g->fifo). In case of error, log a
 *   descriptive message using nvgpu_err() and return NULL.
 *   - In case any of the further steps fail, release the allocated channel by calling
 *   \ref free_channel(g->fifo, ch) before returning.
 * - Call \ref BUG_ON(ch->g != NULL) to check that the channel is not already active.
 * - Assign ch->g = g.
 *     - In case any of the further steps fail, assign NULL to ch->g.
 * - Assign ch->runlist_id = f->runlists[runlist_id].
 * - Assign ch->is_privileged_channel = is_privileged_channel.
 * - Assign ch->pid = tid.
 * - Assign ch->tgid = pid.
 * - If CONFIG_NVGPU_USERD is set:
 *   - Initialize this channel's userd memory by calling \ref nvgpu_userd_init_channel(g, ch).
 *     In case of error, log a descriptive message using nvgpu_err() and return NULL.
 * - Allocate instance block memory for this channel by calling
 *   \ref gops_channel.alloc_inst "g->ops.channel.alloc_inst(g, ch)". In case of error, log a
 *   descriptive message using nvgpu_err() and return NULL.
 * - Set ch->tsgid to NVGPU_INVALID_TSG_ID.
 * - Set ch->ctxsw_timeout_accumulated_ms to 0.
 * - Set ch->ctxsw_timeout_gpfifo_get to 0.
 * - Set ch->ctxsw_timeout_max_ms to \ref nvgpu_get_poll_timeout(g).
 * - Set ch->ctxsw_timeout_debug_dump to true.
 * - Set ch->unserviceable to true.
 * - Set ch->obj_class to 0.
 * - Set ch->subctx_id to 0.
 * - Set ch->runqueue_sel to 0.
 * - Set ch->vpr to false;
 * - Set ch->mmu_nack_handled to false.
 *
 * - Initialize ch->notifier_wq with nvgpu_cond_init().
 *   In case of error, log a descriptive message using nvgpu_err() and return NULL.
 * - Initialize ch->semaphore_wq by nvgpu_cond_init().
 *   In case of error, log a descriptive message using nvgpu_err() and return NULL.
 * - Call \ref nvgpu_spinlock_acquire(&ch->ref_obtain_lock) to guard access to the following.
 *   - Set ch->referenceable to true.
 *   - Set the refcount to one by \ref nvgpu_atomic_set(&ch->ref_count, 1).
 * - Release the lock by \ref nvgpu_spinlock_release(&ch->ref_obtain_lock).
 * - Return \a ch.
 */
struct nvgpu_channel *nvgpu_channel_open_new(struct gk20a *g,
		u32 runlist_id,
		bool is_privileged_channel,
		pid_t pid, pid_t tid);

/**
 * @brief Setup and bind the channel
 *
 * @param c [in]	Channel pointer.
 *                      - No validation is performed on pointer validity.
 *                        See the description for valid states.
 * @param args [in]	Setup bind arguments.
 *                      - No validation is performed on pointer validity.
 *                        See the description for valid states.
 *
 * Configure gpfifo and userd for the channel. Configure channel instance block and commit it to
 * hardware. Add the channel to the runlist.
 *
 * If \ref NVGPU_CHANNEL_SETUP_BIND_FLAGS_USERMODE_SUPPORT is specified in arguments flags, this
 * function will set up userspace-managed userd and gpfifo for the channel, using buffers (dmabuf fd
 * and offsets) provided in args. A submit token is passed back to be written in the doorbell
 * register in the usermode region to notify the GPU for new work on this channel.
 *
 * @note An address space needs to have been bound to the channel before calling this function.
 *
 * - Initialize g = c->g.
 * - Call \ref channel_setup_bind_prechecks "channel_setup_bind_prechecks(c, args)". In case of
 *   error, log a descriptive message with nvgpu_err() return the same error. A failure here means
 *   that the channel was not in a valid state for the requested operation. See
 *   channel_setup_bind_prechecks() for details.
 * - If \ref CONFIG_NVGPU_DETERMINISTIC_CHANNELS is set:
 *   - If args->flags has the \ref NVGPU_SETUP_BIND_FLAGS_SUPPORT_DETERMINISTIC bit set:
 *     - Call \ref nvgpu_rwsem_down_read(&g->deterministic_busy).
 *     - Call gk20a_busy(). In case of error, call \ref nvgpu_rwsem_up_read(&g->deterministic_busy)
 *       and return the same error.
 *     - Set c->deterministic to true.
 *     - Call \ref nvgpu_rwsem_up_read(&g->deterministic_busy).
 * - If args->flags has the \ref NVGPU_SETUP_BIND_FLAGS_USERMODE_SUPPORT bit set:
 *   - Call \ref nvgpu_channel_setup_usermode "nvgpu_channel_setup_usermode(c, args)". Store the
 *     return code.
 *     - When successful, \ref nvgpu_channel_setup_usermode(c, args) maps usermode gpfifo and
 *       userd buffers, initializes host state in memory and marks channel active
 *       by adding to the channel runlist.
 *     - This function will further call \ref nvgpu_channel_update_runlist(c, true) to
 *       add channel in the channel runlist in a blocking manner.
 * - Else:
 *   - If \ref CONFIG_NVGPU_KERNEL_MODE_SUBMIT is set:
 *     - If \ref #gk20a.os_channel.open "g->os_channel.open" is not NULL,
 *       call g->os_channel.open(c).
 *     - Call \ref channel_setup_kernelmode "channel_setup_kernelmode(c, args)".
 *       Store the return code.
 *   - Else:
 *     - Store -EINVAL for below.
 * - If the above calls failed:
 *   - If \ref CONFIG_NVGPU_DETERMINISTIC_CHANNELS is set and nvgpu_channel_is_deterministic()
 *     returns true:
 *     - call \ref nvgpu_rwsem_down_read(&g->deterministic_busy).
 *     - call gk20a_idle();
 *     - set c->deterministic to false.
 *     - call \ref nvgpu_rwsem_up_read(&g->deterministic_busy).
 *     - Return the error code stored above.
 * - Else:
 *     - Call \ref gops_channel.bind "g->ops.channel.bind(c)".
 *     - Return 0.
 *
 * @retval 0 success
 * @retval -EINVAL if channel is not bound to an address space.
 * @retval -EINVAL if attempting to use kernel mode submit in a safety build.
 * @retval -EINVAL if num_gpfifo_entries in #args is greater than 2^31.
 * @retval -EEXIST if gpfifo has already been allocated for this channel.
 * @retval -E2BIG if there is no space available to append channel to runlist.
 * @retval -ETIMEDOUT if runlist update timed out.
 */
int nvgpu_channel_setup_bind(struct nvgpu_channel *c,
		struct nvgpu_setup_bind_args *args);

/**
 * @brief Add/remove channel to/from runlist.
 *
 * @param c [in]	Channel pointer (must be non-NULL).
 *                      - No validation is performed on this parameter.
 * @param add [in]	True to add a channel, false to remove it.
 *                      - No validation is performed on this parameter.
 *
 * When \a add is true, adds \a c to runlist.
 * When \a add is false, removes \a c from runlist.
 *
 * Block until H/W is done transitioning to the new runlist.
 *
 * This is a wrapper for gops_runlist.update_for_channel
 *
 * @retval 0 Success
 * @retval -E2BIG in case there are not enough entries in runlist buffer to
 *         accommodate all active channels/TSGs.
 * @retval -ETIMEDOUT runlist update timed out.
 */
int nvgpu_channel_update_runlist(struct nvgpu_channel *c, bool add);

/**
 * @brief Wait until atomic counter is equal to N.
 *
 * @param ch [in]		Channel pointer (must be non-NULL).
 *               		- No validation is performed on this parameter.
 * @param counter [in]		The counter to check.
 *               		- No validation is performed on this parameter.
 * @param wait_value [in]	The target value for the counter.
 *               		- No validation is performed on this parameter.
 * @param c [in]		The condition variable to sleep on. This
 *				condition variable is typically signaled
 *				by the thread which updates the counter.
 *               		- No validation is performed on this parameter.
 * @param caller_name [in]	Function name of caller.
 *               		- No validation is performed on this parameter.
 * @param counter_name [in]	Counter name.
 *               		- No validation is performed on this parameter.
 *
 * Blocks until an atomic counter is equal to a specified value.
 * This is typically used to check the number of references on a channel.
 * While waiting on the counter to reach \a wait_value, periodic warnings are logged with
 * current and expected counter values.
 *
 * @note There is no timeout for this function. It will wait indefinitely
 *       until counter is equal to \a wait_value.
 *
 * Loop forever doing the following:
 *   - Use NVGPU_COND_WAIT() on \a c to wait for the following condition for five seconds:
 *     nvgpu_atomic_read(counter) == wait_value
 *   - If the wait returns successfully, break out of the loop.
 *   - Print a descriptive warning with nvgpu_warn() including \a ch->chid, \a counter_name,
 *     \a nvgpu_atomic_read(counter) and \a wait_value.
 *   - Call channel_dump_ref_actions() to log debug information if enabled.
 */
void nvgpu_channel_wait_until_counter_is_N(
	struct nvgpu_channel *ch, nvgpu_atomic_t *counter, int wait_value,
	struct nvgpu_cond *c, const char *caller, const char *counter_name);

/**
 * @brief Free channel's usermode buffers.
 *
 * @param ch [in]	Channel pointer (must be non-NULL).
 *
 * Frees userspace-managed userd and gpfifo buffers.
 */
void nvgpu_channel_free_usermode_buffers(struct nvgpu_channel *c);

/**
 * @brief Size of a gpfifo entry
 *
 * @return Size of a gpfifo entry in bytes.
 */
static inline u32 nvgpu_get_gpfifo_entry_size(void)
{
	return nvgpu_safe_cast_u64_to_u32(sizeof(struct nvgpu_gpfifo_entry));
}

#ifdef CONFIG_DEBUG_FS
void trace_write_pushbuffers(struct nvgpu_channel *c, u32 count);
#else
static inline void trace_write_pushbuffers(struct nvgpu_channel *c, u32 count)
{
	(void)c;
	(void)count;
}
#endif

/**
 * @brief Mark channel as unserviceable.
 *
 * @param ch [in]	Channel pointer.
 *
 * Once unserviceable, it is not possible to take extra references to
 * the channel.
 */
void nvgpu_channel_set_unserviceable(struct nvgpu_channel *ch);

/**
 * @brief Check if channel is unserviceable
 *
 * @param ch [in]	Channel pointer.
 *
 * @return True if channel is unserviceable, false otherwise.
 */
bool nvgpu_channel_check_unserviceable(struct nvgpu_channel *ch);

/**
 * @brief Signal on wait queues (notify_wq and semaphore_wq).
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param ch [in]	Channel pointer.
 *
 * Unblock pending waits on this channel (semaphore and error
 * notifier wait queues).
 *
 */
void nvgpu_channel_wakeup_wqs(struct gk20a *g, struct nvgpu_channel *ch);

#ifdef CONFIG_NVGPU_USERD
/**
 * @brief Channel userd physical address.
 *
 * @param ch [in]	Channel pointer.
 *
 * @return Physical address of channel's userd region.
 */
static inline u64 nvgpu_channel_userd_addr(struct nvgpu_channel *ch)
{
	return nvgpu_safe_add_u64(nvgpu_mem_get_addr(ch->g, ch->userd_mem),
				  ch->userd_offset);
}

/**
 * @brief Channel userd GPU VA.
 *
 * @param c [in]	Channel pointer.
 *
 * @return GPU virtual address of channel's userd region, or
 *	   0ULL if not mapped.
 */
static inline u64 nvgpu_channel_userd_gpu_va(struct nvgpu_channel *c)
{
	struct nvgpu_mem *mem = c->userd_mem;
	return (mem->gpu_va != 0ULL) ?
		nvgpu_safe_add_u64(mem->gpu_va, c->userd_offset) : 0ULL;
}
#endif

/**
 * @brief Allocate channel instance block.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param ch [in]	Channel pointer.
 *
 * Instance block is allocated in vidmem if supported by GPU,
 * sysmem otherwise.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ENOMEM in case there is not enough memory.
 */
int nvgpu_channel_alloc_inst(struct gk20a *g, struct nvgpu_channel *ch);

/**
 * @brief Free channel instance block.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param ch [in]	Channel pointer.
 */
void nvgpu_channel_free_inst(struct gk20a *g, struct nvgpu_channel *ch);

/**
 * @brief Set error notifier.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param ch [in]	Channel pointer.
 * @param error_notifier [in]	Error notifier code.
 *
 * If an application has installed an error notifier buffer with
 * #NVGPU_IOCTL_CHANNEL_SET_ERROR_NOTIFIER, this function updates
 * the buffer with a timestamp and #error_notifier for the error code (e.g.
 * #NVGPU_ERR_NOTIFIER_FIFO_ERROR_MMU_ERR_FLT).
 *
 * @note This function does not wake up the notifer_wq.
 */
void nvgpu_channel_set_error_notifier(struct gk20a *g, struct nvgpu_channel *ch,
			u32 error_notifier);

/**
 * @brief Get channel from instance block.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param inst_ptr [in]	Instance block physical address.
 *
 * Search for the channel which instance block physical address is
 * equal to #inst_ptr. If channel is found, an extra reference is
 * taken on the channel, and should be released with #nvgpu_channel_put.
 *
 * @return Pointer to channel, or NULL if channel was not found.
 */
struct nvgpu_channel *nvgpu_channel_refch_from_inst_ptr(struct gk20a *g,
			u64 inst_ptr);

/**
 * @brief Dump debug information for all channels.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param o [in]	Debug context (which provides methods to
 *			output data).
 *
 * Dump human-readeable information about active channels.
 */
void nvgpu_channel_debug_dump_all(struct gk20a *g,
		 struct nvgpu_debug_context *o);

#ifdef CONFIG_NVGPU_DEBUGGER
int nvgpu_channel_deferred_reset_engines(struct gk20a *g,
		struct nvgpu_channel *ch);
#endif

#ifdef CONFIG_NVGPU_CHANNEL_WDT
/**
 * @brief Rewind the timeout on each non-dormant channel.
 *
 * Reschedule the timeout of each active channel for which timeouts are running
 * as if something was happened on each channel right now. This should be
 * called when a global hang is detected that could cause a false positive on
 * other innocent channels.
 */
void nvgpu_channel_restart_all_wdts(struct gk20a *g);
/**
 * @brief Enable or disable full debug dump on wdt error.
 *
 * Set the policy on whether or not to do the verbose channel and gr debug dump
 * when the channel gets recovered as a result of a watchdog timeout.
 */
void nvgpu_channel_set_wdt_debug_dump(struct nvgpu_channel *ch, bool dump);
#else
static inline void nvgpu_channel_restart_all_wdts(struct gk20a *g)
{
	(void)g;
}
static inline void nvgpu_channel_set_wdt_debug_dump(struct nvgpu_channel *ch,
		bool dump)
{
	(void)ch;
	(void)dump;
}
#endif

/**
 * @brief Get the maximum number of subcontexts for a channel.
 *
 * This function retrieves the maximum number of subcontexts that can be created
 * for a channel based on the GPU instance. The steps are as follows:
 * -# Retrieve the GPU structure from the channel structure.
 * -# Retrieve the channel RMOS-specific data from the channel structure.
 * -# Call nvgpu_grmgr_get_gpu_instance_max_veid_count() to get the maximum
 *    number of VEID (subcontext IDs) supported by the GPU instance.
 * -# Return the maximum number of VEID as the maximum number of subcontexts.
 *
 * @param [in]  ch  Pointer to the channel structure.
 *
 * @return The maximum number of subcontexts supported by the GPU instance.
 */
u32 nvgpu_channel_get_max_subctx_count(struct nvgpu_channel *ch);

/**
 * @brief Enable the channel.
 *
 * @param ch [in]	The channel to enable.
 */
void nvgpu_channel_enable(struct nvgpu_channel *ch);

/**
 * @brief Disable the channel.
 *
 * @param ch [in]	The channel to disable.
 */
void nvgpu_channel_disable(struct nvgpu_channel *ch);
#endif
