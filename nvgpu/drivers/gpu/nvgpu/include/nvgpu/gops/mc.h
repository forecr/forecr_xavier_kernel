/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_MC_H
#define NVGPU_GOPS_MC_H

#include <nvgpu/types.h>
#include <nvgpu/mc.h>
#include <nvgpu/engines.h>

/**
 * @file
 *
 * MC HAL interface.
 */
struct gk20a;
struct nvgpu_device;

/**
 * MC HAL operations.
 *
 * @see gpu_ops.
 */
struct gops_mc {
	/**
	 * @brief Get the GPU architecture, implementation and revision.
	 *
	 * @param g [in]	The GPU driver struct.
	 * @param arch [out]	GPU architecture level pointer. Can be
	 *			NULL if not needed by caller.
	 * @param impl [out]	GPU architecture pointer. Can be
	 *			NULL if not needed by caller.
	 * @param rev [out]	Chip revision pointer. Can be
	 *			NULL if not needed by the caller.
	 *
	 * This function is invoked to get the GPU architecture, implementation
	 * and revision level of the GPU chip before #nvgpu_finalize_poweron.
	 * These values are used for chip specific SW/HW handling in the
	 * driver.
	 *
	 * Steps:
	 * - Read the register mc_boot_0_r().
	 * - Architecture ID is placed in \arch
	 * - GPU implementation ID is placed in \a impl
	 * - Chip revision is placed in \a rev
	 *
	 * @return value of mc_boot_0_r().
	 */
	u32 (*get_chip_details)(struct gk20a *g,
				u32 *arch, u32 *impl, u32 *rev);

	/**
	 * @brief Read the stalling interrupts status register.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * This function is invoked to get stalling interrupts reported
	 * by the GPU before invoking the ISR.
	 *
	 * Steps:
	 * - Read and return the value of register
	 *   mc_intr_r(#NVGPU_CIC_INTR_STALLING).
	 *
	 * @return value read from mc_intr_r(#NVGPU_CIC_INTR_STALLING).
	 */
	u32 (*intr_stall)(struct gk20a *g);

	/**
	 * @brief Interrupt Service Routine (ISR) for handling the stalling
	 *        interrupts.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * This function is called by OS interrupt unit on receiving
	 * stalling interrupt for servicing it.
	 *
	 * Steps:
	 * - Read mc_intr_r(#NVGPU_CIC_INTR_STALLING) register to get the
	 *   stalling interrupts reported.
	 * - For the FIFO engines with pending interrupt invoke corresponding
	 *   handlers.
	 *   - Invoke g->ops.gr.intr.stall_isr if GR interrupt is pending.
	 *   - Invoke nvgpu_ce_stall_isr if CE interrupt is pending.
	 * - For other units with pending interrupt invoke corresponding
	 *   handlers.
	 *   - Invoke g->ops.fb.intr.isr if HUB interrupt is pending, determined
	 *     by calling g->ops.mc.is_intr_hub_pending.
	 *   - Invoke g->ops.fifo.intr_0_isr if FIFO interrupt is pending. The
	 *     FIFO interrupt bit in mc_intr_r(#NVGPU_CIC_INTR_STALLING) is
	 *     mc_intr_pfifo_pending_f.
	 *   - Invoke g->ops.pmu.pmu_isr if PMU interrupt is pending.
	 *     The PMU interrupt bit in mc_intr_r(#NVGPU_CIC_INTR_STALLING)
	 *     is mc_intr_pmu_pending_f.
	 *   - Invoke g->ops.priv_ring.isr if PRIV_RING interrupt is pending.
	 *     The PRIV_RING interrupt bit in mc_intr_r(#NVGPU_CIC_INTR_STALLING)
	 *     is mc_intr_priv_ring_pending_f.
	 *   - Invoke g->ops.mc.ltc_isr if LTC interrupt is pending. The
	 *     LTC interrupt bit in mc_intr_r(#NVGPU_CIC_INTR_STALLING) is
	 *     mc_intr_ltc_pending_f.
	 *   - Invoke g->ops.bus.isr if BUS interrupt is pending. The
	 *     BUS interrupt bit in mc_intr_r(#NVGPU_CIC_INTR_STALLING) is
	 *     mc_intr_pbus_pending_f.
	 */
	void (*isr_stall)(struct gk20a *g);

	/**
	 * @brief Read the non-stalling interrupts status register.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * This function is invoked to get the non-stalling interrupts reported
	 * by the GPU before invoking the ISR.
	 *
	 * Steps:
	 * - Read and return the value of the register
	 *   mc_intr_r(#NVGPU_CIC_INTR_NONSTALLING).
	 *
	 * @return value read from mc_intr_r(#NVGPU_CIC_INTR_NONSTALLING).
	 */
	u32 (*intr_nonstall)(struct gk20a *g);

	/**
	 * @brief Interrupt Service Routine (ISR) for handling the non-stalling
	 *        interrupts.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * This function is called by OS interrupt unit on receiving
	 * non-stalling interrupt for servicing it.
	 *
	 * Steps:
	 * - Read mc_intr_r(#NVGPU_CIC_INTR_NONSTALLING) register to get the
	 *   non-stalling interrupts reported.
	 * - Invoke g->ops.fifo.intr_1_isr if FIFO non-stalling interrupt
	 *   is pending, determined by calling mc_intr_pfifo_pending_f.
	 * - For the FIFO engines with pending interrupt invoke corresponding
	 *   handlers.
	 *   - Invoke g->ops.gr.intr.nonstall_isr if GR interrupt is pending.
	 *   - Invoke g->ops.ce.isr_nonstall if CE interrupt is pending.
	 *
	 * @return bitmask of operations that are executed on non-stall
	 * workqueue.
	 */
	u32 (*isr_nonstall)(struct gk20a *g);

	/**
	 * @brief Check if stalling or engine interrupts are pending.
	 *
	 * @param g [in]			The GPU driver struct.
	 * @param engine_id [in]		Active engine id.
	 *					- Min: 0
	 *					- Max: NV_HOST_NUM_ENGINES
	 * @param eng_intr_pending [out]	Pointer to get pending
	 *					interrupt.
	 *
	 * This function is invoked while polling for preempt completion.
	 *
	 * Steps:
	 * - Read mc_intr_r(#NVGPU_CIC_INTR_STALLING) register to get
	 *   the interrupts reported.
	 * - Get the engine interrupt mask corresponding to \a engine_id.
	 * - Check if the bits for engine interrupt mask are set in the
	 *   mc_intr_r(#NVGPU_CIC_INTR_STALLING) register by AND'ing values
	 *   read in above two steps. Store the result in \a eng_intr_pending.
	 * - Initialize the stalling interrupt mask with bitmask for FIFO, HUB,
	 *   PRIV_RING, PBUS, LTC unit interrupts.
	 * - Return true if bits from above stalling interrupt mask or the
	 *   engine interrupt mask are set in the
	 *   mc_intr_r(#NVGPU_CIC_INTR_STALLING) register. Else, return false.
	 *
	 * @return true if stalling or engine interrupt is pending, else false.
	 */
	bool (*is_stall_and_eng_intr_pending)(struct gk20a *g,
				u32 engine_id, u32 *eng_intr_pending);

	/**
	 * @brief Interrupt Service Routine (ISR) for handling the Level Two
	 *        Cache (LTC) interrupts.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * This function is invoked to handle the LTC interrupts from
	 * #isr_stall.
	 *
	 * Steps:
	 * - Read mc_intr_ltc_r register to get the interrupts status for LTCs.
	 * - For each ltc from index 0 to nvgpu_ltc_get_ltc_count(\a g)
	 *   - If interrupt bitmask is set in the interrupts status register
	 *     - Invoke g->ops.ltc.intr.isr.
	 */
	void (*ltc_isr)(struct gk20a *g);

	/** @cond DOXYGEN_SHOULD_SKIP_THIS */

	void (*intr_mask)(struct gk20a *g);

	void (*intr_enable)(struct gk20a *g);

	void (*intr_stall_unit_config)(struct gk20a *g, u32 unit,
				bool enable);

	void (*intr_nonstall_unit_config)(struct gk20a *g, u32 unit,
				bool enable);

	void (*intr_stall_pause)(struct gk20a *g);

	void (*intr_stall_resume)(struct gk20a *g);

	void (*intr_nonstall_pause)(struct gk20a *g);

	void (*intr_nonstall_resume)(struct gk20a *g);

	int (*enable_units)(struct gk20a *g, u32 units, bool enable);

	int (*enable_dev)(struct gk20a *g, const struct nvgpu_device *dev,
			bool enable);

	int (*enable_devtype)(struct gk20a *g, u32 devtype, bool enable);

	void (*fbpa_isr)(struct gk20a *g);

#ifdef CONFIG_NVGPU_LS_PMU
	bool (*is_enabled)(struct gk20a *g, u32 unit);
#endif

	bool (*is_intr1_pending)(struct gk20a *g, u32 unit, u32 mc_intr_1);

	bool (*is_mmu_fault_pending)(struct gk20a *g);

	bool (*is_intr_hub_pending)(struct gk20a *g, u32 mc_intr);

#ifdef CONFIG_NVGPU_NON_FUSA
	void (*log_pending_intrs)(struct gk20a *g);
#endif

	void (*fb_reset)(struct gk20a *g);

#ifdef CONFIG_NVGPU_DGPU
	bool (*is_intr_nvlink_pending)(struct gk20a *g, u32 mc_intr);
#endif

	/**
	 * @brief Reset HW engines.
	 *
	 * @param g [in]		The GPU driver struct.
	 * @param devtype [in]		Type of device.
	 *
	 * This function is invoked to reset the engines while initializing
	 * GR, CE and other engines during #nvgpu_finalize_poweron.
	 *
	 * Steps:
	 * - Compute reset mask for all engines of given devtype.
	 * - Disable given HW engines.
	 *   - Acquire g->mc.enable_lock spinlock.
	 *   - Read mc_device_enable_r register and clear the bits in read value
	 *     corresponding to HW engines to be disabled.
	 *   - Write mc_device_enable_r with the updated value.
	 *   - Poll mc_device_enable_r to confirm register write success.
	 *   - Release g->mc.enable_lock spinlock.
	 * - If GR engines are being reset, reset GPCs.
	 * - Enable the HW engines.
	 *   - Acquire g->mc.enable_lock spinlock.
	 *   - Read mc_device_enable_r register and set the bits in read value
	 *     corresponding to HW engines to be enabled.
	 *   - Write mc_device_enable_r with the updated value.
	 *   - Poll mc_device_enable_r to confirm register write success.
	 *   - Release g->mc.enable_lock spinlock.
	 */
	int (*reset_engines_all)(struct gk20a *g, u32 devtype);
	void (*elpg_enable)(struct gk20a *g);

	/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

	/**
	 * @brief Retrieves the interrupt unit information for a given unit.
	 *
	 * This function looks up the vector ID and subtree mask for the specified interrupt
	 * unit. It sets up the interrupt unit information structure with these values,
	 * which are used to handle interrupts for the unit. The function handles different
	 * types of interrupt units, including bus, priv ring, LTC, PMU, FBPA, MMU fault ECC error,
	 * MMU non-replayable fault, MMU non-replayable fault error, MMU info fault, MMU replayable
	 * fault, MMU replayable fault error, GR NONSTALL, CE NONSTALL, GR STALL, CE STALL,
	 * RUNLIST TREE 0, RUNLIST TREE 1, and GSP. It also validates that the vector IDs for a unit
	 * reside in the same subtree.
	 *
	 * The steps performed by the function are as follows:
	 * -# Initialize the return variable to false to indicate failure by default.
	 * -# Obtain a pointer to the interrupt unit information structure for the given unit.
	 * -# Use a switch statement to handle different interrupt units.
	 * -# For recognized interrupt units, retrieve the vector ID using the appropriate
	 *    function and set the vector ID size to one.
	 * -# For MMU interrupt units, note that vector IDs are set up in framebuffer
	 *    initialization and skip further processing.
	 * -# For GR and CE NONSTALL interrupt units, read the base vector ID from the control
	 *    register, calculate the subtree and subtree mask, and mark the interrupt unit
	 *    information as valid.
	 * -# For GR STALL and CE STALL interrupt units, read the base vector ID from the control
	 *    register, calculate the subtree and subtree mask, and mark the interrupt unit
	 *    information as valid.
	 * -# For RUNLIST TREE 0 and RUNLIST TREE 1 interrupt units, log the interrupt unit for
	 *    debugging purposes.
	 * -# For GSP interrupt units, retrieve the vector ID using the appropriate function and
	 *    set the vector ID size to one.
	 * -# If the interrupt unit is not supported, set the return variable to false and
	 *    indicate completion.
	 * -# If the processing for the interrupt unit is complete, skip the remaining steps.
	 * -# For interrupt units with multiple vector IDs, iterate over each vector ID.
	 * -# Validate that all vector IDs for the unit reside in the same subtree.
	 * -# Calculate the subtree mask for each vector ID and combine it with the existing
	 *    subtree mask.
	 * -# Mark the interrupt unit information as valid.
	 * -# Set the return variable to true to indicate success.
	 *
	 * @param [in] g     The GPU structure for which the interrupt unit information is
	 *                   being retrieved.
	 * @param [in] unit  The interrupt unit for which information is being retrieved.
	 *
	 * @return true if the interrupt unit information was successfully retrieved.
	 * @return false if the interrupt unit is not supported or an error occurred.
	 */
	bool (*intr_get_unit_info)(struct gk20a *g, u32 unit);

	/**
	 * @brief To bring dummy GR1 engine out of reset in GA10X_NEXT.
	 *
	 * @param g [in]		The GPU driver struct.
	 * @param devtype [in]		Type of device.
	 * @param enable [in]		Action to perform.
	 *
	 * This function is invoked to disable and enable the GR1 as part of
	 * SW reset for GA10X_NEXT.
	 *
	 */
	void (*gr1_out_of_reset)(struct gk20a *g, u32 devtype, bool enable);

	/**
	 * @brief Retrieve the base vector for engine non-stall interrupts.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * This function is invoked to get the base vector for engine non-stall
	 * interrupts. An engine's non-stall interrupt vector is the sum of this
	 * base vector and the engine's interrupt ID.
	 *
	 * @return the base vector for engine non-stall interrupts.
	 */
	u32 (*get_eng_nonstall_base_vector)(struct gk20a *g);

	/**
	 * @brief Retrieve the base vector for engine stall interrupts.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * This function is invoked to get the base vector for engine stall
	 * interrupts. An engine's stall interrupt vector is the sum of this
	 * base vector and the engine's interrupt ID.
	 *
	 * @return the base vector for engine stall interrupts.
	 */
	u32 (*get_eng_stall_base_vector)(struct gk20a *g);

};

#endif
