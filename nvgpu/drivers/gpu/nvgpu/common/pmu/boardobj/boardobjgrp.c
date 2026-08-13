// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025, NVIDIA CORPORATION & AFFILIATES.
// All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/pmu/super_surface.h>
#include <nvgpu/pmu/allocator.h>

#include "boardobj.h"

static int check_boardobjgrp_param(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp)
{
	(void)g;

	if (pboardobjgrp == NULL) {
		return -EINVAL;
	}

	if (!pboardobjgrp->bconstructed) {
		return -EINVAL;
	}

	if (pboardobjgrp->pmu.unitid == BOARDOBJGRP_UNIT_ID_INVALID) {
		return -EINVAL;
	}

	if (pboardobjgrp->classid == BOARDOBJGRP_GRP_CLASS_ID_INVALID) {
		return -EINVAL;
	}

	/* If no objects in the group, return early */
	if (BOARDOBJGRP_IS_EMPTY(pboardobjgrp)) {
		return -EINVAL;
	}

	return 0;
}

/*
 * Inserts a previously constructed Board Object into a Board Object Group for
 * tracking. Objects are inserted in the array based on the given index.
 */
static int
obj_insert_final(struct boardobjgrp *pboardobjgrp,
	struct pmu_board_obj *obj, u16 index)
{
	struct gk20a *g;

	if (pboardobjgrp == NULL) {
		return -EINVAL;
	}

	g = pboardobjgrp->g;

	nvgpu_log_info(g, " ");

	if (obj == NULL) {
		return -EINVAL;
	}

	if (index > pboardobjgrp->objslots) {
		return -EINVAL;
	}

	if (pboardobjgrp->ppobjects[index] != NULL) {
		return -EINVAL;
	}

	/*
	 * Check that this BOARDOBJ has not already been added to a
	 * BOARDOBJGRP
	 */
	if (obj->idx != CTRL_BOARDOBJ_IDX_INVALID_16BIT) {
		return -EINVAL;
	}

	pboardobjgrp->ppobjects[index] = obj;
	pboardobjgrp->objmaxidx = (u16)(BOARDOBJGRP_IS_EMPTY(pboardobjgrp) ?
			index : max(pboardobjgrp->objmaxidx, index));
	obj->idx = index;

	pboardobjgrp->objmask |= BIT32(index);

	nvgpu_log_info(g, " Done");

	return nvgpu_boardobjgrpmask_bit_set(pboardobjgrp->mask, index);
}

/*
 * Retrieves a Board Object from a Board Object Group using the group's index.
 */
static struct pmu_board_obj *obj_get_by_idx_final(
		struct boardobjgrp *pboardobjgrp, u16 index)
{
	if (!boardobjgrp_idxisvalid(pboardobjgrp, index)) {
		return NULL;
	}
	return pboardobjgrp->ppobjects[index];
}

/*
 * Retrieve Board Object immediately following one pointed by @ref currentindex
 * filtered out by the provided mask. If (mask == NULL) => no filtering.
 */
static struct pmu_board_obj *obj_get_next_final(
		struct boardobjgrp *pboardobjgrp, u16 *currentindex,
		struct boardobjgrpmask *mask)
{
	struct pmu_board_obj *obj_next = NULL;
	u16 objmaxidx;
	u16 index;

	if (currentindex == NULL) {
		return NULL;
	}

	if (pboardobjgrp == NULL) {
		return NULL;
	}

	/* Search from next element unless first object was requested */
	index = (*currentindex != CTRL_BOARDOBJ_IDX_INVALID_16BIT) ?
		(u16)(*currentindex + 1U) : 0U;

	/* For the cases below in which we have to return NULL */
	*currentindex = CTRL_BOARDOBJ_IDX_INVALID_16BIT;


	/* Validate provided mask */
	if (mask != NULL) {
		if (!(nvgpu_boardobjgrpmask_sizeeq(pboardobjgrp->mask, mask))) {
			return NULL;
		}
	}

	objmaxidx = pboardobjgrp->objmaxidx;

	if (objmaxidx != CTRL_BOARDOBJ_IDX_INVALID_16BIT) {
		for (; index <= objmaxidx; index++) {
			obj_next = pboardobjgrp->ppobjects[index];
			if (obj_next != NULL) {
				/* Filter results using client provided mask.*/
				if (mask != NULL) {
					if (!nvgpu_boardobjgrpmask_bit_get(mask,
						index)) {
						obj_next = NULL;
						continue;
					}
				}
				*currentindex = index;
				break;
			}
		}
	}

	return obj_next;
}

static int pmu_data_inst_get_stub(struct gk20a *g,
	struct nv_pmu_boardobjgrp *boardobjgrppmu,
	struct nv_pmu_boardobj **pmu_obj, u16 idx)
{
	(void)boardobjgrppmu;
	(void)pmu_obj;
	(void)idx;
	nvgpu_log_info(g, " ");
	return -EINVAL;
}


static int pmu_status_inst_get_stub(struct gk20a *g,
	void *pboardobjgrppmu,
	struct nv_pmu_boardobj_query **obj_pmu_status, u16 idx)
{
	(void)pboardobjgrppmu;
	(void)obj_pmu_status;
	(void)idx;
	nvgpu_log_info(g, " ");
	return -EINVAL;
}

static int obj_remove_and_destroy_final(
		struct boardobjgrp *pboardobjgrp,
		u16 index)
{
	int status = 0;
	int stat;
	struct gk20a *g = pboardobjgrp->g;

	nvgpu_log_info(g, " ");

	if (!boardobjgrp_idxisvalid(pboardobjgrp, index)) {
		return -EINVAL;
	}

	if (pboardobjgrp->objmaxidx == CTRL_BOARDOBJ_IDX_INVALID_16BIT) {
		return -EINVAL;
	}

	status = pboardobjgrp->ppobjects[index]->destruct(
			pboardobjgrp->ppobjects[index]);

	pboardobjgrp->ppobjects[index] = NULL;

	pboardobjgrp->objmask &= ~BIT32(index);

	stat = nvgpu_boardobjgrpmask_bit_clr(pboardobjgrp->mask, index);
	if (stat != 0) {
		if (status == 0) {
			status = stat;
		}
	}

	/* objmaxidx requires update only if that very object was removed */
	if (pboardobjgrp->objmaxidx == index) {
		pboardobjgrp->objmaxidx =
			nvgpu_boardobjgrpmask_bit_idx_highest(
					pboardobjgrp->mask);
	}

	return status;
}

static int pmu_cmd_destroy_impl(struct gk20a *g,
	struct boardobjgrp_pmu_cmd *cmd)
{
	struct nvgpu_mem *mem = &cmd->surf.sysmem_desc;

	nvgpu_pmu_allocator_surface_free(g, mem);
	return 0;
}

static int destruct_super(struct boardobjgrp *pboardobjgrp)
{
	struct pmu_board_obj *obj;
	struct gk20a *g = pboardobjgrp->g;
	int status = 0;
	int stat;
	u16  index;

	nvgpu_log_info(g, " ");

	if (pboardobjgrp->mask == NULL) {
		return -EINVAL;
	}
	if (pboardobjgrp->ppobjects == NULL) {
		return -EINVAL;
	}

	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		stat = pboardobjgrp->objremoveanddestroy(pboardobjgrp, index);
		if (status == 0) {
			status = stat;
		}

		pboardobjgrp->ppobjects[index] = NULL;
		pboardobjgrp->objmask &= ~BIT32(index);
	}

	pboardobjgrp->objmask = 0;

	if (pboardobjgrp->objmaxidx != CTRL_BOARDOBJ_IDX_INVALID_16BIT) {
		if (status == 0) {
			status = -EINVAL;
		}

		WARN_ON(true);
	}

	/* Destroy the PMU CMD data */
	stat = pmu_cmd_destroy_impl(g, &pboardobjgrp->pmu.set);
	if (status == 0) {
		status = stat;
	}

	stat = pmu_cmd_destroy_impl(g, &pboardobjgrp->pmu.getstatus);
	if (status == 0) {
		status = stat;
	}

	nvgpu_list_del(&pboardobjgrp->node);

	pboardobjgrp->bconstructed = false;

	return status;
}

static int is_pmu_cmd_id_valid(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct boardobjgrp_pmu_cmd *cmd)
{
	int err = 0;

	(void)g;
	(void)cmd;

	if (pboardobjgrp->pmu.rpc_func_id ==
		BOARDOBJGRP_GRP_RPC_FUNC_ID_INVALID) {
		err = -EINVAL;
	}

	return err;
}

static int pmu_cmd_pmu_init_handle_impl(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp,
	struct boardobjgrp_pmu_cmd *pcmd)
{
	int status = 0;
	struct nvgpu_mem *sysmem_desc = &pcmd->surf.sysmem_desc;

	nvgpu_log_info(g, " ");

	if (is_pmu_cmd_id_valid(g,
			pboardobjgrp, pcmd) != 0) {
		nvgpu_err(g, "cmd ID invalid.");
		goto pmu_cmd_pmu_init_handle_impl_exit;
	}

	if (pcmd->fbsize == 0U) {
		/* This is not an error, it's a valid case. Some board object groups
		 * have no data to send to PMU, it is safe to skip allocation.
		 */
		nvgpu_log_info(g, "fbsize is zero.");
		goto pmu_cmd_pmu_init_handle_impl_exit;
	}

	/*
	 * Check if memory is already allocated and valid.
	 * If so, reuse it if the size matches, or free and reallocate if size differs.
	 * This prevents memory leaks during regime changes.
	 */
	if (nvgpu_mem_is_valid(sysmem_desc)) {
		if (sysmem_desc->size == pcmd->fbsize) {
			/* Memory already allocated with correct size, reuse it */
			nvgpu_log_info(g, "Reusing existing PMU memory (size=%u)", pcmd->fbsize);
			pcmd->buf = (struct nv_pmu_boardobjgrp_super *)sysmem_desc->cpu_va;
			/* Initialize reused buffer to zero for consistency */
			memset(pcmd->buf, 0, pcmd->fbsize);
			goto pmu_cmd_pmu_init_handle_impl_exit;
		} else {
			/* Size mismatch, free existing memory first */
			nvgpu_log_info(g, "PMU memory size mismatch (old=%zu, new=%u), reallocating",
				       sysmem_desc->size, pcmd->fbsize);
			nvgpu_pmu_allocator_surface_free(g, sysmem_desc);
		}
	}

	status = nvgpu_pmu_allocator_sysmem_surface_alloc(g, sysmem_desc, pcmd->fbsize);
	if (status != 0) {
		nvgpu_err(g, "failed to allocate memory\n");
		return -ENOMEM;
	}

	/* we only have got sysmem later this will get copied to vidmem
	surface*/
	pcmd->surf.vidmem_desc.size = 0;

	pcmd->buf = (struct nv_pmu_boardobjgrp_super *)sysmem_desc->cpu_va;

	pmu_cmd_pmu_init_handle_impl_exit:
	return status;
}

static int pmu_init_handle_impl(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp)
{
	int status = 0;

	nvgpu_log_info(g, " ");

	status = pmu_cmd_pmu_init_handle_impl(g, pboardobjgrp,
		&pboardobjgrp->pmu.set);
	if (status != 0) {
		nvgpu_err(g, "failed to init pmu set cmd");
		goto pmu_init_handle_impl_exit;
	}

	status = pmu_cmd_pmu_init_handle_impl(g, pboardobjgrp,
		&pboardobjgrp->pmu.getstatus);
	if (status != 0) {
		nvgpu_err(g, "failed to init get status command");
		goto pmu_init_handle_impl_exit;
	}

	/* If the GRP_SET CMD has not been allocated, nothing left to do. */
	if ((is_pmu_cmd_id_valid(g,
			pboardobjgrp, &pboardobjgrp->pmu.set) != 0)||
		(BOARDOBJGRP_IS_EMPTY(pboardobjgrp))) {
		goto pmu_init_handle_impl_exit;
	}

	/* Send the BOARDOBJGRP to the pmu via RM_PMU_BOARDOBJ_CMD_GRP. */
	status = pboardobjgrp->pmuset(g, pboardobjgrp);
	if (status != 0) {
		nvgpu_err(g, "failed to send boardobg grp to PMU");
	}

	pmu_init_handle_impl_exit:
	return status;
}

/*
 * Add cleanup function to properly free PMU command memory
 * This ensures no memory leaks during shutdown or reinitilization
 */
static void pmu_cmd_pmu_cleanup_handle_impl(struct gk20a *g,
	struct boardobjgrp_pmu_cmd *pcmd)
{
	struct nvgpu_mem *sysmem_desc = &pcmd->surf.sysmem_desc;

	nvgpu_log_info(g, "Cleaning up PMU command memory");

	if (nvgpu_mem_is_valid(sysmem_desc)) {
		nvgpu_pmu_allocator_surface_free(g, sysmem_desc);
		pcmd->buf = NULL;
	}
}

static void pmu_cleanup_handle_impl(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp)
{
	nvgpu_log_info(g, "Cleaning up board object group PMU handles");

	/* Clean up set command memory */
	pmu_cmd_pmu_cleanup_handle_impl(g, &pboardobjgrp->pmu.set);

	/* Clean up get status command memory */
	pmu_cmd_pmu_cleanup_handle_impl(g, &pboardobjgrp->pmu.getstatus);
}

static int pmu_cmd_send_rpc(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp,
	struct boardobjgrp_pmu_cmd *pcmd,
	bool copy_out)
{
	struct nvgpu_pmu *pmu = g->pmu;
	struct nv_pmu_rpc_struct_board_obj_grp_cmd rpc;
	int status = 0;

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0,
		sizeof(struct nv_pmu_rpc_struct_board_obj_grp_cmd));

	rpc.class_id = pboardobjgrp->classid;
	rpc.command_id = copy_out ?
		NV_PMU_BOARDOBJGRP_CMD_GET_STATUS :
		NV_PMU_BOARDOBJGRP_CMD_SET;

	rpc.hdr.unit_id   = pboardobjgrp->pmu.unitid;
	rpc.hdr.function = pboardobjgrp->pmu.rpc_func_id;
	rpc.hdr.flags    = 0x0;

	status = nvgpu_pmu_rpc_execute(pmu, (u8 *)&rpc,
		U16(sizeof(rpc) - sizeof(rpc.scratch)),
		pcmd->dmem_buffer_size,
		NULL, NULL, copy_out);

	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC, status=0x%x", status);
	}

	return status;
}

/*
* Sends a BOARDOBJGRP to the PMU via the PMU_BOARDOBJ_CMD_GRP interface.
* This interface leverages @ref boardobjgrp_pmudatainit to populate the
* structure.
*/
static int pmu_set_impl(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp)
{
	struct nvgpu_pmu *pmu = g->pmu;
	int status = 0;
	struct boardobjgrp_pmu_cmd *pcmd =
		(struct boardobjgrp_pmu_cmd *)(&pboardobjgrp->pmu.set);

	nvgpu_log_info(g, " ");

	if (check_boardobjgrp_param(g, pboardobjgrp) != 0) {
		nvgpu_err(g, "check boardobjgrp param failed.");
		return -EINVAL;
	}

	if ((pcmd->buf == NULL) ||
		(pboardobjgrp->pmu.rpc_func_id ==
		BOARDOBJGRP_GRP_RPC_FUNC_ID_INVALID)) {
		nvgpu_err(g, "NULL buf or Invalid Id (0xff): buf %p rpc_func_id 0x%02x classID: 0x%02x",
			pcmd->buf, pboardobjgrp->pmu.rpc_func_id, pboardobjgrp->classid);
		return -EINVAL;
	}

	if (pboardobjgrp->pmudatainit == NULL) {
		nvgpu_err(g, "pmudatainit is NULL on classID: 0x%02x",
			pboardobjgrp->classid);
		return -EINVAL;
	}

	/* Initialize PMU buffer with BOARDOBJGRP data. */
	(void) memset(pcmd->buf, 0x0, pcmd->fbsize);
	status = pboardobjgrp->pmudatainit(g, pboardobjgrp,
			pcmd->buf);
	if (status != 0) {
		nvgpu_err(g, "could not parse pmu data");
		goto pmu_set_impl_exit;
	}

	/*
	 * Reset the boolean that indicates set status
	 * for most recent instance of BOARDOBJGRP.
	 */
	pboardobjgrp->pmu.bset = false;

	/*
	 * copy constructed pmu boardobjgrp data from
	 * sysmem to pmu super surface present in FB
	 */
	nvgpu_mem_wr_n(g, nvgpu_pmu_super_surface_mem(g,
		pmu, pmu->super_surface),
		pcmd->super_surface_offset, pcmd->buf,
		pcmd->fbsize);

	/* Send the SET PMU CMD to the PMU using RPC*/
	status = pmu_cmd_send_rpc(g, pboardobjgrp,
			pcmd, false);
	if (status != 0) {
		nvgpu_err(g, "could not send SET CMD to PMU");
		goto pmu_set_impl_exit;
	}

	pboardobjgrp->pmu.bset = true;

	pmu_set_impl_exit:
	return status;
}

/*
* Gets the dynamic status of the PMU BOARDOBJGRP via the
* PMU_BOARDOBJ_CMD_GRP GET_STATUS interface.
*/
static int
pmu_get_status_impl(struct gk20a *g, struct boardobjgrp *pboardobjgrp,
	struct boardobjgrpmask *mask)
{
	struct nvgpu_pmu *pmu = g->pmu;
	int status  = 0;
	struct boardobjgrp_pmu_cmd *pcmd =
		(struct boardobjgrp_pmu_cmd *)(&pboardobjgrp->pmu.getstatus);

	nvgpu_log_info(g, " ");

	if (check_boardobjgrp_param(g, pboardobjgrp) != 0) {
		return -EINVAL;
	}

	if ((pcmd->buf == NULL) ||
		(pboardobjgrp->pmu.rpc_func_id ==
		BOARDOBJGRP_GRP_RPC_FUNC_ID_INVALID)) {
		return -EINVAL;
	}

	/*
	 * Can only GET_STATUS if the BOARDOBJGRP has been
	 * previously SET to the PMU
	 */
	if (!pboardobjgrp->pmu.bset) {
		return -EINVAL;
	}

	/*
	 * Initialize PMU buffer with the mask of
	 * BOARDOBJGRPs for which to retrieve status
	 */
	(void) memset(pcmd->buf, 0x0, pcmd->fbsize);
	status = pboardobjgrp->pmuhdrdatainit(g, pboardobjgrp,
			pcmd->buf, mask);
	if (status != 0) {
		nvgpu_err(g, "could not init PMU HDR data");
		goto pmu_get_status_impl_exit;
	}

	/*
	 * copy constructed pmu boardobjgrp data from
	 * sysmem to pmu super surface present in FB
	 */
	nvgpu_mem_wr_n(g, nvgpu_pmu_super_surface_mem(g,
			pmu, pmu->super_surface),
			pcmd->super_surface_offset,
			pcmd->buf, pcmd->fbsize);
	/* Send the GET_STATUS PMU CMD to the PMU */
	status = pmu_cmd_send_rpc(g, pboardobjgrp,
			pcmd, true);
	if (status != 0) {
		nvgpu_err(g, "could not send GET_STATUS cmd to PMU");
		goto pmu_get_status_impl_exit;
	}

	/*copy the data back to sysmem buffer that belongs to command*/
	nvgpu_mem_rd_n(g, nvgpu_pmu_super_surface_mem(g,
		pmu, pmu->super_surface),
		pcmd->super_surface_offset,
		pcmd->buf, pcmd->fbsize);

	pmu_get_status_impl_exit:
	return status;
}

int nvgpu_boardobjgrp_construct_super(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp)
{
	nvgpu_log_info(g, " ");

	if (pboardobjgrp == NULL) {
		return -EINVAL;
	}

	if (pboardobjgrp->ppobjects == NULL) {
		return -EINVAL;
	}

	if (pboardobjgrp->mask == NULL) {
		return -EINVAL;
	}

	pboardobjgrp->g = g;
	pboardobjgrp->objmask = 0;

	pboardobjgrp->classid = BOARDOBJGRP_GRP_CLASS_ID_INVALID;
	pboardobjgrp->classType = NV2080_CTRL_BOARDOBJGRP_CLASS_TYPE_INVALID;
	pboardobjgrp->pmu.unitid = BOARDOBJGRP_UNIT_ID_INVALID;
	pboardobjgrp->pmu.bset = false;
	pboardobjgrp->pmu.rpc_func_id = BOARDOBJGRP_GRP_RPC_FUNC_ID_INVALID;
	pboardobjgrp->pmu.set.id = BOARDOBJGRP_GRP_CMD_ID_INVALID;
	pboardobjgrp->pmu.getstatus.id = BOARDOBJGRP_GRP_CMD_ID_INVALID;

	/* Initialize basic interfaces */
	pboardobjgrp->destruct =  destruct_super;
	pboardobjgrp->objinsert  = obj_insert_final;
	pboardobjgrp->objgetbyidx = obj_get_by_idx_final;
	pboardobjgrp->objgetnext = obj_get_next_final;
	pboardobjgrp->objremoveanddestroy =
		obj_remove_and_destroy_final;

	pboardobjgrp->pmuinithandle = pmu_init_handle_impl;
	pboardobjgrp->pmucleanuphandle = pmu_cleanup_handle_impl;
	pboardobjgrp->pmuhdrdatainit = nvgpu_boardobjgrp_pmu_hdr_data_init_super;
	pboardobjgrp->pmudatainit = nvgpu_boardobjgrp_pmu_data_init_super;
	pboardobjgrp->pmuset = pmu_set_impl;
	pboardobjgrp->pmugetstatus = pmu_get_status_impl;

	pboardobjgrp->pmudatainstget = pmu_data_inst_get_stub;
	pboardobjgrp->pmustatusinstget = pmu_status_inst_get_stub;

	pboardobjgrp->objmaxidx = CTRL_BOARDOBJ_IDX_INVALID_16BIT;
	pboardobjgrp->bconstructed = true;

	nvgpu_list_add(&pboardobjgrp->node, &g->boardobjgrp_head);

	return 0;
}


int nvgpu_boardobjgrp_pmucmd_construct_impl(struct gk20a *g, struct boardobjgrp
	*pboardobjgrp, struct boardobjgrp_pmu_cmd *cmd, u8 id, u8 msgid,
	u16 hdrsize, u16 entrysize, u32 fbsize, u32 ss_offset, u8 rpc_func_id)
{
	nvgpu_log_fn(g, " ");

	/* Copy the parameters into the CMD*/
	cmd->id = id;
	cmd->msgid = msgid;
	cmd->dmem_buffer_size = ((hdrsize > entrysize) ? hdrsize : entrysize);
	cmd->super_surface_offset = ss_offset;
	pboardobjgrp->pmu.rpc_func_id = rpc_func_id;
	cmd->fbsize = fbsize;

	nvgpu_log_fn(g, "DONE");
	return 0;
}

int nvgpu_boardobjgrp_pmu_hdr_data_init_super(struct gk20a *g, struct boardobjgrp
	*pboardobjgrp, struct nv_pmu_boardobjgrp_super *pboardobjgrppmu,
	struct boardobjgrpmask *mask)
{
	(void)mask;

	nvgpu_log_info(g, " ");

	if (pboardobjgrp == NULL) {
		return -EINVAL;
	}
	if (pboardobjgrppmu == NULL) {
		return -EINVAL;
	}
	pboardobjgrppmu->type = pboardobjgrp->type;
	pboardobjgrppmu->class_id = pboardobjgrp->classid;
	pboardobjgrppmu->classType = pboardobjgrp->classType;
	pboardobjgrppmu->obj_slots = BOARDOBJGRP_PMU_SLOTS_GET(pboardobjgrp);
	pboardobjgrppmu->flags = 0;

	nvgpu_log_info(g, " Done");
	return 0;
}

int nvgpu_boardobjgrp_pmu_data_init_legacy(struct gk20a *g,
	struct boardobjgrp 	*pboardobjgrp,
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	int status = 0;
	struct pmu_board_obj *obj = NULL;
	struct nv_pmu_boardobj *pmu_obj = NULL;
	u16 index;

	nvgpu_log_info(g, " ");

	if (pboardobjgrp == NULL) {
		return -EINVAL;
	}
	if (pboardobjgrppmu == NULL) {
		return -EINVAL;
	}

	nvgpu_boardobjgrp_e32_hdr_set((struct nv_pmu_boardobjgrp *)
			(void *)pboardobjgrppmu, pboardobjgrp->objmask);

	BOARDOBJGRP_FOR_EACH_INDEX_IN_MASK(32, index, pboardobjgrp->objmask) {
		/* Obtain pointer to the current instance of the
		 * Object from the Group */
		obj = pboardobjgrp->objgetbyidx(pboardobjgrp, index);
		if (NULL == obj) {
			nvgpu_err(g, "could not get object instance");
			status = -EINVAL;
			goto nvgpu_boardobjgrp_pmu_data_init_legacy_exit;
		}

		status = pboardobjgrp->pmudatainstget(g,
				(struct nv_pmu_boardobjgrp *)
				(void *)pboardobjgrppmu,
				&pmu_obj, index);
		if (status != 0) {
			nvgpu_err(g, "could not get object instance");
			goto nvgpu_boardobjgrp_pmu_data_init_legacy_exit;
		}

		/* Initialize the PMU Data */
		status = obj->pmudatainit(g, obj, pmu_obj);
		if (status != 0) {
			nvgpu_err(g,
				"could not parse pmu for device %d", index);
			goto nvgpu_boardobjgrp_pmu_data_init_legacy_exit;
		}
	}
	BOARDOBJGRP_FOR_EACH_INDEX_IN_MASK_END

	nvgpu_boardobjgrp_pmu_data_init_legacy_exit:
	nvgpu_log_info(g, " Done");
	return status;
}


int nvgpu_boardobjgrp_pmu_data_init_super(struct gk20a *g, struct boardobjgrp
	*pboardobjgrp, struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	int status = 0;
	struct pmu_board_obj *obj = NULL;
	struct nv_pmu_boardobj	*pmu_obj = NULL;
	u16 index;

	nvgpu_log_info(g, " ");

	if (pboardobjgrp == NULL) {
		return -EINVAL;
	}
	if (pboardobjgrppmu == NULL) {
		return -EINVAL;
	}

	/* Initialize the PMU HDR data.*/
	status = pboardobjgrp->pmuhdrdatainit(g, pboardobjgrp, pboardobjgrppmu,
							pboardobjgrp->mask);
	if (status != 0) {
		nvgpu_err(g, "unable to init boardobjgrp pmuhdr data");
		goto boardobjgrp_pmu_data_init_super_exit;
	}

	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		status = pboardobjgrp->pmudatainstget(g,
				(struct nv_pmu_boardobjgrp *)
				(void *)pboardobjgrppmu, &pmu_obj, index);
		if (status != 0) {
			nvgpu_err(g, "could not get object instance - index: %d", index);
			goto boardobjgrp_pmu_data_init_super_exit;
		}

		/* Initialize the PMU Data and send to PMU */
		status = obj->pmudatainit(g, obj, pmu_obj);
		if (status != 0) {
			nvgpu_err(g,
				"could not parse pmu for device %d", index);
			goto boardobjgrp_pmu_data_init_super_exit;
		}
	}

	boardobjgrp_pmu_data_init_super_exit:
	nvgpu_log_info(g, " Done");
	return status;
}

void nvgpu_boardobjgrp_e32_hdr_set(struct nv_pmu_boardobjgrp *hdr, u32 objmask)
{
	u32 slots = objmask;

	HIGHESTBITIDX_32(slots);
	slots++;

	hdr->super.type = CTRL_BOARDOBJGRP_TYPE_E32;
	hdr->super.class_id  = 0;
	hdr->super.obj_slots = (u16)slots;
	hdr->obj_mask = objmask;
}

/*
 * Public function to cleanup PMU board object group memory
 * Call this when shutting down or reinitializing
 */
void nvgpu_boardobjgrp_pmu_cleanup(struct gk20a *g, struct boardobjgrp *pboardobjgrp)
{
	nvgpu_log_info(g, "Cleaning up board object group PMU resources");

	if (pboardobjgrp != NULL && pboardobjgrp->pmucleanuphandle != NULL)
		pboardobjgrp->pmucleanuphandle(g, pboardobjgrp);
}

