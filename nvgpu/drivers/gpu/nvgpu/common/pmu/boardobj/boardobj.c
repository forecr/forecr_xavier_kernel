// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/kmem.h>
#include <nvgpu/gk20a.h>

#include "boardobj.h"

/*
* Destructor for the base board object. Called by each device-Specific
* implementation of the BOARDOBJ interface to destroy the board object.
* This has to be explicitly set by each device that extends from the
* board object.
*/
static int destruct_super(struct pmu_board_obj *obj)
{
	if (obj == NULL) {
		return -EINVAL;
	}

	nvgpu_list_del(&obj->node);
	if (obj->allocated) {
		nvgpu_kfree(obj->g, obj);
	}

	return 0;
}

/*
* check whether the specified BOARDOBJ object implements the queried
* type/class enumeration.
*/
static bool implements_super(struct gk20a *g, struct pmu_board_obj *obj,
	u8 type)
{
	nvgpu_log_info(g, " ");

	return (0U != (obj->type_mask & BIT32(type)));
}

int pmu_board_obj_pmu_data_init_super(struct gk20a *g,
		struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	nvgpu_log_info(g, " ");
	if (obj == NULL) {
		return -EINVAL;
	}
	if (pmu_obj == NULL) {
		return -EINVAL;
	}
	pmu_obj->type = obj->type;
	nvgpu_log_info(g, " Done");
	return 0;
}

int pmu_board_obj_construct_super(struct gk20a *g,
		struct pmu_board_obj  *obj, void *args)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)args;

	nvgpu_log_info(g, " ");

	if ((obj_tmp == NULL) || (obj == NULL)) {
		return -EINVAL;
	}

	obj->allocated = true;
	obj->g = g;
	obj->type = obj_tmp->type;
	obj->idx = CTRL_BOARDOBJ_IDX_INVALID;
	obj->type_mask =
			BIT32(obj->type) | obj_tmp->type_mask;
	obj->implements = implements_super;
	obj->destruct = destruct_super;
	obj->pmudatainit = pmu_board_obj_pmu_data_init_super;
	nvgpu_list_add(&obj->node, &g->boardobj_head);
	return 0;
}

u8 pmu_board_obj_get_type(void *obj)
{
	return (((struct pmu_board_obj *)(obj))->type);
}

u8 pmu_board_obj_get_idx(void *obj)
{
	return (((struct pmu_board_obj *)(obj))->idx);
}

