/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_COMMON_H
#define NVGPU_FIFO_COMMON_H

/**
 * @file
 * @page common.fifo common.fifo
 *
 * Overview
 * ========
 *
 * The FIFO unit is responsible for managing GPU Job submissions.
 * primarily provides services to:
 * - Open/Close Channels.
 * - Open/Close TSGs.
 * - Bind Channel to a TSG.
 * - Unbind Channel from a TSG is not supported. Channel will be unbound
 *   from TSG when it is closed.
 * - Allocate GPFIFO.
 * - Preempt TSG.
 * - Update Runlist with allocated Channels and TSGs.
 * - Report errors for interrupts triggered by FIFO unit of h/w.
 *   E.g. PBDMA, Runlist etc.
 *
 * The FIFO code also makes sure that all of the necessary SW and HW
 * initialization for engines, pdbma, runlist, channel and tsg subsystems
 * are taken care of before the GPU begins executing work.
 *
 * Data Structures
 * ===============
 *
 * The major data structures exposed to users of the FIFO unit in nvgpu relate
 * to managing Engines, Runlists, Channels and Tsgs.
 * Following is a list of these structures:
 *
 *   + struct nvgpu_fifo
 *
 *       TODO
 *
 *   + struct nvgpu_runlist
 *
 *       TODO
 *
 *   + struct nvgpu_channel
 *
 *       TODO
 *
 *   + struct nvgpu_tsg
 *
 *       TODO
 *
 * Static Design
 * =============
 *
 * Top level FIFO Unit
 * ---------------------
 *
 * Host's scheduler detects when there is work to do and decides what channel
 * will be scheduled to run on PBDMA. Before a channel can be scheduled, s/w
 * need to bind an instance block for that channel. Binding the instance
 * is done by updating an array of registers indexed by channel id ranging
 * from 0 to max channels supported by underlying h/w. Area of memory
 * corresponding to array of registers indexed by channel id is called
 * Channel RAM. Channel RAM register for a channel id also contains other
 * fields e.g. "Bound" set to true indicates entry is valid, "Enabled" set to true
 * indicates channel is enabled and "Pending" set to true indicates channel has
 * work.
 *
 *   + include/nvgpu/fifo.h
 *   + include/nvgpu/gops/fifo.h
 *
 * Runlist
 * -------
 *
 * This is list of TSGs and channels bound to the TSG, that Host should schedule
 * for an engine. S/w submits runlist by writing base and length of engine
 * runlist h/w register. Channels bound to the TSG share the same timeslice and
 * global context (virtual address space, constant buffers etc.). Timeslice is
 * used by h/w to let channel/s of a TSG run before scheduling another TSG. Each
 * channel in the TSG runs till it runs out of work or encounters switch events
 * e.g. wait for semaphore, wait for syncpoint etc.
 *
 *   + include/nvgpu/runlist.h
 *   + include/nvgpu/gops/runlist.h
 *
 * Pbdma
 * -----
 *
 * The PBDMA (Pushbuffer Direct Memory Access) unit fetches pushbuffer
 * data from memory, and generates methods.
 * It executes some of the methods itself and sends rest to engines.
 * Pushbuffers are stored in memory and contains the operations that a
 * GPU context will execute. These pushbuffer descriptors are stored in a
 * circular buffer called the GPFIFO. Each entry in the GPFIFO contains
 * the virtual address and length of the pushbuffer entry. When a work
 * is added, new pushbuffer is created and a new GPFIFO entry is added.
 * There can be multiple PBDMAs delegating work to multiple Engines.
 * PBDMA uses instance block of the channel to load RAMFC image, send context
 * request to Engine and wait for context ack before sending any methods to the
 * Engine.
 *
 *   + include/nvgpu/pbdma.h
 *   + include/nvgpu/pbdma_status.h
 *
 * Engines
 * -------
 *
 * This subunit detects available engines, from h/w configuration registers.
 * It detects how many engines are present and their engine types (e.g.
 * GR or CE). It also collects information on runlists and PBDMAs that
 * are servicing those engines.
 * In addition to engine information, the subunit also provides APIs to get
 * engine status, i.e. context switch status, current context loaded on engine,
 * and next context to be loaded on engine.
 * For each TSG, common.gr allocates an engine context. The context contains
 * the status of the engine. It is saved when the TSG is context switch out of
 * the engine, and restored when the TSG is scheduled to run on the engine.
 * This context is accessed by h/w via the Channel instance block.
 *
 *   + include/nvgpu/engines.h
 *   + include/nvgpu/engine_status.h
 *   + include/nvgpu/gops/engine.h
 *
 * Preempt
 * -------
 *
 * TODO
 *
 *   + include/nvgpu/preempt.h
 *
 * Channel
 * -------
 *
 * Channel is used by application to communicate with GPU.
 * Number of concurrent channels supported is configured by h/w.
 * Each channel has Host State stored in memory structure called RAMFC. This is
 * initialized by s/w before a channel is submitted. RAMFC is part of larger
 * structure called RAMIN (Instance Block). RAMIN also contains engine context
 * pointers to store engine state.
 * Each channel has its own virtual memory space. This allows process isolation.
 * User mode accessible memory area for GPFIFO is called USERD. Pointer to USERD
 * memory is also stored in RAMFC.
 * Channel RAM (ON Chip RAM) is indexed by channel id to know the pointer to
 * memory area containing RAMFC data. Channel RAM also contains status of the
 * channel e.g. idle, pending, on engine etc.
 *
 *   + include/nvgpu/channel.h
 *   + include/nvgpu/gops/channel.h
 *
 * Tsg
 * -------
 *
 * Set of channels with shared context and address space is referred as TSG
 * (Time Slice Group).
 * Number of concurrent TSGs supported is configured by h/w.
 * Multiple channels sharing a single context are grouped into a TSG.
 * Global context is shared across all channels in a TSG. Each channel in the
 * TSG runs until it runs out of work or encounters a switch event.
 *
 *   + include/nvgpu/tsg.h
 *
 * RAM
 * -------
 *
 * TODO
 *
 *   + include/nvgpu/gops/ramin.h
 *   + include/nvgpu/gops/ramfc.h
 *
 * Sync
 * ----
 *
 * ### Semaphore ###
 *  It is a synchronization method used by jobs being submitted to GPU.
 *
 * ### Syncpoint ###
 *  It is a synchronization method used by HOST1X clients. Few of the HOST1x
 *  clients are GPU, Display etc.
 *
 *   + include/nvgpu/channel_sync.h
 *   + include/nvgpu/channel_sync_syncpt.h
 *   + include/nvgpu/gops/sync.h
 *
 * Usermode
 * --------
 *
 *  Usermode region includes a doorbell used to notify h/w of pending work.
 *  nvrm_gpu will typically write a token (which contains GPU channel ID)
 *  in the doorbell region and then GPU scheduler will scan the
 *  corresponding channel for any pending work.
 *
 *   + include/nvgpu/gops/usermode.h
 *
 * Resource utilization
 * --------------------
 *
 * This unit is used to submit work to GPU. It also relies on services from nvgpu-mon
 * to get the interrupts from @ref common.fifo propagated to CPU.
 *
 * Below are the constraints imposed by GPU H/W
 *
 * - number of channels, read from H/W register
 * - number of TSGs (equal to number of channels)
 * - number of syncpt (need one per channel)
 * - number of PBDMAs: constant generated from H/W manuals
 * - number of engines: constant generated from H/W manuals
 * - number of runlists: constant generated from H/W manuals
 *
 * External APIs
 * -------------
 *
 *   + TODO
 *
 *
 * Supporting Functionality
 * ========================
 *
 * There's a fair amount of supporting functionality:
 *
 *   + TODO
 *     - TODO
 *   + TODO
 *   + TODO
 *     - TODO
 *     - TODO
 *
 * Documentation for this will be filled in!
 *
 * Dependencies
 * ------------
 *
 * Dynamic Design
 * ==============
 *
 * This section describes dynamic aspects of the design. Each section identifies a
 * representative use case and describes how it is processed.
 *
 * Initialization
 * --------------
 *
 *  Both h/w and s/w need to be configured for FIFO block to be usable.
 *
 * ### H/W Setup ###
 *
 *  - Reset FIFO unit.
 *  - Load PROD (production) values in h/w registers if required.
 *  - Configure h/w registers to enable timeouts. Setting timeouts
 *    help s/w know if h/w did not respond within set time limits.
 *  - Enable/Disable interrupts for PBDMA/Rulist/Channel etc. These are the
 *    interrupts supported by h/w for s/w to make use of them for notifying
 *    errors to safety services or to synchronize the completion of a job.
 *  - Set base address of USERD.
 *  - Configure clock gating (SLCG settings for CE2 and FIFO, and BLCG settings
 *    for FIFO).
 *
 * ### S/W Setup ###
 *
 *  - Engine: Allocate memory for engine information read from h/w configuration
 *    registers and save for s/w to use later on. The init routine creates a list
 *    of active GR and CE engines, and for each of these engines, collects
 *    engine information, such as:
 *    - list of runlists servicing those engines,
 *    - mmu_fault_id range (used to look up faulty engine in MMU fault), or
 *    - engine reset number.
 *  - Runlist: For each runlist actually used by an engine, a context is allocated,
 *    and initialized with data read from h/w configuration. In particular, the
 *    init routine determines the engines using this runlist, and the PBDMAs
 *    servicing this runlist. Buffers for runlists are allocated as well. Double
 *    buffering is used for runlists, which allows preparing a new runlist in one
 *    buffer, while the other one is being used by H/W.
 *  - PBDMA: Allocate memory for PBDMA map, and read h/w configuration to build a
 *    mapping between PBDMAs and runlists.
 *  - USERD: Allocate userd and do the required set up.
 *  - Channel: Allocate Channels and set defaults.
 *  - TSG: Allocate TSGs and set defaults.
 *
 *  In case a sub-unit fails to initialize, related nvgpu_\ *subunit*\ _setup_sw
 *  shall make sure to clean up any allocated resource. At upper level,
 *  gops_fifo.init_fifo_support shall call cleanup_sw function for all sub-units
 *  that were already initialized. An error is returned to @ref common.init.
 *
 * Error Reporting
 * ---------------
 *
 * ### Interrupts ###
 *
 *   Interrupts enabled in FIFO unit during FIFO init will trigger interrupts to notify
 *   below errors supported by the h/w.
 *
 * #### Host Machine check errors ####
 *
 *   All errors emanating from PBDMA and FIFO units are a consequence of incorrect
 *   hardware programming, and shall be reported to nvgpu-mon.
 *
 *   gops_fifo.intr_0_isr handles FIFO interrupts and reports below errors:
 *
 *   - GPU_HOST_PFIFO_BIND_ERROR
 *   - GPU_HOST_PFIFO_SCHED_ERROR
 *   - GPU_HOST_PFIFO_CHSW_ERROR
 *   - GPU_HOST_PFIFO_MEMOP_TIMEOUT_ERROR
 *   - GPU_HOST_PFIFO_LB_ERROR
 *   - GPU_HOST_PFIFO_FB_FLUSH_TIMEOUT_ERROR
 *
 *   gops_pbdma.handle_intr_0 handles PBDMA interrupts and reports below errors:
 *
 *   - GPU_HOST_PBDMA_TIMEOUT_ERROR
 *   - GPU_HOST_PBDMA_EXTRA_ERROR
 *   - GPU_HOST_PBDMA_GPFIFO_PB_ERROR
 *   - GPU_HOST_PBDMA_METHOD_ERROR
 *   - GPU_HOST_PBDMA_SIGNATURE_ERROR
 *
 *   gops_pbdma.handle_intr_1 handles PBDMA interrupts and reports below errors:
 *
 *   - GPU_HOST_PBDMA_HCE_ERROR
 *
 *   After reporting above critical errors, nvgpu-mon will request nvgpu-rm to
 *   enter quiesced state.
 *
 * .. note::
 *    gops_fifo.intr_0_isr and gops_fifo_intr_1 also handle below interrupt,
 *    which are benign, and do not trigger any error reporting:
 *
 *    - RUNLIST_EVENT: cleared by s/w at the end of runlist polling.
 *    - CHANNEL_INTR: used for tracking job completion, only for kernel mode submit.
 *
 * .. note::
 *    Timeouts on register accesses operations are detected and reported by @ref common.bus
 *
 * #### Host Context Switch Timeout ####
 *
 *   The timeout on CTXSW Req-Ack Protocol consists of counters for each engine connected
 *   to Host that detects when a request for saving, switching, or restoring a context
 *   is issued from Host to one of the engines, and the request fails to complete
 *   within the programmed time period. The timeout is programmed during FIFO init.
 *
 *   When common.fifo gets a CTXSW_TIMEOUT interrupt, it shall report
 *   GPU_HOST_SWERR_PFIFO_CTXSW_TIMEOUT_ERROR to nvgpu-mon.
 *   The error is critical, and may occur multiple times for the same
 *   context, as long as it cannot be context switched out.
 *
 *   In non-safety build, a Channel Watchdog mechanism aborts the Channel after the
 *   number of consecutive context switch timeouts exceeds a threshold. This mechanism
 *   is not enabled in safety build. Instead we rely on the application level task
 *   monitor to detect that GPU tasks are not completing on time.
 *
 * ### S/W Timeouts ###
 *
 * #### Preempt timeout ####
 *
 *   After triggering TSG preempt, common.fifo polls PBDMA and engines to wait for completion.
 *   common.fifo must wait for up to 100 ms to confirm that TSG is preempted from PBDMA.
 *   It should also wait for up to 100 ms to confirm that TSG is preempted from engines.
 *
 *   Preempt TSG occurs in non-mission mode, when unbinding channel from TSG, or aborting TSG.
 *
 *   In case of PBDMA timeout, common.fifo shall report GPU_HOST_SWERR_PBDMA_PREEMPT_ERROR
 *   to nvgpu-mon. This will request SW quiesce.
 *
 *   In case of engine timeout, we expect other HW safety mechanisms such as FECS
 *   watchdog to detect issues that prevented saving current context. An BUG_ON
 *   makes sure that sw_quiesce has been requested, when attempting to recover
 *   from preempt timeout.
 *
 * #### Runlist update timeout ####
 *
 *   After updating runlist, common.fifo polls NV_PFIFO_ENG_RUNLIST until PENDING is false.
 *   common.fifo must wait for up to 3000 ms for runlist update to complete after a new
 *   runlist has been submitted.
 *
 *   Runlist update occurs in non-mission mode, when adding/removing channel/TSGs. The
 *   pending bit is a debug only feature. As a result logging a warning in case of
 *   timeout is sufficient.
 *
 *   We expect other HW safety mechanisms such as PBDMA timeout to detect issues that caused
 *   pending to not clear. It's possible bad base address could cause some MMU faults too.
 *
 *   Worst case we rely on the application level task monitor to detect the GPU tasks are not
 *   completing on time.
 *
 * Safety
 * ======
 *
 * General safety checks
 * ---------------------
 *
 * All subunits are checking error codes from called APIs.
 * Errors are propagated to the caller.
 *
 * HOST programming machine checks
 * -------------------------------
 * The Host Programming Machine Checks consist of various engine instruction, and state
 * consistency checkers that detect illegal values. Collectively, the Host Programming
 * Machine Checks detect illegal and invalid values in the pushbuffer specifications and
 * pushbuffer instructions that are being fetched and uncompressed for the downstream engines.
 *
 * Host programming machine checks errors are reported as an interrupt from the GPU.
 * @ref common.fifo initializes and enables Host programming machines checks, and reports any
 * error to nvgpu-mon.
 *
 * HOST timeout machine checks
 * ---------------------------
 * The Host Timeout Machine Checks consist of a few timeout counters that detect when
 * state configuration accesses or memory flush operations fail to complete within the
 * programmed time period. Collectively, the Host Timeout Machine Checks detect when a
 * register configuration access or memory management request is not acknowledged for
 * a long duration, as specified by the timeout period.  The implication of the timeout
 * detection is that there may have been an error in the register configuration pipeline
 * or memory access pipeline or subsystem.
 *
 * Host timeout machine checks errors are reported as an interrupt from the GPU.
 * @ref common.fifo initializes and enables Host timeout machines checks, and reports any
 * error to nvgpu-mon.
 *
 * Security
 * ========
 *
 * Debugger feature and Tracing support are disabled as part of driver
 * code compilation.
 *
 * Driver is also being compliant with CERT C specification.
 *
 * @ref common.fifo does not leak any sensitive information in error logs.
 *
 * @ref common.fifo validates all return values from nvhost api.
 * This API propagates any error from NvHost. Other APIs (NvMap, NvClock, L0SS, or
 * QNX_BSP) have not used directly by @ref common.fifo.
 *
 * @ref common.fifo maintains separate structures for GPU channels and handles them
 * independently. Each channel use its own GPU Address Space, which ensures
 * isolation of buffers mapped to GPU.
 *
 * Open Items
 * ==========
 *
 * Any open items can go here.
 */

#include <nvgpu/types.h>
#include <nvgpu/lock.h>
#include <nvgpu/kref.h>
#include <nvgpu/list.h>
#include <nvgpu/swprofile.h>

/**
 * H/w defined value for Channel ID type
 */
#define ID_TYPE_CHANNEL			0U
/**
 * H/w defined value for Tsg ID type
 */
#define ID_TYPE_TSG			1U
/**
 * S/w defined value for Runlist ID type
 */
#define ID_TYPE_RUNLIST			2U
/**
 * S/w defined value for unknown ID type.
 */
#define ID_TYPE_UNKNOWN			(~U32(0U))
/**
 * Invalid ID.
 */
#define INVAL_ID			(~U32(0U))
/**
 * Timeout after which ctxsw timeout interrupt (if enabled by s/w) will be
 * triggered by h/w if context fails to context switch.
 */
#ifdef CONFIG_NVGPU_BUILD_CONFIGURATION_IS_SAFETY
#define CTXSW_TIMEOUT_PERIOD_MS		20U
#else
#define CTXSW_TIMEOUT_PERIOD_MS		100U
#endif


/** Subctx id 0 */
#define CHANNEL_INFO_VEID0		0U

/** Pbdma id 0 */
#define CHANNEL_INFO_PBDMA0		0U

struct gk20a;
struct nvgpu_runlist;
struct nvgpu_channel;
struct nvgpu_tsg;
struct nvgpu_swprofiler;

struct nvgpu_fifo {
	/** Pointer to GPU driver struct. */
	struct gk20a *g;

	/** Number of channels supported by the h/w. */
	unsigned int num_channels;

	/** Runlist entry size in bytes as supported by h/w. */
	unsigned int runlist_entry_size;

	/** Number of runlist entries per runlist as supported by the h/w. */
	unsigned int num_runlist_entries;

	/**
	 * Array of pointers to the engines that host controls. The size is
	 * based on the GPU litter value HOST_NUM_ENGINES. This is indexed by
	 * engine ID. That is to say, if you want to get a device that
	 * corresponds to engine ID, E, then host_engines[E] will give you a
	 * pointer to that device.
	 *
	 * If a given element is NULL, that means that there is no engine for
	 * the given engine ID. This is expected for chips that do not populate
	 * the full set of possible engines for a given family of chips. E.g
	 * a GV100 has a lot more engines than a gv11b.
	 */
	const struct nvgpu_device **host_engines;

	/**
	 * Total number of engines supported by the chip family. See
	 * #host_engines above.
	 */
	u32 max_engines;

	/**
	 * The list of active engines; it can be (and often is) smaller than
	 * #host_engines. This list will have exactly #num_engines engines;
	 * use #num_engines to iterate over the list of devices with a for-loop.
	 */
	const struct nvgpu_device **active_engines;

	/**
	 * Length of the #active_engines array.
	 */
	u32 num_engines;

	/**
	 * Pointers to runlists, indexed by real hw runlist_id.
	 * If a runlist is active, then runlists[runlist_id] points
	 * to one entry in active_runlist_info. Otherwise, it is NULL.
	 */
	struct nvgpu_runlist **runlists;
	/** Number of runlists supported by the h/w. */
	u32 max_runlists;

	/** Array of actual HW runlists that are present on the GPU. */
	struct nvgpu_runlist *active_runlists;
	/** Number of active runlists. */
	u32 num_runlists;

	struct nvgpu_swprofiler kickoff_profiler;
	struct nvgpu_swprofiler recovery_profiler;
	struct nvgpu_swprofiler eng_reset_profiler;

#ifdef CONFIG_NVGPU_USERD
	struct nvgpu_mutex userd_mutex;
	struct nvgpu_mem *userd_slabs;
	u32 num_userd_slabs;
	u32 num_channels_per_slab;
	u64 userd_gpu_va;
#endif

	/**
	 * Number of channels in use. This is incremented by one when a
	 * channel is opened and decremented by one when a channel is closed by
	 * userspace.
	 */
	unsigned int used_channels;
	/**
	 * This is the zero initialized area of memory allocated by kernel for
	 * storing channel specific data i.e. #nvgpu_channel struct info for
	 * #num_channels number of channels.
	 */
	struct nvgpu_channel *channel;
	/** List of channels available for allocation */
	struct nvgpu_list_node free_chs;
	/**
	 * Lock used to read and update #free_chs list. Channel entry is
	 * removed when a channel is opened and added back to the #free_ch list
	 * when channel is closed by userspace.
	 * This lock is also used to protect #used_channels.
	 */
	struct nvgpu_mutex free_chs_mutex;

	/** Lock used to prevent multiple recoveries. */
	struct nvgpu_mutex engines_reset_mutex;

	/** Lock used to update h/w runlist registers for submitting runlist. */
	struct nvgpu_spinlock runlist_submit_lock;

	/**
	 * This is the zero initialized area of memory allocated by kernel for
	 * storing TSG specific data i.e. #nvgpu_tsg struct info for
	 * #num_channels number of TSG.
	 */
	struct nvgpu_tsg *tsg;
	/**
	 * Lock used to read and update #nvgpu_tsg.in_use. TSG entry is
	 * in use when a TSG is opened and not in use when TSG is closed
	 * by userspace. Refer #nvgpu_tsg.in_use in tsg.h.
	 */
	struct nvgpu_mutex tsg_inuse_mutex;

	/**
	 * Pointer to a function that will be executed when FIFO support
	 * is requested to be removed. This is supposed to clean up
	 * all s/w resources used by FIFO module e.g. Channel, TSG, PBDMA,
	 * Runlist, Engines and USERD.
	 */
	void (*remove_support)(struct nvgpu_fifo *f);

	/**
	 * nvgpu_fifo_setup_sw is skipped if this flag is set to true.
	 * This gets set to true after successful completion of
	 * nvgpu_fifo_setup_sw.
	 */
	bool sw_ready;

	/** FIFO interrupt related fields. */
	struct nvgpu_fifo_intr {
		/** Share info between isr and non-isr code. */
		struct nvgpu_fifo_intr_isr {
			/** Lock for fifo isr. */
			struct nvgpu_mutex mutex;
		} isr;
		/** PBDMA interrupt specific data. */
		struct nvgpu_fifo_intr_pbdma {
			/** H/w specific unrecoverable PBDMA interrupts. */
			u32 device_fatal_0;
			/**
			 * H/w specific recoverable PBDMA interrupts that are
			 * limited to channels. Fixing and clearing the
			 * interrupt will allow PBDMA to continue.
			 */
			u32 channel_fatal_0;
			/** H/w specific recoverable PBDMA interrupts. */
			u32 restartable_0;
		} pbdma;
	} intr;

#ifdef CONFIG_NVGPU_DEBUGGER
	unsigned long deferred_fault_engines;
	bool deferred_reset_pending;
	struct nvgpu_mutex deferred_reset_mutex;
#endif

	/** Max number of sub context i.e. veid supported by the h/w. */
	u32 max_subctx_count;
	/** Used for vgpu. */
	u32 channel_base;
};

static inline const char *nvgpu_id_type_to_str(unsigned int id_type)
{
	const char *str = NULL;

	switch (id_type) {
	case ID_TYPE_CHANNEL:
		str = "Channel";
		break;
	case ID_TYPE_TSG:
		str = "TSG";
		break;
	case ID_TYPE_RUNLIST:
		str = "Runlist";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

/**
 * @brief Initialize FIFO unit.
 *
 * @param g [in]	The GPU driver struct.
 *                      - The function does not perform g parameter validation.
 *
 * - Invoke gops_fifo.setup_sw() to initialize nvgpu_fifo variables
 * and sub-modules. In case of failure, return and propagate the resulting
 * error code.
 * - Check if gops_fifo.init_fifo_setup_hw() is initialized.
 * If yes, invoke gops_fifo.init_fifo_setup_hw() to handle FIFO unit
 * h/w setup. In case of failure, use nvgpu_fifo_cleanup_sw_common()
 * to clear FIFO s/w metadata.
 *
 * @retval 0		in case of success.
 * @retval -ENOMEM	in case there is not enough memory available.
 * @retval <0		other unspecified errors.
 */
int nvgpu_fifo_init_support(struct gk20a *g);

/**
 * @brief Initialize FIFO software metadata and mark it ready to be used.
 *
 * @param g [in]	The GPU driver struct.
 *                      - The function does not perform g parameter validation.
 *
 * - Check if #nvgpu_fifo.sw_ready is set to true i.e. s/w setup is already done
 * (pointer to nvgpu_fifo is obtained using g->fifo). In case setup is ready,
 * return 0, else continue to setup.
 * - Invoke nvgpu_fifo_setup_sw_common() to perform sw setup.
 * - Mark FIFO sw setup ready by setting #nvgpu_fifo.sw_ready to true.
 *
 * @retval 0		in case of success.
 * @retval -ENOMEM	in case there is not enough memory available.
 * @retval <0		other unspecified errors.
 */
int nvgpu_fifo_setup_sw(struct gk20a *g);

/**
 * @brief Initialize FIFO software metadata sequentially for sub-units channel,
 *        tsg, pbdma, engine, runlist and userd.
 *
 * @param g [in]	The GPU driver struct.
 *                      - The function does not perform g parameter validation.
 *
 * - Obtain #nvgpu_fifo pointer from GPU pointer as g->fifo.
 * - Initialize mutexes of type nvgpu_mutex needed by the FIFO module using
 * \ref nvgpu_mutex_init(&nvgpu_fifo.intr.isr.mutex) and
 * \ref nvgpu_mutex_init(&nvgpu_fifo.engines_reset_mutex).
 * - Use nvgpu_channel_setup_sw() to setup channel data structures. In case
 * of failure, print error and return with error.
 * - Use nvgpu_tsg_setup_sw() to setup tsg data structures. In case
 * of failure, clean up channel structures using nvgpu_channel_cleanup_sw()
 * and return with error.
 * - Check if gops_pbdma.setup_sw() is set. If yes, invoke
 * gops_pbdma.setup_sw() to setup pbdma data structures. In case of
 * failure, clean up tsg using nvgpu_tsg_cleanup_sw() and channel
 * structures, then return with error.
 * - Use nvgpu_engine_setup_sw() to setup engine data structures. In case
 * of failure, if gops_pbdma.cleanup_sw() is set, clean up pbdma
 * using gops_pbdma.cleanup_sw(). Further clean up tsg and channel
 * structures then return with error.
 * - Use nvgpu_runlist_setup_sw() to setup runlist data structures.
 * In case of failure, clean up engine using nvgpu_engine_cleanup_sw() and
 * pbdma, tsg, channel structures then return with error.
 * - Invoke gops_userd.setup_sw()" to setup userd data structures.
 * In case of failure, clean up runlist using nvgpu_runlist_cleanup_sw()
 * and engine, pbdma, tsg, channel structures then return with error.
 * - Initialize nvgpu_fifo.remove_support() function pointer.
 *
 * @note In case of failure, cleanup sw metadata for sub-units that are
 *       initialized.
 *
 * @retval 0		in case of success.
 * @retval -ENOMEM	in case there is not enough memory available.
 * @retval <0		other unspecified errors.
 */
int nvgpu_fifo_setup_sw_common(struct gk20a *g);

/**
 * @brief Clean up FIFO software metadata.
 *
 * @param g [in]	The GPU driver struct.
 *                      - The function does not perform g parameter validation.
 *
 * Use nvgpu_fifo_cleanup_sw_common() to free FIFO software metadata.
 *
 * @return	None
 */
void nvgpu_fifo_cleanup_sw(struct gk20a *g);

/**
 * @brief Clean up FIFO sub-unit metadata.
 *
 * @param g [in]	The GPU driver struct.
 *                      - The function does not perform g parameter validation.
 *
 * - Invoke gops_userd.cleanup_sw() to free userd data structures.
 * - Use nvgpu_channel_cleanup_sw() to free channel data structures.
 * - Use nvgpu_tsg_cleanup_sw() to free tsg data structures.
 * - Use nvgpu_runlist_cleanup_sw() to free runlist data structures.
 * - Use nvgpu_engine_cleanup_sw() to free engine data structures.
 * - Check if gops_pbdma.cleanup_sw() is set. If yes, invoke
 *   gops_pbdma.cleanup_sw() to free pbdma data structures.
 * - Destroy mutexes of type #nvgpu_mutex needed by the FIFO module using
 * \ref nvgpu_mutex_destroy(&nvgpu_fifo.intr.isr.mutex) and
 * \ref nvgpu_mutex_destroy(&nvgpu_fifo.engines_reset_mutex).
 * Mark FIFO s/w clean up complete by setting #nvgpu_fifo.sw_ready to false.
 *
 * @return	None
 */
void nvgpu_fifo_cleanup_sw_common(struct gk20a *g);

/**
 * @brief Decode PBDMA channel status and Engine status read from h/w register.
 *
 * @param index [in]	Status value used to index into the constant array of
 *			constant characters.
 *                      - The function validates that \a index is not greater
 *                        than #pbdma_ch_eng_status_str array size.
 *
 * Check \a index is within #pbdma_ch_eng_status_str array size. If \a index
 * is valid, return string at \a index of #pbdma_ch_eng_status_str array. Else
 * return "not found" string.
 *
 * @return PBDMA and channel status as a string
 */
const char *nvgpu_fifo_decode_pbdma_ch_eng_status(u32 index);

/**
 * @brief Suspend FIFO support while preparing GPU for poweroff.
 *
 * @param g [in]	The GPU driver struct.
 *                      - The function does not perform g parameter validation.
 *
 * - Check if gops_mm.is_bar1_supported() is set. If yes, invoke
 * gops_mm.is_bar1_supported() to disable BAR1 snooping.
 * - Invoke disable_fifo_interrupts() to disable FIFO stalling and
 * non-stalling interrupts at FIFO unit and MC level.
 *
 * @retval 0 is always returned.
 */
int nvgpu_fifo_suspend(struct gk20a *g);

/**
 * @brief Emergency quiescing of FIFO.
 *
 * @param g [in]	The GPU driver struct.
 *                      - The function does not perform g parameter validation.
 *
 * Put FIFO into a non-functioning state to ensure that no corrupted
 * work is completed because of the fault. This is because the freedom
 * from interference may not always be shown between the faulted and
 * the non-faulted TSG contexts.
 * - Set runlist_mask = U32_MAX, to indicate all runlists
 * - Invoke gops_runlist.write_state() with runlist mask as U32_MAX and state as #RUNLIST_DISABLED
 * to disable all runlists.
 * - Invoke gops_fifo.preempt_runlists_for_rc() with runlist mask as U32_MAX
 * to preempt all runlists.
 *
 * @return	None
 */
void nvgpu_fifo_sw_quiesce(struct gk20a *g);

#endif /* NVGPU_FIFO_COMMON_H */
