// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/types.h>
#include <nvgpu/string.h>
#include <nvgpu/timers.h>
#include <nvgpu/pmu/therm.h>
#include <nvgpu/pmu/pmuif/therm_inf.h>
#include "therm_dev.h"
#include "therm_channel.h"
#include "thrm.h"
#include "therm_channel_provider_v10.h"
#include "therm_channel_debug_v10.h"
#include "therm_channel_aggregation_v10.h"
#include "therm_channel_estimation_v10.h"
#include "../boardobj/ucode_boardobj_inf.h"
// Define CTRL_BOARDOBJ_IDX_INVALID if not already defined
#ifndef CTRL_BOARDOBJ_IDX_INVALID
#define CTRL_BOARDOBJ_IDX_INVALID 255U
#endif

static u8  devinitThermChannel2xAggregationModeConvertToInternal
(
	struct gk20a *g,
	u8 entryIdx,
	u8 vbiosMode
)
{
	u8 AggregationMode = NV2080_CTRL_THERMAL_THERM_CHANNEL_AGGREGATION_MODE_INVALID;
	switch (vbiosMode)
	{
		case NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_AGGREGATION_MODE_MAX:
		{
			AggregationMode = NV2080_CTRL_THERMAL_THERM_CHANNEL_AGGREGATION_MODE_MAX;
			break;
		}
		case NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_AGGREGATION_MODE_AVG:
		{
			AggregationMode = NV2080_CTRL_THERMAL_THERM_CHANNEL_AGGREGATION_MODE_AVG;
			break;
		}
		default:
		{
			nvgpu_err(g, "Invalid Aggregation Mode 0x%x for entry %d", vbiosMode, entryIdx);
			break;
		}
	}
	return AggregationMode;
}
static u8 devinitThermChannel2xEstimationModeConvertToInternal
(
	struct gk20a *g,
	u8 entryIdx,
	u8 vbiosMode
)
{
	u8 EstimationMode = NV2080_CTRL_THERMAL_THERM_CHANNEL_ESTIMATION_MODE_INVALID;
	switch (vbiosMode)
	{
		case NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_ESTIMATION_MODE_SUM:
		{
			EstimationMode = NV2080_CTRL_THERMAL_THERM_CHANNEL_ESTIMATION_MODE_SUM;
			break;
		}
		case NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_ESTIMATION_MODE_DIFF:
		{
			EstimationMode = NV2080_CTRL_THERMAL_THERM_CHANNEL_ESTIMATION_MODE_DIFF;
			break;
		}
		default:
		{
			nvgpu_err(g, "Invalid Estimation Mode 0x%x for entry %d", vbiosMode, entryIdx);
			break;
		}
	}
	return EstimationMode;
}
// Define the thermChannel2xHals array as static global at top of file
static NV_VBIOS_THERM_CHANNEL_2X_HAL thermChannel2xHals[] =
{
	// Data related to HAL_0_DEFAULT
	{
		NV_VBIOS_THERM_CHANNEL_2X_HAL_0_DEFAULT,
		NV_VBIOS_THERM_CHANNEL_2X_HAL_0_CH_TYPE__COUNT,
		{
			{ NV_VBIOS_CH_TYPE(0, GPU_OFFSET_MAX), NV_NVGPU_CH_TYPE(GPU_OFFSET_MAX), true, },
			{ NV_VBIOS_CH_TYPE(0, GPU_AVG),        NV_NVGPU_CH_TYPE(GPU_AVG),        true, },
			{ NV_VBIOS_CH_TYPE(0, MEM_MAX),        NV_NVGPU_CH_TYPE(MEM_MAX),        true, },
			{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} // Pad to 18
		}
	},
	{
		NV_VBIOS_THERM_CHANNEL_2X_HAL_1_GB20Y_MULTI_DIE,
		NV_VBIOS_THERM_CHANNEL_2X_HAL_1_CH_TYPE__COUNT,
		{
			{ NV_VBIOS_CH_TYPE(1, GPU_OFFSET_MAX),      NV_NVGPU_CH_TYPE(GPU_OFFSET_MAX),      false, },
			{ NV_VBIOS_CH_TYPE(1, GPU_AVG),             NV_NVGPU_CH_TYPE(GPU_AVG),             true, },
			{ NV_VBIOS_CH_TYPE(1, MEM_MAX),             NV_NVGPU_CH_TYPE(MEM_MAX),             true, },
			{ NV_VBIOS_CH_TYPE(1, GPC_OFFSET_MAX),      NV_NVGPU_CH_TYPE(GPC_OFFSET_MAX),      false, },
			{ NV_VBIOS_CH_TYPE(1, HBI_OFFSET_MAX),      NV_NVGPU_CH_TYPE(HBI_OFFSET_MAX),      false, },
			{ NV_VBIOS_CH_TYPE(1, NVL_OFFSET_MAX),      NV_NVGPU_CH_TYPE(NVL_OFFSET_MAX),      false, },
			{ NV_VBIOS_CH_TYPE(1, DIE0_OFFSET_MAX),     NV_NVGPU_CH_TYPE(DIE0_OFFSET_MAX),     false, },
			{ NV_VBIOS_CH_TYPE(1, DIE0_AVG),            NV_NVGPU_CH_TYPE(DIE0_AVG),            false, },
			{ NV_VBIOS_CH_TYPE(1, DIE0_MIN),            NV_NVGPU_CH_TYPE(DIE0_MIN),            false, },
			{ NV_VBIOS_CH_TYPE(1, DIE0_GPC_OFFSET_MAX), NV_NVGPU_CH_TYPE(DIE0_GPC_OFFSET_MAX), false, },
			{ NV_VBIOS_CH_TYPE(1, DIE0_GPC_AVG),        NV_NVGPU_CH_TYPE(DIE0_GPC_AVG),        false, },
			{ NV_VBIOS_CH_TYPE(1, DIE0_GPC_MIN),        NV_NVGPU_CH_TYPE(DIE0_GPC_MIN),        false, },
			{ NV_VBIOS_CH_TYPE(1, DIE1_OFFSET_MAX),     NV_NVGPU_CH_TYPE(DIE1_OFFSET_MAX),     false, },
			{ NV_VBIOS_CH_TYPE(1, DIE1_AVG),            NV_NVGPU_CH_TYPE(DIE1_AVG),            false, },
			{ NV_VBIOS_CH_TYPE(1, DIE1_MIN),            NV_NVGPU_CH_TYPE(DIE1_MIN),            false, },
			{ NV_VBIOS_CH_TYPE(1, DIE1_GPC_OFFSET_MAX), NV_NVGPU_CH_TYPE(DIE1_GPC_OFFSET_MAX), false, },
			{ NV_VBIOS_CH_TYPE(1, DIE1_GPC_AVG),        NV_NVGPU_CH_TYPE(DIE1_GPC_AVG),        false, },
			{ NV_VBIOS_CH_TYPE(1, DIE1_GPC_MIN),        NV_NVGPU_CH_TYPE(DIE1_GPC_MIN),        false, },
		}
	},
	{
		NV_VBIOS_THERM_CHANNEL_2X_HAL_2_GB20Y_SINGLE_DIE,
		NV_VBIOS_THERM_CHANNEL_2X_HAL_2_CH_TYPE__COUNT,
		{
			{ NV_VBIOS_CH_TYPE(2, GPU_OFFSET_MAX), NV_NVGPU_CH_TYPE(GPU_OFFSET_MAX), false, },
			{ NV_VBIOS_CH_TYPE(2, GPU_AVG),        NV_NVGPU_CH_TYPE(GPU_AVG),        true, },
			{ NV_VBIOS_CH_TYPE(2, MEM_MAX),        NV_NVGPU_CH_TYPE(MEM_MAX),        true, },
			{ NV_VBIOS_CH_TYPE(2, GPC_OFFSET_MAX), NV_NVGPU_CH_TYPE(GPC_OFFSET_MAX), false, },
			{ NV_VBIOS_CH_TYPE(2, HBI_OFFSET_MAX), NV_NVGPU_CH_TYPE(HBI_OFFSET_MAX), false, },
			{ NV_VBIOS_CH_TYPE(2, NVL_OFFSET_MAX), NV_NVGPU_CH_TYPE(NVL_OFFSET_MAX), false, },
			{ NV_VBIOS_CH_TYPE(2, GPC_AVG),        NV_NVGPU_CH_TYPE(GPC_AVG),        false, },
			{ NV_VBIOS_CH_TYPE(2, GPC_MIN),        NV_NVGPU_CH_TYPE(GPC_MIN),        false, },
			{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} // Pad to 18
		}
	},
	{
		NV_VBIOS_THERM_CHANNEL_2X_HAL_3_GB20Y,
		NV_VBIOS_THERM_CHANNEL_2X_HAL_3_CH_TYPE__COUNT,
		{
			{ NV_VBIOS_CH_TYPE(3, GPU_OFFSET_MAX), NV_NVGPU_CH_TYPE(GPU_OFFSET_MAX), false, },
			{ NV_VBIOS_CH_TYPE(3, GPU_AVG),        NV_NVGPU_CH_TYPE(GPU_AVG),        true, },
			{ NV_VBIOS_CH_TYPE(3, MEM_MAX),        NV_NVGPU_CH_TYPE(MEM_MAX),        true, },
			{ NV_VBIOS_CH_TYPE(3, GPC_OFFSET_MAX), NV_NVGPU_CH_TYPE(GPC_OFFSET_MAX), false, },
			{ NV_VBIOS_CH_TYPE(3, GPC_AVG),        NV_NVGPU_CH_TYPE(GPC_AVG),        false, },
			{ NV_VBIOS_CH_TYPE(3, GPC_MIN),        NV_NVGPU_CH_TYPE(GPC_MIN),        false, },
			{ NV_VBIOS_CH_TYPE(3, SYS_OFFSET_MAX), NV_NVGPU_CH_TYPE(SYS_OFFSET_MAX), false, },
			{ NV_VBIOS_CH_TYPE(3, SYS_AVG),        NV_NVGPU_CH_TYPE(SYS_AVG),        false, },
			{ NV_VBIOS_CH_TYPE(3, SYS_MIN),        NV_NVGPU_CH_TYPE(SYS_MIN),        false, },
			{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} // Pad to 18
		}
	},
	{
		NV_VBIOS_THERM_CHANNEL_2X_HAL_4_GB20Y,
		NV_VBIOS_THERM_CHANNEL_2X_HAL_4_CH_TYPE__COUNT,
		{
			{ NV_VBIOS_CH_TYPE(4, GPU_OFFSET_MAX),          NV_NVGPU_CH_TYPE(GPU_OFFSET_MAX),          false, },
			{ NV_VBIOS_CH_TYPE(4, GPU_OFFSET_MAX_SECURE),   NV_NVGPU_CH_TYPE(GPU_OFFSET_MAX_SECURE),   false, },
			{ NV_VBIOS_CH_TYPE(4, GPU_AVG),                 NV_NVGPU_CH_TYPE(GPU_AVG),                 true,  },
			{ NV_VBIOS_CH_TYPE(4, GPC_OFFSET_MAX),          NV_NVGPU_CH_TYPE(GPC_OFFSET_MAX),          false, },
			{ NV_VBIOS_CH_TYPE(4, GPC_AVG),                 NV_NVGPU_CH_TYPE(GPC_AVG),                 false, },
			{ NV_VBIOS_CH_TYPE(4, GPC_MIN),                 NV_NVGPU_CH_TYPE(GPC_MIN),                 false, },
			{ NV_VBIOS_CH_TYPE(4, SYS_OFFSET_MAX),          NV_NVGPU_CH_TYPE(SYS_OFFSET_MAX),          false, },
			{ NV_VBIOS_CH_TYPE(4, SYS_AVG),                 NV_NVGPU_CH_TYPE(SYS_AVG),                 false, },
			{ NV_VBIOS_CH_TYPE(4, SYS_MIN),                 NV_NVGPU_CH_TYPE(SYS_MIN),                 false, },
			{ NV_VBIOS_CH_TYPE(4, GPU_MIN),                 NV_NVGPU_CH_TYPE(GPU_MIN),                 false, },
			{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} // Pad to 18
		}
	},
};

static s32 devinitThermChannel2xHalInfoGet
(
	u8    halId,
	u8    entryIdx,
	u8   *pChType,
	bool *pbPublicVisibility
)
{
	s32 status = 0;

	// Ensure that pChType and pbPublicVisibility are not NULL
	if ((pChType == NULL) || (pbPublicVisibility == NULL)) {
		status = -ENOMEM;
		goto _devinitThermChannel2xHalInfoGet_exit;
	}

	// Initialize pChType and pbPublicVisibility
	*pChType             = NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_NOT_ASSIGNED;
	*pbPublicVisibility  = false;

	//
	// Sanity checks for HAL ID:
	// - Ensure that HAL ID is less than max supported HALs AND
	// - Ensure that HAL ID is correctly wired in NV_VBIOS_THERM_CHANNEL_2X_HAL
	//
	if ((halId >= NV_VBIOS_THERM_CHANNEL_2X_HAL__COUNT) ||
		(halId != thermChannel2xHals[halId].halId))
	{
		status = -EINVAL;
		goto _devinitThermChannel2xHalInfoGet_exit;
	}

	// Ensure that HAL entries are less than or equal to 2X_HAL_ENTRY_COUNT
	if(thermChannel2xHals[halId].numEntries >= NV_VBIOS_THERM_CHANNEL_2X_HAL_ENTRY_COUNT)
	{
		status = -EINVAL;
		goto _devinitThermChannel2xHalInfoGet_exit;
	}

	if (entryIdx < thermChannel2xHals[halId].numEntries)
	{
		//
		// Sanity checks for vbiosChType:
		// vbiosChType should match with table entry index     OR
		// vbiosChType should be NOT_ASSIGNED
		//
		if((thermChannel2xHals[halId].channelEntry[entryIdx].vbiosChType == entryIdx)
		|| (thermChannel2xHals[halId].channelEntry[entryIdx].vbiosChType == NV_VBIOS_THERM_CHANNEL_2X_HAL_CH_TYPE_NOT_ASSIGNED))
		{
			*pChType          = thermChannel2xHals[halId].channelEntry[entryIdx].ChType;
			*pbPublicVisibility = thermChannel2xHals[halId].channelEntry[entryIdx].bPublicVisibility;
		}
		else
		{
			status = -EINVAL;
			goto _devinitThermChannel2xHalInfoGet_exit;
		}
	}

_devinitThermChannel2xHalInfoGet_exit:
	return status;
}
static u8 devinitThermChannel2xClassConvertToInternal
(
	u8 vbiosClass
)
{
	u8 Class = NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_INVALID;
	switch (vbiosClass)
	{
		case NV_VBIOS_THERM_CHANNEL_2X_ENTRY_CLASS_PROVIDER_V10:
		{
			Class = NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_PROVIDER_V10;
			break;
		}
		case NV_VBIOS_THERM_CHANNEL_2X_ENTRY_CLASS_DEBUG_V10:
		{
			Class = NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10;
			break;
		}
		case NV_VBIOS_THERM_CHANNEL_2X_ENTRY_CLASS_AGGREGATION_V10:
		{
			Class = NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_AGGREGATION_V10;
			break;
		}
		case NV_VBIOS_THERM_CHANNEL_2X_ENTRY_CLASS_ESTIMATION_V10:
		{
			Class = NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_ESTIMATION_V10;
			break;
		}
		default:
		{
			Class = NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_INVALID;
			break;
		}
	}
	return Class;
}
static int _therm_channel_pmudatainit_device(struct gk20a *g,
			struct pmu_board_obj *obj,
			struct nv_pmu_boardobj *pmu_obj)
{
	int status = 0;
	struct therm_channel *pchannel;
	struct therm_channel_3x *pchannel_3x;
	struct nv_pmu_therm_therm_channel_device_boardobj_set *pset;

		if (!obj || obj->type < 1 || obj->type > 4) {
		nvgpu_err(g, "Invalid obj or obj->type will crash PMU firmware");
		return -EINVAL;
	}

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		nvgpu_err(g,
			"error updating pmu boardobjgrp for therm channel 0x%x",
			status);
		status = -ENOMEM;
		goto done;
	}

	pchannel = (struct therm_channel *)(void *)obj;
	pchannel_3x = (struct therm_channel_3x *)(void *)obj;
	pset = (struct nv_pmu_therm_therm_channel_device_boardobj_set *)(void *)pmu_obj;

	pchannel->temp_min = pchannel->temp_min << 8; // to match padding of SFXP11.5 format
	pchannel->temp_max = pchannel->temp_max << 8; // to match padding of SFXP11.5 format
	pset->super.scaling = pchannel->scaling;
	pset->super.offsetSw = pchannel->offsetSW;
	pset->super.temp_min = pchannel->temp_min;
	pset->super.temp_max = pchannel->temp_max;
	pset->super.bScaling = pchannel->bScaling;
	pset->super.bBounds = pchannel->bBounds;
	pset->super.tempsim_bSupported = pchannel->tempSim.bSupported;
	pset->super.tempsim_bEnabled = pchannel->tempSim.bEnabled;
	pset->super.chType = pchannel->chType;
	pset->super.bPublicVisibility = pchannel_3x->bPublicVisibility;

	pset->super.thermDevIdx = pchannel->super.idx;
	if (pchannel->super.type == NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_PROVIDER_V10) {
		struct therm_channel_provider_v10 *provider = (struct therm_channel_provider_v10 *)(void *)obj;
		pset->super.thermDevProvIdx = provider->thermDevProvIdx;
	} else if (pchannel->super.type == NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10) {
		struct therm_channel_debug_v10 *debug_channel = (struct therm_channel_debug_v10 *)(void *)obj;
		pset->super.thermDevProvIdx = debug_channel->thermDevProvIdxStart;
	}

done:
	return status;
}
s32 thermChannelIfaceModel10Construct_3X(struct gk20a *g,
			struct pmu_board_obj **ppboardobj, size_t size, void *pArgs)
{
	int status = 0;
	struct therm_channel_3x *pChTemp = (struct therm_channel_3x*)pArgs;
	struct therm_channel_3x *pChannel3x = NULL;

	if (*ppboardobj == NULL) {
		*ppboardobj = nvgpu_kzalloc(g, size);
		if (*ppboardobj == NULL) {
			nvgpu_err(g, "Failed to allocate memory for thermal device 3X");
			status = -ENOMEM;
			goto thermDeviceIfaceModel10Construct_SUPER_exit;
		}
	}
	memset((void *)*ppboardobj, 0, size);
	// Set pChProvider10 to point to the allocated object
	pChannel3x = (struct therm_channel_3x*)*ppboardobj;
	status = pmu_board_obj_construct_super(g, *ppboardobj, pArgs);
	if (status != 0) {
		nvgpu_err(g, "failed to construct pchannel_3x");
		return status;
	}
	// Avoid memcpy() to avoid memory overlap after construct_super()
	pChannel3x->super.bBounds            = pChTemp->super.bBounds;
	pChannel3x->super.temp_min          = pChTemp->super.temp_min;
	pChannel3x->super.temp_max          = pChTemp->super.temp_max;
	pChannel3x->super.bScaling           = pChTemp->super.bScaling;
	pChannel3x->super.scaling            = pChTemp->super.scaling;
	pChannel3x->super.offsetSW           = pChTemp->super.offsetSW;
	pChannel3x->super.tempSim.bSupported = pChTemp->super.tempSim.bSupported;
	pChannel3x->super.tempSim.bEnabled   = pChTemp->super.tempSim.bEnabled;

	pChannel3x->super.chType             = pChTemp->super.chType;

	pChannel3x->bPublicVisibility = pChTemp->bPublicVisibility;
	pChannel3x->bVisOverrideSupported = pChTemp->bVisOverrideSupported;

thermDeviceIfaceModel10Construct_SUPER_exit:
	return status;
}
struct therm_channel *construct_channel_device(struct gk20a *g,
			void *pargs, size_t pargs_size, u8 class_id)
{
	struct pmu_board_obj *obj = NULL;
	struct therm_channel *pchannel;
	struct therm_channel_device *pchannel_device;
	int status = 0;
	u8 objType = 0;
	u16 scale_shift = BIT16(8);
	struct therm_channel_device *therm_device = (struct therm_channel_device*)pargs;

	if (g->pmu->therm_pmu->therm_channelobjs == NULL) {
		nvgpu_err(g, "therm_channelobjs not initialized");
		return NULL;
	}
	if(g->pmu->therm_pmu == NULL) {
		nvgpu_err(g, "g->pmu->therm_pmu is NULL");
	}

	switch (class_id) {
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEVICE:
		{
			status = thermChannelIfaceModel10Construct_3X(g, &obj, pargs_size, pargs);

			pchannel_device = nvgpu_kzalloc(g, pargs_size);
			if (pchannel_device == NULL) {
				return NULL;
			}
			obj = (struct pmu_board_obj *)(void *)pchannel_device;
			status = pmu_board_obj_construct_super(g, obj, pargs);
			if (status != 0) {
				return NULL;
			}
			obj->pmudatainit = _therm_channel_pmudatainit_device;
			pchannel = (struct therm_channel *)(void *)obj;
			pchannel_device = (struct therm_channel_device *)(void *)obj;
			g->ops.therm.get_internal_sensor_limits(&pchannel->temp_max,
				&pchannel->temp_min);
			pchannel->scaling = S16(scale_shift);
			pchannel->offsetSW  = 0;
			// Important: Use the correct field names from NV_PMU_THERM_THERM_CHANNEL_DEVICE_BOARDOBJ_SET
			// Map from therm_dev_idx to thermDevIdx
			pchannel_device->therm_dev_idx = therm_device->therm_dev_idx;
			pchannel_device->therm_dev_prov_idx = therm_device->therm_dev_prov_idx;

			if (status != 0) {
				return NULL;
			}
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_PROVIDER_V10:
		{
			struct therm_channel *pchannel_in = (struct therm_channel *)pargs;
			status = thermChannelIfaceModel10Construct_PROVIDER_V10(g, &obj, pargs_size, pargs);

			if (obj != NULL) {
				struct therm_channel *pchannel_after = (struct therm_channel *)(void *)obj;
				if (pchannel_after->chType == 0 && pchannel_in->chType != 0) {
					pchannel_after->chType = pchannel_in->chType;
				}
			}

			obj->pmudatainit = _therm_channel_pmudatainit_device;

			if (status != 0) {
				nvgpu_err(g, "fail to construct PROVIDER_V10 %d", status);
				return NULL;
			}
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10:
		{
			status = thermChannelIfaceModel10Construct_DEBUG_V10(g, &obj, pargs_size, pargs);
			if (status != 0) {
				nvgpu_err(g, "fail to construct DEBUG_V10 %d", status);
				return NULL;
			}
			obj->pmudatainit = _therm_channel_pmudatainit_device;
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_AGGREGATION_V10:
		{
			status = thermChannelIfaceModel10Construct_AGGREGATION_V10(g, &obj, pargs_size, pargs);
			if (status != 0) {
				nvgpu_err(g, "fail to construct AGGREGATION_V10 %d", status);
				return NULL;
			}
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_ESTIMATION_V10:
		{
			status = thermChannelIfaceModel10Construct_ESTIMATION_V10(g, &obj, pargs_size, pargs);
			if (status != 0) {
				nvgpu_err(g, "fail to construct ESTIMATION_V10 %d", status);
				return NULL;
			}
			break;
		}
		default:
		{
			nvgpu_err(g, "Invalid therm channel device type: %d", objType);
			return NULL;
		}
	}
	nvgpu_log_info(g, " Done");
	return (struct therm_channel *)obj;
}
static int _therm_channel_pmudata_instget(struct gk20a *g,
			struct nv_pmu_boardobjgrp *pmuboardobjgrp,
			struct nv_pmu_boardobj **pmu_obj,
			u16 idx)
{
	struct nv_pmu_therm_therm_channel_boardobj_grp_set *pgrp_set =
		(struct nv_pmu_therm_therm_channel_boardobj_grp_set *)
		pmuboardobjgrp;
	int status = 0;

	nvgpu_log_info(g, " ");

	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
			pgrp_set->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		status = -EINVAL;
		goto done;
	}

	*pmu_obj = (struct nv_pmu_boardobj *)
		&pgrp_set->objects[idx].data.obj;
	nvgpu_log_info(g, " Done");

done:
	return status;
}
static int therm_channel_pmustatus_instget(struct gk20a *g,
	void *pboardobjgrppmu, struct nv_pmu_boardobj_query
	**obj_pmu_status, u16 idx)
{
	struct nv_pmu_therm_therm_channel_boardobj_grp_get_status *pmu_status =
		(struct nv_pmu_therm_therm_channel_boardobj_grp_get_status *)
		(void *)pboardobjgrppmu;
	int status = 0;

	(void)g;

	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
		pmu_status->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		status = -EINVAL;
		goto done;
	}

	*obj_pmu_status = (struct nv_pmu_boardobj_query *)
			&pmu_status->objects[idx].data.obj;

done:
	return status;
}
static int devinit_get_therm_channel_table(struct gk20a *g,
				struct therm_channels *pthermchannelobjs)
{
	int status = 0;
	u8 *therm_channel_table_ptr = NULL;
	u8 *curr_therm_channel_table_ptr = NULL;
	struct therm_channel_2x_header therm_channel_table_header = { 0 };
	struct therm_channel_2x_entry* therm_channel_table_entry = NULL;
	struct therm_channel *pChannel = NULL;
	struct boardobjgrp *pboardobjgrp = NULL;
	u8 class_id = 0;
	u8 flags    = 0;
	u8 mode     = 0;
	u32  param0 = 0;
	u32  param1 = 0;
	u32  param2 = 0;
	u32  index  = 0;
	u32  obj_index = 0;
	u32  invalidEntryCnt = 0;
	size_t therm_channel_size = 0;
	size_t remaining_bytes = 0;
	bool table_has_valid_entries = false;
	union {
		struct pmu_board_obj obj;
		struct therm_channel channel;
		struct therm_channel_3x v3x;
		struct therm_channel_provider_v10 provider;
		struct therm_channel_debug_v10 debug;
		struct therm_channel_aggregation_v10 aggregation;
		struct therm_channel_estimation_v10 estimation;
	} therm_channel_data;
	nvgpu_log_info(g, " ");

	// Reset the boardobjgrp state before processing any entries
	pboardobjgrp = &pthermchannelobjs->super.super;

	// Use the CTRL_BOARDOBJ_IDX_INVALID constant (255) for initialization
	// But DON'T set objmaxidx directly - nvgpu_boardobjgrp_construct_super will handle this
	pboardobjgrp->objmask = 0; // Ensure mask starts empty

	// Clear each bit in the mask individually
	for (u32 i = 0; i < pboardobjgrp->mask->bitcount; i++) {
		nvgpu_boardobjgrpmask_bit_clr(pboardobjgrp->mask, i);
	}

	/* Get the thermal channel table from VBIOS FIRST before accessing any data */
	therm_channel_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_PERF_TOKEN),
						THERMAL_CHANNEL_TABLE);
	if (therm_channel_table_ptr == NULL) {
		nvgpu_err(g, " NULL thermal channel table ptr");
		status = -EINVAL;
		goto done;
	}
	/* Validate that the thermal channel table pointer is within BIOS bounds */
	if (therm_channel_table_ptr < g->bios->data ||
		therm_channel_table_ptr >= (g->bios->data + g->bios->size)) {
		nvgpu_err(g, "Thermal channel table pointer %p is outside BIOS bounds [%p, %p)",
			  therm_channel_table_ptr, g->bios->data, g->bios->data + g->bios->size);
		status = -EINVAL;
		goto done;
	}
	/* Calculate remaining bytes in BIOS from thermal channel table start */
	remaining_bytes = g->bios->size - (therm_channel_table_ptr - g->bios->data);
	/* Ensure we have enough bytes for basic header access */
	if (remaining_bytes < 8) {
		nvgpu_err(g, "Not enough bytes for basic header access. Need 8, have %zu", remaining_bytes);
		status = -EINVAL;
		goto done;
	}

	/* Ensure we have enough bytes for full header */
	if (remaining_bytes < VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09) {
		nvgpu_err(g, "Not enough bytes for full header. Need %d, have %zu",
			  VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09, remaining_bytes);
		status = -EINVAL;
		goto done;
	}
	// Copy the header from the VBIOS to the therm_channel_table_header 2.0 structure
	nvgpu_memcpy((u8 *)&therm_channel_table_header, therm_channel_table_ptr, VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09);

	if (therm_channel_table_header.version !=
			VBIOS_THERM_CHANNEL_VERSION_2X) {
		nvgpu_err(g, "Thermal channel table version mismatch, expected %d, got %d",
			VBIOS_THERM_CHANNEL_VERSION_2X, therm_channel_table_header.version);
		status = -EINVAL;
		goto done;
	}
	if (therm_channel_table_header.header_size <
			VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09) {
		nvgpu_err(g, "Thermal channel table header size overflow, expected %d, got %d",
			VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09, therm_channel_table_header.header_size);
		status = -EINVAL;
		goto done;
	}

	curr_therm_channel_table_ptr = (therm_channel_table_ptr + VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09);

	/* Quick scan to detect if table contains only invalid/dummy entries */
	table_has_valid_entries = false;
	for (u32 scan_idx = 0; scan_idx < therm_channel_table_header.num_table_entries; scan_idx++) {
		u32 scan_offset = therm_channel_table_header.table_entry_size * scan_idx;
		struct therm_channel_2x_entry *scan_entry = (struct therm_channel_2x_entry *)
			(curr_therm_channel_table_ptr + scan_offset);

		/* Check bounds for scan */
		if ((scan_offset + sizeof(struct therm_channel_2x_entry)) > (remaining_bytes - VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09)) {
			break;
		}

		if (scan_entry->class_id != NV_VBIOS_THERM_CHANNEL_2X_ENTRY_CLASS_INVALID &&
			scan_entry->param0 != 0xFFFFFFFF) {
			table_has_valid_entries = true;
			break;
		}
	}

	if (!table_has_valid_entries) {
		status = 0; /* Not an error - just no thermal channels available */
		goto done;
	}

	for (index = 0; index < therm_channel_table_header.num_table_entries; index++) {
		u32 entry_offset;
		u32 entry_end_offset;
		size_t available_bytes;
		/* Calculate entry address */
		entry_offset = therm_channel_table_header.table_entry_size * index;
		therm_channel_table_entry = (struct therm_channel_2x_entry *)
			(curr_therm_channel_table_ptr + entry_offset);

		/* Validate entry bounds before accessing */
		entry_end_offset = entry_offset + therm_channel_table_header.table_entry_size;
		available_bytes = remaining_bytes - VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09;

		if (entry_end_offset > available_bytes) {
			nvgpu_err(g, "Entry %d extends beyond available data. Need %d bytes, have %zu",
				  index, entry_end_offset, available_bytes);
			status = -EINVAL;
			goto done;
		}

		/* Validate minimum entry size */
		if (therm_channel_table_header.table_entry_size < sizeof(struct therm_channel_2x_entry)) {
			nvgpu_err(g, "Entry %d size %d too small for therm_channel_2x_entry (need %zu)",
				  index, therm_channel_table_header.table_entry_size, sizeof(struct therm_channel_2x_entry));
			status = -EINVAL;
			goto done;
		}
		memset(&therm_channel_data, 0, sizeof(therm_channel_data));
		/* Extract and validate raw entry fields */
		flags = therm_channel_table_entry->flags;
		param0 = therm_channel_table_entry->param0;
		param1 = therm_channel_table_entry->param1;
		param2 = therm_channel_table_entry->param2;
		class_id = devinitThermChannel2xClassConvertToInternal(therm_channel_table_entry->class_id);

		switch (class_id) {
			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_INVALID:
				{
				invalidEntryCnt++;
				// To avoid TOCTOU vulnerability, we check the invalidEntryCnt here and exit if it is greater than the maximum number of entries
				if (invalidEntryCnt > therm_channel_table_header.num_table_entries) {
					nvgpu_err(g, "Invalid entry count reached maximum, invalidEntryCnt is %d", invalidEntryCnt);
					goto done;
				}
				continue;
				break;
				}
			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_PROVIDER_V10:
				{
				therm_channel_data.provider.thermDevIdx =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_PROVIDER_DEV_INDEX);
				therm_channel_data.provider.thermDevProvIdx =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_PROVIDER_PROV_INDEX);
				therm_channel_size = sizeof(struct therm_channel_provider_v10);
				break;
				}
			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10:
				{
				therm_channel_data.debug.thermDevIdx =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_DEBUG_DEV_INDEX);
				therm_channel_data.debug.thermDevProvIdxStart =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_DEBUG_PROV_INDEX_START);
				therm_channel_data.debug.thermDevProvIdxEnd =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_DEBUG_PROV_INDEX_END);
				therm_channel_size = sizeof(struct therm_channel_debug_v10);
				break;
				}
			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_AGGREGATION_V10:
				{
				therm_channel_data.aggregation.thermDevIdx =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_DEV_INDEX);
				therm_channel_data.aggregation.thermDevProvIdxStart =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_PROV_INDEX_START);
				therm_channel_data.aggregation.thermDevProvIdxEnd =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_PROV_INDEX_END);
				therm_channel_data.aggregation.thermChannelIdxNext =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_NEXT_CHANNEL);
				mode = BIOS_GET_FIELD(u8, flags, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_AGGREGATION_MODE);
				therm_channel_data.aggregation.mode = devinitThermChannel2xAggregationModeConvertToInternal(g, index, mode);
				therm_channel_size = sizeof(struct therm_channel_aggregation_v10);
				break;
				}
			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_ESTIMATION_V10:
				{

				therm_channel_data.estimation.thermChannelIdx1 =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_ESTIMATION_CHANNEL_1);
				// Extract the second channel index from the param0 field
				therm_channel_data.estimation.thermChannelIdx2 =
						BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_ESTIMATION_CHANNEL_2);
				// Extract the estimation mode from the flags field
				mode = BIOS_GET_FIELD(u8, flags, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_ESTIMATION_MODE);
				// Convert the estimation mode to the internal format
				therm_channel_data.estimation.mode = devinitThermChannel2xEstimationModeConvertToInternal(g, index, mode);
				// Set the size of the therm_channel_estimation_v10 structure
				therm_channel_size = sizeof(struct therm_channel_estimation_v10);

				break;
				}
			default:
				{
				// Set the size of the therm_channel_device structure
				therm_channel_size = sizeof(struct therm_channel_device);
				break;
				}
		}
		// Initialize data for BOARDOBJ parent class
		therm_channel_data.obj.type = class_id;
		// Extract the scaling flag from the flags field
		therm_channel_data.channel.bScaling = BIOS_GET_FIELD(u8, flags, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_SCALING);
		// Extract the bounds flag from the flags field
		therm_channel_data.channel.bBounds = BIOS_GET_FIELD(u8, flags, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_BOUNDS);
		// Extract the software temperature simulation flag from the flags field
		therm_channel_data.channel.tempSim.bSupported = BIOS_GET_FIELD(u8, flags, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_SW_TEMP_SIM);
		// Initialize scaling parameters from VBIOS when scaling is enabled.
		// Otherwise set scaling slope to 1.0 and offset to 0.0
		if (therm_channel_data.channel.bScaling)
		{
			therm_channel_data.channel.scaling =
					BIOS_GET_FIELD(u32, param1, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM1_SCALING_SLOPE);
			therm_channel_data.channel.offsetSW =
					BIOS_GET_FIELD(u32, param1, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM1_SCALING_OFFSETSW);
		}
		else
		{
			therm_channel_data.channel.scaling  = 0x0100u;
			therm_channel_data.channel.offsetSW = 0x0;
		}
		// Initalize bounds from VBIOS when enabled. Otherwise bounds to 0C.
		if (therm_channel_data.channel.bBounds) {
			s16 temp11x5;

			temp11x5 = BIOS_GET_FIELD(u32, param2, NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM2_TEMP_MIN);

			therm_channel_data.channel.temp_min = NV_TYPES_NVSFXP11_5_TO_NV_TEMP(temp11x5);

			temp11x5 = param2 >> NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM2_BOUND_CEIL_SHIFT;

			therm_channel_data.channel.temp_max = NV_TYPES_NVSFXP11_5_TO_NV_TEMP(temp11x5);
		}
		else{
			therm_channel_data.channel.temp_min = 0;
			therm_channel_data.channel.temp_max = 0;
		}
		 // Get channel type and visibility details from HAL
		// For debug channels, derive channel type from debug channel parameters
		if (class_id == NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10) {
			// Debug channels: derive type from thermDevProvIdxStart/End range and index
			// This is the legal way to determine debug channel types based on VBIOS data
			u8 debug_start = therm_channel_data.debug.thermDevProvIdxStart;
			u8 debug_end = therm_channel_data.debug.thermDevProvIdxEnd;
			u8 debug_range = debug_end - debug_start + 1;
			u8 debug_offset = index - 10; // Debug channels start at index 10

			// Map debug channels based on their provider range and offset
			// This provides a deterministic mapping based on VBIOS parameters
			if (debug_range >= 4 && debug_offset < 4) {
				// GPC debug channels (0-3)
				therm_channel_data.channel.chType = NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_GPC_0 + debug_offset;

			} else if (debug_offset == 4) {
				// SYS debug channel
				therm_channel_data.channel.chType = NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_SYS;

			} else if (debug_offset == 5) {
				// SOC debug channel
				therm_channel_data.channel.chType = NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_SOC_0;

			} else {
				// Fallback for unknown debug channels
				nvgpu_err(g, "Debug channel %d: unknown mapping", index);
			}

			// Debug channels are typically public
			therm_channel_data.v3x.bPublicVisibility = true;
			status = 0; // Success

		} else {
			// Regular (non-debug) channels use normal HAL lookup
			status = devinitThermChannel2xHalInfoGet((u8)therm_channel_table_header.typeHAL,
											  index,
											  &therm_channel_data.channel.chType,
											  &therm_channel_data.v3x.bPublicVisibility);
		}

		if (status != 0) {
			nvgpu_err(g,"[%s]:%d. devinitThermChannel2xHalInfoGet() failed to get channel type and visibility details.\n", __func__,__LINE__);
			goto done;
		}

		therm_channel_data.channel.relloc = 0;
		therm_channel_data.channel.tgtGPU = 0;
		therm_channel_data.channel.flags  = 0;

		pChannel = construct_channel_device(g, &therm_channel_data,
					therm_channel_size, class_id);

		if (pChannel == NULL) {
			nvgpu_err(g,
				"unable to create thermal device for %d type %d",
				index, therm_channel_data.obj.type);
			status = -ENOMEM;
			goto done;
		}

		status = boardobjgrp_objinsert(&pthermchannelobjs->super.super,
				(struct pmu_board_obj *)pChannel, (u8)index);

		if (status != 0) {
			nvgpu_err(g,
			"unable to insert thermal device boardobj for %d", index);
			status = -ENOMEM;
			goto done;
		}
		// Only increment obj_index if we successfully inserted an object
		++obj_index;
	}
done:

	if (obj_index > 0) {
		nvgpu_log_info(g, "SUCCESS: %d thermal channels are now configured", obj_index);
	} else {
		nvgpu_err(g, "ERROR: No thermal channels configured (expected for dev platforms)");
	}

	nvgpu_log_info(g, " DONE %x", status);
	return status;
}
static s32 therm_channel_init_objs(struct gk20a *g)
{
	void *temp_ptr = NULL;
	/* Check if therm_channelobjs is already initialized */
	if(g->pmu->therm_pmu->therm_channelobjs != NULL) {
		nvgpu_err(g, "therm_channelobjs already initialized");
		return 0;
	}
	/* Allocate memory for therm_channelobjs */
	temp_ptr = nvgpu_kzalloc_impl(g, sizeof(struct therm_channels), NVGPU_GET_IP);
	if(temp_ptr == NULL) {
		nvgpu_err(g, "error allocating memory for therm_channelobjs");
		return -ENOMEM;
	}
	/* Assign the allocated memory to therm_channelobjs */
	g->pmu->therm_pmu->therm_channelobjs = temp_ptr;
	return 0;
}
int therm_channel_sw_setup(struct gk20a *g)
{
	int status;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct therm_channels *pthermchannelobjs;
	struct boardobjgrp_e255 *pboardobjgrp_e255 = NULL;

	/* Check if therm_pmu is initialized */
	if ((g->pmu == NULL) || (g->pmu->therm_pmu == NULL)) {
		nvgpu_err(g, "PMU or therm_pmu not initialized");
		status = -ENOMEM;
		goto done;
	}

	/* Check if therm_channelobjs is initialized */
	if (g->pmu->therm_pmu->therm_channelobjs == NULL) {
		nvgpu_err(g, "therm_channelobjs not initialized, re-allocate memory for therm_channelobjs");
		status = therm_channel_init_objs(g);
		if (status != 0) {
			nvgpu_err(g, "error initializing therm_channelobjs");
			status = -EINVAL;
			goto done;
		}
	}

	/* If already constructed, do not re-construct (suspend/resume pattern) */
	pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;
	if (pboardobjgrp->bconstructed) {
		nvgpu_pmu_dbg(g, "therm channel boardobjgrp already constructed, skipping reinit");
		return 0;
	}

	nvgpu_pmu_dbg(g, "Constructing therm channel boardobjgrp for first time");

	/* Initialize board object group mask */
	pboardobjgrp_e255 = &g->pmu->therm_pmu->therm_channelobjs->super;
	status = nvgpu_boardobjgrpmask_init(&(&pboardobjgrp_e255->mask)->super, 32U, NULL);
	if (status != 0) {
		nvgpu_err(g, "error initializing boardobjgrp mask");
		status = -EINVAL;
		goto done;
	}
	/* Construct the board object group */
	status = nvgpu_boardobjgrp_construct_e255(g, pboardobjgrp_e255);
	if (status != 0) {
		nvgpu_err(g, "error creating boardobjgrp for therm devices, status - 0x%x",
				 status);
		status = -EINVAL;
		goto done;
	}
	/* Setup board object group */
	pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;

	pthermchannelobjs = g->pmu->therm_pmu->therm_channelobjs;

	/* Set the thermal channels class type to 3X */
	pboardobjgrp->classType = NV2080_CTRL_THERMAL_THERM_CHANNELS_CLASS_TYPE_3X;

	/* Override the Interfaces */
	pboardobjgrp->pmudatainstget = _therm_channel_pmudata_instget;
	pboardobjgrp->pmustatusinstget = therm_channel_pmustatus_instget;

	/* Get the therm_channel_table from VBIOS and parse into boardobjgrp structure */
	status = devinit_get_therm_channel_table(g, pthermchannelobjs);
	if (status != 0) {
		nvgpu_err(g, "error getting therm_channel_table from VBIOS");
		status = -EINVAL;
		goto done;
	}

	/* Construct PMU board object group */
	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, THERM, THERM_CHANNEL);

	/* Construct PMU board object command group set */
	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			therm, THERM, therm_channel, THERM_CHANNEL);
	if (status != 0) {
		nvgpu_err(g, "error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
				 status);
		status = -EINVAL;
		goto done;
	}

	/* Construct PMU board object command group get status */
	status = BOARDOBJGRP_PMU_CMD_GRP_GET_STATUS_CONSTRUCT(g, pboardobjgrp,
			therm, THERM, therm_channel, THERM_CHANNEL);
	if (status != 0) {
		nvgpu_err(g,
			"error constructing THERM_GET_STATUS interface - 0x%x",
			status);
		status = -EINVAL;
		goto done;
	}
	nvgpu_log_info(g, "BOARDOBJGRP_PMU_CMD_GRP_GET_STATUS_CONSTRUCT done");
done:
	return status;
}


/* Function to directly extract temperatures from raw PMU buffer */
static int extract_temp_from_raw_buffer(struct gk20a *g, u8 channel_idx, u32 *temp_out)
{
	struct boardobjgrp *pboardobjgrp;
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu;
	u8 *raw_buffer;
	size_t thermal_data_start = 0x30; /* Thermal data starts at offset 0x30 */
	size_t entry_size = 64; /* Each thermal entry is 64 bytes */
	size_t channel_offset = 0;
	u32 *type_ptr = NULL;
	u32 channel_type = 0;
	u32 raw_temp = 0;
	int status = 0;

	if (g == NULL || g->pmu == NULL || g->pmu->therm_pmu == NULL ||
	    g->pmu->therm_pmu->therm_channelobjs == NULL || temp_out == NULL) {
		status = -EINVAL;
		goto done;
	}

	pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;
	pboardobjgrppmu = pboardobjgrp->pmu.getstatus.buf;

	if (pboardobjgrppmu == NULL) {
		status = -EINVAL;
		goto done;
	}

	raw_buffer = (u8 *)pboardobjgrppmu;

	/* Calculate the offset for this channel's thermal data */
	channel_offset = thermal_data_start + (channel_idx * entry_size);

	/* Check the channel type to determine structure */
	type_ptr = (u32 *)(raw_buffer + channel_offset);
	channel_type = *type_ptr;

	if (channel_type == NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_PROVIDER_V10) {
		struct raw_thermal_entry *entry = (struct raw_thermal_entry *)(raw_buffer + channel_offset);

		raw_temp = entry->temp_value;

		if (raw_temp == 0 ||
			(raw_temp > PMU_TEMP_MAX_150C && raw_temp < (u32)PMU_TEMP_MIN_40C)) {
			nvgpu_err(g,
				"Channel[%d] invalid temperature 0x%x (out of -40°C to 150°C range)",
				channel_idx, raw_temp);
			status = -ENODATA;
			goto done;
		}
	} else if (channel_type == NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10) {
		/*
		 * NVGPU will not use debug channels as there are no requests,
		 * comment out the code for future usage
		 */
		/*
		struct debug_channel_raw_temp *entry =
			(struct debug_channel_raw_temp *)(raw_buffer + channel_offset);
		bool found_temp = false;
		for (int i = 0; i < 8; i++) {
			u32 candidate_temp = entry->temp_array[i];

			if ((candidate_temp > 0 && candidate_temp <= PMU_TEMP_MAX_150C) ||
			    (candidate_temp >= (u32)PMU_TEMP_MIN_40C)) {
				raw_temp = candidate_temp;
				found_temp = true;

				break;
			}
		}
		if (!found_temp) {
			nvgpu_pmu_dbg(g,
				"Debug Channel[%d] do not have a valid temperature ",
				channel_idx);
			status = -ENODATA;
			goto done;
		}
		*/
	} else {
		nvgpu_err(g, "Channel[%d] unknown type=0x%x", channel_idx, channel_type);
		status = -ENODATA;
		goto done;
	}

	/* Validate temperature (reasonable range check: -40°C to 150°C)
	 * PMU format: temp_celsius = temp_raw / 256
	 */

	*temp_out = raw_temp;

done:
	return status;
}

/* Enhanced therm_channel_currtemp_update that uses raw buffer parsing */
static int therm_channel_currtemp_update_raw(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj, u8 channel_idx)
{

	struct therm_channel_get_status *therm_channel_obj;
	u32 raw_temp = 0;
	int status;

	nvgpu_log_info(g, " ");

	therm_channel_obj = (struct therm_channel_get_status *)(void *)obj;

	status = extract_temp_from_raw_buffer(g, channel_idx, &raw_temp);

	if (status == 0 && raw_temp > 0) {
		/* Successfully extracted valid temperature from raw buffer */
		therm_channel_obj->curr_temp = raw_temp;

	} else {

		struct nv_pmu_therm_therm_channel_boardobj_get_status *pmu_status;

		pmu_status = (struct nv_pmu_therm_therm_channel_boardobj_get_status *)(void *)pmu_obj;

		if (pmu_status != NULL) {
			therm_channel_obj->curr_temp = pmu_status->current_temp;
		} else {
			therm_channel_obj->curr_temp = 0;
			nvgpu_pmu_dbg(g,
				"ERROR: Channel[%d] - both raw buffer and PMU status failed",
				channel_idx);
			status = -ENODATA;
		}
	}

	return status;
}

static int therm_channel_boardobj_grp_get_status(struct gk20a *g)
{
	struct boardobjgrp *pboardobjgrp = NULL;
	struct boardobjgrpmask *pboardobjgrpmask;
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu;
	struct pmu_board_obj *obj = NULL;
	struct nv_pmu_boardobj_query *pboardobjpmustatus = NULL;
	int status;
	u16 index;

	nvgpu_log_info(g, " ");

	if (g == NULL) {
		status = -EINVAL;
		goto done;
	}

	if (g->pmu == NULL || g->pmu->therm_pmu == NULL ||
		g->pmu->therm_pmu->therm_channelobjs == NULL) {
		nvgpu_err(g, "PMU or therm_pmu not initialized");
		status = -EINVAL;
		goto done;
	}

	pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;
	pboardobjgrpmask = &g->pmu->therm_pmu->therm_channelobjs->super.mask.super;
	status = pboardobjgrp->pmugetstatus(g, pboardobjgrp, pboardobjgrpmask);
	if (status != 0) {
		nvgpu_err(g, "err getting boardobjs from pmu");
		goto done;
	}

	pboardobjgrppmu = pboardobjgrp->pmu.getstatus.buf;
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		status = pboardobjgrp->pmustatusinstget(g,
				(struct nv_pmu_boardobjgrp *)(void *)pboardobjgrppmu,
				&pboardobjpmustatus, index);
		if (status != 0) {
			nvgpu_err(g, "could not get status object instance");
			goto done;
		}
		status = therm_channel_currtemp_update_raw(g, obj,
				(struct nv_pmu_boardobj *)(void *)pboardobjpmustatus, index);
		if (status != 0) {
			nvgpu_err(g, "could not update therm_channel status");
			goto done;
		}
	}

done:
	return status;
}

int nvgpu_pmu_therm_channel_get_curr_temp(struct gk20a *g, u32 *temp)
{
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	struct therm_channel_get_status *therm_channel_status = NULL;
	int status;
	u16 index;

	if ((g == NULL) || (temp == NULL)) {
		status = -EINVAL;
		goto done;
	}

	if (((g->pmu == NULL) || (g->pmu->therm_pmu == NULL)) ||
		(g->pmu->therm_pmu->therm_channelobjs == NULL)) {
		nvgpu_err(g, "PMU or therm_pmu not initialized");
		status = -EINVAL;
		goto done;
	}

	status = therm_channel_boardobj_grp_get_status(g);
	if (status != 0) {
		nvgpu_err(g, "therm_channel get status failed");
		goto done;
	}

	pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		therm_channel_status = (struct therm_channel_get_status *)(void *)obj;

		if (therm_channel_status->curr_temp != 0U) {
			*temp = therm_channel_status->curr_temp;
			break;
		}
	}

done:
	return status;
}

int therm_channel_pmu_setup(struct gk20a *g)
{
	int status = 0;
	struct boardobjgrp *pboardobjgrp = NULL;

	/* Validate input parameter */
	if (g == NULL) {
		status = -EINVAL;
		goto done;
	}
	/* Check if PMU and therm_pmu are initialized */
	if ((g->pmu == NULL) || (g->pmu->therm_pmu == NULL)) {
		nvgpu_err(g, "PMU or therm_pmu not initialized");
		status = -ENOMEM;
		goto done;
	}
	/* Initialize PMU handle if board object group is not empty */
	if (!BOARDOBJGRP_IS_EMPTY( &g->pmu->therm_pmu->therm_channelobjs->super.super)) {
		pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;
		status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);
		if (status != 0) {
			nvgpu_err(g, "error initializing PMU for therm channels");
			goto done;
		}
	}
done:
	return status;
}

/*
 * Get specific temperature type (max, avg, min) from thermal channels
 * Returns temperature in PMU format (divide by 256 for Celsius)
 */
int nvgpu_pmu_therm_channel_get_temperature(struct gk20a *g, s32 *temp, u8 temp_type)
{
	struct boardobjgrp *pboardobjgrp = NULL;
	struct pmu_board_obj *obj = NULL;
	struct therm_channel_get_status *therm_channel_status = NULL;
	int status = 0;
	u8 channel_idx;
	u16 index;

	if ((g == NULL) || (temp == NULL)) {
		status = -EINVAL;
		goto done;
	}

	if (((g->pmu == NULL) || (g->pmu->therm_pmu == NULL)) ||
	    ((g->pmu->therm_pmu->therm_channelobjs == NULL))) {
		nvgpu_err(g, "PMU or therm_pmu not initialized");
		status = -EINVAL;
		goto done;
	}

	/* Determine channel index and type name based on temperature type */
	switch (temp_type) {
	case NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_GTZ_MAX:
		/* Channel 0 for max temperature */
		channel_idx = NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_GTZ_MAX;
		break;

	case NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_GTZ_AVG:
		/* Channel 2 for average temperature */
		channel_idx = NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_GTZ_AVG;
		break;

	case NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_GTZ_MIN:
		/* Channel 9 for minimum temperature */
		channel_idx = NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_GTZ_MIN;
		break;

	default:
		nvgpu_err(g, "Invalid temperature type: 0x%x", temp_type);
		status = -EINVAL;
		goto done;
	}

	/* Get latest temperature data from PMU */
	status = therm_channel_boardobj_grp_get_status(g);
	if (status != 0) {
		nvgpu_err(g, "therm_channel get status failed");
		goto done;
	}

	pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;

	/* Find the specific channel object by iterating through all channels */
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		if (index == channel_idx) {
			therm_channel_status = (struct therm_channel_get_status *)(void *)obj;
			break;
		}
	}

	if (therm_channel_status == NULL) {
		nvgpu_err(g, "Channel[%d] not found in board object group", channel_idx);
		status = -ENOENT;
		goto done;
	}

	/* Validate temperature reading
	 * Valid temperature range: -40°C to 150°C in PMU format
	 * PMU format: temp_celsius = temp_raw / 256
	 * For negative temperatures, we need to check if the value is within signed range
	 */
	if (therm_channel_status->curr_temp != 0U) {
		s32 temp_signed = (s32)therm_channel_status->curr_temp;

		/* Check if temperature is within valid range */
		if ((temp_signed >= PMU_TEMP_MIN_40C && temp_signed <= PMU_TEMP_MAX_150C)) {
			*temp = temp_signed;
		} else {
			nvgpu_err(g, "Channel[%d] temperature out of range: 0x%x (%d°C)",
				channel_idx, therm_channel_status->curr_temp, temp_signed / 256);
			status = -ENODATA;
		}
	} else {
		nvgpu_err(g, "Channel[%d] no valid temperature data (temp=0x%x)",
			channel_idx, therm_channel_status->curr_temp);
		status = -ENODATA;
	}

done:
	return status;
}
