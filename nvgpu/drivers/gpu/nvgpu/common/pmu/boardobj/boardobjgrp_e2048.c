// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp_e2048.h>

static int boardobjgrp_pmu_hdr_data_init_e2048(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu,
		struct boardobjgrpmask *mask)
{
	struct nv_pmu_boardobjgrp_e2048 *pgrpe2048 =
		(struct nv_pmu_boardobjgrp_e2048 *)(void *)pboardobjgrppmu;
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
				&pgrpe2048->obj_mask.super);
	if (status != 0) {
		nvgpu_err(g, "e2048 init:failed export grpmask");
		return status;
	}

	return nvgpu_boardobjgrp_pmu_hdr_data_init_super(g,
			pboardobjgrp, pboardobjgrppmu, mask);
}

int nvgpu_boardobjgrp_construct_e2048(struct gk20a *g,
			      struct boardobjgrp_e2048 *pboardobjgrp_e2048)
{
	int status = 0;
	u16  objslots;

	nvgpu_log_info(g, " ");

	objslots = 2048;
	status = boardobjgrpmask_e2048_init(&pboardobjgrp_e2048->mask, NULL);
	if (status != 0) {
		goto nvgpu_boardobjgrpconstruct_e2048_exit;
	}

	pboardobjgrp_e2048->super.type      = CTRL_BOARDOBJGRP_TYPE_E2048;
	pboardobjgrp_e2048->super.ppobjects = pboardobjgrp_e2048->objects;
	pboardobjgrp_e2048->super.objslots  = objslots;
	pboardobjgrp_e2048->super.mask     = &(pboardobjgrp_e2048->mask.super);

	status = nvgpu_boardobjgrp_construct_super(g, &pboardobjgrp_e2048->super);
	if (status != 0) {
		goto nvgpu_boardobjgrpconstruct_e2048_exit;
	}

	pboardobjgrp_e2048->super.pmuhdrdatainit =
		boardobjgrp_pmu_hdr_data_init_e2048;

nvgpu_boardobjgrpconstruct_e2048_exit:
	return status;
}

