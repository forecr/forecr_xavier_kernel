// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/debug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engine_status.h>
#include <nvgpu/engines.h>
#include <nvgpu/fifo.h>
#include <nvgpu/string.h>

#include <nvgpu/hw/gm20b/hw_fifo_gm20b.h>

#include "engine_status_gm20b.h"

static void gm20b_populate_invalid_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	status_info->ctx_id = ENGINE_STATUS_CTX_ID_INVALID;
	status_info->ctx_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID;
	status_info->ctx_next_id =
		ENGINE_STATUS_CTX_NEXT_ID_INVALID;
	status_info->ctx_next_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID;
	status_info->ctxsw_status = NVGPU_CTX_STATUS_INVALID;
}

static void gm20b_populate_valid_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	bool id_type_tsg;
	u32 engine_status = status_info->reg_data;

	status_info->ctx_id =
		fifo_engine_status_id_v(status_info->reg_data);
	id_type_tsg = fifo_engine_status_id_type_v(engine_status) ==
			fifo_engine_status_id_type_tsgid_v();
	status_info->ctx_id_type =
		id_type_tsg ? ENGINE_STATUS_CTX_ID_TYPE_TSGID :
			ENGINE_STATUS_CTX_ID_TYPE_CHID;
	status_info->ctx_next_id =
		ENGINE_STATUS_CTX_NEXT_ID_INVALID;
	status_info->ctx_next_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID;
	status_info->ctxsw_status = NVGPU_CTX_STATUS_VALID;
}

static void gm20b_populate_load_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	bool next_id_type_tsg;
	u32 engine_status = status_info->reg_data;

	status_info->ctx_id = ENGINE_STATUS_CTX_ID_INVALID;
	status_info->ctx_id_type = ENGINE_STATUS_CTX_ID_TYPE_INVALID;
	status_info->ctx_next_id =
		fifo_engine_status_next_id_v(
			status_info->reg_data);
	next_id_type_tsg = fifo_engine_status_next_id_type_v(engine_status) ==
			fifo_engine_status_next_id_type_tsgid_v();
	status_info->ctx_next_id_type =
		next_id_type_tsg ? ENGINE_STATUS_CTX_NEXT_ID_TYPE_TSGID :
			ENGINE_STATUS_CTX_NEXT_ID_TYPE_CHID;
	status_info->ctxsw_status = NVGPU_CTX_STATUS_CTXSW_LOAD;
}

static void gm20b_populate_save_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	bool id_type_tsg;
	u32 engine_status = status_info->reg_data;

	status_info->ctx_id =
		fifo_engine_status_id_v(status_info->reg_data);
	id_type_tsg = fifo_engine_status_id_type_v(engine_status) ==
			fifo_engine_status_id_type_tsgid_v();
	status_info->ctx_id_type =
		id_type_tsg ? ENGINE_STATUS_CTX_ID_TYPE_TSGID :
			ENGINE_STATUS_CTX_ID_TYPE_CHID;
	status_info->ctx_next_id =
		ENGINE_STATUS_CTX_NEXT_ID_INVALID;
	status_info->ctx_next_id_type = ENGINE_STATUS_CTX_NEXT_ID_TYPE_INVALID;
	status_info->ctxsw_status = NVGPU_CTX_STATUS_CTXSW_SAVE;
}

static void gm20b_populate_switch_ctxsw_status_info(
		struct nvgpu_engine_status_info *status_info)
{
	bool id_type_tsg;
	bool next_id_type_tsg;
	u32 engine_status = status_info->reg_data;

	status_info->ctx_id =
		fifo_engine_status_id_v(status_info->reg_data);
	id_type_tsg = fifo_engine_status_id_type_v(engine_status) ==
			fifo_engine_status_id_type_tsgid_v();
	status_info->ctx_id_type =
		id_type_tsg ? ENGINE_STATUS_CTX_ID_TYPE_TSGID :
			ENGINE_STATUS_CTX_ID_TYPE_CHID;
	status_info->ctx_next_id =
		fifo_engine_status_next_id_v(
			status_info->reg_data);
	next_id_type_tsg = fifo_engine_status_next_id_type_v(engine_status) ==
			fifo_engine_status_next_id_type_tsgid_v();
	status_info->ctx_next_id_type =
		next_id_type_tsg ? ENGINE_STATUS_CTX_NEXT_ID_TYPE_TSGID :
			ENGINE_STATUS_CTX_NEXT_ID_TYPE_CHID;
	status_info->ctxsw_status = NVGPU_CTX_STATUS_CTXSW_SWITCH;
}

void gm20b_read_engine_status_info(struct gk20a *g, u32 engine_id,
		struct nvgpu_engine_status_info *status)
{
	u32 engine_reg_data;
	u32 ctxsw_state;

	(void) memset(status, 0, sizeof(*status));

	if (engine_id == NVGPU_INVALID_ENG_ID) {
		/* just return NULL info */
		return;
	}
	engine_reg_data = nvgpu_readl(g, fifo_engine_status_r(engine_id));

	status->reg_data = engine_reg_data;

	/* populate the engine_state enum */
	status->is_busy = fifo_engine_status_engine_v(engine_reg_data) ==
			fifo_engine_status_engine_busy_v();

	/* populate the engine_faulted_state enum */
	status->is_faulted = fifo_engine_status_faulted_v(engine_reg_data) ==
			fifo_engine_status_faulted_true_v();

	/* populate the ctxsw_in_progress_state */
	status->ctxsw_in_progress = ((engine_reg_data &
			fifo_engine_status_ctxsw_in_progress_f()) != 0U);

	/* populate the ctxsw related info */
	ctxsw_state = fifo_engine_status_ctx_status_v(engine_reg_data);

	status->ctxsw_state = ctxsw_state;

	if (ctxsw_state == fifo_engine_status_ctx_status_valid_v()) {
		gm20b_populate_valid_ctxsw_status_info(status);
	} else if (ctxsw_state ==
			fifo_engine_status_ctx_status_ctxsw_load_v()) {
		gm20b_populate_load_ctxsw_status_info(status);
	} else if (ctxsw_state ==
			fifo_engine_status_ctx_status_ctxsw_save_v()) {
		gm20b_populate_save_ctxsw_status_info(status);
	} else if (ctxsw_state ==
			fifo_engine_status_ctx_status_ctxsw_switch_v()) {
		gm20b_populate_switch_ctxsw_status_info(status);
	} else {
		gm20b_populate_invalid_ctxsw_status_info(status);
	}
}
