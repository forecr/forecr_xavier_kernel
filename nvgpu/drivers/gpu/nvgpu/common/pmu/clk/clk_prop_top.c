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
#include "common/pmu/clk/clk_fll.h"
#include "clk_vf_rel.h"
#include "clk_prop_top.h"

struct vbios_clock_prop_top_table_hal vbiosClkPropTopHALs = {
	NV2080_CTRL_CLK_CLK_PROP_TOP_HAL_GA10X_TESLA_3,
	{
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_COMPUTE,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_0,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_1,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_2,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_0,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_1,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_2,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_3,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_4,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_3,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_4,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_10,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_11,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_12,
		NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_13,
	},
	// Policy HAL
	.topPolHal = {
		.topPolEntries[0] = {
			.topPolId = NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_ID_GRAPHICS_MEMORY,
			.type     = NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_TYPE_1X_SLIDER,
			.data     = {
				.v1xSlider = {
					.defaultPoint = 0,
					.numPoints    = 5,
					.points[0]    = {
						.name  = NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_BASE,
						.topId = NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_0
					},
					.points[1]    = {
						.name  = NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_INTERMEDIATE_1,
						.topId = NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_1
					},
					.points[2]    = {
						.name  = NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_INTERMEDIATE_2,
						.topId = NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_2
					},
					.points[3]    = {
						.name  = NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_INTERMEDIATE_3,
						.topId = NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_3
					},
					.points[4]    = {
						.name  = NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_EXTREME,
						.topId = NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_4
					}
				}
			}
		}
	}
};


static s32 devinitClockPropTopTableHALTranslate_1X(struct gk20a *g,
	u8 topHal,
	struct vbios_clock_prop_top_table_hal **ppTopHal)
{
	// Check that Clock Propagation Topology HAL is supported.
	if (topHal != vbiosClkPropTopHALs.topHal) {
		nvgpu_err(g,
			"Unrecognized Clock Propagation Topology Table Topology HAL - topHal=0x%02x.",
			topHal);
		return -EINVAL;
	}

	*ppTopHal = &vbiosClkPropTopHALs;

	return 0;
}

static s32 devinitClockPropTopTable10HeaderSearchSpaceHALTranslate(
	struct gk20a *g,
	u8 searchSpaceHal,
	u8 *pOutput)
{
	s32 status = 0;

	if (pOutput == NULL) {
		nvgpu_err(g, "Invalid output pointer");
		status = -EINVAL;
	} else {
		switch (searchSpaceHal) {
		case VBIOS_CLOCK_PROP_TOP_TABLE_10_HEADER_FREQUENCY_SEARCH_SPACE_HAL_10:
		{
			*pOutput = NV2080_CTRL_CLK_CLK_PROP_TOP_FREQUENCY_SEARCH_SPACE_HAL_10;
			break;
		}
		default:
			nvgpu_err(g, "Unsupported Freq search space HAL value %d",
				searchSpaceHal);
			status = -EINVAL;
			break;
		}
	}

	return status;
}

static struct clk_prop_top *clkPropTopConstruct(struct gk20a *g,
			struct clk_prop_top *pArgs)
{
	s32 status = 0;
	u8 objType;
	struct pmu_board_obj *pBoardObj = NULL;
	struct clk_prop_top *pObj = NULL;
	struct pmu_board_obj *pObjTmp = (struct pmu_board_obj *)pArgs;

	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL) {
		status = -EINVAL;
		goto done;
	}

	objType = pmu_board_obj_get_type(pArgs);
	if (objType != NV2080_CTRL_CLK_CLK_PROP_TOP_TYPE_1X) {
		status = -EINVAL;
		goto done;
	}

	pObj = nvgpu_kzalloc(g, sizeof(struct clk_prop_top));
	if (pObj == NULL) {
		status = -ENOMEM;
		goto done;
	}
	pBoardObj = &pObj->super;

	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_PROP_TOP_TYPE_MODEL_10);
	status = pmu_board_obj_construct_super(g,
				pBoardObj, pArgs);
	if (status != 0) {
		status = -EINVAL;
		goto done;
	}
	// Todo: construct / Set BOARDOBJ interfaces. (Model10)

	// Todo: Override BOARDOBJ_IFACE_MODEL_10 function pointers
done:
	if (status != 0) {
		nvgpu_err(g, "Error constructing CLK_PROP_TOP: status=%d.",
		status);
	}
	nvgpu_pmu_dbg(g, " done status %x", status);

	return pObj;
}

static s32 devinit_get_clk_prop_top_table(struct gk20a *g,
			u8 *clkPropTopsTablePtr,
			struct nvgpu_clk_prop_tops *pPropTops)
{
	s32 status = 0;
	struct vbios_clock_prop_top_table_10_header  header = { 0 };
	struct vbios_clock_prop_top_table_10_entry   entry  = { 0 };
	struct vbios_clock_prop_top_table_hal      *pTopHAL = NULL;
	struct ctrl_clk_clk_prop_top_info_frequency_search_space *pFreqSearchSpace =
		&pPropTops->freqSearchSpace;
	struct clk_prop_top   *pPropTop;
	struct clk_prop_top   *pV1x = NULL;
	struct pmu_board_obj  *pBoardObj;
	u8 *entryPtr;
	u8 data8;
	u8 relIdx;
	u32 i, j, lclMask;

	nvgpu_pmu_dbg(g, " ");

	memcpy(&header, clkPropTopsTablePtr, sizeof(header));
	if (header.entry_size < VBIOS_CLOCK_PROP_TOP_TABLE_10_ENTRY_SIZE_17) {
		status = -EINVAL;
		nvgpu_err(g, "Unrecognized Clock Propagation Topology Table entry size - size=0x%02x.",
			header.entry_size);
		goto done_nofree;
	}
	status = devinitClockPropTopTableHALTranslate_1X(g, header.top_hal, &pTopHAL);
	if (status != 0) {
		goto done_nofree;
	}
	pPropTops->topHal = pTopHAL->topHal;

	if (header.header_size >= VBIOS_CLOCK_PROP_TOP_TABLE_10_HEADER_SIZE_09) {
		pFreqSearchSpace->bUseHigherIsoSecFmaxVminMhz =
			(((u8)VBIOS_CLOCK_PROP_TOP_TABLE_10_HEADER_FLAGS0_USE_HIGHER_ISO_SEC_FMAX_VMIN_MHZ_ENABLED) ==
			BIOS_GET_FIELD(u8, header.flags0,
				VBIOS_CLOCK_PROP_TOP_TABLE_10_HEADER_FLAGS0_USE_HIGHER_ISO_SEC_FMAX_VMIN_MHZ));

		pFreqSearchSpace->minRatioClkPropTopIdx =
			(u16)((header.minRatioClkPropTopIdx == 0xff) ?
			NV2080_CTRL_CLK_CLK_PROP_TOP_IDX_INVALID : header.minRatioClkPropTopIdx);

		pFreqSearchSpace->maxRatioClkPropTopIdx =
			(u16)((header.maxRatioClkPropTopIdx == 0xff) ?
			NV2080_CTRL_CLK_CLK_PROP_TOP_IDX_INVALID : header.maxRatioClkPropTopIdx);
		status = devinitClockPropTopTable10HeaderSearchSpaceHALTranslate(g,
				header.searchSpaceHal, &pFreqSearchSpace->searchSpaceHal);
		if (status != 0) {
			goto done_nofree;
		}
	}

	pV1x = nvgpu_kzalloc(g, sizeof(struct clk_prop_top));
	if (pV1x == NULL) {
		status = -ENOMEM;
		goto done_nofree;
	}
	pBoardObj = &pV1x->super;

	// Parse each entry.
	for (i = 0; i < header.entry_count; i++) {
		/* To avoid MISRA violation,
		 * this is a single point of exit
		 * from this for loop in the case
		 * of an error is encountered.
		 */
		if (status != 0)
			break;
		//
		// Zero out the clkPropTopData structure to remove whatever changes were
		// made from previous iteration.
		//
		memset(pV1x, 0x0, sizeof(struct clk_prop_top));

		// Read base entry from the VBIOS.
		entryPtr = clkPropTopsTablePtr + header.header_size + (i * header.entry_size);
		memcpy(&entry, entryPtr, sizeof(entry));

		data8 = BIOS_GET_FIELD(u8, entry.flags0,
				NV_VBIOS_CLOCK_PROP_TOP_TABLE_10_ENTRY_FLAGS0_TYPE);

		// Skip parsing disabled clock enumeration entries.
		if (data8 == (u8)NV_VBIOS_CLOCK_PROP_TOP_TABLE_10_ENTRY_FLAGS0_TYPE_DISABLED) {
			continue;
		} else if (data8 == (u8)NV_VBIOS_CLOCK_PROP_TOP_TABLE_10_ENTRY_FLAGS0_TYPE_1X) {
			pBoardObj->type = NV2080_CTRL_CLK_CLK_PROP_TOP_TYPE_1X;
		} else {
			nvgpu_err(g, "Invalid Clock Propagation Topology Type = %d, entry = %d.",
						data8, i);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		if ((pTopHAL->topId[i] == NV2080_CTRL_CLK_CLK_PROP_TOP_ID_INVALID) ||
			(i >= NV2080_CTRL_CLK_CLK_PROP_TOP_ID_MAX)) {
			nvgpu_err(g,
				"Invalid Clock Propagation Topology HAL - ENTRY mapping. HAL = %d, topId[%d] = %d.",
				pTopHAL->topHal, i, pTopHAL->topId[i]);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}
		// Update top id.
		pV1x->topId = pTopHAL->topId[i];

		// Parse clock domain index mask.
		status = nvgpu_boardobjgrp_construct_e255(g,
					&(pV1x->clkPropTopRelMask));
		if (status != 0) {
			nvgpu_err(g, "Error constructing clock mask - status: %d.",
						status);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		for (j = 0 ; j < VBIOS_CLOCK_PROP_TOP_TABLE_10_CLK_PROP_REL_MASK_ARRAY_SIZE; j++) {
			lclMask = entry.clkPropRelMask[j];
			for (relIdx = 0; lclMask != 0; relIdx++) {
				if ((((u32)0x01) & lclMask) == 0U) {
					lclMask >>= 1;
					continue;
				}

				status = nvgpu_boardobjgrpmask_bit_set(
						&(pV1x->clkPropTopRelMask.mask.super),
						(relIdx + (j * 32)));
				if (status != 0) {
					nvgpu_err(g,
							"Error setting clock mask: relIdx=%d, status=%d.",
							(relIdx + (j * 32)), status);
					break;
				}
				lclMask >>= 1U;
			}
			if (status != 0) {
				break;
			}
		}
		// check for failure in mask bit setting
		if (status != 0) {
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}
		// Construct CLK_PROP_TOP for the entry.
		pPropTop = clkPropTopConstruct(g, pV1x);
		if (pPropTop == NULL) {
			nvgpu_err(g,
					"Could not construct CLK_PROP_TOP for entry %d.", i);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		// Insert CLK_PROP_TOP into the CLK_PROP_TOPS group.
		status = boardobjgrp_objinsert(&pPropTops->super.super,
					&pPropTop->super, (u8)i);
		if (status != 0) {
			nvgpu_err(g,
					"Could not insert CLK_PROP_TOP entry %d into CLK_PROP_TOPS group - status=%d",
					i, status);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}
	}

	nvgpu_kfree(g, pV1x);

done_nofree:
	nvgpu_pmu_dbg(g, " done status %x", status);

	return status;
}


s32 clk_prop_top_sw_setup(struct gk20a *g)
{
	s32 status = 0;
	struct nvgpu_clk_prop_tops *clkproptopobjs = NULL;
	struct boardobjgrp *pboardobjgrp = NULL;
	u8 version;
	u8 *prop_table_ptr = NULL;
	u32 i;

	prop_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
			CLOCK_PROP_TOP_TABLE);

	if (prop_table_ptr == NULL) {
		status = -1;
		nvgpu_err(g, "VBIOS Clock Propagation Topology table not found");
		goto done;
	}

	// Check VBIOS table version.
	version = *prop_table_ptr;

	if (version != (u8)VBIOS_CLOCK_PROP_TOP_TABLE_VERSION_10) {
		nvgpu_err(g, "Unsupported Clock Propagation Topology table version 0x%x.",
				version);
		status = -EINVAL;
		goto done;
	}
	status = nvgpu_boardobjgrp_construct_e32(g,
			&g->pmu->clk_pmu->clk_topobjs->super);
	if (status != 0) {
		nvgpu_err(g,
			"Error creating boardobjgrp for Propagation Topology, status: %d.",
			status);
		goto done;
	}

	clkproptopobjs = g->pmu->clk_pmu->clk_topobjs;
	// Set uninitialized state for clk_topobjs
	for (i = 0; i < NV2080_CTRL_CLK_CLK_PROP_TOP_ID_MAX; i++) {
		clkproptopobjs->topIdToIdxMap[i] =
					NV2080_CTRL_CLK_CLK_PROP_TOP_IDX_INVALID;
	}
	clkproptopobjs->activeTopId       = NV2080_CTRL_CLK_CLK_PROP_TOP_ID_INVALID;
	clkproptopobjs->activeTopIdForced = NV2080_CTRL_CLK_CLK_PROP_TOP_ID_INVALID;

	// construct and set the boardobj class ID
	pboardobjgrp = &g->pmu->clk_pmu->clk_topobjs->super.super;
	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, CLK_PROP_TOP);

	// todo: init RPC structs.

	// Parse VBIOS Clock Propagation Regime Table.
	status = devinit_get_clk_prop_top_table(g, prop_table_ptr,
				clkproptopobjs);
	if (status != 0) {
		nvgpu_err(g, "Error parsing the Clock Propagation Topology Vbios tables - %d.",
					status);
	}

done:
	return status;
}

s32 clk_prop_top_pmu_setup(struct gk20a *g)
{
	/* todo: implement sending to PMU */
	return -EINVAL;
}

s32 clk_prop_top_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmu->clk_pmu->clk_topobjs != NULL) {
		return 0;
	}

	g->pmu->clk_pmu->clk_topobjs = nvgpu_kzalloc(g,
			sizeof(struct nvgpu_clk_prop_tops));
	if (g->pmu->clk_pmu->clk_topobjs == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void clk_prop_top_free_pmupstate(struct gk20a *g)
{
	nvgpu_kfree(g, g->pmu->clk_pmu->clk_topobjs);
	g->pmu->clk_pmu->clk_topobjs = NULL;
}
