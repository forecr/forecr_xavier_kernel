// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pbdma_status.h>
#include <nvgpu/string.h>

#include <nvgpu/hw/gm20b/hw_fifo_gm20b.h>

#include "pbdma_status_gm20b.h"

static void gm20b_populate_invalid_chsw_status_info(
		struct nvgpu_pbdma_status_info *status_info)
{
	status_info->id = PBDMA_STATUS_ID_INVALID;
	status_info->id_type = PBDMA_STATUS_ID_TYPE_INVALID;
	status_info->next_id = PBDMA_STATUS_NEXT_ID_INVALID;
	status_info->next_id_type = PBDMA_STATUS_NEXT_ID_TYPE_INVALID;
	status_info->chsw_status = NVGPU_PBDMA_CHSW_STATUS_INVALID;
}

static void gm20b_populate_valid_chsw_status_info(
		struct nvgpu_pbdma_status_info *status_info)
{
	bool id_type_tsg;
	u32 engine_status = status_info->pbdma_reg_status;

	status_info->id = fifo_pbdma_status_id_v(engine_status);
	id_type_tsg = fifo_pbdma_status_id_type_v(engine_status) ==
			fifo_pbdma_status_id_type_tsgid_v();
	status_info->id_type =
		id_type_tsg ? PBDMA_STATUS_ID_TYPE_TSGID :
			PBDMA_STATUS_ID_TYPE_CHID;
	status_info->next_id = PBDMA_STATUS_NEXT_ID_INVALID;
	status_info->next_id_type = PBDMA_STATUS_NEXT_ID_TYPE_INVALID;
	status_info->chsw_status = NVGPU_PBDMA_CHSW_STATUS_VALID;
}

static void gm20b_populate_load_chsw_status_info(
		struct nvgpu_pbdma_status_info *status_info)
{
	bool next_id_type_tsg;
	u32 engine_status = status_info->pbdma_reg_status;

	status_info->id = PBDMA_STATUS_ID_INVALID;
	status_info->id_type = PBDMA_STATUS_ID_TYPE_INVALID;
	status_info->next_id = fifo_pbdma_status_next_id_v(engine_status);
	next_id_type_tsg = fifo_pbdma_status_next_id_type_v(engine_status) ==
			fifo_pbdma_status_next_id_type_tsgid_v();
	status_info->next_id_type =
		next_id_type_tsg ? PBDMA_STATUS_NEXT_ID_TYPE_TSGID :
			PBDMA_STATUS_NEXT_ID_TYPE_CHID;
	status_info->chsw_status = NVGPU_PBDMA_CHSW_STATUS_LOAD;
}

static void gm20b_populate_save_chsw_status_info(
		struct nvgpu_pbdma_status_info *status_info)
{
	bool id_type_tsg;
	u32 engine_status = status_info->pbdma_reg_status;

	status_info->id = fifo_pbdma_status_id_v(engine_status);
	id_type_tsg = fifo_pbdma_status_id_type_v(engine_status) ==
			fifo_pbdma_status_id_type_tsgid_v();
	status_info->id_type =
		id_type_tsg ? PBDMA_STATUS_ID_TYPE_TSGID :
			PBDMA_STATUS_ID_TYPE_CHID;
	status_info->next_id = PBDMA_STATUS_NEXT_ID_INVALID;
	status_info->next_id_type = PBDMA_STATUS_NEXT_ID_TYPE_INVALID;
	status_info->chsw_status = NVGPU_PBDMA_CHSW_STATUS_SAVE;
}

static void gm20b_populate_switch_chsw_status_info(
		struct nvgpu_pbdma_status_info *status_info)
{
	bool id_type_tsg;
	bool next_id_type_tsg;
	u32 engine_status = status_info->pbdma_reg_status;

	status_info->id = fifo_pbdma_status_id_v(engine_status);
	id_type_tsg = fifo_pbdma_status_id_type_v(engine_status) ==
			fifo_pbdma_status_id_type_tsgid_v();
	status_info->id_type =
		id_type_tsg ? PBDMA_STATUS_ID_TYPE_TSGID :
			PBDMA_STATUS_ID_TYPE_CHID;
	status_info->next_id = fifo_pbdma_status_next_id_v(engine_status);
	next_id_type_tsg = fifo_pbdma_status_next_id_type_v(engine_status) ==
			fifo_pbdma_status_next_id_type_tsgid_v();
	status_info->next_id_type =
		next_id_type_tsg ? PBDMA_STATUS_NEXT_ID_TYPE_TSGID :
			PBDMA_STATUS_NEXT_ID_TYPE_CHID;
	status_info->chsw_status = NVGPU_PBDMA_CHSW_STATUS_SWITCH;
}

void gm20b_read_pbdma_status_info(struct gk20a *g, u32 pbdma_id,
		struct nvgpu_pbdma_status_info *status)
{
	u32 pbdma_reg_status;
	u32 pbdma_channel_status;

	(void) memset(status, 0, sizeof(*status));

	pbdma_reg_status = nvgpu_readl(g, fifo_pbdma_status_r(pbdma_id));

	status->pbdma_reg_status = pbdma_reg_status;

	/* populate the chsw related info */
	pbdma_channel_status = fifo_pbdma_status_chan_status_v(
		pbdma_reg_status);

	status->pbdma_channel_status = pbdma_channel_status;

	if (pbdma_channel_status == fifo_pbdma_status_chan_status_valid_v()) {
		gm20b_populate_valid_chsw_status_info(status);
	} else if (pbdma_channel_status ==
			fifo_pbdma_status_chan_status_chsw_load_v()) {
		gm20b_populate_load_chsw_status_info(status);
	} else if (pbdma_channel_status ==
			fifo_pbdma_status_chan_status_chsw_save_v()) {
		gm20b_populate_save_chsw_status_info(status);
	} else if (pbdma_channel_status ==
			fifo_pbdma_status_chan_status_chsw_switch_v()) {
		gm20b_populate_switch_chsw_status_info(status);
	} else {
		gm20b_populate_invalid_chsw_status_info(status);
	}
}
