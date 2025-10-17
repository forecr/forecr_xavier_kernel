/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_H
#define NVGPU_GR_H

#include <nvgpu/types.h>
#include <nvgpu/enabled.h>

/**
 * @file
 * @page common.gr common.gr
 *
 * Overview
 * ========
 *
 * @ref common.gr unit is responsible for managing the GR engine on the GPU.
 * There are two aspects of GR engine support managed by this unit:
 *
 * - GR engine h/w.
 * - GR engine s/w context images.
 *
 * GR engine h/w management
 * ------------------------
 *
 * @ref common.gr unit has below responsibilities to manage GR engine h/w:
 *
 * - Reset and enable GR engine h/w.
 * - Allocate all necessary s/w data structures to hold GR engine
 *   configuration.
 * - Configure GR engine h/w to a known good state.
 * - Populate all s/w data structures while initializing the h/w.
 *   e.g. populate number of available GPC/TPC/SM components.
 * - Provide APIs to retrieve GR engine configuration.
 * - Enable and handle all GR engine interrupts and exceptions.
 * - Suspend GR engine while preparing GPU for poweroff.
 * - Remove GR engine s/w support as part of removing GPU support.
 *
 * GR engine s/w context image management
 * --------------------------------------
 *
 * @ref common.gr unit has below responsibilities to manage GR engine context
 * images:
 *
 * - Manage all global context images.
 * - Manage GR engine context (per GPU Time Slice Group).
 * - Manage subcontext (per GPU channel).
 * - Allocate Golden context image.
 * - Map/unmap all global context images into GR engine context.
 *
 * Data Structures
 * ===============
 *
 * All the major data structures are defined privately in @ref common.gr
 * unit. However @ref common.gr unit exposes below public data structures
 * to support ucode handling in @ref common.acr unit:
 *
 *   + struct nvgpu_ctxsw_ucode_segment
 *
 *       This struct describes single ucode segment.
 *
 *   + struct nvgpu_ctxsw_ucode_segments
 *
 *       This struct describes the ucode layout and includes description
 *       of boot/data/code segments of ucode.
 *
 * Static Design
 * =============
 *
 *   + include/nvgpu/gr/fs_state.h
 *   + include/nvgpu/gr/setup.h
 *   + include/nvgpu/gr/config.h
 *   + include/nvgpu/gr/ctx.h
 *   + include/nvgpu/gr/subctx.h
 *   + include/nvgpu/gr/global_ctx.h
 *   + include/nvgpu/gr/obj_ctx.h
 *   + include/nvgpu/gr/gr_falcon.h
 *   + include/nvgpu/gr/gr_intr.h
 *   + include/nvgpu/gr/gr_utils.h
 *
 * Subunits Overview
 * -----------------
 *
 * @ref common.gr.init - Initialization subunit
 *  This subunit helps in setting up GR Engine. For s/w it helps to allocate
 *  and initialize various data structures. For h/w it helps to reset and
 *  configure the registers.
 *
 * @ref common.gr.config - Configuration subunit
 *  This subunit reads the h/w registers from nvgpu-rm @ref common.fuse and
 *  @ref common.top units and keeps track of the GR h/w properties and provides
 *  interfaces for s/w queries.
 *
 * @ref common.gr.setup - Setup subunit
 *  This subunit helps to allocate and free the memory for context and subcontext
 *  related to channel. It also helps to validate and setup the preemption modes.
 *
 * @ref common.gr.fs_state - Floorsweeping state subunit
 *  This subunit helps to set the post floorsweeping. Configure the GR engine
 *  with available GPC/TPC/SM count and mapping.
 *
 * @ref common.gr.falcon - Falcon subunit
 *  This subunit helps in communications with FECS firmware.
 *  FECS cordinates context switching for graphics engine.
 *
 * @ref common.gr.ctx - Context subunit.
 *  This subunit helps for allocation and management of the
 *  context in GR engine. This subunit makes use of other subunits like
 *  @ref common.gr.global_ctx, @ref common.gr.obj_ctx, @ref common.gr.subctx and
 *  @ref common.gr.ctxsw_prog.
 *
 * @ref common.gr.global_ctx - Global Context subunit.
 *  This subunit helps global context buffer management.
 *
 * @ref common.gr.obj_ctx - Context Object subunit.
 *  This subunit helps to initialize the context state registers. Helps to handle
 *  the golden context image and also helps in preparing and committing
 *  global context buffers to h/w. The golden context image is created only once
 *  while creating the first context image. This will be done as part of allocate
 *  object context. Golden context image is saved after explicitly initializing
 *  the first context image. All subsequent context images are initialized by
 *  loading the saved context image instead of explicit initialization.
 *  For golden context verification capture the golden image twice and compare
 *  the contents. Correct golden image is captured only if the contents match
 *  from both the images.
 *
 * @ref common.gr.subctx - SubContext Subunit
 *  This subunit helps in allocation and management of subcontext buffers.
 *
 * @ref common.gr.ctxsw_prog - Context programming subunit
 *  This subunit helps the context related h/w register programming.
 *
 * @ref common.gr.intr - Interrupt subunit
 *  This subunit helps to enable/disable GR Engine related interrupts. Helps to
 *  handle the interrupts (For e.g. exceptions,errors,etc) and clear the interrupts.
 *  Also reports the error to nvgpu-mon as needed.
 *
 * @ref common.gr.ecc - ECC subunit
 *  This subunit helps to allocate and deallocate the ECC corrected and
 *  uncorrected error counters for various subunits of GR. These error counts are
 *  reported back to nvgpu-mon as needed.
 *
 * @ref common.gr.utils - Utility subunit
 *  This subunit provides generic functions to get handles to various gr subunits
 *  from @ref common.gr structure.
 *
 * Resource utilization
 * --------------------
 * This unit helps to initialize the GR engine and the creation of context for channel.
 * It also relies on services from nvgpu-mon to get the interrupts triggered.
 *
 * External APIs
 * -------------
 *
 * Supporting Functionality
 * ========================
 *
 * Dependencies
 * ------------
 *
 * Dynamic Design
 * ==============
 * Application requires to create and initialize channel context before
 * submitting any work to GR h/w. Application can optionally change
 * the compute preemption mode.
 *
 * This section describes dynamic aspects of the design.  Each section identifies
 * a representative use case and describes how it is processed.
 *
 * Initialization
 * --------------
 *
 *    Both h/w and s/w need to be configured for @ref common.gr block to be usable.
 *    The @ref common.gr unit initialization process will be handled in three phases.
 *
 *    **Phase I - Prepare s/w**
 *      This sections executes a subset of s/w initialization sequences that is
 *      required to enable GR h/w engine. s/w make use of priv_ring and top
 *      registers to find the GPU device information.
 *      This includes GPU architecture, revision, class values.
 *      SM architecture information like total number of warps, SPA version,
 *      SM version. GR engine configurations like GPC/TPC units, maximum number
 *      of LTC and LTS, GPC/TPC supported masks and available context
 *      preemption modes.
 *      Reads the netlist ucode and do the memory allocation for falcon init.
 *
 *    **Phase II - Enable h/w**
 *      This sections reset the GR engine. Load non_context netlist portion to h/w
 *      and wait maximum up to 3000 ms for GR engine to get idle.
 *
 *    **Phase III - GR Init support**
 *      This sections initializes remaining support for GR engine functionality.
 *
 *      * Initialize context switch ucode.
 *          - Load ctxsw firmware through ctxsw mailbox method submission.
 *          - Set values for ctxsw watch dog timer.
 *          - Wait up to 3000 ms for a success/failure response to ensure proper
 *            firmware boot.
 *          - In case of failure, report the failure back to qnx.sdl.
 *      * Read Golden Context image size from FECS.
 *      * Allocate all internal data memory and memory for global context buffers.
 *      * Initialize GR engine h/w registers.
 *      * Initialize and enable GR interrupt registers.
 *      * Initialize Preemption state.
 *      * Complete Floorsweep and wait up to 3000 ms to get GR engine idle.
 *
 * Runtime Support
 * ---------------
 *
 *    Application requires to create and initialize channel context before
 *    submitting any work to GR h/w. Application can optionally change
 *    the compute preemption mode.
 *
 * Error Reporting
 * ---------------
 *
 *   Interrupts enabled in GR Engine unit during GR init will trigger interrupts
 *   to notify errors/events supported by the h/w.
 *
 *   Following interrupt fields are handled in GR Engine unit
 *
 *   * NOTIFY
 *      Notification interrupts. The method following the Notify method is
 *      captured in the TRAPPED_ADDR and TRAPPED_DATA regs.
 *
 *   * SEMAPHORE
 *      Semaphore interrupts.
 *
 *   * ILLEGAL_METHOD
 *      Illegal methods can be passed to FECS for service by firmware.
 *
 *   * ILLEGAL_CLASS
 *      This indicates the class of the current subchannel is not recognized
 *      by the hardware
 *
 *   * ILLEGAL_NOTIFY
 *      This indicates that a notify method was not followed by a NOOperation
 *      method belonging to the same subchannel.
 *
 *   * FIRMWARE_METHOD
 *      This indicates that the method in TRAPPED_ADDR is a SetFalcon method.
 *
 *   * FECS_ERROR
 *      This indicates FECS has an interrupt condition. FECS interrupt routine
 *      handles errors related to fault raised for failure on transaction during
 *      ctxsw, watchdog timer expiration during ctxsw, ctxsw mailbox interrupts,
 *      etc. Those errors are reported back to qnx-sdl unit in nvgpu-rm.
 *
 *   * CLASS_ERROR
 *      This indicates a class error has occured.
 *
 *   * EXCEPTION
 *      This indicates an exception condition exists in the GR pipe. There are
 *      separate exception registers which contains the information about the
 *      source of an exception reported.
 *
 *   @ref common.mc unit calls @ref common.gr stalling interrupt function. This function
 *   handles any pending GR engine interrupts, record and report ecc errors of
 *   affected subunits to nvgpu-rm qnx-sdl, clears pending interrupts.
 *
 * Safety
 * ======
 * Following safety requirements are handled by @ref common.gr unit.
 *
 * *SM_debug mode*
 *   NvGPU software ensures SM_debug mode is set to OFF by default.
 *
 * *GR engine Golden context verification*
 *   @ref common.gr shall verify the golden context validity before capturing it.
 *   @ref common.gr.obj_ctx helps to handle this safety requirements.
 *   Refer @ref common.gr.obj_ctx subunit under Static Design section.
 *
 * This section describes the temporal constraints applied during @ref common.gr
 * initialization.
 *
 *  -  After configuring the GR h/w engine, the s/w must wait a maximum up to
 *     3000 ms for GR engine to get idle.
 *  -  During various stages of loading the sw bundle from netlist,
 *     s/w must wait a maximum up to 3000 ms for idling the GR engine.
 *  -  s/w must wait up to 1 ms for imem/dmem scrubbing of any falcon unit
 *     to complete.
 *  -  s/w must ensure iGPU CTXSW firmware is loaded before issuing any method
 *     to the firmware. After loading CTXSW firwmare, s/w polls for handshake
 *     complete status to ensure firmware is loaded successfully.
 *  -  s/w must set watchdog timeout value before issuing any method to
 *     CTXSW firmware.
 *  -  s/w must wait to to 3000ms to ensure the correct response codes
 *     in CTXSW firmware mailboxes. s/w submits various methods to CTXSW firmware
 *     and wait with this timeout for success/failure response for each submit.
 *  -  s/w must query the golden image size and bind the instance block before
 *     issuing command to save the golden context image to CTXSW firmware.
 *  -  Complete Floorsweep and wait up to 3000 ms to get GR engine idle.
 *
 * Security
 * ========
 *
 * Debugger feature and Tracing support are disabled as part of driver
 * code compilation.
 *
 * Driver is also being compliant with CERT C specification.
 *
 * Satisfies NVGPU_RM_SECURITY_MEASURES atom from nvgpu-rm
 *
 * Open Items
 * ==========
 *
 */

struct gk20a;
struct nvgpu_gr;
struct nvgpu_gr_config;
struct netlist_av_list;
struct nvgpu_priv_addr_map;

enum enum_enabled_gfx_mig_flags {
	NVGPU_SUPPORT_2D = (NVGPU_MAX_ENABLED_BITS + 1),
	NVGPU_SUPPORT_3D,
	NVGPU_SUPPORT_I2M,
	NVGPU_SUPPORT_ZBC,
	NVGPU_SUPPORT_ZBC_STENCIL,
	NVGPU_SUPPORT_PREEMPTION_GFXP,
	NVGPU_MAX_ENABLED_GR_CLASS_BITS
};

/**
 * @brief Allocate memory for GR struct and initialize the minimum SW
 *        required to enable GR engine HW as phase 1 of GR engine
 *        initialization.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This function allocates memory for GR struct (i.e. struct nvgpu_gr).
 * Number of GR instances are queried from #nvgpu_grmgr_get_num_gr_instances()
 * and size is allocated for each instance.
 *
 * This function executes only a subset of s/w initialization sequence
 * that is required to enable GR engine h/w in #nvgpu_gr_enable_hw().
 *
 * This initialization includes allocating memory for internal data
 * structures required to enable h/w. This function allocates memory
 * for FECS ECC error counters and GR interrupt structure.
 *
 * Note that all rest of the s/w initialization is completed in
 * #nvgpu_gr_init_support() function.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ENOMEM if memory allocation fails for GR struct.
 */
int nvgpu_gr_alloc(struct gk20a *g);

/**
 * @brief Free GR struct.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This function ensures that memory allocated for GR struct is released
 * during deinitialization.
 */
void nvgpu_gr_free(struct gk20a *g);

int nvgpu_gr_init_ctx_bufs(struct gk20a *g, struct nvgpu_gr *gr);

/**
 * @brief Reset and enable GR engine HW as phase 2 of GR engine
 *        initialization.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This function enables GR engine h/w. This includes:
 * - Resetting GR engine in MC.
 * - Load PROD register values.
 * - Disable elcg until GR engine gets enabled.
 * - Enable fifo access.
 * - Enabling GR engine interrupts.
 * - Load non_context init.
 * - Ensure falcon memory is scrubbed.
 * - Wait till GR engine status busy bit gets cleared.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ETIMEDOUT if falcon mem scrubbing times out.
 * @retval -EAGAIN if GR engine idle wait times out.
 */
int nvgpu_gr_enable_hw(struct gk20a *g);

/**
 * @brief Initialize CTXSW, GR engine HW, GR engine interrupts, and
 *        necessary SW context support as phase 3 (final phase) of GR
 *        engine initialization.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This function initializes all the GR engine support and
 * functionality. This includes:
 * - Initializing context switch ucode. In case of failure, report
 *   it to qnx.sdl using #gr_intr_report_ctxsw_error().
 * - Reading Golden context image size from FECS micro controller.
 * - Allocating memory for all internal data structures such as GR ctx desc
 *   and ECC.
 * - Allocating global context buffers.
 * - Initializing GR engine h/w registers to known good values.
 * - Enabling GR engine stalling and non-stalling interrupts and exceptions
 *   at GR unit and MC level.
 * - Reading GR engine configuration (like number of GPC/TPC/SM etc)
 *   after considering floorsweeping.
 *
 * This function must be called in this sequence:
 * - nvgpu_gr_alloc()
 * - nvgpu_gr_enable_hw()
 * - nvgpu_gr_init_support()
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ENOENT if context switch ucode is not found.
 * @retval -ETIMEDOUT if context switch ucode times out.
 * @retval -ETIMEDOUT if reading golden context size times out.
 * @retval -ENOMEM if memory allocation fails for any internal data
 *         structure.
 * @retval -ENOMEM if mapping to address space fails for any internal data
 *         structure.
 * @retval -ETIMEDOUT if falcon method opcode submission returns
 *         ucode wait status of \a WAIT_UCODE_TIMEOUT.
 * @retval -EINVAL if falcon method opcode submission returns
 *         ucode wait status of \a WAIT_UCODE_ERROR.
 */
int nvgpu_gr_init_support(struct gk20a *g);

/**
 * @brief Set GR s/w ready status.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param enable [in]	Boolean flag.
 *
 * This function sets/unsets GR s/w ready status in struct nvgpu_gr.
 * Setting of flag is typically needed during initialization of GR s/w.
 * Unsetting of flag is needed while preparing for poweroff.
 */
void nvgpu_gr_sw_ready(struct gk20a *g, bool enable);

/**
 * @brief Get number of SMs in GR engine.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This function returns number of SMs available in GR engine.
 * Note that this count is initialized only after GR engine is
 * completely initialized through #nvgpu_gr_init_support().
 *
 * @return number of available SMs in GR engine.
 */
u32 nvgpu_gr_get_no_of_sm(struct gk20a *g, u32 instance_id);

/**
 * @brief Suspend GR engine.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This function is typically called while preparing for GPU power off.
 * This function makes sure that GR engine is idle before power off.
 * It will also disable all GR engine stalling and non-stalling
 * interrupts and exceptions at GR unit level and GR engine
 * stalling and non-stalling interrupts at MC level.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -EAGAIN if GR engine idle wait times out.
 */
int nvgpu_gr_suspend(struct gk20a *g);

/**
 * @brief Remove GR engine s/w support.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This is typically called while removing entire GPU driver.
 * This function will ensure that all memory and other system resources
 * allocated during GR s/w initialization are released appropriately.
 */
void nvgpu_gr_remove_support(struct gk20a *g);

/**
 * @brief Get base register offset of a given GPC.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param gpc [in]	Valid GPC index.
 *
 * This function calculates and returns base register offset of a given
 * GPC. GPC index must be less than value returned by
 * #nvgpu_gr_config_get_gpc_count(), otherwise an assert is raised.
 *
 * @return base register offset of a given GPC.
 */
u32 nvgpu_gr_gpc_offset(struct gk20a *g, u32 gpc);

/**
 * @brief Get base register offset of a given TPC in a GPC.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param tpc [in]	Valid TPC index.
 *
 * This function calculates and returns base register offset of a given
 * TPC within a GPC. TPC index must be less than value returned by
 * #nvgpu_gr_config_get_max_tpc_per_gpc_count(), otherwise an assert
 * is raised.
 *
 * @return base register offset of a given TPC.
 */
u32 nvgpu_gr_tpc_offset(struct gk20a *g, u32 tpc);

/**
 * @brief Get base register offset of a given SM in a GPC/TPC.
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param sm [in]	Valid SM index.
 *
 * This function calculates and returns base register offset of a given
 * SM within a GPC/TPC pair. SM index must be less than value returned by
 * #nvgpu_gr_config_get_sm_count_per_tpc(), otherwise an assert
 * is raised.
 *
 * @return base register offset of a given SM.
 */
u32 nvgpu_gr_sm_offset(struct gk20a *g, u32 sm);

/**
 * @brief Get pri base register offset of a given ROP instance within a GPC
 *
 * @param g [in]	Pointer to GPU driver struct.
 * @param rop [in]	ROP index.
 *
 * This function calculates and returns base register offset of a given
 * ROP within a GPC.
 *
 * @return base register offset of a given ROP.
 */
u32 nvgpu_gr_rop_offset(struct gk20a *g, u32 rop);

u32 nvgpu_gr_get_syspipe_id(struct gk20a *g, u32 gr_instance_id);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
/**
 * @brief Wait for GR engine to be initialized
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * Calling this function ensures that GR engine initialization i.e.
 * nvgpu_gr_init_support() function call is complete.
 */
void nvgpu_gr_wait_initialized(struct gk20a *g);
#endif
bool nvgpu_gr_is_tpc_addr(struct gk20a *g, u32 addr);
u32 nvgpu_gr_get_tpc_num(struct gk20a *g, u32 addr);

#if defined(CONFIG_NVGPU_RECOVERY) || defined(CONFIG_NVGPU_DEBUGGER)
int nvgpu_gr_disable_ctxsw(struct gk20a *g);
int nvgpu_gr_enable_ctxsw(struct gk20a *g);
#endif
#ifdef CONFIG_NVGPU_ENGINE_RESET
int nvgpu_gr_reset(struct gk20a *g);
#endif

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
void nvgpu_gr_init_reset_enable_hw_non_ctx_local(struct gk20a *g);
void nvgpu_gr_init_reset_enable_hw_non_ctx_global(struct gk20a *g);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

bool nvgpu_is_gr_class_enabled(struct gk20a *g, u32 flag, u32 gr_instance_id);
void nvgpu_set_gr_class_enabled(struct gk20a *g, u32 flag, bool state,
		u32 gr_instance_id);
#ifdef CONFIG_NVGPU_DEBUGGER
int nvgpu_gr_search_pri_addr_map(struct gk20a *g,
		struct nvgpu_priv_addr_map *p_map, u32 addr, u32 max_offsets,
		u32 *priv_offset, u32 *num_offsets);
#endif

bool nvgpu_gr_set_mmu_nack_pending(struct gk20a *g, u32 gr_instance_id,
		u32 ctx_id);

#endif /* NVGPU_GR_H */
