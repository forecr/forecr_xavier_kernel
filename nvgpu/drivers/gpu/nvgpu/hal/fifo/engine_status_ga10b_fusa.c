// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/debug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engine_status.h>
#include <nvgpu/engines.h>
#include <nvgpu/device.h>
#include <nvgpu/fifo.h>
#include <nvgpu/string.h>

#include "engine_status_ga10b.h"
#include <nvgpu/hw/ga10b/hw_runlist_ga10b.h>

/**
 * @brief Populates the engine status structure with invalid context switch (ctxsw) information.
 *
 * This function sets all fields of the provided nvgpu_engine_status_info structure to
 * indicate that the context switch status is invalid. This is typically used to initialize
 * the structure when no valid context switch information is available.
 *
 * The steps performed by the function are as follows:
 * -# Set the context ID in the status structure to #ENGINE_STATUS_CTX_ID_INVALID.
 * -# Set the context ID type in the status structure to #ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID.
 * -# Set the next context ID in the status structure to #ENGINE_STATUS_CTX_NEXT_ID_INVALID.
 * -# Set the next context ID type in the status structure to
 *    #ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID.
 * -# Set the context switch status in the status structure to #NVGPU_CTX_STATUS_INVALID.
 *
 * @param [in,out] status_info  Pointer to the structure where the invalid context switch status is
 *                              to be populated.
 */
static void populate_invalid_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	status_info->ctx_id = ENGINE_STATUS_CTX_ID_INVALID;
	status_info->ctx_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID;
	status_info->gfid = ENGINE_STATUS_GFID_INVALID;
	status_info->ctx_next_id =
		ENGINE_STATUS_CTX_NEXT_ID_INVALID;
	status_info->ctx_next_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID;
	status_info->next_gfid = ENGINE_STATUS_GFID_INVALID;
	status_info->ctxsw_status = NVGPU_CTX_STATUS_INVALID;
}

/**
 * @brief Populates the engine status structure with valid context switch (ctxsw) information.
 *
 * This function updates the provided nvgpu_engine_status_info structure with valid context
 * switch information. It sets the current context ID and type based on the engine status
 * register data and marks the next context ID and type as invalid, indicating that there
 * is no pending context switch. The ctxsw status is set to valid.
 *
 * The steps performed by the function are as follows:
 * -# Extract the current context ID from the engine status register data using the
 *    'runlist_engine_status0_tsgid_v()' function and store it in the status structure.
 * -# Set the context ID type in the status structure to #ENGINE_STATUS_CTX_ID_TYPE_TSGID.
 * -# Set the next context ID in the status structure to #ENGINE_STATUS_CTX_NEXT_ID_INVALID.
 * -# Set the next context ID type in the status structure to
 *    #ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID.
 * -# Set the context switch status in the status structure to #NVGPU_CTX_STATUS_VALID.
 *
 * @param [in,out] status_info  Pointer to the structure where the valid context switch status is to
 *                              be populated.
 */
static void populate_valid_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	status_info->ctx_id =
		runlist_engine_status0_tsgid_v(status_info->reg_data);
	status_info->ctx_id_type = ENGINE_STATUS_CTX_ID_TYPE_TSGID;
	status_info->gfid = runlist_engine_status1_gfid_v(status_info->reg1_data);
	status_info->ctx_next_id =
		ENGINE_STATUS_CTX_NEXT_ID_INVALID;
	status_info->ctx_next_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID;
	status_info->next_gfid = ENGINE_STATUS_GFID_INVALID;
	status_info->ctxsw_status = NVGPU_CTX_STATUS_VALID;
}

/**
 * @brief Populates the engine status structure with context switch load information.
 *
 * This function updates the provided nvgpu_engine_status_info structure to reflect that a
 * context switch load operation is in progress. It sets the current context ID and type to
 * invalid, indicating that the current context is not valid, and sets the next context ID
 * and type based on the engine status register data, indicating the context that is being
 * loaded. The ctxsw status is set to NVGPU_CTX_STATUS_CTXSW_LOAD to represent the load status.
 *
 * The steps performed by the function are as follows:
 * -# Set the current context ID in the status structure to #ENGINE_STATUS_CTX_ID_INVALID.
 * -# Set the current context ID type in the status structure to #ENGINE_STATUS_CTX_ID_TYPE_INVALID.
 * -# Extract the next context ID from the engine status register data using the
 *    'runlist_engine_status0_next_tsgid_v()' function and store it in the status structure.
 * -# Set the next context ID type in the status structure to #ENGINE_STATUS_CTX_NEXT_ID_TYPE_TSGID.
 * -# Set the context switch status in the status structure to #NVGPU_CTX_STATUS_CTXSW_LOAD.
 *
 * @param [in,out] status_info  Pointer to the structure where the context switch load status is to
 *                              be populated.
 */
static void populate_load_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	status_info->ctx_id = ENGINE_STATUS_CTX_ID_INVALID;
	status_info->ctx_id_type = ENGINE_STATUS_CTX_ID_TYPE_INVALID;
	status_info->gfid = ENGINE_STATUS_GFID_INVALID;
	status_info->ctx_next_id =
		runlist_engine_status0_next_tsgid_v(status_info->reg_data);
	status_info->ctx_next_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_TSGID;
	status_info->next_gfid =
		runlist_engine_status1_next_gfid_v(status_info->reg1_data);
	status_info->ctxsw_status = NVGPU_CTX_STATUS_CTXSW_LOAD;
}

/**
 * @brief Populates the engine status structure with context switch save information.
 *
 * This function updates the provided nvgpu_engine_status_info structure to reflect that a
 * context switch save operation is in progress. It sets the current context ID and type
 * based on the engine status register data, indicating the context that is being saved,
 * and marks the next context ID and type as invalid, indicating that there is no pending
 * context to be loaded. The ctxsw status is set to NVGPU_CTX_STATUS_CTXSW_SAVE to represent
 * the save status.
 *
 * The steps performed by the function are as follows:
 * -# Extract the current context ID from the engine status register data using the
 *    'runlist_engine_status0_tsgid_v()' function and store it in the status structure.
 * -# Set the current context ID type in the status structure to #ENGINE_STATUS_CTX_ID_TYPE_TSGID.
 * -# Set the next context ID in the status structure to #ENGINE_STATUS_CTX_NEXT_ID_INVALID.
 * -# Set the next context ID type in the status structure to
 *    #ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID.
 * -# Set the context switch status in the status structure to #NVGPU_CTX_STATUS_CTXSW_SAVE.
 *
 * @param [in,out] status_info  Pointer to the structure where the context switch save status is to
 *                              be populated.
 */
static void populate_save_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	status_info->ctx_id =
		runlist_engine_status0_tsgid_v(status_info->reg_data);
	status_info->ctx_id_type = ENGINE_STATUS_CTX_ID_TYPE_TSGID;
	status_info->gfid = runlist_engine_status1_gfid_v(status_info->reg1_data);
	status_info->ctx_next_id =
		ENGINE_STATUS_CTX_NEXT_ID_INVALID;
	status_info->ctx_next_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID;
	status_info->next_gfid = ENGINE_STATUS_GFID_INVALID;
	status_info->ctxsw_status = NVGPU_CTX_STATUS_CTXSW_SAVE;
}

/**
 * @brief Populates the engine status structure with context switch information for a switch operation.
 *
 * This function updates the provided nvgpu_engine_status_info structure to reflect that a
 * context switch operation is in progress, where one context is being switched out and another
 * is being switched in. It sets both the current and next context IDs and types based on the
 * engine status register data. The ctxsw status is set to NVGPU_CTX_STATUS_CTXSW_SWITCH to
 * represent the switch status.
 *
 * The steps performed by the function are as follows:
 * -# Extract the current context ID from the engine status register data using the
 *    'runlist_engine_status0_tsgid_v()' function and store it in the status structure.
 * -# Set the current context ID type in the status structure to #ENGINE_STATUS_CTX_ID_TYPE_TSGID.
 * -# Extract the next context ID from the engine status register data using the
 *    'runlist_engine_status0_next_tsgid_v()' function and store it in the status structure.
 * -# Set the next context ID type in the status structure to #ENGINE_STATUS_CTX_NEXT_ID_TYPE_TSGID.
 * -# Set the context switch status in the status structure to #NVGPU_CTX_STATUS_CTXSW_SWITCH.
 *
 * @param [in,out] status_info  Pointer to the structure where the context switch information for a
 *                              switch operation is to be populated.
 */
static void populate_switch_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	status_info->ctx_id =
		runlist_engine_status0_tsgid_v(status_info->reg_data);
	status_info->ctx_id_type = ENGINE_STATUS_CTX_ID_TYPE_TSGID;
	status_info->gfid = runlist_engine_status1_gfid_v(status_info->reg1_data);
	status_info->ctx_next_id =
		runlist_engine_status0_next_tsgid_v(status_info->reg_data);
	status_info->ctx_next_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_TSGID;
	status_info->next_gfid =
		runlist_engine_status1_next_gfid_v(status_info->reg1_data);
	status_info->ctxsw_status = NVGPU_CTX_STATUS_CTXSW_SWITCH;
}

u32 ga10b_rleng_status0_reg_off(u32 rleng_id)
{
	return runlist_engine_status0_r(rleng_id);
}

u32 ga10b_rleng_status1_reg_off(u32 rleng_id)
{
	return runlist_engine_status1_r(rleng_id);
}

void ga10b_read_engine_status_info(struct gk20a *g, u32 engine_id,
		struct nvgpu_engine_status_info *status)
{
	u32 engine_reg0_data;
	u32 engine_reg1_data;
	u32 ctxsw_state;
	const struct nvgpu_device *dev;

	(void) memset(status, 0, sizeof(*status));

	if (!nvgpu_engine_check_valid_id(g, engine_id)) {
		/* just return NULL info */
		return;
	}

	dev = g->fifo.host_engines[engine_id];

	engine_reg0_data = nvgpu_readl(g, nvgpu_safe_add_u32(
		dev->rl_pri_base,
		g->ops.engine_status.get_rleng_status0_reg_off(dev->rleng_id)));

	engine_reg1_data = nvgpu_readl(g, nvgpu_safe_add_u32(
		dev->rl_pri_base,
		g->ops.engine_status.get_rleng_status1_reg_off(dev->rleng_id)));

	status->reg_data = engine_reg0_data;
	status->reg1_data = engine_reg1_data;

	/* populate the engine_state enum */
	status->is_busy = runlist_engine_status0_engine_v(engine_reg0_data) ==
			runlist_engine_status0_engine_busy_v();

	/* populate the engine_faulted_state enum */
	status->is_faulted =
			runlist_engine_status0_faulted_v(engine_reg0_data) ==
			runlist_engine_status0_faulted_true_v();

	/* populate the ctxsw_in_progress_state */
	status->ctxsw_in_progress = ((engine_reg0_data &
			runlist_engine_status0_ctxsw_in_progress_f()) != 0U);

	/* populate the ctxsw related info */
	ctxsw_state = runlist_engine_status0_ctx_status_v(engine_reg0_data);

	status->ctxsw_state = ctxsw_state;

	/* check for ctx_status switch/load/save before valid */
	if (ctxsw_state ==
			runlist_engine_status0_ctx_status_switch_v()) {
		populate_switch_ctxsw_status_info(status);
	} else if (ctxsw_state ==
			runlist_engine_status0_ctx_status_load_v()) {
		populate_load_ctxsw_status_info(status);
	} else if (ctxsw_state ==
			runlist_engine_status0_ctx_status_save_v()) {
		populate_save_ctxsw_status_info(status);
	} else if (ctxsw_state == runlist_engine_status0_ctx_status_valid_v()) {
		populate_valid_ctxsw_status_info(status);
	} else {
		populate_invalid_ctxsw_status_info(status);
	}
}
