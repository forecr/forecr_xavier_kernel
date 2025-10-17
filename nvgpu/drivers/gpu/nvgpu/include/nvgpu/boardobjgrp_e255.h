/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BOARDOBJGRP_E255_H
#define NVGPU_BOARDOBJGRP_E255_H

#include <nvgpu/boardobjgrpmask.h>
#include <nvgpu/boardobjgrp.h>

/*
 * boardobjgrp_e255 is @ref BOARDOBJGRP child class allowing storage of up
 * to 255 @ref BOARDOBJ object pointers with single static 255-bit mask denoting
 * valid object pointers.
 */
struct boardobjgrp_e255 {
	struct boardobjgrp  super;
	struct pmu_board_obj *objects[CTRL_BOARDOBJGRP_E255_MAX_OBJECTS];
	struct boardobjgrpmask_e255  mask;
};

#define boardobjgrp_pmu_data_init_e255(g, pboardpbjgrp, pboardobjgrppmu) \
		nvgpu_boardobjgrp_pmu_data_init_super(g, \
		pboardpbjgrp, pboardobjgrppmu)

/* Constructor and destructor */
int nvgpu_boardobjgrp_construct_e255(struct gk20a *g,
	struct boardobjgrp_e255 *pboardobjgrp_e255);
int boardobjgrp_destruct_e255(struct boardobjgrp *pboardobjgrp);

#endif /* NVGPU_BOARDOBJGRP_E255_H */
