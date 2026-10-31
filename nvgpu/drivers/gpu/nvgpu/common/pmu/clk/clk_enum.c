// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bios.h>
#include <nvgpu/kmem.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/boardobjgrp_e255.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu/clk/clk.h>

#include "common/pmu/clk/ucode_clk_inf.h"
#include "common/pmu/clk/clk.h"
#include "clk_enum.h"

static s32 clk_enum_construct(struct gk20a *g, struct pmu_board_obj **obj,
		size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_enum *pclkenum = NULL;
	struct clk_enum *ptmpenum =
			(struct clk_enum *)pargs;
	s32 status = 0;

	nvgpu_pmu_dbg(g, " ");

	pclkenum = nvgpu_kzalloc(g, size);
	if (pclkenum == NULL) {
		status = -ENOMEM;
		goto done;
	}

	obj_tmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_ENUM_TYPE_1X);
	status = pmu_board_obj_construct_super(g,
			(struct pmu_board_obj *)(void *)pclkenum, pargs);
	if (status != 0) {
		status = -EINVAL;
		goto done;
	}

	*obj = (struct pmu_board_obj *)pclkenum;
	pclkenum->b_ocov_enabled = ptmpenum->b_ocov_enabled;
	pclkenum->freq_min_mhz = ptmpenum->freq_min_mhz;
	pclkenum->freq_max_mhz = ptmpenum->freq_max_mhz;
done:
	nvgpu_pmu_dbg(g, " done status %x", status);

	return status;
}

static struct clk_enum *construct_clk_enum(struct gk20a *g, void *pargs)
{
	s32 status = 0;
	struct pmu_board_obj *obj = NULL;

	nvgpu_pmu_dbg(g, " ");

	nvgpu_pmu_dbg(g, " type - %x", pmu_board_obj_get_type(pargs));
	if (pmu_board_obj_get_type(pargs) != NV2080_CTRL_CLK_CLK_ENUM_TYPE_1X) {
		nvgpu_err(g, "unsupported clk_enum type %x in VBIOS table",
			pmu_board_obj_get_type(pargs));
		goto done;
	}
	status = clk_enum_construct(g, &obj,
			sizeof(struct clk_enum), pargs);

	if (status != 0) {
		if (obj != NULL && obj->destruct != NULL) {
			status = obj->destruct(obj);
			if (status != 0) {
				nvgpu_err(g, "destruct failed err=%d", status);
			}
		}
		nvgpu_err(g, "clk_enum construct failed err=%d", status);
		obj = NULL;
	}
done:
	nvgpu_pmu_dbg(g, " done status %x", status);

	return (struct clk_enum *)obj;
}

/*!
 * Parse Clocks Enumeration Table with version 0x10.
 */
static s32 devinit_get_clk_enum_table(struct gk20a *g,
	struct nvgpu_clk_enums *penumobjs)
{

	s32 status = 0;
	u8 *enum_table_ptr = NULL;
	struct vbios_clk_enum_table_v10_header header = { 0 };
	struct vbios_clk_enum_table_v10_entry entry = { 0 };
	union {
		struct pmu_board_obj boardObj;
		struct clk_enum v1x;
	} clkEnumData;
	struct clk_enum *pEnum;
	u32 index;
	u8 *entryPtr;
	u8 data8;

	nvgpu_pmu_dbg(g, " ");

	enum_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
						CLOCK_ENUM_TABLE);
	if (enum_table_ptr == NULL) {
		status = -1;
		nvgpu_err(g, "VBIOS Clock Enumeration table not found");
		goto done;
	}

	nvgpu_memcpy((u8 *)&header, enum_table_ptr,
			sizeof(struct vbios_clk_enum_table_v10_header));

	if (header.entry_size < VBIOS_CLOCK_ENUM_TABLE_10_ENTRY_SIZE_05) {
		status = -EINVAL;
		nvgpu_err(g,
				"Unrecognized Clock Enumeration Table entry size: %d",
				header.entry_size);
		goto done;
	}

	// Parse each entry.
	for (index = 0; index < header.entry_count; index++) {
		/* To avoid MISRA violation,
		 * this is a single point of exit
		 * from this for loop in the case
		 * of an error is encountered.
		 */
		if (status != 0)
			break;

		// Zero out the clkDomainData structure to remove whatever changes were
		// made from previous iteration.
		memset((u8 *)&clkEnumData, 0x0, sizeof(clkEnumData));
		// Get pointer to the current entry.
		entryPtr = enum_table_ptr + header.header_size + (index * header.entry_size);

		nvgpu_memcpy((u8 *)&entry, entryPtr,
			VBIOS_CLOCK_ENUM_TABLE_10_ENTRY_SIZE_05);
		// Skip disabled entries
		data8 = BIOS_GET_FIELD(u8, entry.flags0,
				VBIOS_CLOCK_ENUM_TABLE_10_ENTRY_FLAGS0_TYPE);
		if (data8 == VBIOS_CLOCK_ENUM_TABLE_10_ENTRY_FLAGS0_TYPE_DISABLED)
			continue;
		if (data8 == VBIOS_CLOCK_ENUM_TABLE_10_ENTRY_FLAGS0_TYPE_1X) {
			clkEnumData.boardObj.type = NV2080_CTRL_CLK_CLK_ENUM_TYPE_1X;
		} else {
			nvgpu_err(g, "Unrecognized Clock Enumeration Type: 0x%x",
					data8);
			status = -EINVAL;
			continue; /* to avoid MISRA violation, single exit point at top of loop */
		}

		// Parse OVOC support.
		clkEnumData.v1x.b_ocov_enabled = (entry.flags0 &
				VBIOS_CLOCK_ENUM_TABLE_10_ENTRY_FLAGS0_OVOC_ENABLED_MASK) ==
				VBIOS_CLOCK_ENUM_TABLE_10_ENTRY_FLAGS0_OVOC_ENABLED_MASK;

		// Parse and copy Min & Max Freq MHz
		clkEnumData.v1x.freq_min_mhz = entry.freq_min_mhz;
		clkEnumData.v1x.freq_max_mhz = entry.freq_max_mhz;

		// Construct Clk_enum Boardobj for the entry.
		pEnum = construct_clk_enum(g, (void *)&clkEnumData);
		if (pEnum == NULL) {
			nvgpu_err(g, "Error constructing Clk_enum boardobj for Entry: %d",
						index);
			status = -EINVAL;
			/* to avoid MISRA violation, exit at top of loop */
			continue;
		}

		// Insert Clk_enum into the Clk_enum group.
		status = boardobjgrp_objinsert(&penumobjs->super.super,
				(struct pmu_board_obj *)pEnum, (u8)index);
		if (status != 0) {
			nvgpu_err(g,
				"Inserting Clk_enum device object to boardObjGrp failed for Entry: %d",
				index);
			/* to avoid MISRA violation, exit at top of loop */
			continue;
		}

	}

done:
	nvgpu_pmu_dbg(g, " done status %x", status);

	return status;
}

static s32 clk_enum_pmudatainit(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	s32 status = 0;

	nvgpu_pmu_dbg(g, " ");

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);
	if (status != 0) {
		nvgpu_err(g, "Error updating pmu boardobjgrp for Clk_enum. status - 0x%x",
			  status);
	}

	nvgpu_pmu_dbg(g, " Done");

	return status;
}

static s32 clk_enum_pmudata_instget(struct gk20a *g,
		struct nv_pmu_boardobjgrp *pmuboardobjgrp,
		struct nv_pmu_boardobj **pmu_obj,
		u8 idx)
{
	struct nv_pmu_clk_clk_enum_boardobj_grp_set  *pgrp_set =
		(struct nv_pmu_clk_clk_enum_boardobj_grp_set *)
		(void *)pmuboardobjgrp;

	nvgpu_pmu_dbg(g, " ");

	/* check whether pmuboardobjgrp has a valid boardobj in index */
	if (((u32)BIT(idx) &
		pgrp_set->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		return -EINVAL;
	}

	*pmu_obj = (struct nv_pmu_boardobj *)
		&pgrp_set->objects[idx].data.obj;
	nvgpu_pmu_dbg(g, " Done");

	return 0;
}

s32 clk_enum_sw_setup(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct nvgpu_clk_enums *pclkenumobjs = NULL;

	nvgpu_pmu_dbg(g, " ");

	status = nvgpu_boardobjgrp_construct_e255(g,
			&g->pmu->clk_pmu->clk_enumobjs->super);
	if (status != 0) {
		nvgpu_err(g,
			"Error creating boardobjgrp for Clk_enum, status: 0x%x",
			status);
		goto done;
	}

	pboardobjgrp = &g->pmu->clk_pmu->clk_enumobjs->super.super;
	pclkenumobjs = g->pmu->clk_pmu->clk_enumobjs;

	/* construct and set the boardobj class ID */
	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, CLK_ENUM);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			clk, CLK, clk_enum, CLK_ENUM);
	if (status != 0) {
		nvgpu_err(g,
			"Error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
			status);
		goto done;
	}

	pboardobjgrp->pmudatainit = clk_enum_pmudatainit;
	pboardobjgrp->pmudatainstget  = clk_enum_pmudata_instget;

	status = devinit_get_clk_enum_table(g, pclkenumobjs);
	if (status != 0) {
		nvgpu_err(g, "Error parsing the Clk_enum Vbios tables");
		goto done;
	}

done:
	nvgpu_pmu_dbg(g, " done status %x", status);

	return status;
}

s32 clk_enum_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmu->clk_pmu->clk_enumobjs != NULL) {
		return 0;
	}

	g->pmu->clk_pmu->clk_enumobjs = nvgpu_kzalloc(g,
			sizeof(struct nvgpu_clk_enums));
	if (g->pmu->clk_pmu->clk_enumobjs == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void clk_enum_free_pmupstate(struct gk20a *g)
{
	nvgpu_kfree(g, g->pmu->clk_pmu->clk_enumobjs);
	g->pmu->clk_pmu->clk_enumobjs = NULL;
}
