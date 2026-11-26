/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BOARDOBJGRP_E2048_H
#define NVGPU_BOARDOBJGRP_E2048_H

#include <nvgpu/boardobjgrpmask.h>
#include <nvgpu/boardobjgrp.h>

/*
 * boardobjgrp_e255 is @ref BOARDOBJGRP child class allowing storage of up
 * to 2048 @ref BOARDOBJ object pointers with single static 2048-bit mask denoting
 * valid object pointers.
 */
struct boardobjgrp_e2048 {
	struct boardobjgrp  super;
	struct pmu_board_obj *objects[CTRL_BOARDOBJGRP_E2048_MAX_OBJECTS];
	struct boardobjgrpmask_e2048  mask;
};

#define boardobjgrp_pmudatainit_e2048(g, pboardpbjgrp, pboardobjgrppmu) \
		nvgpu_boardobjgrp_pmu_data_init_super(g, \
		pboardpbjgrp, pboardobjgrppmu)

/* Constructor and destructor */
int nvgpu_boardobjgrp_construct_e2048(struct gk20a *g,
	struct boardobjgrp_e2048 *pboardobjgrp_e2048);

int boardobjgrp_destruct_e2048(struct boardobjgrp *pboardobjgrp);

#endif /* NVGPU_BOARDOBJGRP_E2000048_H */
