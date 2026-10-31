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
#include "common/pmu/clk/clk_domain.h"
#include "clk_vf_rel.h"
#include "clk_prop_top_rel.h"

#define VBIOS_CLOCKS_TABLE_1X_HAL_NUM_CLOCK_ENTRIES                         0x0F
#define VBIOS_CLOCKS_TABLE_1X_NUM_HALS                                      0x0A
#define VBIOS_CLOCKS_TABLE_1X_NUM_FACTORY_OC_CLK_DOMAINS                    0x02
#define VBIOS_CLOCKS_TABLE_1X_NUM_FACTORY_OC_VPSTATE_IDXS                   0x02
#define VBIOS_CLOCKS_TABLE_1X_NUM_CPMU_CPLUT_MAPPINGS                       0x10

typedef u8 VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE;
/*!
 * Enumeration of clock domain types.
 */
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_LOGICAL                   0x0
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_PHYSICAL_WITH_LOGICAL     0x1
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_PHYSICAL                  0x2
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_INVALID                   0x0FF

/*!
 * Invalid index of clock domain.
 */
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_IDX_INVALID                   0x0FFU

#define VBIOS_CLOCKS_TABLE_1X_HAL_CPMU_CPLUT_CLK_REL_MAPPING_RESERVED      \
		{ CLKWHICH_DEFAULT, CLKWHICH_DEFAULT }

/*!
 * Structure with LOGICAL clock domain type specific data.
 */
struct vbios_clocks_table_1x_hal_clk_domain_logical {
	u8 clkDomainPhysicalIdxFirst;
	u8 clkDomainPhysicalIdxLast;
};

/*!
 * Structure with PHYSICAL clock domain type specific data.
 */
struct vbios_clocks_table_1x_hal_clk_domain_physical {
	u8 clkDomainLogicalIdx;
};

/*!
 * Type-specific data union.
 */
union vbios_clocks_table_1x_hal_clk_domain_data {
	struct vbios_clocks_table_1x_hal_clk_domain_logical  logical;
	struct vbios_clocks_table_1x_hal_clk_domain_physical  physical;
};

/*!
 * Structure representing the HAL enumeration of a CLK_DOMAIN from the Clocks
 * Table Domains
 */
struct vbios_clocks_table_1x_hal_clock_entry {
	NV_PMU_CLK_CLKWHICH                               domain;
	bool                                              bNoiseAwareCapable;
	u8                                                clkVFCurveCount;
	NV2080_CTRL_CLK_PUBLIC_DOMAIN                     publicDomain;
	VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE         clkDomainType;
	union vbios_clocks_table_1x_hal_clk_domain_data   clkDomainData;
	NV2080_CTRL_CLK_CLK_DOMAIN_CPMU_CLOCK_ID          cpmuClkId;
};

struct vbios_clocks_table_1x_hal_cpmu_cplut_clk_rel_mapping {
	u32 clkSrc;
	u32 clkDst;
};

struct vbios_clocks_table_1x_hal_cpmu_cplut_clk_rel_mappings {
	u8 numMappings;
	struct vbios_clocks_table_1x_hal_cpmu_cplut_clk_rel_mapping
		columnMappings[VBIOS_CLOCKS_TABLE_1X_NUM_CPMU_CPLUT_MAPPINGS];
};

/*!
 * Structure representing the HAL enumeration of FactoryOC Table Clocks
 */
struct vbios_clocks_table_1x_hal_factory_oc_clocks_entry {
	NV_PMU_CLK_CLKWHICH  domain;
};

/*!
 * Structure representing the HAL enumeration of FactoryOC Table vpState
 */
struct vbios_clocks_table_1x_hal_factory_oc_vpstate_entry {
	u8 vpstateIdx; //<! NV2080_CTRL_PERF_VPSTATES_IDX_<XYZ>
};

/*!
 * Structure representing the HAL enumeration of all CLK_DOMAINs from the Clocks
 * Table Domains
 */
struct vbios_clocks_table_1x_hal {
	u8 nvClocksHAL;
	struct vbios_clocks_table_1x_hal_clock_entry
		domains[VBIOS_CLOCKS_TABLE_1X_HAL_NUM_CLOCK_ENTRIES];
	struct vbios_clocks_table_1x_hal_factory_oc_clocks_entry
		factoryOCDomains[VBIOS_CLOCKS_TABLE_1X_NUM_FACTORY_OC_CLK_DOMAINS];
	struct vbios_clocks_table_1x_hal_factory_oc_vpstate_entry
		factoryOCvpstateIdx[VBIOS_CLOCKS_TABLE_1X_NUM_FACTORY_OC_VPSTATE_IDXS];
	struct vbios_clocks_table_1x_hal_cpmu_cplut_clk_rel_mappings
		cpmuCplutClkRelMappings;
};

/*!
 * Table Clocks Table 1.0 Clock HALs.
 */
struct vbios_clocks_table_1x_hal vbiosClocks1XClocksHALs[1] = {
	{
		NV2080_CTRL_CLK_CLK_DOMAIN_HAL_GB20Y,
		{
			{ CLKWHICH_GPCCLK,     true,  4, NV2080_CTRL_CLK_PUBLIC_DOMAIN_GRAPHICS, VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_PHYSICAL, { .physical = {VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_IDX_INVALID, }}, 0x0,                                              },
			{ CLKWHICH_XBARCLK,    true,  1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID,  VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_PHYSICAL, { .physical = {VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_IDX_INVALID, }}, 0x1,                                              },
			{ CLKWHICH_MCLK,       false, 1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID,  VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_PHYSICAL, { .physical = {VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_IDX_INVALID, }}, NV2080_CTRL_CLK_CLK_DOMAIN_CPMU_CLOCK_ID_INVALID, },
			{ CLKWHICH_NVDCLK,     true,  1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID,  VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_PHYSICAL, { .physical = {VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_IDX_INVALID, }}, 0x3,                                              },
			{ CLKWHICH_UPROCCLK,   true,  1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID,  VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_PHYSICAL, { .physical = {VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_IDX_INVALID, }}, NV2080_CTRL_CLK_CLK_DOMAIN_CPMU_CLOCK_ID_INVALID, },
		},
		{
			{ CLKWHICH_GPCCLK  },
			{ CLKWHICH_MCLK    },
		},
		{
			{ NV2080_CTRL_PERF_VPSTATES_IDX_RATEDTDP   },
			{ NV2080_CTRL_PERF_VPSTATES_IDX_TURBOBOOST },
		},
		{
			3,
			{
				VBIOS_CLOCKS_TABLE_1X_HAL_CPMU_CPLUT_CLK_REL_MAPPING_RESERVED,  // 0
				{ CLKWHICH_GPCCLK,  CLKWHICH_XBARCLK },                         // 1
				{ CLKWHICH_XBARCLK, CLKWHICH_NVDCLK },                          // 2
			}
		}
	}
};

// forward declaration
static u32 clkPropTopRel1xRatioCalcRatioInverse(struct gk20a *g, u32 ratio);

static s32 devinitClocksTableHALTranslate_1X(struct gk20a *g,
		u8 clocksHal, struct vbios_clocks_table_1x_hal **ppClocksHAL)
{
	// We only support one HAL type
	if (clocksHal != NV2080_CTRL_CLK_CLK_DOMAIN_HAL_GB20Y) {
		nvgpu_err(g, "Unsupported clocks HAL: 0x%02x", clocksHal);
		return -EINVAL;
	}
	*ppClocksHAL = &vbiosClocks1XClocksHALs[0];
	return 0;
}

static s32 clkPropTopRelIfaceModel10Construct_1X(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj,
		u16      size,
		void      *pArgs)
{
	s32 status = 0;
	struct clk_prop_top_rel *pClkPropTopRel = NULL;
	struct pmu_board_obj *pBoardObj = NULL;
	struct pmu_board_obj *pObjTmp = (struct pmu_board_obj *)pArgs;
	struct clk_prop_top_rel *pTmpClkPropTopRel = (struct clk_prop_top_rel *) pArgs;

	pClkPropTopRel = nvgpu_kzalloc(g, size);
	if (pClkPropTopRel == NULL) {
		status = -ENOMEM;
		goto done;
	}
	pBoardObj = &pClkPropTopRel->super;

	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X);
	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_PROP_TOP_TYPE_MODEL_10);
	status = pmu_board_obj_construct_super(g,
				pBoardObj, pArgs);
	if (status != 0) {
		status = -EINVAL;
		goto done;
	}
	*ppBoardObj = pBoardObj;

	// Set type-specific parameters.
	pClkPropTopRel->clkDomainIdxSrc = pTmpClkPropTopRel->clkDomainIdxSrc;
	pClkPropTopRel->clkDomainIdxDst = pTmpClkPropTopRel->clkDomainIdxDst;
	pClkPropTopRel->bBiDirectional  = pTmpClkPropTopRel->bBiDirectional;

	// Todo: construct / Set BOARDOBJ interfaces. (Model10)

	// Todo: Override BOARDOBJ_IFACE_MODEL_10 function pointers

done:

	return status;
}

static s32 clkPropTopRelIfaceModel10Construct_1X_RATIO(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj,
		u16      size,
		void      *pArgs)
{
	s32 status = 0;
	struct clk_prop_top_rel_1x_ratio  *pRel1xRatioTmp =
		(struct clk_prop_top_rel_1x_ratio  *)pArgs;
	struct clk_prop_top_rel_1x_ratio  *pRel1xRatio;
	struct pmu_board_obj *pObjTmp = (struct pmu_board_obj *)pArgs;

	// Call into CLK_PROP_TOP_REL_1X super constructor.
	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_RATIO);
	status = clkPropTopRelIfaceModel10Construct_1X(g, ppBoardObj, size, pArgs);
	if (status != 0) {
		nvgpu_err(g,
				"Error constructing CLK_PROP_TOP_REL_1X: status=%d",
				status);
		goto done;
	}
	pRel1xRatio = (struct clk_prop_top_rel_1x_ratio *) *ppBoardObj;

	// Todo Set BoardObj interfaces to 1X_RATIO

	// Set type-specific parameters.
	pRel1xRatio->ratio        = pRel1xRatioTmp->ratio;
	pRel1xRatio->ratioInverse =
		clkPropTopRel1xRatioCalcRatioInverse(g, pRel1xRatio->ratio);

done:

	return status;
}

static s32 clkPropTopRelIfaceModel10Construct_1X_VOLT(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj,
		u16      size,
		void      *pArgs)
{
	s32 status = 0;
	struct clk_prop_top_rel_1x_volt  *pRel1xVoltTmp =
		(struct clk_prop_top_rel_1x_volt  *)pArgs;
	struct clk_prop_top_rel_1x_volt  *pRel1xVolt;
	struct pmu_board_obj *pObjTmp = (struct pmu_board_obj *)pArgs;

	// Call into CLK_PROP_TOP_REL_1X super constructor.
	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_VOLT);
	status = clkPropTopRelIfaceModel10Construct_1X(g, ppBoardObj, size, pArgs);
	if (status != 0) {
		nvgpu_err(g,
				"Error constructing CLK_PROP_TOP_REL_1X: status=%d",
				status);
		goto done;
	}

	pRel1xVolt = (struct clk_prop_top_rel_1x_volt *) *ppBoardObj;

	// Todo: Set BOARDOBJ interfaces.

	// Set type-specific parameters.
	pRel1xVolt->voltRailIdx = pRel1xVoltTmp->voltRailIdx;

done:
	return status;
}

static struct clk_prop_top_rel *clkPropTopRelIfaceModel10Construct(struct gk20a *g,
		struct clk_prop_top_rel *pArgs)
{
	s32 status = -EINVAL;
	u8 objType;
	struct pmu_board_obj *pBoardObj = NULL;

	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL) {
		status = -EINVAL;
		goto done;
	}

	objType = pmu_board_obj_get_type(pArgs);
	switch (objType) {
	case NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_RATIO:
	{
		status = clkPropTopRelIfaceModel10Construct_1X_RATIO(g, &pBoardObj,
					sizeof(struct clk_prop_top_rel_1x_ratio), pArgs);
		break;
	}
	case NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_VOLT:
	{
		status = clkPropTopRelIfaceModel10Construct_1X_VOLT(g, &pBoardObj,
					sizeof(struct clk_prop_top_rel_1x_volt), pArgs);
		break;
	}
	default:
	{
		nvgpu_err(g, "Unsupported CLK_PROP_TOP_REL type = 0x%08x.", objType);
		break;
	}

	}

done:
	if (status != 0) {
		nvgpu_err(g, "Error constructing CLK_PROP_TOP_REL: status=%d.",
		status);
	}
	nvgpu_pmu_dbg(g, " done status %x", status);

	return (struct clk_prop_top_rel *) pBoardObj;
}

static s32 devinit_get_clk_prop_top_rel_table(struct gk20a *g,
			u8 *clkPropTopRelsTablePtr,
			struct nvgpu_clk_prop_top_rels *pPropTopRels)
{
	s32 status = 0;
	struct vbios_clock_prop_top_rel_table_10_header header = { 0 };
	struct vbios_clock_prop_top_rel_table_10_table_rel_arr_entry
			tableArrEntry = { 0 };
	struct vbios_clock_prop_top_rel_table_10_entry entry = { 0 };
	union {
		struct pmu_board_obj                boardObj;
		struct clk_prop_top_rel             super;
		struct clk_prop_top_rel_1x          v1x;
		struct clk_prop_top_rel_1x_ratio    v1xRatio;
		struct clk_prop_top_rel_1x_volt     v1xVolt;
	} clkPropTopRelData;
	struct clk_prop_top_rel *pPropTopRel;
	u8 i;
	u8 *entryPtr;
	u8 data8;
	struct vbios_clocks_table_1x_hal *pClocksHal = NULL;

	nvgpu_pmu_dbg(g, " ");

	// Get Clocks HAL. We only support one type.
	status = devinitClocksTableHALTranslate_1X(g,
				NV2080_CTRL_CLK_CLK_DOMAIN_HAL_GB20Y,
				&pClocksHal);
	if (status != 0) {
		nvgpu_err(g,
			"Error translating Clocks HAL - status=0x%08x.", status);
		goto done;
	}

	memcpy(&header, clkPropTopRelsTablePtr, sizeof(header));
	if (header.entry_size < VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_TABLE_REL_ARR_ENTRY_SIZE_04) {
		status = -EINVAL;
		nvgpu_err(g, "Unrecognized Clock Propagation Topology Relationship Table entry size: 0x%02x.",
			header.entry_size);
		goto done;
	}

	if (header.table_array_entry_size <
		VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_TABLE_REL_ARR_ENTRY_SIZE_04) {
		nvgpu_err(g, "Unrecognized Clock Propagation Topology Relationship Array entry size: 0x%02x",
			header.table_array_entry_size);
		status = -EINVAL;
		goto done;
	}

	// Parse Table Relationship Array entry
	pPropTopRels->tableRelTupleCount = header.table_array_entry_count;

	for (i = 0; i < header.table_array_entry_count; i++) {
		entryPtr = clkPropTopRelsTablePtr + header.header_size +
					(header.entry_count * header.entry_size) +
					(i * header.table_array_entry_size);
		memcpy(&tableArrEntry, entryPtr, sizeof(tableArrEntry));
		pPropTopRels->tableRelTuple[i].freqMHzSrc = tableArrEntry.freqMHzSrc;
		pPropTopRels->tableRelTuple[i].freqMHzDst = tableArrEntry.freqMHzDst;
	}

	// Extract CPMU relationships from Clocks HAL
	pPropTopRels->cpmuRelTupleCount = pClocksHal->cpmuCplutClkRelMappings.numMappings;

	for (i = 0; i < NV2080_CTRL_CLK_CLK_PROP_TOP_REL_CPMU_REL_TUPLE_MAX; i++) {
		if (i < pClocksHal->cpmuCplutClkRelMappings.numMappings) {
			u32 clkIdx;
			/* To avoid MISRA violation,
			 * this is a single point of exit
			 * from this for loop in the case
			 * of an error is encountered.
			 */
			if (status != 0)
				break;

			// Handle "reserved" case easier
			if (pClocksHal->cpmuCplutClkRelMappings.columnMappings[i].clkSrc == CLKWHICH_DEFAULT &&
				pClocksHal->cpmuCplutClkRelMappings.columnMappings[i].clkDst == CLKWHICH_DEFAULT) {
				pPropTopRels->cpmuRelTuple[i].clkDomainIdxSrc = CTRL_BOARDOBJ_IDX_INVALID_16BIT;
				pPropTopRels->cpmuRelTuple[i].clkDomainIdxDst = CTRL_BOARDOBJ_IDX_INVALID_16BIT;
				continue;
			}

			status = nvgpu_pmu_clk_domain_get_index_by_domain(
						g,
						pClocksHal->cpmuCplutClkRelMappings.columnMappings[i].clkSrc,
						&clkIdx);
			if (status != 0) {
				nvgpu_err(g, "Error CLKWHICH Src (0x%x) for CPMU CPLUT Mapping  (%d) - status: %d",
					pClocksHal->cpmuCplutClkRelMappings.columnMappings[i].clkSrc,
					i, status);
				/* to avoid MISRA violation, single exit point at top of loop */
				continue;
			}
			pPropTopRels->cpmuRelTuple[i].clkDomainIdxSrc = (u16)clkIdx;

			status = nvgpu_pmu_clk_domain_get_index_by_domain(
						g,
						pClocksHal->cpmuCplutClkRelMappings.columnMappings[i].clkDst,
						&clkIdx);
			if (status != 0) {
				nvgpu_err(g, "Error CLKWHICH Dst (0x%x) for CPMU CPLUT Mapping (%d) - status: %d",
					pClocksHal->cpmuCplutClkRelMappings.columnMappings[i].clkDst,
					i, status);
				/* to avoid MISRA violation, single exit point at top of loop */
				continue;
			}

			pPropTopRels->cpmuRelTuple[i].clkDomainIdxDst = (u16)clkIdx;
		} else {
			pPropTopRels->cpmuRelTuple[i].clkDomainIdxSrc = CTRL_BOARDOBJ_IDX_INVALID_16BIT;
			pPropTopRels->cpmuRelTuple[i].clkDomainIdxDst = CTRL_BOARDOBJ_IDX_INVALID_16BIT;
		}
	}
	if (status != 0) {
		goto done;
	}

	// Check the table base entry sizes:
	if (header.entry_size < VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_ENTRY_SIZE_05) {
		status = -EINVAL;
		nvgpu_err(g,
				"Unrecognized Clock Propagation Topology Relationship Table entry size - size=0x%02x.",
				header.entry_size);
		goto done;
	}

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
		// Zero out the clkPropTopRelData structure to remove whatever changes were
		// made from previous iteration.
		//
		memset(&clkPropTopRelData, 0x0, sizeof(clkPropTopRelData));

		// Read base entry from the VBIOS.
		entryPtr = clkPropTopRelsTablePtr +
					header.header_size +
					(i * header.entry_size);

		memcpy(&entry, entryPtr, sizeof(entry));

		// Skip parsing disabled clock enumeration entries.
		data8 = BIOS_GET_FIELD(u8, entry.flags0,
				NV_VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_ENTRY_FLAGS0_TYPE);
		if (data8 == NV_VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_ENTRY_FLAGS0_TYPE_DISABLED) {
			continue;
		}

		switch (data8) {
		case NV_VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_ENTRY_FLAGS0_TYPE_1X_RATIO:
		{
			u16 ratio;

			clkPropTopRelData.boardObj.type =
				NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_RATIO;

			ratio = BIOS_GET_FIELD(u16, entry.param0,
				NV_VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_ENTRY_PARAM0_RATIO_RATIO);

			// Convert FXP 4.12 -> FXP 16.16
			clkPropTopRelData.v1xRatio.ratio = (((u32)ratio) << 4);

			break;
		}
		case NV_VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_ENTRY_FLAGS0_TYPE_1X_VOLT:
		{
			clkPropTopRelData.boardObj.type =
				NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_VOLT;

			clkPropTopRelData.v1xVolt.voltRailIdx = BIOS_GET_FIELD(u8,
				entry.param0,
				NV_VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_ENTRY_PARAM0_VOLT_RAIL_IDX);

			break;
		}
		default:
		{
			nvgpu_err(g, "Invalid Clock Propagation Topology Relationship Type = %d, entry = %d.",
						data8, i);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		}

		// Update the common super class params.
		data8 = BIOS_GET_FIELD(u8, entry.flags0,
				NV_VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_ENTRY_FLAGS0_BIDIRECTIONAL_REL);
		clkPropTopRelData.super.bBiDirectional  = (data8 ==
				NV_VBIOS_CLOCK_PROP_TOP_REL_TABLE_10_ENTRY_FLAGS0_BIDIRECTIONAL_REL_ENABLED) ? true : false;
		clkPropTopRelData.super.clkDomainIdxSrc = entry.clkDomainIdxSrc;
		clkPropTopRelData.super.clkDomainIdxDst = entry.clkDomainIdxDst;

		// Construct CLK_PROP_TOP_REL for the entry.
		pPropTopRel = clkPropTopRelIfaceModel10Construct(g, &clkPropTopRelData.super);
		if (pPropTopRel == NULL) {
			status = -EINVAL;
			nvgpu_err(g,
						"Could not construct CLK_PROP_TOP_REL for entry %d.", i);
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		// Insert CLK_PROP_TOP_REL into the CLK_PROP_TOP_RELS group.
		status = pPropTopRels->super.super.objinsert(
					&pPropTopRels->super.super, &pPropTopRel->super, i);
		if (status != 0) {
			nvgpu_err(g,
						"Could not insert CLK_PROP_TOP_REL entry %d into CLK_PROP_TOP_RELS group - status=%d",
						i, status);
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

	}

done:
	nvgpu_pmu_dbg(g, " done status %x", status);

	return status;
}

s32 clk_prop_top_rel_sw_setup(struct gk20a *g)
{
	s32 status = 0;
	struct nvgpu_clk_prop_top_rels *clkproptoprelobjs = NULL;
	u8 version;
	u8 *prop_table_ptr = NULL;

	prop_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
			CLOCK_PROP_TOP_REL_TABLE);

	if (prop_table_ptr == NULL) {
		status = -1;
		nvgpu_err(g, "VBIOS Clock Propagation Topology Relationship table not found");
		goto done;
	}

	// Check VBIOS table version.
	version = *prop_table_ptr;

	if (version != (u8)VBIOS_CLOCK_PROP_TOP_REL_TABLE_VERSION_10) {
		nvgpu_err(g,
			"Unsupported Clock Propagation Topology Relationship table version: 0x%x",
			version);
		status = -EINVAL;
		goto done;
	}

	status = nvgpu_boardobjgrp_construct_e255(g,
			&g->pmu->clk_pmu->clk_top_relobjs->super);
	if (status != 0) {
		nvgpu_err(g,
			"Error creating boardobjgrp for Propagation Topology Relationship, status: %d.",
			status);
		goto done;
	}

	clkproptoprelobjs = g->pmu->clk_pmu->clk_top_relobjs;

	status = devinit_get_clk_prop_top_rel_table(g, prop_table_ptr,
				clkproptoprelobjs);

	if (status != 0) {
		nvgpu_err(g,
			"Error parsing the Clock Propagation Topology Relationship Vbios tables - %d.",
			status);
	}

done:
	return status;
}

s32 clk_prop_top_rel_pmu_setup(struct gk20a *g)
{
	/* todo: implement sending to PMU */
	return -EINVAL;
}

s32 clk_prop_top_rel_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmu->clk_pmu->clk_top_relobjs != NULL) {
		return 0;
	}

	g->pmu->clk_pmu->clk_top_relobjs = nvgpu_kzalloc(g,
			sizeof(struct nvgpu_clk_prop_top_rels));
	if (g->pmu->clk_pmu->clk_top_relobjs == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void clk_prop_top_rel_free_pmupstate(struct gk20a *g)
{
	nvgpu_kfree(g, g->pmu->clk_pmu->clk_top_relobjs);
	g->pmu->clk_pmu->clk_top_relobjs = NULL;
}

/*!
 * Helper interface to calculate inverse ratio for given input ratio.
 *
 * @param[in]  ratio  Input ratio. Fixed point 16x16
 *
 * @return inverse ratio on success
 * @return zero on error
 */
static u32 clkPropTopRel1xRatioCalcRatioInverse(struct gk20a *g, u32 ratio)
{
	u32 ratioInverse = 0;
	u64 ratioTemp;

	// If input ratio is zero, return output inverse zero
	if (ratio == 0) {
		nvgpu_err(g,
			"Invalid ratio input: ratio - %d.", ratio);
		goto clkPropTopRel1xRatioCalcRatioInverse_exit;
	}

	//
	// Calculation logic:
	//
	// Here we have two FXP numbers x and y with scale s.
	// If we perform simple division, we get wrong result.
	//      x.s / y.s = x * s / y * s = x / y != (x / y).s
	//
	// For retaining the scaling factor and precision, we must perform the following.
	//      ((u64)x.s * (1.s)) / y.s = (x / y).s
	//
	ratioTemp = ((((u64)1) << 32) / ratio);

	// Check for overflow.
	if (u64_hi32(ratioTemp) != 0) {
		nvgpu_err(g,
			"Current ratio resulted in overflow during ratioInverse Calculation: ratio - %d, result = %d.",
			ratio, u64_hi32(ratioTemp));
		goto clkPropTopRel1xRatioCalcRatioInverse_exit;
	}
	ratioInverse = u64_lo32(ratioTemp);

clkPropTopRel1xRatioCalcRatioInverse_exit:
	return ratioInverse;
}
