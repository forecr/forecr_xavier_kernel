// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/engine_status.h>

bool nvgpu_engine_status_is_ctxsw_switch(struct nvgpu_engine_status_info
		*engine_status)
{
	return engine_status->ctxsw_status == NVGPU_CTX_STATUS_CTXSW_SWITCH;
}

bool nvgpu_engine_status_is_ctxsw_load(struct nvgpu_engine_status_info
		*engine_status)
{
	return engine_status->ctxsw_status == NVGPU_CTX_STATUS_CTXSW_LOAD;
}

bool nvgpu_engine_status_is_ctxsw_save(struct nvgpu_engine_status_info
		*engine_status)
{
	return	engine_status->ctxsw_status == NVGPU_CTX_STATUS_CTXSW_SAVE;
}

bool nvgpu_engine_status_is_ctxsw(struct nvgpu_engine_status_info
		*engine_status)
{
	return (nvgpu_engine_status_is_ctxsw_switch(engine_status) ||
		nvgpu_engine_status_is_ctxsw_load(engine_status) ||
		nvgpu_engine_status_is_ctxsw_save(engine_status));
}

bool nvgpu_engine_status_is_ctxsw_invalid(struct nvgpu_engine_status_info
		*engine_status)
{
	return engine_status->ctxsw_status == NVGPU_CTX_STATUS_INVALID;
}

bool nvgpu_engine_status_is_ctxsw_valid(struct nvgpu_engine_status_info
		*engine_status)
{
	return engine_status->ctxsw_status == NVGPU_CTX_STATUS_VALID;
}
bool nvgpu_engine_status_is_ctx_type_tsg(struct nvgpu_engine_status_info
		*engine_status)
{
	return engine_status->ctx_id_type == ENGINE_STATUS_CTX_ID_TYPE_TSGID;
}
bool nvgpu_engine_status_is_next_ctx_type_tsg(struct nvgpu_engine_status_info
		*engine_status)
{
	return engine_status->ctx_next_id_type ==
		ENGINE_STATUS_CTX_NEXT_ID_TYPE_TSGID;
}

void nvgpu_engine_status_get_ctx_id_type(struct nvgpu_engine_status_info
		*engine_status, u32 *ctx_id, u32 *ctx_type)
{
	*ctx_id = engine_status->ctx_id;
	*ctx_type = engine_status->ctx_id_type;
}

void nvgpu_engine_status_get_next_ctx_id_type(struct nvgpu_engine_status_info
		*engine_status, u32 *ctx_next_id,
		u32 *ctx_next_type)
{
	*ctx_next_id = engine_status->ctx_next_id;
	*ctx_next_type = engine_status->ctx_next_id_type;
}
