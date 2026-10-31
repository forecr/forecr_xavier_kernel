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



static s32 clkVfRelInModel10Construct_4X_RATIO(struct gk20a *g,
		struct clk_vf_rel_4x_ratio *pVfRelRatio4x,
		struct clk_vf_rel_4x_ratio *pArgs)
{
	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL || pVfRelRatio4x == NULL)
		return -EINVAL;

	// Set type-specific parameters.
	nvgpu_memcpy((u8 *)(pVfRelRatio4x->slaveEntries), (u8 *)(pArgs->slaveEntries),
				(sizeof(struct nv_pmu_clk_clk_vf_rel_table_secondary_entry)
				* NV2080_CTRL_CLK_CLK_VF_REL_RATIO_SECONDARY_ENTRIES_MAX));

	return 0;
}

static s32 clkVfRelInModel10Construct_4X_RATIO_VOLT(
			struct gk20a *g,
			struct clk_vf_rel_4x_ratio_volt *pVfRelRatioVolt4x,
			struct clk_vf_rel_4x_ratio_volt *pArgs)
{
	s32 status = 0;
	u64 maxFreqStepSizeMHz;
	u8 vfSmoothEntryIdx;

	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL || pVfRelRatioVolt4x == NULL) {
		status = -EINVAL;
		goto done;
	}

	// Set type-specific parameters.
	pVfRelRatioVolt4x->vfSmoothDataGrp = pArgs->vfSmoothDataGrp;

	// Maths to calculate the max step size between two consecutive VF points.
	// U32 * UFXP 20.12 = UFXP 52.12
	// (U64 >> 12) = U52
	// U32 = (U32)U52
	// 10000 * 0.0112 = 112 MHz, Very low risk of overflow.
	// Performing smoothening calculation for all ramp rates.
	for (vfSmoothEntryIdx = 0;
			vfSmoothEntryIdx < pVfRelRatioVolt4x->vfSmoothDataGrp.vfSmoothDataEntriesCount;
			vfSmoothEntryIdx++) {
		maxFreqStepSizeMHz = g->pmu->clk_pmu->avfs_fllobjs->lut_step_size_uv;
		maxFreqStepSizeMHz =  maxFreqStepSizeMHz *
			pVfRelRatioVolt4x->vfSmoothDataGrp.vfSmoothDataEntries[vfSmoothEntryIdx].maxVFRampRate;

		// Check for overflow
		if (maxFreqStepSizeMHz >> 12 > U16_MAX) {
			status = -EOVERFLOW;
			goto done;
		}
		pVfRelRatioVolt4x->vfSmoothDataGrp.vfSmoothDataEntries[vfSmoothEntryIdx].maxFreqStepSizeMHz =
			(u16)(maxFreqStepSizeMHz >> 12);
	}

done:
	return status;
}

static s32 clkVfRelVfFlatten_SUPER(struct gk20a *g,
		struct clk_vf_rel *pVfRel, u8 clkDomainIdx)
{
	s32 status = -EINVAL;

	nvgpu_pmu_dbg(g, " ");
	// Todo: implement the VF flatten process v5.0 once we have
	// VF Points 5.0 class refactor implemented.
	(void) pVfRel;
	(void) clkDomainIdx;

	return status;
}

static s32 clkVfRelIfaceModel10Construct_SUPER(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj, u16 size, void *pArgs)
{
	struct clk_vf_rel *pVfRelTmp = (struct clk_vf_rel *)pArgs;
	struct clk_vf_rel *pVfRel = NULL;
	s32 status = 0;

	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL || ppBoardObj == NULL) {
		status = -EINVAL;
		goto done;
	}

	pVfRel = nvgpu_kzalloc(g, size);
	if (pVfRel == NULL) {
		status = -ENOMEM;
		goto done;
	}
	*ppBoardObj = (struct pmu_board_obj *)pVfRel;
	status = pmu_board_obj_construct_super(g,
				*ppBoardObj, pArgs);
	if (status != 0) {
		status = -EINVAL;
		goto done;
	}
	// Todo: Set BOARDOBJ interfaces. (Model10)

	// Set CLK_VF_REL interfaces.
	pVfRel->vfFlatten = clkVfRelVfFlatten_SUPER;

	// Set VF REL class specific params.
	pVfRel->rail_idx       = pVfRelTmp->rail_idx;
	pVfRel->b_ocov_enabled = pVfRelTmp->b_ocov_enabled;
	pVfRel->freq_max_mhz   = pVfRelTmp->freq_max_mhz;
	pVfRel->volt_delta_uv  = pVfRelTmp->volt_delta_uv;
	pVfRel->freq_delta     = pVfRelTmp->freq_delta;
	pVfRel->vf_entry_pri   = pVfRelTmp->vf_entry_pri;

	nvgpu_memcpy((u8 *)(pVfRel->vf_entries_sec), (u8 *)(pVfRelTmp->vf_entries_sec),
				(sizeof(struct mv_pmu_clk_clk_vf_rel_vf_entry_sec)
				* NV2080_CTRL_CLK_CLK_VF_REL_VF_ENTRY_SEC_MAX));

done:
	nvgpu_pmu_dbg(g, " done status %x", status);
	return status;
}

static s32 clkVfRelIfaceModel10Construct_50(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj, u16 size, void *pArgs)
{
	struct pmu_board_obj *pObjTmp = (struct pmu_board_obj *)pArgs;
	struct clk_vf_rel_50 *pVfRel50Tmp = (struct clk_vf_rel_50 *)pArgs;
	struct clk_vf_rel_50 *pVfRel50;
	s32 status = 0;

	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL || ppBoardObj == NULL) {
		status = -EINVAL;
		goto done;
	}

	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50);

	status = clkVfRelIfaceModel10Construct_SUPER(g, ppBoardObj, size, pArgs);
	if (status != 0)
		goto done;

	pVfRel50 = (struct clk_vf_rel_50 *) *ppBoardObj;

	// Set BOARDOBJ interfaces.  Todo. (Model10)

	// Set VF REL class specific params.
	pVfRel50->safe_vf_curve_tuple_idx    = pVfRel50Tmp->safe_vf_curve_tuple_idx;
	pVfRel50->vf_tuple_idx_first         = pVfRel50Tmp->vf_tuple_idx_first;
	pVfRel50->vf_tuple_idx_last          = pVfRel50Tmp->vf_tuple_idx_last;
	pVfRel50->picked_vf_tuple_idx        = pVfRel50Tmp->picked_vf_tuple_idx;
	pVfRel50->current_vf_curve_cache_idx = pVfRel50Tmp->current_vf_curve_cache_idx;

	pVfRel50->super.vf_entry_pri.safeVfCurveTupleIdx = pVfRel50Tmp->safe_vf_curve_tuple_idx;
	pVfRel50->super.vf_entry_pri.vfTupleIdxFirst     = pVfRel50Tmp->vf_tuple_idx_first;
	pVfRel50->super.vf_entry_pri.vfTupleIdxLast      = pVfRel50Tmp->vf_tuple_idx_last;

done:
	nvgpu_pmu_dbg(g, " done status %x", status);
	return status;
}

static s32 clkVfRelIfaceModel10Construct_50_RATIO(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj, u16 size, void *pArgs)
{
	struct pmu_board_obj *pObjTmp = (struct pmu_board_obj *)pArgs;
	struct clk_vf_rel_50_ratio *pVfRelRatio50Tmp = (struct clk_vf_rel_50_ratio *)pArgs;
	struct clk_vf_rel_50_ratio *pVfRelRatio50;
	s32 status = 0;

	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL || ppBoardObj == NULL) {
		status = -EINVAL;
		goto done;
	}

	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO);
	status = clkVfRelIfaceModel10Construct_50(g, ppBoardObj, size, pArgs);
	if (status != 0)
		goto done;

	pVfRelRatio50 = (struct clk_vf_rel_50_ratio *) *ppBoardObj;

	// Set BOARDOBJ interfaces.  Todo. (Model10)

	// Construct CLK_VF_REL_4X_RATIO interface.
	status = clkVfRelInModel10Construct_4X_RATIO(g,
			&pVfRelRatio50->ratio,
			&pVfRelRatio50Tmp->ratio);

done:
	nvgpu_pmu_dbg(g, " done status %x", status);
	return status;
}


static s32 clkVfRelIfaceModel10Construct_50_RATIO_VOLT(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj, u16 size, void *pArgs)
{
	struct pmu_board_obj *pObjTmp = (struct pmu_board_obj *)pArgs;
	struct clk_vf_rel_50_ratio_volt *pVfRelRatioVolt50Tmp = (struct clk_vf_rel_50_ratio_volt *)pArgs;
	struct clk_vf_rel_50_ratio_volt *pVfRelRatioVolt50;
	s32 status = 0;

	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL || ppBoardObj == NULL) {
		status = -EINVAL;
		goto done;
	}

	// Call into CLK_VF_REL super constructor.
	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO_VOLT);
	status = clkVfRelIfaceModel10Construct_50_RATIO(g, ppBoardObj, size, pArgs);
	if (status != 0)
		goto done;

	pVfRelRatioVolt50 = (struct clk_vf_rel_50_ratio_volt *) *ppBoardObj;

	// Todo: Wire in RPC related:  pmuDataInit, getInfo, etc.

	// Construct CLK_VF_REL_4X_RATIO_VOLT interface.
	status = clkVfRelInModel10Construct_4X_RATIO_VOLT(g,
				&pVfRelRatioVolt50->ratioVolt,
				&pVfRelRatioVolt50Tmp->ratioVolt);

	// Set CLK_VF_REL_50_RATIO_VOLT class specific params.
	pVfRelRatioVolt50->vfGenMethod                 = pVfRelRatioVolt50Tmp->vfGenMethod;
	pVfRelRatioVolt50->devinitVfeIdx               = pVfRelRatioVolt50Tmp->devinitVfeIdx;
	pVfRelRatioVolt50->favDeratedSubCurveVFEEqnIdx =
			pVfRelRatioVolt50Tmp->favDeratedSubCurveVFEEqnIdx;

done:
	nvgpu_pmu_dbg(g, " done status %x", status);
	return status;
}

static struct clk_vf_rel *clkVfRelConstruct(struct gk20a *g, void *pArgs)
{
	struct pmu_board_obj *pBoardObj = NULL;
	struct clk_vf_rel *pObj = NULL;
	s32 status = 0;
	u8 objType;

	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL) {
		status = -EINVAL;
		goto done;
	}

	objType = pmu_board_obj_get_type(pArgs);

	switch (objType) {
	case NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO_VOLT:
	{
		status = clkVfRelIfaceModel10Construct_50_RATIO_VOLT(g, &pBoardObj,
					sizeof(struct clk_vf_rel_50_ratio_volt), pArgs);
		break;
	}
	default:
	{
		nvgpu_err(g, "Unsupported CLK_VF_REL type = 0x%08x.", objType);
	}
	}
	pObj = (struct clk_vf_rel *)pBoardObj;

done:
	nvgpu_pmu_dbg(g, " done status %x", status);
	return pObj;
}

/*!
 * Parse Clocks VF Relationship Table with version 0x50.
 */
static s32 devinit_get_clk_vf_rel_table(struct gk20a *g,
			struct nvgpu_clk_vf_rels *pvfrelobjs)
{
	s32 status = 0;
	u8 *vfrel_table_ptr = NULL;
	struct vbios_clock_vf_rel_table_20_header header = { 0 };
	struct vbios_clock_vf_rel_table_20_entry entry = { 0 };
	struct vbios_clock_vf_rel_table_20_secondary_entry secondaryEntry = { 0 };
	struct vbios_clock_vf_rel_table_20_vf_entry_sec vfEntrySec = { 0 };
	union {
		struct pmu_board_obj            boardObj;
		struct clk_vf_rel               vfRel;
		struct clk_vf_rel_50            vfRel50;
		struct clk_vf_rel_50_ratio      vfRelRatio50;
		struct clk_vf_rel_50_ratio_volt vfRelRatioVolt50;
		struct clk_vf_rel_50_ratio_freq vfRelRatioFreq50;
		struct clk_vf_rel_50_table      vfRelTable50;
		struct clk_vf_rel_50_table_freq vfRelTableFreq50;
	} clkVfRelData;
	struct clk_vf_rel *pVfRel = NULL;
	int i, j;
	u8 *entryPtr = NULL;
	u8 *secEntryPtr = NULL;
	u8 vfEntryCountSec = 0;
	u32 vfSecSz = 0;
	u8 vfEntryCountSecMax = 0;

	nvgpu_pmu_dbg(g, " ");

	if (pvfrelobjs == NULL) {
		status = -EINVAL;
		goto done;
	}

	vfrel_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
						VF_RELATIONSHIP);
	if (vfrel_table_ptr == NULL) {
		status = -1;
		nvgpu_err(g, "VBIOS VF Relationship table not found");
		goto done;
	}

	memcpy(&header, vfrel_table_ptr, sizeof(header));
	// Sanity check table header size
	if (header.header_size < VBIOS_CLOCK_VF_REL_TABLE_20_HEADER_SIZE_08) {
		nvgpu_err(g, "Unrecognized Clock VF Rel Table header size: 0x%x",
				header.header_size);
		status = -EINVAL;
		goto done;
	}

	// Sanity check base entry size
	if (header.entry_size < VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_SIZE_23) {
		nvgpu_err(g, "Unrecognized Clock VF Rel Table base entry size: 0x%x",
					header.header_size);
		status = -EINVAL;
		goto done;
	}
	// Sanity check secondary entry size
	if (header.secondary_entry_size < VBIOS_CLOCK_VF_REL_TABLE_20_SECONDARY_ENTRY_SIZE_03) {
		nvgpu_err(g, "Unrecognized Clock VF Rel Table secondary entry size: 0x%x",
					header.secondary_entry_size);
		status = -EINVAL;
		goto done;
	}
	// Sanity check vf entry size
	if (header.vf_entry_size_sec >= VBIOS_CLOCK_VF_REL_TABLE_20_VF_ENTRY_SEC_SIZE_06) {
		vfSecSz = VBIOS_CLOCK_VF_REL_TABLE_20_VF_ENTRY_SEC_SIZE_06;
	} else if (header.vf_entry_size_sec >= VBIOS_CLOCK_VF_REL_TABLE_20_VF_ENTRY_SEC_SIZE_04) {
		vfSecSz = VBIOS_CLOCK_VF_REL_TABLE_20_VF_ENTRY_SEC_SIZE_04;
	} else {
		nvgpu_err(g, "Unrecognized Clock VF Rel Table vf entry size: 0x%x",
					header.vf_entry_size_sec);
		status = -EINVAL;
		goto done;
	}
	//
	// Save off the secondary and VF entry counts.  These will be used to allocate
	// and iterate over arrays within CLK_VF_REL objects.
	//
	pvfrelobjs->slave_entry_count = (u8)header.secondary_entry_count;
	pvfrelobjs->vf_entry_count_sec = (u8)header.vf_entry_count_sec;

	// Parse each entry.
	for (i = 0; i < header.entry_count; i++) {
		u8 type8;
		u8 flag8;
		/* To avoid MISRA violation,
		 * this is a single point of exit
		 * from this for loop in the case
		 * of an error is encountered.
		 */
		if (status != 0)
			break;
		//
		// Zero out the clkDomainData structure to remove whatever changes were
		// made from previous iteration.
		//
		memset((u8 *)&clkVfRelData, 0x0, sizeof(clkVfRelData));

		// Read base entry from the VBIOS.
		entryPtr = vfrel_table_ptr + header.header_size +
					i * (header.entry_size +
					(header.secondary_entry_count * header.secondary_entry_size) +
					(header.vf_entry_count_sec * header.vf_entry_size_sec));
		nvgpu_memcpy((u8 *)&entry, entryPtr,
				VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_SIZE_23);
		type8 = BIOS_GET_FIELD(u8, entry.flags0,
				NV_VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_FLAGS0_TYPE);
		if (type8 == NV_VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_FLAGS0_TYPE_DISABLED) {
			continue;
		}
		// Map VBIOS type to boardobj type
		switch (type8) {
		case NV_VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_FLAGS0_TYPE_RATIO_VOLT:
		{
			clkVfRelData.boardObj.type =
				NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO_VOLT;
			break;
		}
		case NV_VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_FLAGS0_TYPE_RATIO_FREQ:
		{
			clkVfRelData.boardObj.type =
				NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO_FREQ;
			break;
		}
		case NV_VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_FLAGS0_TYPE_TABLE_FREQ:
		{
			clkVfRelData.boardObj.type =
				NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_TABLE_FREQ;
			break;
		}
		default:
		{
			nvgpu_err(g, "Unrecognized type for entry: %d - type:0x%x",
				i, type8);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}
		}

		flag8 = BIOS_GET_FIELD(u8, entry.flags0,
				NV_VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_FLAGS0_OVOC_ENABLED);
		clkVfRelData.vfRel.b_ocov_enabled = (flag8 == 1 ? true : false);
		clkVfRelData.vfRel.freq_max_mhz = (u16)entry.freq_max_mhz;
		clkVfRelData.vfRel.vf_entry_pri.vfeIdx = (u16)entry.primary_vfe_idx;
		clkVfRelData.vfRel50.safe_vf_curve_tuple_idx = (u16)entry.safe_vf_curve_tuple_idx;
		clkVfRelData.vfRel50.vf_tuple_idx_first = (u16)entry.vf_tuple_idx_first;
		clkVfRelData.vfRel50.vf_tuple_idx_last = (u16)entry.vf_tuple_idx_last;

		// Init default values for VF rel v50.
		clkVfRelData.vfRel50.picked_vf_tuple_idx = NV2080_CTRL_CLK_CLK_VF_TUPLE_IDX_INVALID;
		clkVfRelData.vfRel50.current_vf_curve_cache_idx = CTRL_BOARDOBJ_IDX_INVALID;

		if (type8 == NV_VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_FLAGS0_TYPE_RATIO_VOLT) {
			struct clk_vf_rel_4x_ratio_volt *pClkVfRelRatioVolt4x =
						&clkVfRelData.vfRelRatioVolt50.ratioVolt;
			// Parse VF Generation method
			clkVfRelData.vfRelRatioVolt50.vfGenMethod = BIOS_GET_FIELD(u8,
				entry.flags0,
				NV_VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_FLAGS0_RATIO_VOLT_VF_GENERATION_METHOD);
			// Initialize smoothing data count
			pClkVfRelRatioVolt4x->vfSmoothDataGrp.vfSmoothDataEntriesCount = 0;
			// VF Smoothening data
			pClkVfRelRatioVolt4x->vfSmoothDataGrp.vfSmoothDataEntries[0].baseVFSmoothVoltuV =
				entry.param0;
			pClkVfRelRatioVolt4x->vfSmoothDataGrp.vfSmoothDataEntries[0].maxVFRampRate =
				entry.param1;
			pClkVfRelRatioVolt4x->vfSmoothDataGrp.vfSmoothDataEntries[0].maxFreqStepSizeMHz = 0;
			pClkVfRelRatioVolt4x->vfSmoothDataGrp.vfSmoothDataEntriesCount++;

			// Sanity check that number of VF smoothing entries is below the MAX.
			if (pClkVfRelRatioVolt4x->vfSmoothDataGrp.vfSmoothDataEntriesCount >
					NV2080_CTRL_CLK_CLK_VF_REL_RATIO_VOLT_VF_SMOOTH_DATA_ENTRIES_MAX) {
				nvgpu_err(g, "Excessive number of smoothing entries");
				status = -EINVAL;
				/* to avoid MISRA violation, single exit point at top of loop */
				continue;
			}
			clkVfRelData.vfRelRatioVolt50.devinitVfeIdx = entry.param2;
			clkVfRelData.vfRelRatioVolt50.favDeratedSubCurveVFEEqnIdx = entry.param3;
		}
		// CPM Max Freq Offset VFE Equation Idx is removed
		// from Clock VF Relationship version 50 table.
		clkVfRelData.vfRel.vf_entry_pri.cpmMaxFreqOffsetVfeIdx =
					NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID;
		// Init default values for VF Point indexes.
		clkVfRelData.vfRel.vf_entry_pri.vfPointIdxFirst =
					NV2080_CTRL_CLK_CLK_VF_POINT_IDX_INVALID;
		clkVfRelData.vfRel.vf_entry_pri.vfPointIdxLast  =
					NV2080_CTRL_CLK_CLK_VF_POINT_IDX_INVALID;

		// Parse secondary entries parameters.
		for (j = 0; j < header.secondary_entry_count; j++) {
			secEntryPtr = entryPtr +
						header.entry_size +
						j * header.secondary_entry_size;
			nvgpu_memcpy((u8 *)&secondaryEntry, secEntryPtr,
				VBIOS_CLOCK_VF_REL_TABLE_20_SECONDARY_ENTRY_SIZE_03);
			// Parse secondary entry dependent on type into temporary arrays.
			switch (clkVfRelData.boardObj.type) {
			case NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO_VOLT:
			case NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO_FREQ:
			{
				clkVfRelData.vfRelRatio50.ratio.slaveEntries[j].clkDomIdx =
					(u8)secondaryEntry.clk_dom_idx;
				clkVfRelData.vfRelRatio50.ratio.slaveEntries[j].ratio     =
					BIOS_GET_FIELD(u8, secondaryEntry.param0,
					NV_VBIOS_CLOCK_VF_REL_TABLE_20_SECONDARY_ENTRY_PARAM0_RATIO_RATIO);
				break;
			}
			case NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_TABLE_FREQ:
			{
				clkVfRelData.vfRelTable50.table.slaveEntries[j].clkDomIdx =
					(u8)secondaryEntry.clk_dom_idx;
				clkVfRelData.vfRelTable50.table.slaveEntries[j].freqMHz   =
					BIOS_GET_FIELD(u16, secondaryEntry.param0,
					NV_VBIOS_CLOCK_VF_REL_TABLE_20_SECONDARY_ENTRY_PARAM0_TABLE_FREQ);
				break;
			}
			default:
				break;
			}
		}

		// Parse secondary VF curves.
		vfEntryCountSec = 0;
		for (j = 0; j < pvfrelobjs->vf_entry_count_sec; j++) {
			secEntryPtr = entryPtr +
					header.entry_size +
					(header.secondary_entry_size
					* header.secondary_entry_count) +
					(j * header.vf_entry_size_sec);

			nvgpu_memcpy((u8 *)&vfEntrySec, secEntryPtr,
				vfSecSz);
			// Secondary VFE indexes.
			clkVfRelData.vfRel.vf_entries_sec[j].vfeIdx = (u16)vfEntrySec.secondaryVfeIdx;
			if (type8 == NV_VBIOS_CLOCK_VF_REL_TABLE_20_ENTRY_FLAGS0_TYPE_RATIO_VOLT) {
				clkVfRelData.vfRel.vf_entries_sec[j].dvcoOffsetVfeIdx =
					(u16)vfEntrySec.param0;
			} else {
				clkVfRelData.vfRel.vf_entries_sec[j].dvcoOffsetVfeIdx =
					(u16)NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID;
			}

			if (((clkVfRelData.vfRel.vf_entries_sec[j].vfeIdx) !=
					NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID) &&
					((clkVfRelData.vfRel.vf_entries_sec[j].dvcoOffsetVfeIdx) !=
					NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID)) {
				vfEntryCountSec++;
			} else if ((((clkVfRelData.vfRel.vf_entries_sec[j].vfeIdx) !=
					NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID) &&
					((clkVfRelData.vfRel.vf_entries_sec[j].dvcoOffsetVfeIdx) ==
					NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID)) ||
					(((clkVfRelData.vfRel.vf_entries_sec[j].vfeIdx) ==
					NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID) &&
					((clkVfRelData.vfRel.vf_entries_sec[j].dvcoOffsetVfeIdx) !=
					NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID))) {
				nvgpu_err(g,
					"Error: NOT all VBIOS entries of secondary VF curve are valid. vfeIdx = %d, dvcoOffsetVfeIdx = %d",
					clkVfRelData.vfRel.vf_entries_sec[j].vfeIdx,
					clkVfRelData.vfRel.vf_entries_sec[j].dvcoOffsetVfeIdx);
				status = -EINVAL;
				/* to avoid MISRA violation, single exit point at top of loop */
				continue;
			} else {
				nvgpu_pmu_dbg(g, "All VBIOS secondary VF curve entries are invalid and the secondary curve count is NOT adjusted.");
			}

			// Init default values for VF Point indexes.
			clkVfRelData.vfRel.vf_entries_sec[j].vfPointIdxFirst =
				NV2080_CTRL_CLK_CLK_VF_POINT_IDX_INVALID;
			clkVfRelData.vfRel.vf_entries_sec[j].vfPointIdxLast  =
				NV2080_CTRL_CLK_CLK_VF_POINT_IDX_INVALID;
			if (header.vf_entry_size_sec >=
				VBIOS_CLOCK_VF_REL_TABLE_20_VF_ENTRY_SEC_SIZE_06) {
				clkVfRelData.vfRel.vf_entries_sec[j].favDeratedSubCurveVFEEqnIdx =
					(u16)vfEntrySec.param1;
			} else {
				clkVfRelData.vfRel.vf_entries_sec[j].favDeratedSubCurveVFEEqnIdx =
						NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID;
			}
		}
		vfEntryCountSecMax = max(vfEntryCountSecMax, vfEntryCountSec);

		// Construct CLK_VF_REL for the entry.
		pVfRel = clkVfRelConstruct(g, &clkVfRelData);
		if (pVfRel == NULL) {
			status = -EINVAL;
			nvgpu_err(g, "Could not construct CLK_VF_REL for entry %d.", i);
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		// Insert CLK_VF_REL into the CLK_VF_RELS group.
		status = boardobjgrp_objinsert(&pvfrelobjs->super.super,
				(struct pmu_board_obj *)pVfRel, (u8)i);
		if (status != 0) {
			nvgpu_err(g,
				"Inserting clk_vf_rel device object to boardObjGrp failed for Entry: %d",
				i);
			/* to avoid MISRA violation, exit at top of loop */
			continue;
		}

	}

	if (status != 0) {
		goto done;
	}
	// Update the secondary VF curves count based on final POR.
	if (pvfrelobjs->vf_entry_count_sec != vfEntryCountSecMax) {
		nvgpu_pmu_dbg(g,
			"Invalid secondary VFE values observed in POR. POR Secondary curve count = %d, Valid VFE values got for curve = %d",
			pvfrelobjs->vf_entry_count_sec, vfEntryCountSecMax);

		pvfrelobjs->vf_entry_count_sec = vfEntryCountSecMax;
	}

done:
	nvgpu_pmu_dbg(g, " done status %x", status);
   return status;
}


s32 clk_vf_rel_sw_setup(struct gk20a *g)
{
	s32 status = 0;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct nvgpu_clk_vf_rels *pclkvfrelobjs = NULL;

	nvgpu_log_info(g, " ");

	if (g->pmu->clk_pmu->clk_vfrelobjs == NULL) {
		nvgpu_err(g, "Cannot find clk_vfrelobjs.");
		goto done;
	}
	status = nvgpu_boardobjgrp_construct_e255(g,
			&g->pmu->clk_pmu->clk_vfrelobjs->super);
	if (status != 0) {
		nvgpu_err(g,
			"Error creating boardobjgrp for Clk_vf_rel, status: %d",
			status);
		goto done;
	}
	pboardobjgrp = &g->pmu->clk_pmu->clk_vfrelobjs->super.super;
	pclkvfrelobjs = g->pmu->clk_pmu->clk_vfrelobjs;

	/* construct and set the boardobj class ID */
	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, CLK_VF_REL);

	// Construct the RPC call Set interface.
	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			clk, CLK, clk_vf_rel, CLK_VF_REL);
	if (status != 0) {
		nvgpu_err(g,
			"Error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - %d",
			status);
		goto done;
	}

	// Parse VBIOS Clock VfRel Table.
	status = devinit_get_clk_vf_rel_table(g, pclkvfrelobjs);
	if (status != 0) {
		nvgpu_err(g, "Error parsing the Clk_vf_rel Vbios tables - %d",
					status);
		goto done;
	}

	//
	// Now that CLK_DOMAIN, CLK_ENUM and CLK_VF_REL structures have been parsed
	// from the VBIOS, link them.
	//
	status = clk_domain_clk_prog_link(g, g->pmu->clk_pmu);
	if (status != 0) {
		nvgpu_err(g,
			"Error linking CLK_DOMAIN, CLK_ENUM and CLK_VF_REL - %d.",
			status);
		goto done;
	}

done:
	nvgpu_log_info(g, " done status %d", status);
	return status;
}

s32 clk_vf_rel_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmu->clk_pmu->clk_vfrelobjs != NULL) {
		return 0;
	}

	g->pmu->clk_pmu->clk_vfrelobjs = nvgpu_kzalloc(g,
			sizeof(struct nvgpu_clk_vf_rels));
	if (g->pmu->clk_pmu->clk_vfrelobjs == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void clk_vf_rel_free_pmupstate(struct gk20a *g)
{
	nvgpu_kfree(g, g->pmu->clk_pmu->clk_vfrelobjs);
	g->pmu->clk_pmu->clk_vfrelobjs = NULL;
}

s32 clk_vf_rel_pmu_setup(struct gk20a *g)
{
	/* todo: implement sending to PMU all clock VfRel boardobjs */
	return -EINVAL;
}

