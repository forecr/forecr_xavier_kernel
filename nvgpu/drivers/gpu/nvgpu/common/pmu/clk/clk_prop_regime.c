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
#include "clk_prop_regime.h"

struct vbios_clock_prop_regime_table_hal
	vbiosClkPropRegimeHALs[] = {
	{
		NV2080_CTRL_CLK_CLK_PROP_REGIME_HAL_GA10X,
		{
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_LOOSE,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_DRAM_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_DRAM_LOCK,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_GPC_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_DISP_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_PCIE_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_XBAR_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_XBAR_LOCK,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_PERF_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_IMP,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_IMP_CLIENT_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_GPC_POWER_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_XBAR_POWER_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_NVVDD_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MSVDD_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MSVDD_PERF_STRICT,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_RSVD_1,
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_RSVD_2,
			 // always last unless we hit array bound:
			NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_INVALID
		}
	}
};

static s32 clkPropRegimeDestruct(struct pmu_board_obj *obj)
{
	// No destruct actions
	(void) obj;
	return 0;
}

static struct clk_prop_regime *clkPropRegimeConstruct(struct gk20a *g,
			struct clk_prop_regime *pArgs)
{
	s32 status = 0;
	u8 objType;
	struct pmu_board_obj *pBoardObj = NULL;
	struct clk_prop_regime *pObj = NULL;
	struct pmu_board_obj *pObjTmp = (struct pmu_board_obj *)pArgs;

	nvgpu_pmu_dbg(g, " ");

	if (pArgs == NULL) {
		status = -EINVAL;
		goto done;
	}

	objType = pmu_board_obj_get_type(pArgs);
	if (objType != NV2080_CTRL_CLK_CLK_PROP_REGIME_TYPE_1X) {
		status = -EINVAL;
		goto done;
	}

	pObj = nvgpu_kzalloc(g, sizeof(struct clk_prop_regime));
	if (pObj == NULL) {
		status = -ENOMEM;
		goto done;
	}
	pBoardObj = &pObj->super;
	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_PROP_REGIME_TYPE_1X);
	pObjTmp->type_mask |= (u32)BIT(NV2080_CTRL_CLK_CLK_PROP_REGIME_TYPE_MODEL_10);
	status = pmu_board_obj_construct_super(g,
				pBoardObj, pArgs);
	if (status != 0) {
		status = -EINVAL;
		goto done;
	}
	// Todo: construct / Set BOARDOBJ interfaces. (Model10)

	// Override BOARDOBJ function pointers
	pBoardObj->destruct = clkPropRegimeDestruct;

	// Todo: Override BOARDOBJ_IFACE_MODEL_10 function pointers
done:
	if (status != 0) {
		nvgpu_err(g, "Error constructing CLK_PROP_REGIME: status=%d.",
		status);
	}
	nvgpu_pmu_dbg(g, " done status %x", status);

	return pObj;
}

static s32 devinitClockPropRegimeTableHALTranslate_1X(struct gk20a *g,
			u8 regimeHal,
			struct vbios_clock_prop_regime_table_hal **ppRegimeHAL)
{
	// Check that Clock Propagation Regime HAL is supported.
	// We only support GA10X regime xlate table.
	if (regimeHal !=  NV2080_CTRL_CLK_CLK_PROP_REGIME_HAL_GA10X) {
		nvgpu_err(g,
		"Unrecognized Clock Propagation Regime Table Regime HAL - regimeHal: 0x%02x.",
		regimeHal);
		return -EINVAL;
	}
	*ppRegimeHAL = &vbiosClkPropRegimeHALs[regimeHal];

	return 0;
}

static s32 devinit_get_clk_prop_regime_table(struct gk20a *g,
			u8 *clkPropRegimesTablePtr,
			struct nvgpu_clk_prop_regimes *pPropRegimes)
{
	u32 status = 0;
	struct vbios_clock_prop_regime_table_10_header header = { 0 };
	struct vbios_clock_prop_regime_table_10_entry  entry  = { 0 };
	struct vbios_clock_prop_regime_table_hal *pRegimeHAL = NULL;
	union {
		struct pmu_board_obj    boardObj;
		struct clk_prop_regime  v1x;
	} clkPropRegimeData;
	struct clk_prop_regime *pPropRegime;

	u32	i;
	u8	*entryPtr;
	u8	data8;
	u8	clkDomIdx;

	nvgpu_pmu_dbg(g, " ");

	memcpy(&header, clkPropRegimesTablePtr, sizeof(header));

	// we only support size of 5 or greater and thus
	// a 32 bit mask size.
	if (header.entry_size < VBIOS_CLOCK_PROP_REGIME_TABLE_10_ENTRY_SIZE_05) {
		nvgpu_err(g, "Unrecognized Clock Propagation Regime Table entry size - size=0x%x.",
			header.entry_size);
		status = -EINVAL;
		goto done;
	}

	// Get the Clock Propagation Regime HAL table.
	status = devinitClockPropRegimeTableHALTranslate_1X(g,
			header.regimeHal, &pRegimeHAL);
	if (status != 0) {
		nvgpu_err(g,
			"Error translating Clock Propagation Regime Table Regime Id HAL - status: %d.",
			status);
		goto done;
	}

	pPropRegimes->regimeHal = pRegimeHAL->regimeHal;


	// Parse each entry.
	for (i = 0; i < header.entry_count; i++) {
		u32 lclMask = 0;
		/* To avoid MISRA violation,
		 * this is a single point of exit
		 * from this for loop in the case
		 * of an error is encountered.
		 */
		if (status != 0)
			break;
		//
		// Zero out the clkPropRegimeData structure for
		// next iteration.
		//
		memset(&clkPropRegimeData, 0x0, sizeof(clkPropRegimeData));

		// Copy base entry from the VBIOS.
		entryPtr = clkPropRegimesTablePtr + header.header_size + (i * header.entry_size);
		memcpy(&entry, entryPtr, sizeof(entry));
		// Skip disabled entries
		data8 = BIOS_GET_FIELD(u8, entry.flags0,
				VBIOS_CLOCK_PROP_REGIME_TABLE_10_ENTRY_FLAGS0_TYPE);

		if (data8 == VBIOS_CLOCK_PROP_REGIME_TABLE_10_ENTRY_FLAGS0_TYPE_DISABLED) {
			continue;
		}

		// We only support Type 1x
		if (data8 == VBIOS_CLOCK_PROP_REGIME_TABLE_10_ENTRY_FLAGS0_TYPE_1X) {
			clkPropRegimeData.boardObj.type = NV2080_CTRL_CLK_CLK_PROP_REGIME_TYPE_1X;
		} else {
			nvgpu_err(g, "Invalid Clock Propagation Regime Type = %d, entry = %d.",
					data8, i);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		if ((pRegimeHAL->regimeId[i] == NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_INVALID) ||
			(i >= NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MAX)) {
			nvgpu_err(g, "Invalid Clock Propagation Regime HAL - ENTRY mapping. HAL = %d, regimeId[%d] = %d.",
					pRegimeHAL->regimeHal, i, pRegimeHAL->regimeId[i]);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		// Update regime id.
		clkPropRegimeData.v1x.regimeId = pRegimeHAL->regimeId[i];

		// Parse clock domain index mask.
		status = nvgpu_boardobjgrpmask_init(
					&clkPropRegimeData.v1x.clkDomainMask.super,
					CTRL_BOARDOBJGRP_E32_MAX_OBJECTS, NULL);
		if (status != 0) {
			nvgpu_err(g,
					"Error constructing clock domain mask - status = %d.",
					status);
			status = -EINVAL;
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		lclMask = entry.clkDomainMask;
		for (clkDomIdx = 0; lclMask != 0; clkDomIdx++) {
			if ((((u32)0x01) & lclMask) == 0U) {
				lclMask >>= 1;
				continue;
			}

			status = nvgpu_boardobjgrpmask_bit_set(
						&clkPropRegimeData.v1x.clkDomainMask.super,
						clkDomIdx);
			if (status != 0) {
				nvgpu_err(g,
						"Error setting clock domain mask: clkDomIdx=%d, status=%d.",
						clkDomIdx, status);
				status = -EINVAL;
				break;
			}
			lclMask >>= 1U;
		}
		// check for failure
		if (status != 0) {
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		// Construct CLK_PROP_REGIME for the entry.
		pPropRegime = clkPropRegimeConstruct(g, &clkPropRegimeData.v1x);
		if (pPropRegime == NULL) {
			status = -EINVAL;
			nvgpu_err(g, "Could not construct CLK_PROP_REGIME for entry %d.", i);
			/* to avoid MISRA violation, single exit point at top of loop */
			continue;
		}

		// Insert CLK_PROP_REGIME into the CLK_PROP_REGIMES group.
		status = boardobjgrp_objinsert(&pPropRegimes->super.super,
				&pPropRegime->super, (u8)i);
		if (status != 0) {
			nvgpu_err(g,
				"Inserting CLK_PROP_REGIME device object to boardObjGrp failed for Entry: %d.",
				i);
			/* to avoid MISRA violation, exit at top of loop */
			continue;
		}

	}

done:
	nvgpu_pmu_dbg(g, " done status %x", status);

	return status;
}

s32 clk_prop_regime_sw_setup(struct gk20a *g)
{
	struct nvgpu_clk_prop_regimes *pclkpropregimeobjs = NULL;
	s32 status = 0;
	u8 version;
	u8 *prop_table_ptr = NULL;

	prop_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
			CLOCK_REGIM_TABLE);

	if (prop_table_ptr == NULL) {
		status = -1;
		nvgpu_err(g, "VBIOS Clock Propagation Regime table not found");
		goto done;
	}

	// Check VBIOS table version.
	version = *prop_table_ptr;

	if (version != VBIOS_CLOCK_PROP_REGIME_TABLE_VERSION_10) {
		nvgpu_err(g, "Unsupported Clock Propagation Regime table version 0x%x.",
				version);
		status = -EINVAL;
		goto done;
	}

	status = nvgpu_boardobjgrp_construct_e32(g,
			&g->pmu->clk_pmu->clk_regimeobjs->super);
	if (status != 0) {
		nvgpu_err(g,
			"Error creating boardobjgrp for Propagation Regime, status: %d.",
			status);
		goto done;
	}

	memset(&g->pmu->clk_pmu->clk_regimeobjs->regimeIdToIdxMap,
		0xffu,
		(sizeof(u16) * NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MAX));

	pclkpropregimeobjs = g->pmu->clk_pmu->clk_regimeobjs;
	// Parse VBIOS Clock Propagation Regime Table.
	status = devinit_get_clk_prop_regime_table(g, prop_table_ptr,
				pclkpropregimeobjs);
	if (status != 0) {
		nvgpu_err(g, "Error parsing the Clock Propagation Regime Vbios tables - %d.",
					status);
		goto done;
	}

done:
	return status;
}

s32 clk_prop_regime_pmu_setup(struct gk20a *g)
{
	/* todo: implement sending to PMU */
	return -EINVAL;
}

s32 clk_prop_regime_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmu->clk_pmu->clk_regimeobjs != NULL) {
		return 0;
	}

	g->pmu->clk_pmu->clk_regimeobjs = nvgpu_kzalloc(g,
			sizeof(struct nvgpu_clk_prop_regimes));
	if (g->pmu->clk_pmu->clk_regimeobjs == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void clk_prop_regime_free_pmupstate(struct gk20a *g)
{
	nvgpu_kfree(g, g->pmu->clk_pmu->clk_regimeobjs);
	g->pmu->clk_pmu->clk_regimeobjs = NULL;
}

