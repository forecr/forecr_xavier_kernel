// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp_e32.h>

int boardobjgrp_pmu_hdr_data_init_e32(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu,
		struct boardobjgrpmask *mask)
{
	struct nv_pmu_boardobjgrp_e32 *pgrpe32 =
		(struct nv_pmu_boardobjgrp_e32 *)(void *)pboardobjgrppmu;
	int status;

	nvgpu_log_info(g, " ");

	if (pboardobjgrp == NULL) {
		return -EINVAL;
	}

	if (pboardobjgrppmu == NULL) {
		return -EINVAL;
	}
	status = nvgpu_boardobjgrpmask_export(mask,
				mask->bitcount,
				&pgrpe32->obj_mask.super);
	if (status != 0) {
		nvgpu_err(g, "e32 init:failed export grpmask");
		return status;
	}

	return nvgpu_boardobjgrp_pmu_hdr_data_init_super(g,
			pboardobjgrp, pboardobjgrppmu, mask);
}

int nvgpu_boardobjgrp_construct_e32(struct gk20a *g,
			      struct boardobjgrp_e32 *pboardobjgrp_e32)
{
	int status;
	u8  objslots;

	nvgpu_log_info(g, " ");
	objslots = 32;

	status = boardobjgrpmask_e32_init(&pboardobjgrp_e32->mask, NULL);
	if (status != 0) {
		goto nvgpu_boardobjgrpconstruct_e32_exit;
	}

	pboardobjgrp_e32->super.type      = CTRL_BOARDOBJGRP_TYPE_E32;
	pboardobjgrp_e32->super.ppobjects = pboardobjgrp_e32->objects;
	pboardobjgrp_e32->super.objslots  = objslots;
	pboardobjgrp_e32->super.mask     = &(pboardobjgrp_e32->mask.super);

	status = nvgpu_boardobjgrp_construct_super(g, &pboardobjgrp_e32->super);
	if (status != 0) {
		goto nvgpu_boardobjgrpconstruct_e32_exit;
	}

	pboardobjgrp_e32->super.pmuhdrdatainit =
			boardobjgrp_pmu_hdr_data_init_e32;

nvgpu_boardobjgrpconstruct_e32_exit:
	return status;
}

