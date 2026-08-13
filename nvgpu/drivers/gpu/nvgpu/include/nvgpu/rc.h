/*
 * Copyright (c) 2011-2022, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_RC_H
#define NVGPU_RC_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * Some hardware faults that halt the gpu are recoverable i.e. after the fault
 * is hit, some hardware/software sequence has to be followed by the nvgpu
 * driver which can make the gpu resume its operation. This file describes such
 * recovery APIs.
 */

/**
 * @defgroup NVGPU_RC_TYPES_DEFINES
 * @ingroup NVGPU_RC_TYPES_DEFINES
 * @{
 */

/**
 * No recovery.
 */
#define RC_TYPE_NO_RC			0U
/**
 * MMU fault recovery.
 */
#define RC_TYPE_MMU_FAULT		1U
/**
 * PBDMA fault recovery.
 */
#define RC_TYPE_PBDMA_FAULT		2U
/**
 * GR fault recovery.
 */
#define RC_TYPE_GR_FAULT		3U
/**
 * Preemption timeout recovery.
 */
#define RC_TYPE_PREEMPT_TIMEOUT		4U
/**
 * CTXSW timeout recovery.
 */
#define RC_TYPE_CTXSW_TIMEOUT		5U
/**
 * Runlist update timeout recovery.
 */
#define RC_TYPE_RUNLIST_UPDATE_TIMEOUT	6U
/**
 * Forced recovery.
 */
#define RC_TYPE_FORCE_RESET		7U
/**
 * Scheduler error recovery.
 */
#define RC_TYPE_SCHED_ERR		8U
/**
 * Copy-engine error recovery.
 */
#define RC_TYPE_CE_FAULT		9U

/**
 * Invalid recovery id.
 */
#define INVAL_ID			(~U32(0U))

/**
 * @}
 */

/*
 * Requires a string literal for the format - notice the string
 * concatination.
 */
#define rec_dbg(g, fmt, args...)					\
	nvgpu_log((g), gpu_dbg_rec, "REC | " fmt, ##args)

struct gk20a;
struct nvgpu_fifo;
struct nvgpu_tsg;
struct nvgpu_channel;
struct nvgpu_pbdma_status_info;
struct mmu_fault_info;

static inline const char *nvgpu_rc_type_to_str(unsigned int rc_type)
{
	const char *str = NULL;

	switch (rc_type) {
	case RC_TYPE_NO_RC:
		str = "None";
		break;
	case RC_TYPE_MMU_FAULT:
		str = "MMU fault";
		break;
	case RC_TYPE_PBDMA_FAULT:
		str = "PBDMA fault";
		break;
	case RC_TYPE_GR_FAULT:
		str = "GR fault";
		break;
	case RC_TYPE_PREEMPT_TIMEOUT:
		str = "Preemption timeout";
		break;
	case RC_TYPE_CTXSW_TIMEOUT:
		str = "CTXSW timeout";
		break;
	case RC_TYPE_RUNLIST_UPDATE_TIMEOUT:
		str = "RL Update timeout";
		break;
	case RC_TYPE_FORCE_RESET:
		str = "Force reset";
		break;
	case RC_TYPE_SCHED_ERR:
		str = "Sched err";
		break;
	case RC_TYPE_CE_FAULT:
		str = "Copy engine err";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

/**
 * @brief Do recovery processing for ctxsw timeout.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * - The function does not perform validation of \a g parameter.
 * @param eng_bitmask [in]	Engine bitmask.
 * - The function does not perform validation of \a eng_bitmask parameter
 *   because it is not used for safety.
 * @param tsg [in]		Pointer to TSG struct.
 * - The function does not perform validation of \a tsg parameter because it is
 *   passed as is to \ref nvgpu_tsg_set_error_notifier.
 * @param debug_dump [in]	Whether debug dump required or not.
 * - The function does not perform validation of \a debug_dump parameter
 *   because it is not used for safety.
 *
 * Trigger SW/HW sequence to recover from timeout during context switch. For
 * safety, the steps performed by this API are
 * - Call \ref nvgpu_tsg_set_error_notifier
 *   "nvgpu_tsg_set_error_notifier(g, tsg, NVGPU_ERR_NOTIFIER_FIFO_ERROR_IDLE_TIMEOUT)"
 *   to set error notifier for the faulting tsg.
 * - Call \ref WARN_ON "WARN_ON(!g->sw_quiesce_pending)" to print warning if
 *   quiesce is not triggered already.
 */
void nvgpu_rc_ctxsw_timeout(struct gk20a *g, u32 eng_bitmask,
				struct nvgpu_tsg *tsg, bool debug_dump);

/**
 * @brief Do recovery processing for PBDMA fault.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * - The function does not perform validation of \a g parameter.
 * @param pbdma_id [in]		Pbdma identifier.
 * - The function does not perform validation of \a pbdma_id parameter
 *   because it is just used for logging.
 * @param error_notifier [in]	Error notifier type to be set.
 * - The function validates \a error_notifier to be in the range
 *   [0, \ref NVGPU_ERR_NOTIFIER_INVAL].
 * @param pbdma_status [in]	Pointer to PBDMA status info.
 * - The function does not perform validation of \a pbdma_status parameter
 *   because it is not deferenced in this function.
 * Do PBDMA fault recovery. Set error notifier as per \a error_notifier and call
 * \ref nvgpu_rc_tsg_and_related_engines to do the recovery. Steps involved are
 * - If \a error_notifier is >= \ref NVGPU_ERR_NOTIFIER_INVAL, set error variable to
 *   -EINVAL, trigger quiesce \ref nvgpu_sw_quiesce "nvgpu_sw_quiesce(g)" and
 *   jump to label \a out.
 * - If \ref nvgpu_pbdma_status_is_chsw_valid
 *   "nvgpu_pbdma_status_is_chsw_valid(pbdma_status)" or
 *   \ref nvgpu_pbdma_status_is_chsw_save
 *   "nvgpu_pbdma_status_is_chsw_save(pbdma_status)" returns true, set id and
 *   id_type to \ref nvgpu_pbdma_status_info "pbdma_status->id" and
 *   \ref nvgpu_pbdma_status_info "pbdma_status->id_type" respectively.
 * - Else if \ref nvgpu_pbdma_status_is_chsw_load
 *   "nvgpu_pbdma_status_is_chsw_load(pbdma_status)" or
 *    \ref nvgpu_pbdma_status_is_chsw_switch
 *   "nvgpu_pbdma_status_is_chsw_switch(pbdma_status)" returns true, set id and
 *   id_type to \ref nvgpu_pbdma_status_info "pbdma_status->next_id" and
 *   \ref nvgpu_pbdma_status_info "pbdma_status->next_id_type" respectively.
 * - Else if \ref nvgpu_pbdma_status_ch_not_loaded
 *   "nvgpu_pbdma_status_ch_not_loaded(pbdma_status)" returns true, log message
 *   but don't set error variable.
 * - Else log error message and set error variable to -EINVAL and trigger
 *   quiesce \ref nvgpu_sw_quiesce "nvgpu_sw_quiesce(g)".
 * - If id_type set in above steps matches with \ref PBDMA_STATUS_ID_TYPE_TSGID,
 *   call \ref nvgpu_tsg_get_from_id to get
 *   pointer to struct \ref nvgpu_tsg and store in variable tsg, then call
 *   \ref nvgpu_tsg_set_error_notifier
 *   "nvgpu_tsg_set_error_notifier(g, tsg, error_notifier)" to set error
 *   notifier buffer. Finally, call
 *   \ref nvgpu_rc_tsg_and_related_engines
 *   "nvgpu_rc_tsg_and_related_engines(g, tsg, true, RC_TYPE_PBDMA_FAULT)"
 *   to do recovery for tsg and related engines.
 * - If id_type set in above steps matches with \ref PBDMA_STATUS_ID_TYPE_CHID,
 *   call \ref nvgpu_channel_from_id to get
 *   pointer to struct \ref nvgpu_channel and store in variable ch. If ch is NULL
 *   log error, set error variable to -EINVAL, trigger quiesce
 *   \ref nvgpu_sw_quiesce "nvgpu_sw_quiesce(g)" and jump to label \a out else get
 *   pointer to struct \ref nvgpu_tsg using API \ref nvgpu_tsg_from_ch
 *   "nvgpu_tsg_from_ch(ch)" and store in variable tsg. If tsg is NULL log
 *   error, put the channel reference by calling \ref nvgpu_channel_put
 *   "nvgpu_channel_put(ch)", set error variable to -EINVAL, trigger quiesce
 *   \ref nvgpu_sw_quiesce "nvgpu_sw_quiesce(g)" and jump to label \a out else
 *   set error notifier buffer by calling \ref nvgpu_tsg_set_error_notifier
 *   "nvgpu_tsg_set_error_notifier(g, tsg, error_notifier)" followed by doing
 *   recovery by calling \ref nvgpu_rc_tsg_and_related_engines
 *   "nvgpu_rc_tsg_and_related_engines(g, tsg, true, RC_TYPE_PBDMA_FAULT)".
 *   Put the channel reference by calling \ref nvgpu_channel_put
 *   "nvgpu_channel_put(ch)".
 * - If id_type is not set to any of \ref PBMDA_STATUS_ID_TYPE_TSGID or
 *   \ref PBMDA_STATUS_ID_TYPE_CHID, log error and set error variable to -EINVAL,
 *   trigger quiesce \ref nvgpu_sw_quiesce "nvgpu_sw_quiesce(g)"
 * - Return error variable.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -EINVAL in case of following cases:
 * 1. the error_notifier is invalid.
 * 2. the pbdma status is invalid.
 * 3. the channel is not referenceable.
 * 4. the channel is not bound to tsg.
 * 5. the id type or next_id type are not indicating channel id type or tsg id type.
 */
int nvgpu_rc_pbdma_fault(struct gk20a *g, u32 pbdma_id, u32 error_notifier,
			struct nvgpu_pbdma_status_info *pbdma_status);

/**
 * @brief Do recovery processing for runlist update timeout.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * - The function does not perform validation of \a g parameter.
 * @param runlist_id [in]	Runlist identifier.
 * - The function does not perform validation of \a runlist_id parameter
 *   because it is not used for safety.
 *
 * Do runlist update timeout recovery. For safety, just print warning by calling
 * \ref WARN_ON "WARN_ON(!g->sw_quiesce_pending)" if quiesce is not triggered
 * already.
 */
void nvgpu_rc_runlist_update(struct gk20a *g, u32 runlist_id);

/**
 * @brief Do recovery processing for preemption timeout.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * - The function does not perform validation of \a g parameter.
 * @param tsg [in]		Pointer to TSG struct.
 * - The function does not perform validation of \a tsg parameter because it is
 *   passed as is to \ref nvgpu_tsg_set_error_notifier().
 *
 * Do preemption timeout recovery. For safety, following steps are performed
 * - Set error notifier buffer to \ref NVGPU_ERR_NOTIFIER_FIFO_ERROR_IDLE_TIMEOUT by
 *   calling \ref nvgpu_tsg_set_error_notifier
 *   "nvgpu_tsg_set_error_notifier(g, tsg, NVGPU_ERR_NOTIFIER_FIFO_ERROR_IDLE_TIMEOUT)".
 * - Call \ref BUG_ON "BUG_ON(!g->sw_quiesce_pending)" if quiesce is not
 *   triggered already.
 */
void nvgpu_rc_preempt_timeout(struct gk20a *g, struct nvgpu_tsg *tsg);

/**
 * @brief Do recovery processing for GR fault.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * - The function does not perform validation of \a g parameter.
 * @param tsg [in]		Pointer to TSG struct.
 * - The function does not perform validation of \a tsg parameter
 *   because it is not used for safety.
 * @param ch [in]		Pointer to channel struct.
 * - The function does not perform validation of \a ch parameter
 *   because it is not used for safety.
 *
 * Do GR fault recovery. For safety, just print warning by calling
 * \ref WARN_ON "WARN_ON(!g->sw_quiesce_pending)" if quiesce is not triggered
 * already.
 */
void nvgpu_rc_gr_fault(struct gk20a *g,
			struct nvgpu_tsg *tsg, struct nvgpu_channel *ch);

/**
 * @brief Do recovery processing for bad TSG sched error.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * - The function does not perform validation of \a g parameter.
 *
 * Do bad TSG sched error recovery. For safety, just print warning by calling
 * \ref WARN_ON "WARN_ON(!g->sw_quiesce_pending)" if quiesce is not triggered
 * already.
 */
void nvgpu_rc_sched_error_bad_tsg(struct gk20a *g);

/**
 * @brief Do recovery processing for TSG and related engines.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * - The function does not perform validation of \a g parameter.
 * @param tsg [in]		Pointer to TSG struct.
 * - The function does not perform validation of \a tsg parameter
 *   because it is not used for safety.
 * @param debug_dump [in]	Whether debug dump required or not.
 * - The function does not perform validation of \a debug_dump parameter
 *   because it is not used for safety.
 * @param rc_type [in]		Recovery type.
 * - The function does not perform validation of \a rc_type parameter
 *   because it is not used for safety.
 *
 * Do TSG and related engines recovery dependending on the \a rc_type. For
 * safety, just print warning by calling
 * \ref WARN_ON "WARN_ON(!g->sw_quiesce_pending)" if quiesce is not triggered
 * already.
 */
void nvgpu_rc_tsg_and_related_engines(struct gk20a *g, struct nvgpu_tsg *tsg,
			 bool debug_dump, u32 rc_type);

/**
 * @brief Do recovery processing for mmu fault.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * - The function does not perform validation of \a g parameter.
 * @param act_eng_bitmask [in]	Engine bitmask.
 * - The function does not perform validation of \a act_eng_bitmask parameter
 *   because it is just used for logging.
 * @param id [in]		Hw identifier.
 * - The function validates \a id parameter to be less than
 *   \ref nvgpu_fifo "g->fifo.num_channels" i.e. in the range
 *   [0, g->fifo.num_channels).
 * @param id_type [in]		Hw id type.
 * - The function validates \a id_type parameter to be less than or equal to
 *   #ID_TYPE_TSG.
 * @param rc_type [in]		Recovery type.
 * - The function does not perform validation of \a rc_type parameter
 *   because it is not used for safety.
 * @param mmufault [in]		Mmu fault info
 * - The function does not perform validation of \a mmufault parameter.
 *   because it is not used for safety.
 *
 * Do mmu fault recovery dependending on the \a rc_type, \a act_eng_bitmask,
 * \a hw_id and \a id_type.
 * For safety, following steps are performed
 * - If \a id is greater than or equal to \ref nvgpu_fifo
 *   "g->fifo.num_channels" i.e. not in valid range
 *   [0, g->fifo.num_channels), set error variable to -EINVAL and jump to label
 *   \a out.
 * - If \a id_type is greater than #ID_TYPE_TSG i.e. not in valid range
 *   [ID_TYPE_CHANNEL, ID_TYPE_TSG], set error variable to -EINVAL and jump to
 *   label \a out.
 * - Log values of \a id, \a id_type and \a act_eng_bitmask.
 * - If \a id is not \ref INVAL_ID and \a id_type is \ref ID_TYPE_TSG, call
 *   \ref nvgpu_tsg_set_ctx_mmu_error "nvgpu_tsg_set_ctx_mmu_error(g, tsg)"
 *   to set mmu fault in error notifier buffer and call
 *   \ref nvgpu_tsg_mark_error "nvgpu_tsg_mark_error(g, tsg)" to mark error
 *   for all channels belonging to the given tsg. Pointer to struct
 *   \ref nvgpu_tsg is retrieved from \ref nvgpu_fifo "&g->fifo.tsg[id]".
 * - print warning by calling
 *   \ref WARN_ON() "WARN_ON(!g->sw_quiesce_pending)" if quiesce is not
 *   triggered already.
 * - At \a out label, if error variable is set, call \ref nvgpu_sw_quiesce
 *   "nvgpu_sw_quiesce(g)".
 * - Return error variable.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -EINVAL in case \a id or \a id_type is invalid.
 */
int nvgpu_rc_mmu_fault(struct gk20a *g, u32 act_eng_bitmask,
			u32 id, unsigned int id_type, unsigned int rc_type,
			 struct mmu_fault_info *mmufault);

/**
 * @brief The core recovery sequence to teardown faulty TSG
 *
 * @param g [in]		Pointer to GPU driver struct.
 * - The function does not perform validation of \a g parameter.
 * @param eng_bitmask [in]	Engine bitmask.
 * - The function does not perform validation of \a eng_bitmask parameter
 *   because it is not used for safety.
 * @param hw_id [in]		Hardware identifier.
 * - The function does not perform validation of \a hw_id parameter
 *   because it is not used for safety.
 * @param id_is_tsg [in]	Whether hw id is TSG or not.
 * - The function does not perform validation of \a id_is_tsg parameter
 *   because it is not used for safety.
 * @param id_is_known [in]	Whether hw id is known or not.
 * - The function does not perform validation of \a id_is_known parameter
 *   because it is not used for safety.
 * @param debug_dump [in]	Whether debug dump required or not.
 * - The function does not perform validation of \a debug_dump parameter
 *   because it is not used for safety.
 * @param rc_type [in]		Recovery type.
 * - The function does not perform validation of \a rc_type parameter
 *   because it is not used for safety.
 *
 * Perform the manual recommended channel teardown sequence depending on the
 * \a rc_type, \a eng_bitmask, \a hw_id, \a id_is_tsg and \a id_is_known. For
 * safety, just print warning by calling
 * \ref WARN_ON "WARN_ON(!g->sw_quiesce_pending)" if quiesce is not triggered
 * already.
 */
void nvgpu_rc_fifo_recover(struct gk20a *g,
			u32 eng_bitmask, /* if zero, will be queried from HW */
			u32 hw_id, /* if ~0, will be queried from HW */
			bool id_is_tsg, /* ignored if hw_id == ~0 */
			bool id_is_known, bool debug_dump, u32 rc_type);


void nvgpu_rc_ce_fault(struct gk20a *g, u32 inst_id);
#endif /* NVGPU_RC_H */
