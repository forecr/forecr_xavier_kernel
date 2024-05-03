/*
 * Copyright (c) 2019-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NVGPU_GOPS_PMU_H
#define NVGPU_GOPS_PMU_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_pmu;
struct nvgpu_hw_err_inject_info_desc;

struct gops_pmu_perf {
	int (*handle_pmu_perf_event)(struct gk20a *g, void *pmu_msg);
};

/**
 * PMU unit and engine HAL operations.
 *
 * This structure stores the PMU unit and engine HAL function pointers.
 *
 * @see gpu_ops
 */
struct gops_pmu {
	/**
	 * @brief Initialize PMU unit ECC support.
	 *
	 * @param g      [in] Pointer to GPU driver struct. This function does
	 *                    not perform any validation of this input parameter.
	 *
	 * This function allocates memory to track the ecc error counts
	 * for PMU unit.
	 * + Call \ref nvgpu_ecc_counter_init
	 *   "nvgpu_ecc_counter_init(g, &g->ecc.pmu.stat, pmu_ecc_uncorrected_err_count)"
	 *   and return if failed.
	 * + Call \ref nvgpu_ecc_counter_init
	 *   "nvgpu_ecc_counter_init(g, &g->ecc.pmu.stat, pmu_ecc_corrected_err_count)"
	 *   and return if failed.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 * @retval -ENOMEM if memory allocation for ecc stats fails.
	 */
	int (*ecc_init)(struct gk20a *g);

	/**
	 * @brief Free PMU unit ECC support.
	 *
	 * @param g   [in] Pointer to GPU driver struct. This function does
	 *                 not perform any validation of this input parameter.
	 *
	 * This function deallocates memory allocated for ecc error counts
	 * for PMU unit.
	 * + Assign *ecc = &g->ecc.
	 * + Free \a pmu_ecc_corrected_err_count by calling \ref
	 *   nvgpu_kfree(g, ecc->pmu.pmu_ecc_corrected_err_count).
	 * + Free \a pmu_ecc_uncorrected_err_count by calling \ref
	 *   nvgpu_kfree(g, ecc->pmu.pmu_ecc_uncorrected_err_count);
	 */
	void (*ecc_free)(struct gk20a *g);

	/**
	 * @brief Interrupt handler for PMU interrupts.
	 *
	 * @param g   [in] The GPU driver struct. This function does
	 *                 not perform any validation of this input parameter.
	 *
	 * This function is responsible for handling all PMU interrupts enabled by
	 * \ref gpos_pmu.pmu_enable_irq "g->ops.pmu.pmu_enable_irq()
	 *  + Assign *pmu = g->pmu.
	 *  + Acquire mutex \a &g->pmu->isr_mutex by calling \ref nvgpu_mutex_acquire().
	 *  + Check if \a pmu->isr_enabled is false which implies PMU interrupts are not
	 *    enabled, release \a &g->pmu->isr_mutex by calling
	 *    \ref nvgpu_mutex_release() and return.
	 *  + Prepare mask by AND'ing values obtained by reading registers
	 *    pwr_falcon_irqmask_r() and pwr_falcon_irqdest_r().
	 *  + Read interrupt status register pwr_falcon_irqstat_r() and assign the
	 *    value to intr.
	 *  + Determine interrupts to be handled by AND'ing intr and mask obtained in
	 *    previous steps and assign the value to intr.
	 *  + If no interrupts are to be handled release \a &g->pmu->isr_mutex by calling
	 *    \ref nvgpu_mutex_release() and return.
	 *  + Handle ECC interrupt by calling \ref gops_pmu.handle_ext_irq
	 *    "g->ops.pmu.handle_ext_irq(g, intr)". Before calling check this
	 *    function pointer is not NULL.
	 *  + Clear the pending interrupts to be handled by writing the value of
	 *    intr to the register pwr_falcon_irqsclr_r().
	 *  + Release \a &g->pmu->isr_mutex by calling \ref nvgpu_mutex_release().
	 */
	void (*pmu_isr)(struct gk20a *g);

	/**
	 * @brief PMU early initialization to allocate memory for PMU unit & set PMU
     *        Engine h/w properties, data structs, sub-unit's data structs based
     *        on the detected chip
	 *
	 * @param g   [in] The GPU driver struct. This function does
	 *                 not perform any validation of this input parameter.
	 *
	 * This is gops function pointer to \ref nvgpu_pmu_early_init
	 * "nvgpu_pmu_early_init()" public function.
	 */
	int (*pmu_early_init)(struct gk20a *g);

	/**
	 * @brief Enable/Disable PMU ECC interrupts.
	 *
	 * @param pmu    [in] The PMU unit struct. This function does
	 *                    not perform any validation of this input parameter.
	 * @param enable [in] boolean parameter to enable/disable. This function does
	 *                    not perform any validation of this input parameter.
	 *
	 * + Disable stall interrupts at the master level by calling
	 *   \ref nvgpu_mc_intr_stall_unit_config
	 *   "nvgpu_mc_intr_stall_unit_config(pmu->g, MC_INTR_UNIT_PMU, MC_INTR_DISABLE)"
	 * + Disable PMU ECC interrupt by calling \ref nvgpu_falcon_set_irq
	 *   "nvgpu_falcon_set_irq(pmu->flcn, false, 0x0, 0x0)"
	 * + If enable parameter is set to true then do the following
	 *   + get intr_dest by calling \ref gops_pmu.get_irqdest "gops_pmu.get_irqdest(pmu->g)".
	 *   + Enable stall interrupts at the master level by calling
	 *     \ref nvgpu_mc_intr_stall_unit_config
	 *     "nvgpu_mc_intr_stall_unit_config(pmu->g, MC_INTR_UNIT_PMU, MC_INTR_ENABLE)"
	 *   + Assign intr_mask = pwr_falcon_irqmset_ext_ecc_parity_f(1).
	 *   + Enable PMU ECC interrupt by calling \ref nvgpu_falcon_set_irq
	 *     "nvgpu_falcon_set_irq(pmu->flcn, true, intr_mask, intr_dest)"
	 */
	void (*pmu_enable_irq)(struct nvgpu_pmu *pmu, bool enable);

	/**
	 * @brief Provide ECC interrupt destination fields
	 *
	 * @param g   [in] The GPU driver struct. This function does
	 *                 not perform any validation of this input parameter.
	 *
	 * Provide ECC interrupt destination fields
	 * + Assign intr_dest to the value obtained by OR'ing
	 *   pwr_falcon_irqdest_host_ext_ecc_parity_f(1) and
	 *   pwr_falcon_irqdest_target_ext_ecc_parity_f(0).
	 * + Return intr_dest.
	 */
	u32 (*get_irqdest)(struct gk20a *g);

	/**
	 * @brief Change the PMU Engine reset state.
	 *
	 * @param g        [in] The GPU driver struct. This function does
	 *                 not perform any validation of this input parameter.
	 * @param do_reset [in] The GPU driver struct. This function does
	 *                 not perform any validation of this input parameter.
	 *
	 * PMU Engine reset state change as per input parameter do_reset.
	 * If do_reset is
	 *  + True  - Bring PMU engine out of reset by writing false to
	 *            pwr_falcon_engine_r() register
	 *  + False - Keep PMU falcon/engine in reset by writing true to
	 *            pwr_falcon_engine_r() register
	 */
	void (*reset_engine)(struct gk20a *g, bool do_reset);

	/**
	 * @brief Query the PMU Engine reset state.
	 *
	 * @param g   [in] The GPU driver struct. This function does
	 *            not perform any validation of this input parameter.
	 *
	 * PMU Engine reset state is read and return as below,
	 *  + True  - If PMU engine in reset.
	 *  + False - If PMU engine is out of reset.
	 *
	 * @return True if in reset else False.
	 */
	bool (*is_engine_in_reset)(struct gk20a *g);

	/**
	 * @brief Setup the normal PMU apertures for standardized access.
	 *
	 * @param g   [in] The GPU driver struct. This function does
	 *            not perform any validation of this input parameter.
	 *
	 * Creates a memory aperture that the PMU may use to access memory in
	 * a specific address-space or mapped into the PMU's virtual-address
	 * space. The aperture is identified using a unique index that will
	 * correspond to a single dmaidx in the PMU framebuffer interface.
	 *    + Assign *mm = &g->mm
	 *    + Assign *inst_block = &mm->pmu.inst_block
	 *    + Get ret_mask by calling \ref nvgpu_aperture_mask
	 *      "nvgpu_aperture_mask (g, inst_block,
	 *      pwr_fbif_transcfg_target_noncoherent_sysmem_f(),
	 *      pwr_fbif_transcfg_target_coherent_sysmem_f(),
	 *      pwr_fbif_transcfg_target_local_fb_f()))"
	 *    + Write value (pwr_fbif_transcfg_mem_type_physical_f() | ret_mask) to
	 *      pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_UCODE) register.
	 *    + Write value pwr_fbif_transcfg_mem_type_virtual_f() to register
	 *      pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_VIRT)
	 *    + Write value (pwr_fbif_transcfg_mem_type_physical_f() | ret_mask) to
	 *      pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_PHYS_VID) register.
	 *    + Write value (pwr_fbif_transcfg_mem_type_physical_f() |
	 *      pwr_fbif_transcfg_target_coherent_sysmem_f()) to
	 *      pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_PHYS_SYS_COH) register.
	 *    + Write value (pwr_fbif_transcfg_mem_type_physical_f() |
	 *      pwr_fbif_transcfg_target_noncoherent_sysmem_f()) to
	 *      pwr_fbif_transcfg_r(GK20A_PMU_DMAIDX_PHYS_SYS_NCOH) register.
	 */
	void (*setup_apertures)(struct gk20a *g);

	/**
	 * @brief Handle PMU ECC errors.
	 *
	 * @param g   [in] The GPU driver struct. This function does
	 *            not perform any validation of this input parameter.
	 *
	 * This function handles PMU ECC corrected and un-corrected errors.
	 * + Check and proceed only if pwr_falcon_irqstat_ext_ecc_parity_true_f() bit
	 *   inside parameter \a intr is set to true.
	 * + Assign \a intr1 to the value obtained by reading interrupt stats register
	 *   pwr_pmu_ecc_intr_status_r().
	 * + Check if any corrected and un-corrected ecc errors present in interrupt status
	 *   by AND'ing \a intr1 with (pwr_pmu_ecc_intr_status_corrected_m() |
	 *   pwr_pmu_ecc_intr_status_uncorrected_m()). If no interrupt present return.
	 * + Assign \a ecc_status to the value obtained by reading register
	 *   pwr_pmu_falcon_ecc_status_r().
	 * + Assign \a ecc_addr to the value obtained by reading register
	 *   pwr_pmu_falcon_ecc_address_r().
	 * + Assign \a corrected_cnt to the value obtained by reading register
	 *   pwr_pmu_falcon_ecc_corrected_err_count_r.
	 * + Assign \a uncorrected_cnt to the value obtained by reading register
	 *   pwr_pmu_falcon_ecc_uncorrected_err_count_r.
	 * + Assign corrected_delta = (corrected_cnt & 0xffffU).
	 * + Assign uncorrected_delta = (uncorrected_cnt & 0xffffU).
	 * + Assign \a corrected_overflow to the value obtained by AND'ing ecc_status
	 *   with (U32(0x1U) << 16U)
	 * + Assign \a uncorrected_overflow to the value obtained by AND'ing ecc_status
	 *   with (U32(0x1U) << 18U)
	 * + if \a corrected_overflow is not zero OR pwr_pmu_ecc_intr_status_corrected_m
	 *   bit in \a intr1 is enabled then clear the interrupt by writing to zero to
	 *   pwr_pmu_falcon_ecc_corrected_err_count_r() register.
	 * + if \a uncorrected_overflow is not zero OR pwr_pmu_ecc_intr_status_uncorrected_m
	 *   bit in \a intr1 is enabled then clear the interrupt by writing to zero to
	 *   pwr_pmu_falcon_ecc_uncorrected_err_count_r() register.
	 * + Write pwr_pmu_falcon_ecc_status_r() register with value
	 *   pwr_pmu_falcon_ecc_status_reset_task_f.
	 * + If \a corrected_overflow is not zero then update the counters per slice by
	 *   adding pwr_pmu_falcon_ecc_corrected_err_count_total_s value to
	 *   \a corrected_delta and updating it to the new value.
	 * + If \a uncorrected_overflow is not zero then update the counters per slice
	 *   by adding pwr_pmu_falcon_ecc_uncorrected_err_count_total_s value to
	 *   \a uncorrected_delta and updating it to the new value.
	 * + Add \a corrected_overflow value to \a
	 *   g->ecc.pmu.pmu_ecc_corrected_err_count[0].counter and updating it to
	 *   the new value.
	 * + Add \a uncorrected_overflow value to \a
	 *   g->ecc.pmu.pmu_ecc_uncorrected_err_count[0].counter and updating it to
	 *   the new value.
	 * + Check the corresponding bit in \a ecc_status and if enabled do the
	 *   following
	 *   + bit pwr_pmu_falcon_ecc_status_corrected_err_imem_m() then report
	 *     error by calling \ref nvgpu_report_ecc_err
	 *     "nvgpu_report_ecc_err(g, NVGPU_ERR_MODULE_PMU, 0,
	 *     GPU_PMU_FALCON_IMEM_ECC_CORRECTED, ecc_addr,
	 *     g->ecc.pmu.pmu_ecc_corrected_err_count[0].counter)".
	 *   + bit pwr_pmu_falcon_ecc_status_uncorrected_err_imem_m() then report
	 *     error by calling \ref nvgpu_report_ecc_err "nvgpu_report_ecc_err (g,
	 *     NVGPU_ERR_MODULE_PMU, 0, GPU_PMU_FALCON_IMEM_ECC_UNCORRECTED, ecc_addr,
	 *     g->ecc.pmu.pmu_ecc_uncorrected_err_count[0].counter)" and return -EFAULT.
	 *   + bit pwr_pmu_falcon_ecc_status_corrected_err_dmem_m(), as this is
	 *     not expected and considered as fatal error, release the mutex
	 *     \a &g->pmu->isr_mutex by calling \ref nvgpu_mutex_release() and then
	 *     call BUG().
	 *   + bit pwr_pmu_falcon_ecc_status_uncorrected_err_dmem_m() the report error
	 *     by calling \ref nvgpu_report_ecc_err "nvgpu_report_ecc_err (g,
	 *     NVGPU_ERR_MODULE_PMU, 0, GPU_PMU_FALCON_DMEM_ECC_UNCORRECTED, ecc_addr,
	 *     g->ecc.pmu.pmu_ecc_uncorrected_err_count[0].counter)" and return -EFAULT.
	 *
	 *@retval -EFAULT in case of uncorrected ecc err.
	 *@return 0 in case of success.
	 */
	void (*handle_ext_irq)(struct gk20a *g, u32 intr);

#ifdef CONFIG_NVGPU_POWER_PG
	int (*pmu_restore_golden_img_state)(struct gk20a *g);
#endif
	/** @cond DOXYGEN_SHOULD_SKIP_THIS */

#ifdef CONFIG_NVGPU_LS_PMU
	int (*pmu_rtos_init)(struct gk20a *g);
	int (*pmu_destroy)(struct gk20a *g, struct nvgpu_pmu *pmu);
	int (*pmu_pstate_sw_setup)(struct gk20a *g);
	int (*pmu_pstate_pmu_setup)(struct gk20a *g);
	void (*pmu_seq_cleanup)(struct gk20a *g,
			struct pmu_sequences *sequences,
			struct pmu_sequence *seq);
#endif

	struct nvgpu_hw_err_inject_info_desc * (*get_pmu_err_desc)
		(struct gk20a *g);

	/**
	 * @brief To know PMU Engine complete support is required or not.
	 *
	 * @param g   [in] The GPU driver struct.
	 *
	 * For some build complete PMU Engine enable/support is not required.
	 * Below are the cases explaining where complete PMU Engine support
	 * is required and not required.
	 *  + For PMU RTOS support complete PMU Engine is required.
	 *  + For iGPU FUSA ACR, PMU Engine Falcon is enough.
	 *
	 * On GV11b FUSA, iGPU FUSA ACR is supported and only PMU Falcon
	 * support is enabled from PMU unit.
	 *
	 * + True  - Support the complete PMU Engine and PMU RTOS support.
	 * + False - Only PMU Engine Falcon is supported.
	 *
	 * @return True for complete PMU Engine and PMU RTOS support.
	 *         False for PMU Engine Falcon support only.
	 */
	bool (*is_pmu_supported)(struct gk20a *g);

	/**
	 * @brief Reset the PMU Engine.
	 *
	 * @param g   [in] The GPU driver struct.
	 *
	 * Does the PMU Engine reset to bring into good known state.
	 * The reset sequence also configures PMU Engine clock gating
	 * and interrupts if interrupt support is enabled.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 * @retval -ETIMEDOUT if PMU engine reset times out.
	 */
	int (*pmu_reset)(struct gk20a *g);

	/**
	 * @brief Clears the PMU BAR0 error status.
	 *
	 * @param g   [in] The GPU driver struct.
	 *
	 * Clears the PMU BAR0 error status by reading status
	 * and writing back.
	 */
	void (*pmu_clear_bar0_host_err_status)(struct gk20a *g);

	/** @endcond */

	/**
	 * @brief Fetch base address of PMU Engine Falcon.
	 *
	 * @param void
	 *
	 * @return Chip specific PMU Engine Falcon base address.
	 *         For GV11B, GV11B PMU Engine Falcon base address will be
	 *         returned.
	 * @retval Chip specific PMU Engine Falcon base address.
	 */
	u32 (*falcon_base_addr)(void);

	/**
	 * @brief Fetch base address of PMU Engine Falcon2.
	 *
	 * @param void
	 *
	 * @return Chip specific PMU Engine Falcon2 base address.
	 *         For Ampere+, PMU Engine Falcon2 base address
	 *         will be returned.
	 */
	u32 (*falcon2_base_addr)(void);
	/**
	 * @brief Checks if PMU DEBUG fuse is blown or not
	 *
	 * @param g   [in] The GPU driver struct.
	 *
	 * DEBUG_MODE bit is checked to know what type signature needs to be
	 * used for Falcon ucode verification. DEBUG_MODE bit indicate that
	 * PMU DEBUG fuse is blown and Debug Signal going to the SCP.
	 *  + True  - Use debug signature.
	 *  + False - use production signature.
	 *
	 * @return True if debug else False.
	 */
	bool (*is_debug_mode_enabled)(struct gk20a *g);

	/**
	 * @brief Setup required configuration for PMU Engine Falcon boot.
	 *
	 * @param g   [in] The GPU driver struct.
	 *
	 * Setup required configuration for PMU Engine Falcon boot by
	 * following belwo steps.
	 *  + setup apertures.
	 *  + Clearing mailbox register used for status.
	 *  + Enable the context interface.
	 *  + The instance block setup.
	 */
	void (*flcn_setup_boot_config)(struct gk20a *g);

	/**
	 * @brief Check for the PMU BAR0 error status.
	 *
	 * @param g           [in] The GPU driver struct.
	 * @param bar0_status [out] The status read from register.
	 * @param etype       [out] Specific error listed below.
	 *
	 * etype error:
	 * + #PMU_BAR0_SUCCESS
	 * + PMU_BAR0_HOST_READ_TOUT
	 * + PMU_BAR0_HOST_WRITE_TOUT
	 * + PMU_BAR0_FECS_READ_TOUT
	 * + PMU_BAR0_FECS_WRITE_TOUT
	 * + PMU_BAR0_CMD_READ_HWERR
	 * + PMU_BAR0_CMD_WRITE_HWERR
	 * + PMU_BAR0_READ_HOSTERR
	 * + PMU_BAR0_WRITE_HOSTERR
	 * + PMU_BAR0_READ_FECSERR
	 * + PMU_BAR0_WRITE_FECSERR
	 *
	 * Reads the PMU BAR0 status register and check for error if read
	 * value is not equal to 0x0, below are the different error
	 * listed.
	 *  + TIMEOUT_HOST
	 *    Indicate that HOST does not respond the PRI request from
	 *    falcon2csb interface.
	 *  + TIMEOUT_FECS
	 *    Indicate that FECS does not respond the PRI request from
	 *    falcon2csb interface.
	 *  + CMD_HWERR
	 *    CMD_HWERR error is generated when SW or FW attempts to
	 *    write the DATA, ADDR, or CTL registers to issue a new PRI
	 *    request but the previous PRI request from falcon2csb is
	 *    still busy or bar0master is disabled.
	 *  + HOSTERR
	 *    Indicate that HOST return ERROR back to BAR0MASTER for
	 *    transaction error caused by falcon2csb request.
	 *  + FECSERR
	 *    Indicate that FECS return ERROR back to BAR0MASTER for
	 *    transaction error caused by falcon2csb request.
	 *
	 * @return 0 in case of success, -EIO in case of failure.
	 * @retval -EIO in case of BAR0 error
	 */
	int (*bar0_error_status)(struct gk20a *g, u32 *bar0_status,
		u32 *etype);

	/**
	 * @brief Validate IMEM/DMEM memory integrity.
	 *
	 * @param g   [in] The GPU driver struct.
	 *
	 * Validate IMEM/DMEM memory integrity by checking ECC status
	 * followed IMEM/DEME error correction status check.
	 *
	 * @return True if corrected else False.
	 */
	bool (*validate_mem_integrity)(struct gk20a *g);

	/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	void (*handle_swgen1_irq)(struct gk20a *g, u32 intr);
	u32 (*get_irqmask)(struct gk20a *g);
	void (*set_mailbox1)(struct gk20a *g, u32 val);
	u32 (*get_ecc_address)(struct gk20a *g);
	u32 (*get_ecc_status)(struct gk20a *g);
	void (*set_ecc_status)(struct gk20a *g, u32 val);
	u32 (*get_irqstat)(struct gk20a *g);
	void (*set_irqsclr)(struct gk20a *g, u32 intr);
	void (*set_irqsset)(struct gk20a *g, u32 intr);
	u32 (*get_exterrstat)(struct gk20a *g);
	void (*set_exterrstat)(struct gk20a *g, u32 intr);
	u32 (*get_exterraddr)(struct gk20a *g);
	u32 (*get_bar0_addr)(struct gk20a *g);
	u32 (*get_bar0_data)(struct gk20a *g);
	u32 (*get_bar0_timeout)(struct gk20a *g);
	u32 (*get_bar0_ctl)(struct gk20a *g);
	u32 (*get_bar0_error_status)(struct gk20a *g);
	void (*set_bar0_error_status)(struct gk20a *g, u32 val);
	u32 (*get_bar0_fecs_error)(struct gk20a *g);
	void (*set_bar0_fecs_error)(struct gk20a *g, u32 val);
	u32 (*get_mailbox)(struct gk20a *g, u32 i);
	u32 (*get_pmu_debug)(struct gk20a *g, u32 i);
	u32 (*pmu_get_mutex_reg)(struct gk20a *g, u32 i);
	void (*pmu_set_mutex_reg)(struct gk20a *g, u32 i, u32 data);
	u32 (*pmu_get_mutex_id)(struct gk20a *g);
	u32 (*pmu_get_mutex_id_release)(struct gk20a *g);
	void (*pmu_set_mutex_id_release)(struct gk20a *g, u32 data);
	u32 (*get_pmu_msgq_head)(struct gk20a *g);
	void (*set_pmu_msgq_head)(struct gk20a *g, u32 data);
	void (*set_pmu_new_instblk)(struct gk20a *g, u32 data);

#ifdef CONFIG_NVGPU_LS_PMU
	u32 (*get_inst_block_config)(struct gk20a *g);
	/* ISR */
	bool (*pmu_is_interrupted)(struct nvgpu_pmu *pmu);
	void (*set_irqmask)(struct gk20a *g);
	/* non-secure */
	int (*pmu_ns_bootstrap)(struct gk20a *g, struct nvgpu_pmu *pmu,
		u32 args_offset);
	/* queue */
	u32 (*pmu_get_queue_head)(u32 i);
	u32 (*pmu_get_queue_head_size)(void);
	u32 (*pmu_get_queue_tail_size)(void);
	u32 (*pmu_get_queue_tail)(u32 i);
	int (*pmu_queue_head)(struct gk20a *g, u32 queue_id,
		u32 queue_index, u32 *head, bool set);
	int (*pmu_queue_tail)(struct gk20a *g, u32 queue_id,
		u32 queue_index, u32 *tail, bool set);
	void (*pmu_msgq_tail)(struct nvgpu_pmu *pmu,
		u32 *tail, bool set);
	/* mutex */
	u32 (*pmu_mutex_size)(void);
	u32 (*pmu_mutex_owner)(struct gk20a *g,
		struct pmu_mutexes *mutexes, u32 id);
	int (*pmu_mutex_acquire)(struct gk20a *g,
		struct pmu_mutexes *mutexes, u32 id,
		u32 *token);
	void (*pmu_mutex_release)(struct gk20a *g,
		struct pmu_mutexes *mutexes, u32 id,
		u32 *token);
	/* perfmon */
	void (*pmu_init_perfmon_counter)(struct gk20a *g);
	void (*pmu_pg_idle_counter_config)(struct gk20a *g, u32 pg_engine_id);
	u32  (*pmu_read_idle_counter)(struct gk20a *g, u32 counter_id);
	u32  (*pmu_read_idle_intr_status)(struct gk20a *g);
	void (*pmu_clear_idle_intr_status)(struct gk20a *g);
	void (*pmu_reset_idle_counter)(struct gk20a *g, u32 counter_id);
	/* PG */
	void (*pmu_setup_elpg)(struct gk20a *g);
	/* debug */
	void (*pmu_dump_elpg_stats)(struct nvgpu_pmu *pmu);
	void (*pmu_dump_falcon_stats)(struct nvgpu_pmu *pmu);
	void (*dump_secure_fuses)(struct gk20a *g);
	/**
	 * @brief Start PMU falcon CPU in secure mode.
	 *
	 * @param g   [in] The GPU driver struct.
	 *
	 * Start PMU falcon CPU in secure mode by writing true to
	 * CPUCTL_ALIAS.
	 */
	void (*secured_pmu_start)(struct gk20a *g);
	/**
	 * @brief Setup DMA transfer base address.
	 *
	 * @param g   [in] The GPU driver struct.
	 *
	 * Setup DMA transfer base address as required for chip.
	 */
	void (*write_dmatrfbase)(struct gk20a *g, u32 addr);
#endif
	/** @endcond */
};

#endif
