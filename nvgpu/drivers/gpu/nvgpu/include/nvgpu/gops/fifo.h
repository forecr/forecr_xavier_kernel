/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GOPS_FIFO_H
#define NVGPU_GOPS_FIFO_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * FIFO HAL interface.
 */
struct gk20a;
struct nvgpu_channel;
struct nvgpu_tsg;
struct mmu_fault_info;

struct gops_fifo {
	/**
 	 * @brief Initialize FIFO unit.
 	 *
 	 * @param g [in]	Pointer to GPU driver struct.
 	 *
	 * This HAL is used to initialize FIFO software context,
	 * then perform GPU h/w initializations. It always maps to
	 * nvgpu_fifo_init_support(), except for vgpu case.
	 *
 	 * @return 0 in case of success, < 0 in case of failure.
 	 */
	int (*fifo_init_support)(struct gk20a *g);

	/**
 	 * @brief Suspend FIFO unit.
 	 *
 	 * @param g [in]	Pointer to GPU driver struct.
 	 *
	 * - Disable BAR1 snooping when supported.
	 * - Disable FIFO interrupts
	 *   - Disable FIFO stalling interrupts
	 *   - Disable ctxsw timeout detection, and clear any pending
	 *     ctxsw timeout interrupt.
	 *   - Disable PBDMA interrupts.
	 *   - Disable FIFO non-stalling interrupts.
	 *
 	 * @return 0 in case of success, < 0 in case of failure.
 	 */
	int (*fifo_suspend)(struct gk20a *g);

	/**
 	 * @brief Preempt TSG.
 	 *
 	 * @param g [in]	Pointer to GPU driver struct.
	 * @param runlist_id [in] Runlist ID.
	 * @param tsgid [in]	TSG ID.
	 * @param gfid [in]		GFID.
 	 *
	 * - Acquire lock for active runlist.
	 * - Write h/w register to trigger TSG preempt for \a tsg.
	 * - Preemption mode (e.g. CTA or WFI) depends on the preemption
	 *   mode configured in the GR context.
	 * - Release lock acquired for active runlist.
	 * - Poll PBDMAs and engines status until preemption is complete,
	 *   or poll timeout occurs.
	 *
	 * On some chips, it is also needed to disable scheduling
	 * before preempting TSG.
	 *
	 * @see #nvgpu_preempt_get_timeout
	 * @see nvgpu_gr_ctx::compute_preempt_mode
	 *
 	 * @return 0 in case preemption succeeded, < 0 in case of failure.
	 * @retval -ETIMEDOUT when preemption was triggered, but did not
	 *         complete within preemption poll timeout.
 	 */
	int (*preempt_tsg)(struct gk20a *g, u32 runlist_id, u32 tsgid, u32 gfid);

	/**
	 * @brief Enable and configure FIFO.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * Reset FIFO unit and configure FIFO h/w settings.
	 * - Enable PMC FIFO.
	 * - Configure clock gating:
	 *   - Set SLCG settings for CE2 and FIFO.
	 *   - Set BLCG settings for FIFO.
	 * - Setup PBDMA hw.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*reset_enable_hw)(struct gk20a *g);

	/**
	 * @brief Handles FIFO interrupt service routine for GA10B.
	 *
	 * This function is responsible for handling the FIFO interrupts for all runlists
	 * in the GA10B GPU architecture. It processes bad TSG, PBDMA, and context switch
	 * timeout interrupts. It also ensures that any unhandled interrupts are logged.
	 * The function operates in a threaded interrupt context, not in an actual ISR.
	 *
	 * The steps performed by the function are as follows:
	 * -# Check if the software is ready to handle interrupts. If not, clear the runlist
	 *    interrupts using ga10b_fifo_runlist_intr_clear() and return.
	 * -# Acquire the mutex lock to protect the FIFO interrupt handling logic.
	 * -# Iterate over all active runlists.
	 *    -# Read the interrupt status for the current runlist.
	 *    -# If a bad TSG interrupt is pending, handle it using ga10b_fifo_handle_bad_tsg().
	 *    -# Iterate over all possible PBDMA indices.
	 *       -# If a PBDMA interrupt is pending for the current index, handle it using
	 *          ga10b_fifo_pbdma_isr(). If an error occurs during handling, log it and
	 *          break the loop to avoid further processing.
	 *    -# If a context switch timeout interrupt is pending, handle it using
	 *       ga10b_fifo_ctxsw_timeout_isr().
	 *    -# Create a mask of all enabled runlist interrupts for both trees.
	 *    -# Log any unhandled interrupts by comparing the handled interrupts with the
	 *       interrupt enable mask.
	 *    -# Clear the processed interrupts for the current runlist.
	 * -# Release the mutex lock acquired earlier.
	 *
	 * @param [in]  g  Pointer to the GPU driver structure.
	 */
	void (*intr_0_isr)(struct gk20a *g);

	/**
	 * @brief ISR for non-stalling interrupts.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * Interrupt Service Routine for FIFO non-stalling interrupts:
	 * - Read interrupt status.
	 * - Clear channel interrupt if pending.
	 *
	 * @return: #NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE
	 */
	u32  (*intr_1_isr)(struct gk20a *g);

	/**
	 * @brief Initialize and read chip specific HW data.
	 *
	 * @param g [in]	The GPU driver struct.
	 *                      - The function does not perform g parameter validation.
	 *
	 * For gv11b, this pointer is mapped to gv11b_init_fifo_setup_hw().
	 * For ga10b, this pointer is mapped to ga10b_init_fifo_setup_hw().
	 *
	 * @return error as an integer.
	 */
	int (*init_fifo_setup_hw)(struct gk20a *g);

	/**
	 * @brief Initialize FIFO software metadata and mark it ready to be used.
	 *
	 * @param g [in]	The GPU driver struct.
	 *                      - The function does not perform g parameter validation.
	 *
	 * - Check if #nvgpu_fifo.sw_ready is set to true i.e. s/w setup is
	 * already done (pointer to nvgpu_fifo is obtained using g->fifo).
	 * In case setup is ready, return 0, else continue to setup.
	 * - Invoke nvgpu_fifo_setup_sw_common() to perform sw setup.
	 * - Mark FIFO sw setup ready by setting #nvgpu_fifo.sw_ready to true.
	 *
	 * @retval 0 in case of success.
	 * @retval -ENOMEM in case there is not enough memory available.
	 * @retval -EINVAL in case condition variable has invalid value.
	 * @retval -EBUSY in case reference condition variable pointer isn't NULL.
	 * @retval -EFAULT in case any faults occurred while accessing condition
	 * variable or attribute.
	 */
	int (*setup_sw)(struct gk20a *g);

	/**
	 * @see nvgpu_fifo_cleanup_sw().
	 */
	void (*cleanup_sw)(struct gk20a *g);

	/**
	 * @brief Preempts a channel by preempting its associated TSG.
	 *
	 * This function attempts to preempt a given channel. If the channel is bound to a TSG (Thread
	 * Scheduling Group), it preempts the TSG. If the channel is not bound to any TSG, the function
	 * logs this information and returns without any further action, as preempting an unbound channel
	 * is considered a no-operation (NOOP).
	 *
	 * The steps performed by the function are as follows:
	 * -# Retrieve the TSG associated with the given channel using 'nvgpu_tsg_from_ch()'.
	 * -# If the channel is not bound to a TSG, log this information and return 0.
	 * -# If the channel is bound to a TSG, log the channel and TSG IDs.
	 * -# Preempt the TSG by invoking the 'preempt_tsg()' function from the GPU operations structure.
	 *
	 * @param [in]  g       Pointer to the GPU driver structure.
	 * @param [in]  ch      Pointer to the channel structure to be preempted.
	 *
	 * @return 0 if the channel is not bound to a TSG.
	 * @return The return value of 'preempt_tsg()' if the channel is bound to a TSG.
	 */
	int (*preempt_channel)(struct gk20a *g, struct nvgpu_channel *ch);
	/**
	 * @brief Preempt requested channel,tsg or runlist.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param id [in]		Tsg or channel or hardware runlist id
	 * @param id_type [in]		channel,tsg or runlist type
	 *
	 * Depending on given \a id_type:
	 * - Preempt channel
	 * - Preempt tsg
	 * - Preempt runlist
	 *
	 * @return: None
	 */
	void (*preempt_trigger)(struct gk20a *g,
			u32 runlist_id, u32 tsgid, unsigned int id_type);
	int (*check_tsg_preempt_is_complete)(struct gk20a *g, struct nvgpu_runlist *rl);

	/**
	 * @brief Checks if the preemption of a TSG is still pending on the hardware.
	 *
	 * The function verifies if the preemption initiated for a TSG (Thread Scheduling Group)
	 * is still in progress by checking the status on all PBDMA (Push Buffer Direct Memory Access)
	 * and engines associated with the TSG's runlist. It also checks if the preemption is complete
	 * using a specific function pointer if available.
	 *
	 * The steps performed by the function are as follows:
	 * -# Assert that the ID type is specifically for a TSG, as this function only supports TSG preemption.
	 * -# Retrieve the runlist associated with the given runlist ID from the GPU's FIFO structure.
	 * -# Log the initiation of a preempt pending check for the given TSG ID.
	 * -# Iterate over each PBDMA unit served by the runlist and check if preemption is pending using
	 *    gv11b_fifo_preempt_poll_pbdma(). If an error is returned, update the return status.
	 * -# Reset the bitmask that tracks which engines need to be reset due to failed preemption.
	 * -# Iterate over each engine served by the runlist and check if preemption is pending using
	 *    gv11b_fifo_preempt_poll_eng(). If an error is returned and no previous errors have been recorded,
	 *    update the return status.
	 * -# If a function is available to check if TSG preemption is complete, call this function and update
	 *    the return status if an error is returned and no previous errors have been recorded.
	 * -# Return the final status which indicates if preemption is still pending or if any errors occurred.
	 *
	 * @param [in]  g                   Pointer to the GPU structure.
	 * @param [in]  runlist_id          ID of the runlist associated with the TSG.
	 * @param [in]  id                  ID of the TSG.
	 * @param [in]  id_type             Type of ID, expected to be #ID_TYPE_TSG.
	 * @param [in]  preempt_retries_left Indicates if preempt retries are still allowed.
	 *
	 * @return 0 if preemption is not pending or completed successfully.
	 * @return Non-zero error code if preemption is still pending or if an error occurred during the check.
	 */
	int (*is_preempt_pending)(struct gk20a *g, u32 runlist_id, u32 id,
		unsigned int id_type, bool preempt_retries_left);

	/**
	 * @brief Enables or disables top-level FIFO interrupts.
	 *
	 * This function configures the top-level FIFO interrupts by enabling or disabling
	 * the interrupt stall units associated with the runlist trees. It enables the
	 * interrupts for RUNLIST_TREE_0 and disables the interrupts for RUNLIST_TREE_1
	 * when enabling is requested. When disabling is requested, it disables the
	 * interrupts for RUNLIST_TREE_0.
	 *
	 * The steps performed by the function are as follows:
	 * -# If the enable parameter is true, perform the following:
	 *    - Enable the interrupt stall unit for #NVGPU_CIC_INTR_UNIT_RUNLIST_TREE_0
	 *      by calling nvgpu_cic_mon_intr_stall_unit_config().
	 *    - Disable the interrupt stall unit for #NVGPU_CIC_INTR_UNIT_RUNLIST_TREE_1
	 *      by calling nvgpu_cic_mon_intr_stall_unit_config(), as all runlist
	 *      interrupts are routed to runlist_tree_0.
	 * -# If the enable parameter is false, disable the interrupt stall unit for
	 *    #NVGPU_CIC_INTR_UNIT_RUNLIST_TREE_0 by calling
	 *    nvgpu_cic_mon_intr_stall_unit_config().
	 *
	 * @param [in] g         Pointer to the GPU driver struct.
	 * @param [in] enable    Boolean flag to enable or disable top-level FIFO interrupts.
	 */
	void (*intr_top_enable)(struct gk20a *g, bool enable);

	/**
	 * @brief Enables or disables FIFO interrupt 0.
	 *
	 * This function controls the enabling or disabling of FIFO interrupt 0, which
	 * includes disabling runlist interrupts, context switch timeout interrupts, and
	 * PBDMA interrupts. If enabling is requested, it will enable the context switch
	 * timeout and PBDMA interrupts after disabling the runlist interrupts.
	 *
	 * The steps performed by the function are as follows:
	 * -# Disable the runlist interrupts by calling ga10b_fifo_runlist_intr_disable().
	 * -# If the enable parameter is false, perform the following:
	 *    - Disable the context switch timeout interrupts by calling
	 *      g->ops.fifo.ctxsw_timeout_enable() with false.
	 *    - Disable the PBDMA interrupts by calling g->ops.pbdma.intr_enable() with false.
	 *    - Return from the function as no further action is required for disabling.
	 * -# If the enable parameter is true, perform the following:
	 *    - Enable the context switch timeout interrupts by calling
	 *      g->ops.fifo.ctxsw_timeout_enable() with true.
	 *    - Enable the PBDMA interrupts by calling g->ops.pbdma.intr_enable() with true.
	 *    - Enable the runlist interrupts by calling ga10b_fifo_runlist_intr_enable().
	 *
	 * @param [in] g       Pointer to the GPU driver struct.
	 * @param [in] enable  Boolean flag to enable or disable FIFO interrupt 0.
	 */
	void (*intr_0_enable)(struct gk20a *g, bool enable);

	/**
	 * Does nothing on ga10b
	 */
	void (*intr_1_enable)(struct gk20a *g, bool enable);

	/**
	 * @brief Enables or disables the context switch timeout mechanism.
	 *
	 * This function configures the context switch timeout mechanism based on the
	 * 'enable' parameter. When enabled, it sets the timeout period and enables
	 * detection. When disabled, it disables detection and sets the timeout period
	 * to the maximum value. The timeout period is scaled according to the platform
	 * if it is running on silicon.
	 *
	 * The steps performed by the function are as follows:
	 * -# Check if the context switch timeout mechanism is to be enabled.
	 *    -# If enabling and the platform is silicon, calculate the scaled timeout
	 *       based on the context switch timeout period in milliseconds and the
	 *       conversion factor from milliseconds to microseconds using
	 *       nvgpu_safe_mult_u32() and nvgpu_ptimer_scale().
	 *    -# Set the timeout configuration register value to the scaled timeout and
	 *       enable the detection feature using bitwise OR of the appropriate
	 *       configuration flags.
	 *    -# If enabling and the platform is not silicon, set the timeout
	 *       configuration register value to the maximum timeout and enable the
	 *       detection feature using bitwise OR of the appropriate configuration flags.
	 *    -# Call ga10b_fifo_ctxsw_timeout_clear_and_enable() with the configured
	 *       timeout value to clear any pending timeouts and enable the new configuration.
	 * -# If the context switch timeout mechanism is to be disabled:
	 *    -# Set the timeout configuration register value to disable the detection
	 *       feature and set the timeout period to the maximum value using bitwise OR
	 *       of the appropriate configuration flags.
	 *    -# Call ga10b_fifo_ctxsw_timeout_disable_and_clear() with the configured
	 *       timeout value to disable the timeout detection and clear any pending timeouts.
	 *
	 * @param [in]  g       Pointer to the GPU driver structure.
	 * @param [in]  enable  Boolean flag to enable or disable the context switch timeout.
	 */
	void (*ctxsw_timeout_enable)(struct gk20a *g, bool enable);
	u32 (*ctxsw_timeout_config)(u32 rleng_id);
	u32 (*ctxsw_timeout_config_size_1)(void);

	/**
	 * @brief Maps an MMU fault ID to a corresponding PBDMA ID.
	 *
	 * This function iterates over all PBDMA units to find the one that matches the
	 * given MMU fault ID. It returns the PBDMA ID that corresponds to the MMU fault ID.
	 * If no matching PBDMA ID is found, it returns INVAL_ID.
	 *
	 * The steps performed by the function are as follows:
	 * -# Initialize the PBDMA ID to iterate from the first PBDMA unit.
	 * -# Iterate over all PBDMA units by getting the total number of PBDMA units
	 *    using g->ops.pbdma.get_num_of_pbdmas().
	 *    -# For each PBDMA unit, get the MMU fault ID associated with it using
	 *       g->ops.pbdma.get_mmu_fault_id().
	 *    -# Compare the retrieved MMU fault ID with the input MMU fault ID.
	 *    -# If a match is found, return the current PBDMA ID.
	 * -# If no match is found after iterating through all PBDMA units, return INVAL_ID.
	 *
	 * @param [in]  g             Pointer to the GPU driver structure.
	 * @param [in]  mmu_fault_id  The MMU fault ID to map to a PBDMA ID.
	 *
	 * @return The PBDMA ID corresponding to the given MMU fault ID, or INVAL_ID if no match is found.
	 */
	u32  (*mmu_fault_id_to_pbdma_id)(struct gk20a *g,
				u32 mmu_fault_id);

	/**
	 * @brief Retriggers interrupts for all runlists on a specified interrupt tree.
	 *
	 * This function iterates over all active runlists and retriggers the interrupts
	 * on the specified interrupt tree. It is used to ensure that any pending interrupt
	 * handling is re-attempted, which is particularly useful in recovery scenarios.
	 *
	 * The steps performed by the function are as follows:
	 * -# Iterate over all active runlists.
	 *    -# For each runlist, write to the runlist interrupt retrigger register with
	 *       the value that triggers the interrupt again using nvgpu_runlist_writel().
	 *
	 * @param [in]  g          Pointer to the GPU driver structure.
	 * @param [in]  intr_tree  The interrupt tree on which to retrigger interrupts.
	 */
	void (*runlist_intr_retrigger)(struct gk20a *g, u32 intr_tree);


	/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	int (*preempt_poll_pbdma)(struct gk20a *g, u32 tsgid,
			 u32 pbdma_id);
	void (*intr_set_recover_mask)(struct gk20a *g);
	void (*intr_unset_recover_mask)(struct gk20a *g);
	bool (*handle_sched_error)(struct gk20a *g);
	bool (*handle_ctxsw_timeout)(struct gk20a *g);
	void (*trigger_mmu_fault)(struct gk20a *g,
			unsigned long engine_ids_bitmask);
	void (*get_mmu_fault_info)(struct gk20a *g, u32 mmu_fault_id,
		struct mmu_fault_info *mmfault);
	void (*get_mmu_fault_desc)(struct mmu_fault_info *mmfault);
	void (*get_mmu_fault_client_desc)(
				struct mmu_fault_info *mmfault);
	void (*get_mmu_fault_gpc_desc)(struct mmu_fault_info *mmfault);
	u32 (*get_runlist_timeslice)(struct gk20a *g);
	u32 (*get_pb_timeslice)(struct gk20a *g);
	bool (*is_mmu_fault_pending)(struct gk20a *g);
	void (*bar1_snooping_disable)(struct gk20a *g);
	bool (*find_pbdma_for_runlist)(struct gk20a *g,
				       u32 runlist_id, u32 *pbdma_id);
	u32 (*runlist_intr_0_en_clear_tree)(u32 intr_tree);
	u32 (*runlist_intr_0_en_set_tree)(u32 intr_tree);
	void (*wait_for_preempt_before_reset)(struct gk20a *g, u32 runlist_id);
	u32 (*runlist_intr_ctrl)(u32 intr_tree);
	u32 (*runlist_intr_0)(void);
	u32 (*get_runlist_intr_0_mask)(void);
	u32 (*get_runlist_intr_0_recover_mask)(void);
	u32 (*get_runlist_intr_0_recover_unmask)(void);
	u32 (*get_runlist_intr_0_ctxsw_timeout_mask)(void);
	u32 (*get_runlist_intr_bad_tsg)(void);
	u32 (*get_runlist_intr_retrigger_reg_off)(u32 intr_tree);
	u32 (*get_runlist_preempt_reg)(void);
	u32 (*get_rleng_ctxsw_timeout_info_reg_off)(u32 rleng_id);

#ifdef CONFIG_NVGPU_RECOVERY
	void (*recover)(struct gk20a *g, u32 gfid, u32 runlist_id,
		u32 id, unsigned int id_type, unsigned int rc_type,
		bool should_defer_reset);
	void (*rc_cleanup_and_reenable_ctxsw)(struct gk20a *g, u32 gfid, u32 runlists_mask,
			u32 tsgid, bool deferred_reset_pending, unsigned int rc_type);
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
	int (*set_sm_exception_type_mask)(struct nvgpu_channel *ch,
			u32 exception_mask);
#endif
	/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

};

#endif
