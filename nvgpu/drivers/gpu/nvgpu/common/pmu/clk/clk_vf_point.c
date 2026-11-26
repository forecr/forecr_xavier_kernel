// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/boardobjgrp_e255.h>
#include <nvgpu/boardobjgrp_e2048.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/string.h>
#include <nvgpu/timers.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/clk_arb.h>
#include <nvgpu/pmu/volt.h>
#include <nvgpu/pmu/perf.h>
#include <nvgpu/pmu/cmd.h>

#include "ucode_clk_inf.h"
#include "clk_vf_point.h"
#include "clk.h"
#include "clk_domain.h"

static s32 clk_vf_point_pmudatainit_super(struct gk20a *g, struct pmu_board_obj
	*obj,	struct nv_pmu_boardobj *pmu_obj);

static int nvgpu_clk_vf_point_copy_pmu_status_to_nvgpu(struct gk20a *g,
	struct nvgpu_clk_vf_point_status *nvgpu_status,
	struct nv_pmu_boardobj_query *pmu_status,
	struct pmu_board_obj *obj);

static s32 clk_vf_point_pmudatainit_super(struct gk20a *g, struct pmu_board_obj
	*obj,	struct nv_pmu_boardobj *pmu_obj);

static s32 clk_vf_points_pmudatainit(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	s32 status = 0;
	struct nv_pmu_clk_clk_vf_point_boardobj_grp_set_pack *pVfPointsPack =
		(struct nv_pmu_clk_clk_vf_point_boardobj_grp_set_pack *)pboardobjgrppmu;
	struct nv_pmu_clk_clk_vf_point_boardobj_grp_set *pVfPointPriGrpSet =
		(struct nv_pmu_clk_clk_vf_point_boardobj_grp_set *)&pVfPointsPack->pri;
	struct nv_pmu_clk_clk_vf_point_sec_boardobj_grp_set *pVfPointSecGrpSet =
		(struct nv_pmu_clk_clk_vf_point_sec_boardobj_grp_set *)&pVfPointsPack->sec;
	struct nv_pmu_clk_clk_vf_point_boardobjgrp_set_header *pVfPointGrpSetHdr =
		(struct nv_pmu_clk_clk_vf_point_boardobjgrp_set_header *) pVfPointPriGrpSet;

	// Copy in the set of policy relationships.
	status = boardobjgrp_pmudatainit_e2048(g, pboardobjgrp,
			(struct nv_pmu_boardobjgrp_super *)pVfPointPriGrpSet);
	if (status != 0) {
		nvgpu_err(g,
		       "BOARDOBJGRP (pVfPointPriGrpSet) pmudatainit returned error: status = %d",
		       status);
		goto done;
	}

	// Copy in the set of Pwr Violations
	status = boardobjgrp_pmudatainit_e2048(g,
			&g->pmu->clk_pmu->clk_vf_pointobjs->sec.super.super,
			(struct nv_pmu_boardobjgrp_super *)pVfPointSecGrpSet);
	if (status != 0) {
		nvgpu_err(g,
			"BOARDOBJGRP (pVfPointSecGrpSet) pmudatainit returned error: status = %d",
			status);
		goto done;
	}

	// Copy in CLK_VF_POINTS data
	pVfPointGrpSetHdr->sparseMMAVFMarginEnabledMask =
		g->pmu->clk_pmu->clk_vf_pointobjs->sparseMMAVFMarginEnabledMask;

done:
	return status;
}

static s32 clk_vf_points_pmudata_instget(struct gk20a *g,
		struct nv_pmu_boardobjgrp *pmuboardobjgrp,
		struct nv_pmu_boardobj **pmu_obj, u16 idx)
{
	struct nv_pmu_clk_clk_vf_point_boardobj_grp_set  *pgrp_set =
		(struct nv_pmu_clk_clk_vf_point_boardobj_grp_set *)
		pmuboardobjgrp;

	nvgpu_log_info(g, " ");

	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (idx >= CTRL_BOARDOBJGRP_E2048_MAX_OBJECTS)
		return -EINVAL;

	*pmu_obj = (struct nv_pmu_boardobj *)
		&pgrp_set->objects[idx].data.obj;
	nvgpu_log_info(g, " Done");
	return 0;
}

static s32 clk_vf_points_pmustatus_instget(struct gk20a *g,
			void *pboardobjgrppmu,
			struct nv_pmu_boardobj_query **obj_pmu_status, u16 idx)
{
	struct nv_pmu_clk_clk_vf_point_boardobj_grp_get_status
	*pgrp_get_status =
		(struct nv_pmu_clk_clk_vf_point_boardobj_grp_get_status *)
		pboardobjgrppmu;
	u8 *base_ptr;
	u8 *vf_data_ptr;
	u8 *max_ptr;

	nvgpu_log_info(g, " ");

	/* Null pointer safety checks */
	if (pboardobjgrppmu == NULL) {
		nvgpu_err(g, "PMU board object group is NULL");
		return -EINVAL;
	}

	if (obj_pmu_status == NULL) {
		nvgpu_err(g, "Output PMU status pointer is NULL");
		return -EINVAL;
	}

	if (pgrp_get_status == NULL) {
		nvgpu_err(g, "PMU group get status is NULL");
		return -EINVAL;
	}

	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (idx >= CTRL_BOARDOBJGRP_E2048_MAX_OBJECTS) {
		nvgpu_err(g, "PMU VF index %u exceeds max objects %u",
			idx, CTRL_BOARDOBJGRP_E2048_MAX_OBJECTS);
		return -EINVAL;
	}

	/*
	 * The PMU uses 144-byte stride between VF points, but the array access is using wrong stride.
	 * Additionally, within each 144-byte block, there's a 16-byte header/padding offset.
	 */
	base_ptr = (u8 *)pgrp_get_status->objects;
	/* 144-byte stride + 16-byte internal offset */
	vf_data_ptr = base_ptr + (idx * 144) + (idx * 16);

	/* Bounds check for memory access */
	max_ptr = base_ptr + (CTRL_BOARDOBJGRP_E2048_MAX_OBJECTS * 144);

	if (vf_data_ptr >= max_ptr) {
		nvgpu_err(g, "VF data pointer %p exceeds max boundary %p for index %u",
			vf_data_ptr, max_ptr, idx);
		return -EINVAL;
	}

	*obj_pmu_status = (struct nv_pmu_boardobj_query *)(void *)vf_data_ptr;

	/* Final validation of returned pointer */
	if (*obj_pmu_status == NULL) {
		nvgpu_err(g, "Final PMU status pointer is NULL for index %u", idx);
		return -EINVAL;
	}

	nvgpu_log_info(g, " Done");
	return 0;
}

s32 clk_vf_point_sw_setup(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct boardobjgrp *pboardobjgrp_sec = NULL;
	struct nvgpu_clk_vf_points *pVfPoints = g->pmu->clk_pmu->clk_vf_pointobjs;

	nvgpu_log_info(g, " ");

	/* If already constructed, do not re-construct (suspend/resume pattern) */
	pboardobjgrp = &g->pmu->clk_pmu->clk_vf_pointobjs->super.super;
	pboardobjgrp_sec = &g->pmu->clk_pmu->clk_vf_pointobjs->sec.super.super;
	if (pboardobjgrp->bconstructed && pboardobjgrp_sec->bconstructed) {
		nvgpu_pmu_dbg(g, "clk_vf_point boardobjgrp already constructed, skipping reinit");
		return 0;
	}

	nvgpu_pmu_dbg(g, "Constructing clk_vf_point boardobjgrp for first time");

	// Construct the CLK_VF_POINTS_SUPER object:
	// Initialize the primary BOARDOBJGRP_E2048
	status = nvgpu_boardobjgrp_construct_e2048(g,
			&g->pmu->clk_pmu->clk_vf_pointobjs->super);
	if (status != 0) {
		nvgpu_err(g,
		"error creating boardobjgrp for clk vfpoint, - status %d",
		status);
		goto done;
	}
	// Populate data for the CLK_DOMAINS PMU interface.
	pVfPoints->vfPointsCacheCounter = 0;
	pVfPoints->bRmCachingSupported  = false;

	// Initialize the secondary BOARDOBJGRP_E2048
	status = nvgpu_boardobjgrp_construct_e2048(g,
			&g->pmu->clk_pmu->clk_vf_pointobjs->sec.super);
	if (status != 0) {
		nvgpu_err(g,
		"error creating secondary boardobjgrp for clk vfpoint, - status %d",
		status);
		goto done;
	}

	pboardobjgrp = &g->pmu->clk_pmu->clk_vf_pointobjs->super.super;

	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, CLK_VF_POINT);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			clk, CLK, clk_vf_point, CLK_VF_POINT);
	if (status != 0) {
		nvgpu_err(g,
			"error constructing PMU_BOARDOBJ_CMD_GRP_SET - status %d",
			status);
		goto done;
	}

	status = BOARDOBJGRP_PMU_CMD_GRP_GET_STATUS_CONSTRUCT(g,
			&g->pmu->clk_pmu->clk_vf_pointobjs->super.super,
			clk, CLK, clk_vf_point, CLK_VF_POINT);
	if (status != 0) {
		nvgpu_err(g,
			"error constructing PMU_BOARDOBJ_CMD_GRP_STATUS - status: %d",
			status);
		goto done;
	}

	pboardobjgrp->pmudatainit = clk_vf_points_pmudatainit;
	pboardobjgrp->pmudatainstget  = clk_vf_points_pmudata_instget;
	pboardobjgrp->pmustatusinstget  = clk_vf_points_pmustatus_instget;

done:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}

s32 clk_vf_point_pmu_setup(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_log_info(g, " ");

	pboardobjgrp = &g->pmu->clk_pmu->clk_vf_pointobjs->super.super;

	if (!pboardobjgrp->bconstructed) {
		nvgpu_err(g, "VF_POINT boardobjgrp not constructed.");
		return -EINVAL;
	}

	status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);

	nvgpu_log_info(g, "Done");
	return status;
}

static s32 clk_vf_point_construct_super(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *pargs)
{
	struct clk_vf_point *pclkvfpoint;
	s32 status = 0;

	nvgpu_log_info(g, " ");
	pclkvfpoint = nvgpu_kzalloc(g, size);
	if (pclkvfpoint == NULL) {
		nvgpu_err(g, "Memory allocation for VF_POINT failed.");
		return -ENOMEM;
	}

	status = pmu_board_obj_construct_super(g,
			(struct pmu_board_obj *)(void *)pclkvfpoint, pargs);
	if (status != 0) {
		nvgpu_err(g, "Constructing VF_POINT super failed - status %d", status);
		return -EINVAL;
	}

	*obj = (struct pmu_board_obj *)(void *)pclkvfpoint;

	pclkvfpoint->super.pmudatainit =
			clk_vf_point_pmudatainit_super;

	return status;
}

static s32 clk_vf_point_pmudatainit_volt(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct clk_vf_point_volt *pclk_vf_point_volt;
	struct nv_pmu_clk_clk_vf_point_volt_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = clk_vf_point_pmudatainit_super(g, obj, pmu_obj);
	if (status != 0) {
		nvgpu_err(g, "VF_POINT_VOLT datainit failed - status %d", status);
		return status;
	}

	pclk_vf_point_volt =
		(struct clk_vf_point_volt *)(void *)obj;

	pset = (struct nv_pmu_clk_clk_vf_point_volt_boardobj_set *)
		pmu_obj;

	pset->source_voltage_uv = pclk_vf_point_volt->source_voltage_uv;
	pset->freq_delta.data = pclk_vf_point_volt->freq_delta.data;
	pset->freq_delta.type = pclk_vf_point_volt->freq_delta.type;

	return status;
}

static s32 clk_vf_point_pmudatainit_freq(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct clk_vf_point_freq *pclk_vf_point_freq;
	struct nv_pmu_clk_clk_vf_point_freq_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = clk_vf_point_pmudatainit_super(g, obj, pmu_obj);
	if (status != 0) {
		nvgpu_err(g, "VF_POINT_FREQ datainit failed - status %d", status);
		return status;
	}

	pclk_vf_point_freq =
		(struct clk_vf_point_freq *)(void *)obj;

	pset = (struct nv_pmu_clk_clk_vf_point_freq_boardobj_set *)
		pmu_obj;

	pset->volt_delta_uv = pclk_vf_point_freq->volt_delta_uv;

	return status;
}

static s32 clkVfPoint_PmuDataInit_50(struct gk20a *g,
	struct pmu_board_obj *pBoardObj,
	struct nv_pmu_boardobj *pPmuData)
{
	s32 status = 0;

	// Copy the object's type
	status = pmu_board_obj_pmu_data_init_super(g, pBoardObj, pPmuData);

	return status;
}

static s32 clkVfPoint_PmuDataInit_50_FREQ(struct gk20a *g,
	struct pmu_board_obj *pBoardObj,
	struct nv_pmu_boardobj *pPmuData)
{
	struct nv_pmu_clk_clk_vf_point_50_freq_boardobj_set *pVfPoint50FreqSet =
		(struct nv_pmu_clk_clk_vf_point_50_freq_boardobj_set *) pPmuData;
	struct clk_vf_point_50_freq *pVfPoint50Freq = (struct clk_vf_point_50_freq *) pBoardObj;
	s32 status = 0;

	// Call BOARDOBJ super class.
	status = clkVfPoint_PmuDataInit_50(g, pBoardObj, pPmuData);
	if (status != 0) {
		nvgpu_err(g,
			"Error initializing CLK_VF_POINT_50 PMU data: status=%d",
			status);
		goto clkVfPointIfaceModel10PmuDataInit_50_FREQ_exit;
	}

	// Set CLK_VF_POINT_50_FREQ-specific data.
	pVfPoint50FreqSet->freqMHz     = pVfPoint50Freq->freqMHz;
	pVfPoint50FreqSet->voltDeltauV = pVfPoint50Freq->voltDeltauv;

clkVfPointIfaceModel10PmuDataInit_50_FREQ_exit:
	return status;
}

static s32 clkVfPoint_PmuDataInit_50_VOLT(struct gk20a *g,
	struct pmu_board_obj *pBoardObj,
	struct nv_pmu_boardobj *pPmuData)
{
	struct nv_pmu_clk_clk_vf_point_50_volt_boardobj_set *pVfPoint50VoltSet =
		(struct nv_pmu_clk_clk_vf_point_50_volt_boardobj_set *) pPmuData;
	struct clk_vf_point_50_volt  *pVfPoint50Volt = (struct clk_vf_point_50_volt *) pBoardObj;
	s32 status = 0;

	// Call BOARDOBJ super class.
	status = clkVfPoint_PmuDataInit_50(g, pBoardObj, pPmuData);
	if (status != 0) {
		nvgpu_err(g,
			"Error initializing CLK_VF_POINT_50 PMU data: status=%d",
			status);
		goto clkVfPointIfaceModel10PmuDataInit_50_VOLT_exit;
	}

	// Set CLK_VF_POINT_50_VOLT-specific data.
	pVfPoint50VoltSet->sourceVoltageuV = pVfPoint50Volt->source_voltage_uv;
	pVfPoint50VoltSet->freqDelta       = pVfPoint50Volt->freq_delta;

clkVfPointIfaceModel10PmuDataInit_50_VOLT_exit:
	return status;
}

static s32 clkVfPoint_PmuDataInit_50_VOLT_SEC(struct gk20a *g,
	struct pmu_board_obj *pBoardObj,
	struct nv_pmu_boardobj *pPmuData)
{
	s32 status = 0;
	struct nv_pmu_clk_clk_vf_point_50_volt_sec_boardobj_set *pVfPoint50VoltSec =
		(struct nv_pmu_clk_clk_vf_point_50_volt_sec_boardobj_set *) pPmuData;
	struct clk_vf_point_50_volt_sec  *pVfPoint50VoltSecSet =
		(struct clk_vf_point_50_volt_sec *) pBoardObj;

	// Call BOARDOBJ super class.
	status = clkVfPoint_PmuDataInit_50_VOLT(g, pBoardObj, pPmuData);
	if (status != 0) {
		nvgpu_err(g,
			"Error initializing CLK_VF_POINT_50_VOLT_SEC PMU data: status=%d",
			status);
		goto clkVfPointIfaceModel10PmuDataInit_50_VOLT_SEC_exit;
	}

	// Set CLK_VF_POINT_50_VOLT_SEC-specific data.
	pVfPoint50VoltSecSet->dvcoOffsetCodeOverride = pVfPoint50VoltSec->dvcoOffsetCodeOverride;

clkVfPointIfaceModel10PmuDataInit_50_VOLT_SEC_exit:
	return status;
}

static s32 clk_vf_point_construct_volt_35(struct gk20a *g,
		struct pmu_board_obj **obj, size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_vf_point_volt *pclkvfpoint;
	struct clk_vf_point_volt *ptmpvfpoint =
			(struct clk_vf_point_volt *)pargs;
	s32 status = 0;

	if (pmu_board_obj_get_type(pargs) !=
			CTRL_CLK_CLK_VF_POINT_TYPE_35_VOLT_PRI) {
		return -EINVAL;
	}

	obj_tmp->type_mask = (u32) BIT(CTRL_CLK_CLK_VF_POINT_TYPE_35_VOLT_PRI);
	status = clk_vf_point_construct_super(g, obj, size, pargs);
	if (status != 0) {
		return -EINVAL;
	}

	pclkvfpoint = (struct clk_vf_point_volt *) (void *) *obj;

	pclkvfpoint->super.super.pmudatainit =
			clk_vf_point_pmudatainit_volt;

	pclkvfpoint->source_voltage_uv = ptmpvfpoint->source_voltage_uv;
	pclkvfpoint->freq_delta = ptmpvfpoint->freq_delta;

	return status;
}

static s32 clk_vf_point_construct_freq_35(struct gk20a *g,
		struct pmu_board_obj **obj, size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_vf_point_freq *pclkvfpoint;

	s32 status = 0;

	if (pmu_board_obj_get_type(pargs) != CTRL_CLK_CLK_VF_POINT_TYPE_35_FREQ) {
		status = -EINVAL;
		goto done;
	}

	obj_tmp->type_mask = (u32) BIT(CTRL_CLK_CLK_VF_POINT_TYPE_35_FREQ);
	status = clk_vf_point_construct_super(g, obj, size, pargs);
	if (status != 0) {
		status = -EINVAL;
		goto done;
	}

	pclkvfpoint = (struct clk_vf_point_freq *)(void *) *obj;

	pclkvfpoint->super.super.pmudatainit =
			clk_vf_point_pmudatainit_freq;

done:
	return status;
}


static s32 clkVfPoint_Construct_50_FREQ(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj, size_t size, void *pArgs)
{
	s32 status = 0;
	struct clk_vf_point_50_freq *pVfPoint50Freq;
	struct clk_vf_point_50_freq *pVfPoint50FreqTmp = (struct clk_vf_point_50_freq *) pArgs;
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *) pArgs;

	nvgpu_log_info(g, " ");
	obj_tmp->type_mask = (u32) BIT(NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_FREQ);

	// Call into super constructor.
	status = clk_vf_point_construct_super(g, ppBoardObj, size, pArgs);
	if (status != 0) {
		nvgpu_err(g,
			"Error constructing CLK_VF_POINT_50: status=%d",
			status);
		goto clkVfPointConstruct_50_FREQ_exit;
	}
	pVfPoint50Freq = (struct clk_vf_point_50_freq *)*ppBoardObj;

	// Set BOARDOBJ interfaces.
	pVfPoint50Freq->super.super.super.pmudatainit = clkVfPoint_PmuDataInit_50_FREQ;

	// Set CLK_VF_POINT_50_FREQ parameters
	pVfPoint50Freq->freqMHz = pVfPoint50FreqTmp->freqMHz;

clkVfPointConstruct_50_FREQ_exit:
	nvgpu_log_info(g, " Done");
	return status;
}

static s32 clkVfPoint_Construct_50_VOLT(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj, size_t size, void *pArgs)
{
	s32 status = 0;
	struct clk_vf_point_50_volt *pVfPoint50Volt;
	struct clk_vf_point_50_volt *pVfPoint50VoltTmp = (struct clk_vf_point_50_volt *)pArgs;
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pArgs;

	nvgpu_log_info(g, " ");
	obj_tmp->type_mask = (u32) BIT(NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_VOLT);

	// Call into super constructor.
	status = clk_vf_point_construct_super(g, ppBoardObj, size, pArgs);
	if (status != 0) {
		nvgpu_err(g,
			"Error constructing CLK_VF_POINT_50: status=%d",
			status);
		goto clkVfPointConstruct_50_VOLT_exit;
	}
	pVfPoint50Volt = (struct clk_vf_point_50_volt *)*ppBoardObj;

	// Set BOARDOBJ interfaces.
	pVfPoint50Volt->super.super.super.pmudatainit = clkVfPoint_PmuDataInit_50_VOLT;


	// Set CLK_VF_POINT_50_VOLT parameters
	pVfPoint50Volt->source_voltage_uv = pVfPoint50VoltTmp->source_voltage_uv;

	// Init PCLK_VF_POINT_50_VOLT frequency deltas to default values
	pVfPoint50Volt->freq_delta.type = CTRL_CLK_CLK_FREQ_DELTA_TYPE_STATIC;
	pVfPoint50Volt->freq_delta.data.delta_khz = 0;

clkVfPointConstruct_50_VOLT_exit:
	nvgpu_log_info(g, " Done");
	return status;
}

static s32 clkVfPoint_Construct_50_VOLT_PRI(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj, size_t size, void *pArgs)
{
	s32 status = 0;

	// Call into super constructor.
	status = clkVfPoint_Construct_50_VOLT(g, ppBoardObj, size, pArgs);
	if (status != 0) {
		nvgpu_err(g,
			"Error constructing CLK_VF_POINT_50_VOLT: status=%d",
			status);
		goto clkVfPointConstruct_50_VOLT_PRI_exit;
	}


clkVfPointConstruct_50_VOLT_PRI_exit:
	return status;
}

static s32 clkVfPoint_Construct_50_VOLT_SEC(struct gk20a *g,
		struct pmu_board_obj **ppBoardObj, size_t size, void *pArgs)
{
	s32 status = 0;
	struct clk_vf_point_50_volt_sec *pVfPoint50VoltSec;
	struct clk_vf_point_50_volt_sec *pVfPoint50VoltSecTmp = (struct clk_vf_point_50_volt_sec *)pArgs;

	// Call into super constructor.
	status = clkVfPoint_Construct_50_VOLT(g, ppBoardObj, size, pArgs);
	if (status != 0) {
		nvgpu_err(g,
			"Error constructing CLK_VF_POINT_50_VOLT: status=%d",
			status);
		goto clkVfPointConstruct_50_VOLT_SEC_exit;
	}
	pVfPoint50VoltSec = (struct clk_vf_point_50_volt_sec *)*ppBoardObj;

	// Set BOARDOBJ interfaces.
	pVfPoint50VoltSec->super.super.super.super.pmudatainit = clkVfPoint_PmuDataInit_50_VOLT_SEC;

	// Set CLK_VF_POINT_50_VOLT_SEC parameters
	pVfPoint50VoltSec->dvcoOffsetCodeOverride = pVfPoint50VoltSecTmp->dvcoOffsetCodeOverride;

clkVfPointConstruct_50_VOLT_SEC_exit:
	return status;
}

struct clk_vf_point *nvgpu_construct_clk_vf_point(struct gk20a *g, void *pargs)
{
	struct pmu_board_obj *obj = NULL;
	s32 status;

	nvgpu_log_info(g, " ");
	switch (pmu_board_obj_get_type(pargs)) {

	case CTRL_CLK_CLK_VF_POINT_TYPE_35_FREQ:
		status = clk_vf_point_construct_freq_35(g, &obj,
			sizeof(struct clk_vf_point_freq), pargs);
		break;

	case CTRL_CLK_CLK_VF_POINT_TYPE_35_VOLT_PRI:
		status = clk_vf_point_construct_volt_35(g, &obj,
			sizeof(struct clk_vf_point_volt), pargs);
		break;

	case NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_VOLT_PRI:
	{
		status = clkVfPoint_Construct_50_VOLT_PRI(g, &obj,
					sizeof(struct clk_vf_point_50_volt_pri), pargs);
		break;
	}
	case NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_VOLT_SEC:
	{
		status = clkVfPoint_Construct_50_VOLT_SEC(g, &obj,
					sizeof(struct clk_vf_point_50_volt_sec), pargs);
		break;
	}
	case NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_FREQ:
	{
		status = clkVfPoint_Construct_50_FREQ(g, &obj,
					sizeof(struct clk_vf_point_50_freq), pargs);
		break;
	}

	default:
		status = -EINVAL;
		break;
	}

	if (status != 0) {
		return NULL;
	}

	nvgpu_log_info(g, " Done");
	return (struct clk_vf_point *)(void *)obj;
}

static s32 clk_vf_point_pmudatainit_super(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;

	nvgpu_log_info(g, " ");

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);

	return status;
}

#ifdef CONFIG_NVGPU_CLK_ARB
s32 nvgpu_clk_arb_find_slave_points(struct nvgpu_clk_arb *arb,
		struct nvgpu_clk_slave_freq *vf_point)
{

	u16 gpc2clk_target;
	struct nvgpu_clk_vf_table *table;
	u32 index;
	s32 status = 0;
	do {
		gpc2clk_target = vf_point->gpc_mhz;

		table = NV_READ_ONCE(arb->current_vf_table);
		/* pointer to table can be updated by callback */
		nvgpu_smp_rmb();

		if (table == NULL) {
			continue;
		}
		if ((table->gpc2clk_num_points == 0U)) {
			nvgpu_err(arb->g, "found empty table");
			status = -EINVAL; ;
		}

		/* round up the freq requests */
		for (index = 0; index < table->gpc2clk_num_points; index++) {
			if ((table->gpc2clk_points[index].gpc_mhz >=
							gpc2clk_target)) {
				gpc2clk_target =
					table->gpc2clk_points[index].gpc_mhz;
				vf_point->sys_mhz =
					table->gpc2clk_points[index].sys_mhz;
				vf_point->xbar_mhz =
					table->gpc2clk_points[index].xbar_mhz;
				vf_point->nvd_mhz =
					table->gpc2clk_points[index].nvd_mhz;
				vf_point->host_mhz =
					table->gpc2clk_points[index].host_mhz;
				break;
			}
		}
		/*
		 * If the requested freq is lower than available
		 * one in VF table, use the VF table freq
		 */
		if (gpc2clk_target > vf_point->gpc_mhz) {
			vf_point->gpc_mhz = gpc2clk_target;
		}
	} while ((table == NULL) ||
		(NV_READ_ONCE(arb->current_vf_table) != table));

	return status;

}

/*get latest vf point data from PMU */
s32 nvgpu_clk_vf_point_cache(struct gk20a *g)
{
	struct nvgpu_clk_vf_points *pclk_vf_points;
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	s32 status = 0;
	struct clk_vf_point_50_freq *pclk_vf_point;
	u16 index;
	u32 voltage_min_uv,voltage_step_size_uv;
	u32 gpcclk_clkmhz=0, gpcclk_voltuv=0;

	nvgpu_log_info(g, " ");

	status = clk_domain_get_vf_tables(g);
	if (status != 0) {
		nvgpu_err(g, "Failed to get VF Curves from PMU");
		goto done;
	}

	pclk_vf_points = g->pmu->clk_pmu->clk_vf_pointobjs;
	pboardobjgrp = &pclk_vf_points->super.super;

	voltage_min_uv = nvgpu_pmu_clk_fll_get_lut_min_volt(g->pmu->clk_pmu);
	voltage_step_size_uv =
			nvgpu_pmu_clk_fll_get_lut_step_size(g->pmu->clk_pmu);
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		pclk_vf_point = (struct clk_vf_point_50_freq *)(void *)obj;
		gpcclk_voltuv =
				voltage_min_uv + index * voltage_step_size_uv;
		status = nvgpu_clk_domain_volt_to_freq(g, 0, &gpcclk_clkmhz,
				&gpcclk_voltuv);
		if (status != 0) {
			nvgpu_err(g,
				"Failed to get freq for requested voltage");
			break;
		}

		pclk_vf_point->freqMHz = (u16)gpcclk_clkmhz;
		pclk_vf_point->voltDeltauv = gpcclk_voltuv;
	}

done:
	clk_domain_free_vf_tables(g);

	return status;
}
#endif

s32 clk_vf_point_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmu->clk_pmu->clk_vf_pointobjs != NULL) {
		return 0;
	}

	g->pmu->clk_pmu->clk_vf_pointobjs = nvgpu_kzalloc(g,
			sizeof(*g->pmu->clk_pmu->clk_vf_pointobjs));
	if (g->pmu->clk_pmu->clk_vf_pointobjs == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void clk_vf_point_free_pmupstate(struct gk20a *g)
{
	nvgpu_kfree(g, g->pmu->clk_pmu->clk_vf_pointobjs);
	g->pmu->clk_pmu->clk_vf_pointobjs = NULL;
}


/**
 * nvgpu_clk_vf_points_get_status - Get VF points status from PMU
 * @g: pointer to gpu instance
 * @vf_points_status: pointer to NVGPU VF points status structure
 *
 * This function retrieves the VF points status from PMU using the boardobjgrp
 * get status interface, similar to RM's subdeviceCtrlCmdClkVfPointsGetStatus_IMPL,
 * but using NVGPU-specific structures.
 *
 * Returns: 0 on success, negative error code on failure
 */
int nvgpu_clk_vf_points_get_status(struct gk20a *g,
	struct nvgpu_clk_vf_points_status *vf_points_status)
{
	struct boardobjgrp *pboardobjgrp;
	struct boardobjgrpmask *pboardobjgrpmask;
	struct nv_pmu_clk_clk_vf_point_boardobjgrp_get_status *pmu_status;
	struct pmu_board_obj *obj = NULL;
	struct nv_pmu_boardobj_query *pboardobjpmustatus = NULL;
	int status;
	u32 vf_point_count = 0;
	u16 index;
	u16 board_obj_idx;

	/* Check if VF points are supported */
	if (g->pmu->clk_pmu == NULL || g->pmu->clk_pmu->clk_vf_pointobjs == NULL)
		return -ENODEV;

	pboardobjgrp = &g->pmu->clk_pmu->clk_vf_pointobjs->super.super;
	pboardobjgrpmask = pboardobjgrp->mask;

	if (nvgpu_boardobjgrpmask_bit_set_count(pboardobjgrpmask) == 0) {
		nvgpu_err(g, "No VF points available");
		return -ENODEV;
	}

	/* Initialize the NVGPU status structure */
	memset(vf_points_status, 0, sizeof(*vf_points_status));

	/* Initialize super class */
	vf_points_status->super.super.type = 0;
	vf_points_status->super.super.index = 0;
	vf_points_status->super.objCount = nvgpu_boardobjgrpmask_bit_set_count(pboardobjgrpmask);

	/* Copy object mask */
	vf_points_status->super.objMask.objCount = nvgpu_boardobjgrpmask_bit_set_count(pboardobjgrpmask);
	memcpy(vf_points_status->super.objMask.data, pboardobjgrpmask->data,
		sizeof(vf_points_status->super.objMask.data));

	/* Call PMU get status interface */
	status = pboardobjgrp->pmugetstatus(g, pboardobjgrp, pboardobjgrpmask);
	if (status != 0) {
		nvgpu_err(g, "Error calling VF_POINT PMU GET_STATUS interface: status=0x%08x", status);
		return status;
	}

	/* Get PMU status buffer */
	pmu_status = (struct nv_pmu_clk_clk_vf_point_boardobjgrp_get_status *)
		pboardobjgrp->pmu.getstatus.buf;

	if (pmu_status == NULL) {
		nvgpu_err(g, "PMU status buffer is NULL");
		return -EINVAL;
	}

	/* Copy VF points data from PMU status to NVGPU structure */
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		if (vf_point_count >= NVGPU_CLK_VF_POINTS_MAX_OBJECTS)
			break;

		/* Use board object's index to get correct PMU status object. */
		board_obj_idx = pmu_board_obj_get_idx(obj);

		nvgpu_log_info(g, "VF Point %u: Board obj idx=%u",
			vf_point_count, board_obj_idx);

		/* Get PMU status for this VF point board obj */
		status = pboardobjgrp->pmustatusinstget(g,
			(struct nv_pmu_boardobjgrp *)(void *)pmu_status,
			&pboardobjpmustatus, board_obj_idx);
		if (status != 0) {
			nvgpu_err(g, "could not get status object instance for board obj idx %d",
				board_obj_idx);
			continue;
		}

		/* Convert PMU status to NVGPU structure */
		status = nvgpu_clk_vf_point_copy_pmu_status_to_nvgpu(g,
			&vf_points_status->vf_points[vf_point_count],
			pboardobjpmustatus, obj);
		if (status != 0) {
			nvgpu_err(g, "could not convert VF point status for board obj idx %d",
				board_obj_idx);
			continue;
		}

		vf_point_count++;
	}
	return 0;
}

/**
 * nvgpu_clk_vf_point_copy_pmu_status_to_nvgpu - Convert PMU status to NVGPU structure
 * @g: pointer to gpu instance
 * @nvgpu_status: pointer to NVGPU VF point status structure
 * @pmu_status: pointer to PMU status structure
 * @obj: pointer to board object
 *
 * Returns: 0 on success, negative error code on failure
 */
static int nvgpu_clk_vf_point_copy_pmu_status_to_nvgpu(struct gk20a *g,
	struct nvgpu_clk_vf_point_status *nvgpu_status,
	struct nv_pmu_boardobj_query *pmu_status,
	struct pmu_board_obj *obj)
{
	union nv_pmu_clk_clk_vf_point_boardobj_get_status_union *pmu_vf_status;
	struct clk_vf_point_50_freq *pclk_vf_point_50_freq;
	struct clk_vf_point_50_volt *pclk_vf_point_50_volt;
	u8 *pmu_data;
	u32 *voltage_ptr;
	u16 *freq_ptr;

	if (nvgpu_status == NULL || pmu_status == NULL || obj == NULL)
		return -EINVAL;

	/* Null pointer safety check */
	if (!pmu_status || !nvgpu_status) {
		nvgpu_err(g, "NULL pointer in status copy");
		return -EINVAL;
	}

	/* Additional null pointer safety check for PMU status buffer */
	if ((void *)pmu_status == NULL || (uintptr_t)pmu_status < 0x1000) {
		nvgpu_err(g, "PMU status buffer is NULL or invalid: %p", pmu_status);
		return -EINVAL;
	}

	/* Hex dump first 10 VF points: raw PMU status data (first 1000 bytes) */
	//if (pmu_board_obj_get_idx(obj) < 10) {
	//	nvgpu_err(g, "=== PMU VF Point %u Raw Data (first 1000 bytes) ===",
	//		pmu_board_obj_get_idx(obj));
	//	nvgpu_err(g, "PMU status size: %zu bytes", sizeof(*pmu_vf_status));
	//	print_hex_dump(KERN_ERR, "PMU_RAW: ", DUMP_PREFIX_OFFSET, 16, 1,
	//			pmu_status, min(1000U, (u32)sizeof(*pmu_vf_status)), true);
	//	//nvgpu_err(g, "=== End PMU VF Point %u Raw Data ===", pmu_board_obj_get_idx(obj));
	//}

	pmu_vf_status = (union nv_pmu_clk_clk_vf_point_boardobj_get_status_union *)pmu_status;

	/* Initialize NVGPU structure */
	memset(nvgpu_status, 0, sizeof(*nvgpu_status));

	/* Copy basic information */
	nvgpu_status->super.type = pmu_board_obj_get_type(obj);
	nvgpu_status->super.index = pmu_board_obj_get_idx(obj);
	nvgpu_status->type = pmu_board_obj_get_type(obj);

	pmu_data = (u8 *)pmu_status;

	/* Safety checks to prevent kernel panic */
	if (!pmu_data || !pmu_status) {
		nvgpu_err(g, "PMU data is NULL");
		return -EINVAL;
	}

	voltage_ptr = (u32 *)(pmu_data + 0x18);  /* Voltage at offset 0x18 (4 bytes) */
	freq_ptr = (u16 *)(pmu_data + 0x2c);     /* Frequency at offset 0x2c (2 bytes) */

	/* Read voltage and frequency directly from memory */
	nvgpu_status->voltage_uv = *voltage_ptr;
	nvgpu_status->freq_mhz = *freq_ptr;  /* Frequency is already in MHz */

	/* Copy frequency and voltage from PMU status based on type */
	switch (pmu_board_obj_get_type(obj)) {
	case NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_FREQ:
		/* Debug: show what we read from direct memory access */
		nvgpu_pmu_dbg(g, "VF Index=%u, Type=v50_freq, volt=%u uV, freq=%u MHz",
			pmu_board_obj_get_idx(obj), nvgpu_status->voltage_uv, nvgpu_status->freq_mhz);

		pclk_vf_point_50_freq = (struct clk_vf_point_50_freq *)obj;
		nvgpu_status->data.v50_freq.active_base_vf_tuple.voltage_uv = nvgpu_status->voltage_uv;
		nvgpu_status->data.v50_freq.active_base_vf_tuple.freq_mhz[0] = nvgpu_status->freq_mhz;
		break;

	case NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_VOLT_PRI:
		nvgpu_pmu_dbg(g, "VF Index=%u, Type=v50_volt_pri, volt=%u uV, freq=%u MHz",
			pmu_board_obj_get_idx(obj), nvgpu_status->voltage_uv, nvgpu_status->freq_mhz);

		pclk_vf_point_50_volt = (struct clk_vf_point_50_volt *)obj;
		nvgpu_status->data.v50_volt_pri.active_base_vf_tuple.voltage_uv =
			nvgpu_status->voltage_uv;
		nvgpu_status->data.v50_volt_pri.active_base_vf_tuple.freq_mhz[0] =
			nvgpu_status->freq_mhz;
		break;

	case NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_VOLT_SEC:
		nvgpu_pmu_dbg(g, "VF Index=%u, Type=v50_volt_sec, volt=%u uV, freq=%u MHz",
			pmu_board_obj_get_idx(obj), nvgpu_status->voltage_uv, nvgpu_status->freq_mhz);

		pclk_vf_point_50_volt = (struct clk_vf_point_50_volt *)obj;
		nvgpu_status->data.v50_volt_sec.active_base_vf_tuple.voltage_uv =
			nvgpu_status->voltage_uv;
		nvgpu_status->data.v50_volt_sec.active_base_vf_tuple.freq_mhz[0] =
			nvgpu_status->freq_mhz;
		break;

	case CTRL_CLK_CLK_VF_POINT_TYPE_35_FREQ:
		/* For v35 FREQ, read from v35_freq union member */
		nvgpu_pmu_dbg(g, "VF Index=%u, Type=v35_freq, volt=%u uV, freq=%u MHz",
			pmu_board_obj_get_idx(obj), nvgpu_status->voltage_uv, nvgpu_status->freq_mhz);
		nvgpu_status->freq_mhz = pmu_vf_status->v35_freq.base_vf_tuple.freqTuple[0].freqMHz;
		nvgpu_status->voltage_uv = pmu_vf_status->v35_freq.base_vf_tuple.voltageuV;

		nvgpu_status->data.v35_freq.base_vf_tuple.voltage_uv = nvgpu_status->voltage_uv;
		nvgpu_status->data.v35_freq.base_vf_tuple.freq_mhz[0] = nvgpu_status->freq_mhz;
		break;

	case CTRL_CLK_CLK_VF_POINT_TYPE_35_VOLT_PRI:
		/* For v35 VOLT_PRI, read from v35_volt_pri union member */
		nvgpu_pmu_dbg(g, "VF Index=%u, Type=v35_volt_pri, volt=%u uV, freq=%u MHz",
			pmu_board_obj_get_idx(obj), nvgpu_status->voltage_uv, nvgpu_status->freq_mhz);
		nvgpu_status->freq_mhz = pmu_vf_status->v35_volt_pri.base_vf_tuple.freqTuple[0].freqMHz;
		nvgpu_status->voltage_uv = pmu_vf_status->v35_volt_pri.base_vf_tuple.voltageuV;

		nvgpu_status->data.v35_volt_pri.base_vf_tuple.voltage_uv = nvgpu_status->voltage_uv;
		nvgpu_status->data.v35_volt_pri.base_vf_tuple.freq_mhz[0] = nvgpu_status->freq_mhz;
		break;

	default:
		/* For unknown types, try to read from super structure */
		nvgpu_pmu_dbg(g, "warning: Unknown VF point type: %u, using default handling",
			pmu_board_obj_get_type(obj));
		nvgpu_status->freq_mhz = pmu_vf_status->super.pair.freq_mhz;
		nvgpu_status->voltage_uv = pmu_vf_status->super.pair.voltage_uv;

		nvgpu_status->data.v30.pair.freq_mhz = nvgpu_status->freq_mhz;
		nvgpu_status->data.v30.pair.voltage_uv = nvgpu_status->voltage_uv;
		break;
	}

	return 0;
}

/**
 * nvgpu_clk_vf_points_get_count - Get number of VF points
 * @g: pointer to gpu instance
 * @num_vf_points: pointer to store number of VF points
 *
 * Returns: 0 on success, negative error code on failure
 */
int nvgpu_clk_vf_points_get_count(struct gk20a *g, u32 *num_vf_points)
{
	struct boardobjgrp *pboardobjgrp;
	struct boardobjgrpmask *pboardobjgrpmask;

	/* Check if VF points are supported */
	if (g->pmu->clk_pmu == NULL || g->pmu->clk_pmu->clk_vf_pointobjs == NULL)
		return -ENODEV;

	pboardobjgrp = &g->pmu->clk_pmu->clk_vf_pointobjs->super.super;
	pboardobjgrpmask = pboardobjgrp->mask;

	*num_vf_points = nvgpu_boardobjgrpmask_bit_set_count(pboardobjgrpmask);

	return 0;
}

/*
 * nvgpu_clk_vf_points_get_vf_point_data - Get individual VF point data
 * @g: pointer to gpu instance
 * @vf_point_index: index of VF point to retrieve
 * @vf_point_status: pointer to store VF point status
 *
 * Returns: 0 on success, negative error code on failure
 */
int nvgpu_clk_vf_points_get_vf_point_data(struct gk20a *g, u32 vf_point_index,
	struct nvgpu_clk_vf_point_status *vf_point_status)
{
	struct nvgpu_clk_vf_points_status *full_status;
	int status;

	/* Allocate memory for the full status structure to avoid stack overflow */
	full_status = nvgpu_kzalloc(g, sizeof(*full_status));
	if (full_status == NULL)
		return -ENOMEM;

	/* Get full VF points status */
	status = nvgpu_clk_vf_points_get_status(g, full_status);
	if (status != 0)
		goto cleanup;

	/* Check bounds */
	if (vf_point_index >= full_status->super.objCount) {
		status = -EINVAL;
		goto cleanup;
	}

	/* Copy specific VF point data */
	memcpy(vf_point_status, &full_status->vf_points[vf_point_index],
		sizeof(*vf_point_status));

	status = 0;

cleanup:
	nvgpu_kfree(g, full_status);
	return status;
}
