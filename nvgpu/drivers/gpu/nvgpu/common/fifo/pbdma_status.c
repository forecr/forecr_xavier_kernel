// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/pbdma_status.h>

bool nvgpu_pbdma_status_is_chsw_switch(struct nvgpu_pbdma_status_info
		*pbdma_status)
{
	return pbdma_status->chsw_status == NVGPU_PBDMA_CHSW_STATUS_SWITCH;
}
bool nvgpu_pbdma_status_is_chsw_load(struct nvgpu_pbdma_status_info
		*pbdma_status)
{
	return pbdma_status->chsw_status == NVGPU_PBDMA_CHSW_STATUS_LOAD;
}
bool nvgpu_pbdma_status_is_chsw_save(struct nvgpu_pbdma_status_info
		*pbdma_status)
{
	return pbdma_status->chsw_status == NVGPU_PBDMA_CHSW_STATUS_SAVE;
}
bool nvgpu_pbdma_status_is_chsw_valid(struct nvgpu_pbdma_status_info
		*pbdma_status)
{
	return pbdma_status->chsw_status == NVGPU_PBDMA_CHSW_STATUS_VALID;
}
bool nvgpu_pbdma_status_ch_not_loaded(struct nvgpu_pbdma_status_info
		*pbdma_status)
{
	return pbdma_status->chsw_status == NVGPU_PBDMA_CHSW_STATUS_INVALID;
}
bool nvgpu_pbdma_status_is_id_type_tsg(struct nvgpu_pbdma_status_info
		*pbdma_status)
{
	return pbdma_status->id_type == PBDMA_STATUS_ID_TYPE_TSGID;
}
bool nvgpu_pbdma_status_is_next_id_type_tsg(struct nvgpu_pbdma_status_info
		*pbdma_status)
{
	return pbdma_status->next_id_type == PBDMA_STATUS_NEXT_ID_TYPE_TSGID;
}
