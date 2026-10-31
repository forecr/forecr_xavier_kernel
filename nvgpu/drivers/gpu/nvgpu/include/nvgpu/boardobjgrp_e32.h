/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BOARDOBJGRP_E32_H
#define NVGPU_BOARDOBJGRP_E32_H

#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrpmask.h>

/*
 * boardobjgrp_e32 is @ref BOARDOBJGRP child class allowing storage of up to 32
 * @ref BOARDOBJ object pointers with single static 32-bit mask denoting valid
 * object pointers.
 */
struct boardobjgrp_e32 {
	/*
	* BOARDOBJGRP super-class. Must be first element of the structure.
	*/
	struct boardobjgrp super;
	/*
	* Statically allocated array of obj-s
	*/
	/* 32 will be replaced with CTRL_BOARDOBJGRP_E32_MAX_OBJECTS */
	struct pmu_board_obj *objects[32];

	/*
	* Statically allocated mask strcuture referenced by super::pMask.
	*/
	struct boardobjgrpmask_e32 mask;
};

/*
 * Wrapper to the _SUPER implementation.  Provided for the child classes which
 * implement this interface.
 */
#define boardobjgrp_pmudatainit_e32(g, pboardpbjgrp, pboardobjgrppmu) \
		nvgpu_boardobjgrp_pmu_data_init_super(g, pboardpbjgrp, \
		pboardobjgrppmu)

/* Constructor and destructor */
int nvgpu_boardobjgrp_construct_e32(struct gk20a *g,
	struct boardobjgrp_e32 *pboardobjgrp_e32);
int boardobjgrp_destruct_e32(struct boardobjgrp *pboardobjgrp);
int boardobjgrp_pmu_hdr_data_init_e32(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu,
		struct boardobjgrpmask *mask);
#endif /* NVGPU_BOARDOBJGRP_E32_H */
