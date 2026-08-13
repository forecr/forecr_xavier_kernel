// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bios.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/pmu/therm.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/pmu/allocator.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/string.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bitops.h>
#include "therm_dev.h"
#include "therm_device_3x.h"
#include "therm_device_gpu_tz_v10.h"
#include "therm_device_gpu_stz_v10.h"
#include "therm_device_gpu_tsense_v10.h"
#include "therm_device_gpu_mini_tsense_v10.h"
#include "therm_device_gpu_gtz_v10.h"
#include "therm_device_mem_v10.h"
#include "thrm.h"
#include <nvgpu/boardobjgrpmask.h>
#include <nvgpu/list.h>
#include <nvgpu/pmu/super_surface.h>
#include <common/pmu/super_surface/super_surface_priv.h>
#include <common/pmu/boardobj/boardobj.h>

/* Forward declaration to avoid circular dependency and MISRA violations*/
struct nv_pmu_boardobj;

static NV_VBIOS_THERM_DEVICE_2X_HAL thermDevice2xHals[] =
{
    // Data related to HAL_0_DEFAULT
    {
        NV_VBIOS_THERM_DEVICE_2X_HAL_0_DEFAULT,
        NV_VBIOS_THERM_DEVICE_2X_HAL_0_DEV_TYPE__COUNT,
        {
            { NV_VBIOS_DEV_TYPE(0, GPU_OFFSET_MAX), NV_THERM_DEV_TYPE(GPU_OFFSET_MAX), false, },
            { NV_VBIOS_DEV_TYPE(0, GPU_AVG),        NV_THERM_DEV_TYPE(GPU_AVG),        true, },
        }
    },
    // Data related to HAL_1_GB20Y
    {
        NV_VBIOS_THERM_DEVICE_2X_HAL_1_GB20Y,
        NV_VBIOS_THERM_DEVICE_2X_HAL_1_DEV_TYPE__COUNT,
        {
              { NV_VBIOS_DEV_TYPE(1, GPU_OFFSET_MAX),         NV_THERM_DEV_TYPE(GPU_OFFSET_MAX),         false, },
              { NV_VBIOS_DEV_TYPE(1, GPU_OFFSET_MAX_SECURE),  NV_THERM_DEV_TYPE(GPU_OFFSET_MAX_SECURE),  false, },
              { NV_VBIOS_DEV_TYPE(1, GPU_AVG),                NV_THERM_DEV_TYPE(GPU_AVG),                false, },
        }
    },
};

bool therm_device_idx_is_valid(struct nvgpu_pmu_therm *therm_pmu, u16 idx)
{
	return boardobjgrp_idxisvalid(
			&(therm_pmu->therm_deviceobjs->super.super), idx);
}

static s32 therm_device_pmudatainit (struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	s32 status = 0;
	nvgpu_pmu_dbg(g, " ");
	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);
	if (status != 0) {
		nvgpu_err(g, "Error updating pmu boardobjgrp for therm_device. status - %d",
				status);
	}
	nvgpu_pmu_dbg(g, " Done");
	return status;
}

static s32 _therm_device_pmudatainit_device(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct therm_device *ptherm_device = NULL;
	struct therm_device_3x *ptherm_device_3x = NULL;
	struct nv_pmu_therm_therm_device_3x_boardobj_set *pset;

	nvgpu_pmu_dbg(g, "Thermal device PMU data init start - device type: 0x%x", obj->type);

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		nvgpu_err(g,
			"error updating pmu boardobjgrp for therm device 0x%x",
			status);
		status = -ENOMEM;
		goto done;
	}

	// Cast to get access to thermal device specific data
	ptherm_device = (struct therm_device *)obj;

	nvgpu_pmu_dbg(g, "Thermal device - type: 0x%x, numProv: %d",
		obj->type, ptherm_device->numProv);

	// Check if this is a 3X thermal device (types 0x14-0x1A)
	if (obj->type >= NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GTZ_V10 &&
	    obj->type <= NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_V10) {

		ptherm_device_3x = (struct therm_device_3x *)obj;
		pset = (struct nv_pmu_therm_therm_device_3x_boardobj_set *)pmu_obj;

		// Set common 3X device fields
		pset->devType = ptherm_device_3x->devType;
		pset->numProv = ptherm_device_3x->numProv;
		pset->provSupportMask = ptherm_device_3x->provSupportMask;

        // TODO: check if this calculation is correct because it has to left shift by 3 to get the correct value.
		pset->errorTemp = ptherm_device_3x->errorTemp<<3;

		// Handle device-specific data based on type
		switch (obj->type) {
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GTZ_V10:
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_STZ_V10:
		{
			struct THERM_DEVICE_GPU_TZ_V10 *tz = (struct THERM_DEVICE_GPU_TZ_V10 *)obj;
			struct nv_pmu_therm_therm_device_gpu_tz_v10_boardobj_set *pTzSet =
				(struct nv_pmu_therm_therm_device_gpu_tz_v10_boardobj_set *)pmu_obj;

			pTzSet->hwIdx = tz->hwIdx;

			break;
		}
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_TSENSE_V10:
		{
			struct THERM_DEVICE_GPU_TSENSE_V10 *tsense = (struct THERM_DEVICE_GPU_TSENSE_V10 *)obj;
			struct nv_pmu_therm_therm_device_gpu_tsense_v10_boardobj_set *pTsenseSet =
				(struct nv_pmu_therm_therm_device_gpu_tsense_v10_boardobj_set *)pmu_obj;

			pTsenseSet->location = tsense->location;

			break;
		}
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10:
		{
			struct THERM_DEVICE_GPU_MINI_TSENSE_V10 *miniTsense = (struct THERM_DEVICE_GPU_MINI_TSENSE_V10 *)obj;
			struct nv_pmu_therm_therm_device_gpu_mini_tsense_v10_boardobj_set *pMiniTsenseSet =
				(struct nv_pmu_therm_therm_device_gpu_mini_tsense_v10_boardobj_set *)pmu_obj;

			pMiniTsenseSet->location = miniTsense->location;

			break;
		}
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_V10:
		{
			struct THERM_DEVICE_MEM_V10 *mem = (struct THERM_DEVICE_MEM_V10 *)obj;
			struct nv_pmu_therm_therm_device_mem_v10_boardobj_set *pMemSet =
				(struct nv_pmu_therm_therm_device_mem_v10_boardobj_set *)pmu_obj;

			pMemSet->sensorType = mem->sensorType;
			pMemSet->refreshPeriodLimitMs = mem->refreshPeriodLimitMs;
			break;
		}
		default:
			nvgpu_pmu_dbg(g, "Unknown 3X thermal device type: 0x%x", obj->type);
			break;
		}
	} else {
		// Legacy thermal device handling
		nvgpu_pmu_dbg(g, "Legacy thermal device type: 0x%x", obj->type);
	}

done:
	return status;
}

static int construct_therm_device(struct gk20a *g,
	struct pmu_board_obj *obj, void *pargs)
{
	return pmu_board_obj_construct_super(g, obj, pargs);
}

static int construct_therm_device_gpu(struct gk20a *g,
	struct pmu_board_obj *obj, void *pargs)
{
	return construct_therm_device(g, obj, pargs);
}

/*!
 * Static helper to get provider mapping specific info for Therm Device Table
 *
 * It return provider mapping specific info like number of provider, provider support mask etc
 *
 * @param[in]  pMapping             Pointer to provider mapping NV_VBIOS_THERM_DEVICE_2X_PROV_MAPPING
 * @param[in]  vbiosProvMappingId   Provider mapping ID NV_VBIOS_THERM_DEVICE_2X_ENTRY_<device type>_PROV_MAPPING_<xyz>
 * @param[out] pProvMappingId     Provider mapping ID NV2080_CTRL_THERMAL_THERM_DEVICE_<device type>_PROV_MAPPING_<xyz>
 * @param[out] pNumProv             Total number of providers supported by current provider mapping
 * @param[out] pProvSupportMask     Provider support mask for current provider mapping
 *
 * @return NV_OK    on success
 */
static s32 devinitThermDevice2xProviderMappingInfoGet
(
	NV_VBIOS_THERM_DEVICE_2X_PROV_MAPPING *pMapping,
	u8 vbiosProvMappingId,
	u8 *pProvMappingId,
	u8 *pNumProv,
	u32 *pProvSupportMask
)
{
	s32 status = 0;
	// Default initialization
	*pProvMappingId = NV2080_CTRL_THERMAL_THERM_DEVICE_PROV_MAPPING_INVALID;
	*pNumProv = NV2080_CTRL_THERMAL_THERM_DEVICE_PROV__NUM_PROV_INVALID;
	*pProvSupportMask = 0;
	// Ensure that numEntries is less than max supported value
	if (pMapping->numEntries > (u8)NV_VBIOS_THERM_DEVICE_2X_PROV_MAPPING_COUNT) {
		/* Set status instead of returning directly to comply with Rule 15.5 */
		status = -EINVAL;
	} else {
		/* Add memory barrier to prevent speculative execution attack */
		nvgpu_speculation_barrier();
		// Update the return values from mapping table
		*pProvMappingId = pMapping->list[vbiosProvMappingId].provMappingId;
		*pNumProv = pMapping->list[vbiosProvMappingId].numProv;
		*pProvSupportMask = pMapping->list[vbiosProvMappingId].provSupportMask;
	}
	return status;
}

static s32 devinitThermDevice2xHalInfoGet
(
    struct gk20a *g,
    u8    halId,
    u8    entryIdx,
    u8   *pDevType,
    bool *pbI2csExposure
)
{
    s32 status = 0;

    // Ensure that pRmDevType and pbI2csExposure are not NULL
	if (pDevType == NULL || pbI2csExposure == NULL) {
		status = -EINVAL;
		goto devinitThermDevice2xHalInfoGet_exit;
	}
    // Initialize pRmDevType and pbI2csExposure
    *pDevType     = NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_NOT_ASSIGNED;
    *pbI2csExposure = false;
    //
    // Sanity checks for HAL ID:
    // - Ensure that HAL ID is less than max supported HALs AND
    // - Ensure that HAL ID is correctly wired in NV_VBIOS_THERM_DEVICE_2X_HAL
    //
    if ((halId >= NV_VBIOS_THERM_DEVICE_2X_HAL__COUNT) ||
        (halId != thermDevice2xHals[halId].halId))
    {
        nvgpu_err(g, "Invalid Therm Device HAL - 0x%02x.\n", halId);
        status = -EINVAL;
        goto devinitThermDevice2xHalInfoGet_exit;
    }
    // Ensure that HAL entries are less than or equal to 2X_HAL_ENTRY_COUNT
    if (thermDevice2xHals[halId].numEntries > NV_VBIOS_THERM_DEVICE_2X_HAL_ENTRY_COUNT) {
        nvgpu_err(g, "Invalid Therm Device HAL - 0x%02x.\n", halId);
        status = -EINVAL;
        goto devinitThermDevice2xHalInfoGet_exit;
    }

    if (entryIdx < thermDevice2xHals[halId].numEntries)
    {

        if ((thermDevice2xHals[halId].deviceEntry[entryIdx].vbiosDevType != entryIdx) &&
            (thermDevice2xHals[halId].deviceEntry[entryIdx].vbiosDevType != NV_VBIOS_THERM_DEVICE_2X_HAL_DEV_TYPE_NOT_ASSIGNED)) {
            status = -EINVAL;
            goto devinitThermDevice2xHalInfoGet_exit;
        }

        *pDevType     = thermDevice2xHals[halId].deviceEntry[entryIdx].devType;
        *pbI2csExposure = thermDevice2xHals[halId].deviceEntry[entryIdx].bI2csExposure;

    } else {

    }
devinitThermDevice2xHalInfoGet_exit:

    return status;
}

/*!
 * Static helper performing conversion from VBIOS class to RM Class
 *
 * It converts NV_VBIOS_THERM_DEVICE_2X_ENTRY_CLASS_<xyz> enum to
 * NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_<xyz>.
 *
 * @param[in] vbiosClass NV_VBIOS_THERM_DEVICE_2X_ENTRY_CLASS_<xyz>
 *
 * @return NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_<xyz>
 */
static u8 devinitThermDevice2xClassConvertToInternal
(
    u8 vbiosClass
)
{
    u8 ClassId = NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_INVALID;
    switch (vbiosClass)
    {
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_CLASS_GPU_GTZ_V10:
        {
            ClassId = NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GTZ_V10;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_CLASS_GPU_STZ_V10:
        {
            ClassId = NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_STZ_V10;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_CLASS_GPU_TSENSE_V10:
        {
            ClassId = NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_TSENSE_V10;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_CLASS_GPU_MINI_TSENSE_V10:
        {
            ClassId = NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_CLASS_MEM_V10:
        {
            ClassId = NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_V10;
            break;
        }
        default:
        {
            ClassId = NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_INVALID;
            break;
        }
    }
    return ClassId;
}

static u8 devinitThermDevice2xGpuTzModeConvertToInternal
(
    u8 vbiosTzMode
)
{
    u8 TzMode = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_INVALID;
    switch (vbiosTzMode)
    {
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_FUNCTIONAL_MODE_MAX:
        {
            TzMode = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_MAX;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_FUNCTIONAL_MODE_AVG:
        {
            TzMode = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_AVG;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_FUNCTIONAL_MODE_MIN:
        {
            TzMode = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_MIN;
            break;
        }
        default:
        {
			TzMode = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_INVALID;
            break;
        }
    }
    return TzMode;
}

static u32 devinitThermDevice2xGpuTzSensorMaskConvertToInternal
(
    struct gk20a *g,
    u8 entryIdx,
    u32 vbiosSensorMask
)
{
    u32 vbiosSensorIdx = 0;
    u32 rmSensorIdx = 0;
    u32 rmSensorMask = 0;
    /* Process each set bit in the VBIOS sensor mask */
    for_each_set_bit(vbiosSensorIdx, (unsigned long *)&vbiosSensorMask, 32) {
        switch (vbiosSensorIdx) {
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_HBI_0:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_0;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_HBI_1:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_1;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_NVL:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_NVL;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_0:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_0;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_1:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_1;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_2:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_2;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_3:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_3;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_4:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_4;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_5:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_5;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_6:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_6;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_7:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_7;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_8:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_8;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_9:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_9;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_10:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_10;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_GPC_11:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_11;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_SW:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_SW;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_SYS:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_SYS;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_SOC_0:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_SOC_0;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_HBI_2:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_2;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_HBI_3:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_3;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_HBI_4:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_4;
            break;
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_TZ_SENSOR_IDX_HBI_5:
            rmSensorIdx = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_5;
            break;
        default:
            nvgpu_err(g, "Invalid GPU TZ sensor index 0x%x for entry %d. SensorMask 0x%x",
                     vbiosSensorIdx, entryIdx, vbiosSensorMask);
            rmSensorMask = 0;
            goto devinitThermDevice2xGpuTzSensorMaskConvertToInternal_exit;
        }
        rmSensorMask |= BIT(rmSensorIdx);
    }
devinitThermDevice2xGpuTzSensorMaskConvertToInternal_exit:
    return rmSensorMask;
}

static u8 devinitThermDevice2xGpuTsenseLocationConvertToInternal
(
    u8 vbiosLocation
)
{
    u8 TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_INVALID;
    switch (vbiosLocation)
    {
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION_HBI_0:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_HBI_0;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION_HBI_1:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_HBI_1;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION_NVL_0:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_NVL_0;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION_GPC_0:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_0;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION_GPC_1:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_1;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION_GPC_2:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_2;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION_GPC_3:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_3;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION_SYS:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_SYS;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION_SOC_0:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_SOC_0;
            break;
        }
        default:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_INVALID;
            break;
        }
    }
    return TsenseLocation;
}

static u8 devinitThermDevice2xGpuMiniTsenseLocationConvertToInternal
(
    u8 vbiosLocation
)
{
    u8 TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_LOC_INVALID;
    switch (vbiosLocation)
    {
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_MINI_TSENSE_LOCATION_SCI_0:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_LOC_SCI_0;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_MINI_TSENSE_LOCATION_SCI_1:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_LOC_SCI_1;
            break;
        }
        default:
        {
            TsenseLocation = NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_LOC_INVALID;
            break;
        }
    }
    return TsenseLocation;
}

static u8 devinitThermDevice2xMemSensorTypeConvertToInternal
(
    u8 vbiosSensorType
)
{
    u8 SensorType = NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_INVALID;
    switch (vbiosSensorType)
    {
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_MEM_SENSOR_TYPE_HBM3_SW_V10:
        {
            SensorType = NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_HBM3_SW_V10;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_MEM_SENSOR_TYPE_GDDRX_SW_V10:
        {
            SensorType = NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_GDDRX_SW_V10;
            break;
        }
        case NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_MEM_SENSOR_TYPE_HBM4_SW_V10:
        {
            SensorType = NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_HBM4_SW_V10;
            break;
        }
        default:
        {
            SensorType = NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_INVALID;
            break;
        }
    }
    return SensorType;
}

static struct pmu_board_obj *therm_device_construct(struct gk20a *g, void* pargs)
{
	struct pmu_board_obj *pBoardObj = NULL;
	struct therm_device *ptherm_device = NULL;
	u8 objType;
	s32 status = 0;
	nvgpu_pmu_dbg(g, " ");
	if (pargs == NULL) {
		return NULL;
	}
	objType = pmu_board_obj_get_type(pargs);
	switch(objType)
	{
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GTZ_V10:
		{
			status = thermDeviceIfaceModel10Construct_GPU_GTZ_V10(g, &pBoardObj, (u32)sizeof(struct THERM_DEVICE_GPU_GTZ_V10),  pargs);
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_STZ_V10:
		{

			status = thermDeviceIfaceModel10Construct_GPU_STZ_V10(g, &pBoardObj,
                          (u32)sizeof(struct THERM_DEVICE_GPU_STZ_V10), pargs);
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_TSENSE_V10:
		{
			status = thermDeviceIfaceModel10Construct_GPU_TSENSE_V10(g, &pBoardObj,
                          (u32)sizeof(struct THERM_DEVICE_GPU_TSENSE_V10), pargs);
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10:
		{
			status = thermDeviceIfaceModel10Construct_GPU_MINI_TSENSE_V10(g, &pBoardObj,
                          (u32)sizeof(struct THERM_DEVICE_GPU_MINI_TSENSE_V10), pargs);
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_V10:
		{
			status = thermDeviceIfaceModel10Construct_MEM_V10(g, &pBoardObj,
                          (u32)sizeof(struct THERM_DEVICE_MEM_V10), pargs);
			break;
		}
		default:
		{
			ptherm_device = nvgpu_kzalloc(g, sizeof(struct therm_device));
			if (ptherm_device == NULL) {
				nvgpu_err(g, "[%s]:%d. - ptherm_device is NULL and allocation failed.", __func__, __LINE__);
				status = -ENOMEM;
				break;
			}
			pBoardObj = (struct pmu_board_obj *)(void *)ptherm_device;
			status = construct_therm_device_gpu(g, pBoardObj, pargs);
			break;
		}
	}
	if (status != 0) {
		nvgpu_err(g,
		"Could not allocate memory for therm_device");
		nvgpu_kfree(g, pBoardObj);
		pBoardObj = NULL;
	}

    pBoardObj->pmudatainit = _therm_device_pmudatainit_device;

	return pBoardObj;
}

static s32 devinit_get_therm_device_table(struct gk20a *g,
				struct therm_devices *pthermdeviceobjs)
{
	int status = 0;
	struct THERM_DEVICES_3X *pDevices3x = NULL;
	const u8 *therm_device_table_ptr = NULL;
	const u8 *curr_therm_device_table_ptr = NULL;
	struct pmu_board_obj *obj_tmp;
	struct therm_device_2x_header therm_device_table_header = { 0 };
	const struct therm_device_2x_entry *therm_device_table_entry = NULL;
	NV_VBIOS_THERM_DEVICE_2X_PROV_MAPPING *pMapping   = NULL;
	u32 index;
	u8 class_id = 0;
	u32 error_temp = 0;
	u32 param0 = 0;
	u32 param1 = 0;
	u32 invalidEntryCnt = 0;
	union {
		struct pmu_board_obj obj;
		struct therm_device therm_device;
		struct therm_device_3x v3x;
		struct THERM_DEVICE_GPU_GTZ_V10 gtz;
		struct THERM_DEVICE_GPU_STZ_V10 stz;
		struct THERM_DEVICE_GPU_TSENSE_V10 tsense;
		struct THERM_DEVICE_GPU_MINI_TSENSE_V10 miniTsense;
		struct THERM_DEVICE_MEM_V10 mem;
	} therm_device_data;
	// GPU_MINI_TSENSE_V10 provider mapping (moved to block scope)
	static NV_VBIOS_THERM_DEVICE_2X_PROV_MAPPING provMappingGpuMiniTsense =
	{
	    NV_VBIOS_THERM_DEVICE_2X_ENTRY_GPU_MINI_TSENSE_V10_PROV_MAPPING__COUNT,
	    {
	        // Data related to _DEFAULT provider mapping
	        {
	            NV_VBIOS_PROV_MAPPING          (GPU_MINI_TSENSE_V10, DEFAULT),
	            NV_PROV_MAPPING             (GPU_MINI_TSENSE_V10, DEFAULT),
	            NV_PROV_MAPPING_NUM_PROV    (GPU_MINI_TSENSE_V10, DEFAULT),
	            (u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_MINI_TSENSE_V10, DEFAULT)) - 1U),
	        },
	    }
	};
	static NV_VBIOS_THERM_DEVICE_2X_PROV_MAPPING provMappingGpuTz =
	{
		NV_VBIOS_THERM_DEVICE_2X_ENTRY_GPU_TZ_V10_PROV_MAPPING__COUNT,
		{
			// Data related to _DEFAULT provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TZ_V10, DEFAULT),
				NV_PROV_MAPPING             (GPU_TZ_V10, DEFAULT),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TZ_V10, DEFAULT),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TZ_V10, DEFAULT)) - 1U),
			},
		}
	};
	static NV_VBIOS_THERM_DEVICE_2X_PROV_MAPPING provMappingGpuTsense =
	{
		NV_VBIOS_THERM_DEVICE_2X_ENTRY_GPU_TSENSE_V10_PROV_MAPPING__COUNT,
		{
			// Data related to _9_BJTS provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TSENSE_V10, 9_BJTS),
				NV_PROV_MAPPING             (GPU_TSENSE_V10, 9_BJTS),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TSENSE_V10, 9_BJTS),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TSENSE_V10, 9_BJTS)) - 1U),
			},
			// Data related to _2_BJTS provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TSENSE_V10, 2_BJTS),
				NV_PROV_MAPPING             (GPU_TSENSE_V10, 2_BJTS),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TSENSE_V10, 2_BJTS),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TSENSE_V10, 2_BJTS)) - 1U),
			},
			// Data related to _3_BJTS provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TSENSE_V10, 3_BJTS),
				NV_PROV_MAPPING             (GPU_TSENSE_V10, 3_BJTS),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TSENSE_V10, 3_BJTS),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TSENSE_V10, 3_BJTS)) - 1U),
			},
			// Data related to _1_BJT provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TSENSE_V10, 1_BJT),
				NV_PROV_MAPPING             (GPU_TSENSE_V10, 1_BJT),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TSENSE_V10, 1_BJT),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TSENSE_V10, 1_BJT)) - 1U),
			},
			// Data related to _4_BJTS provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TSENSE_V10, 4_BJTS),
				NV_PROV_MAPPING             (GPU_TSENSE_V10, 4_BJTS),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TSENSE_V10, 4_BJTS),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TSENSE_V10, 4_BJTS)) - 1U),
			},
			// Data related to _5_BJTS provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TSENSE_V10, 5_BJTS),
				NV_PROV_MAPPING             (GPU_TSENSE_V10, 5_BJTS),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TSENSE_V10, 5_BJTS),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TSENSE_V10, 5_BJTS)) - 1U),
			},
			// Data related to _6_BJTS provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TSENSE_V10, 6_BJTS),
				NV_PROV_MAPPING             (GPU_TSENSE_V10, 6_BJTS),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TSENSE_V10, 6_BJTS),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TSENSE_V10, 6_BJTS)) - 1U),
			},
			// Data related to _7_BJTS provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TSENSE_V10, 7_BJTS),
				NV_PROV_MAPPING             (GPU_TSENSE_V10, 7_BJTS),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TSENSE_V10, 7_BJTS),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TSENSE_V10, 7_BJTS)) - 1U),
			},
			// Data related to _8_BJTS provider mapping
			{
				NV_VBIOS_PROV_MAPPING          (GPU_TSENSE_V10, 8_BJTS),
				NV_PROV_MAPPING             (GPU_TSENSE_V10, 8_BJTS),
				NV_PROV_MAPPING_NUM_PROV    (GPU_TSENSE_V10, 8_BJTS),
				(u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(GPU_TSENSE_V10, 8_BJTS)) - 1U),
			},
		}
	};
    // MEM_V10 provider mapping
    static NV_VBIOS_THERM_DEVICE_2X_PROV_MAPPING provMappingMem =
    {
        NV_VBIOS_THERM_DEVICE_2X_ENTRY_MEM_V10_PROV_MAPPING__COUNT,
        {
            // Data related to _8_SENSORS provider mapping
            {
                NV_VBIOS_PROV_MAPPING          (MEM_V10, 8_SENSORS),
                NV_PROV_MAPPING             (MEM_V10, 8_SENSORS),
                NV_PROV_MAPPING_NUM_PROV    (MEM_V10, 8_SENSORS),
                (u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(MEM_V10, 8_SENSORS)) - 1U),
            },
            // Data related to _4_SENSORS provider mapping
            {
                NV_VBIOS_PROV_MAPPING          (MEM_V10, 4_SENSORS),
                NV_PROV_MAPPING             (MEM_V10, 4_SENSORS),
                NV_PROV_MAPPING_NUM_PROV    (MEM_V10, 4_SENSORS),
                (u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(MEM_V10, 4_SENSORS)) - 1U),
            },
            // Data related to _1_SENSOR provider mapping
            {
                NV_VBIOS_PROV_MAPPING          (MEM_V10, 1_SENSOR),
                NV_PROV_MAPPING             (MEM_V10, 1_SENSOR),
                NV_PROV_MAPPING_NUM_PROV    (MEM_V10, 1_SENSOR),
                (u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(MEM_V10, 1_SENSOR)) - 1U),
            },
            // Data related to _2_SENSORS provider mapping
            {
                NV_VBIOS_PROV_MAPPING          (MEM_V10, 2_SENSORS),
                NV_PROV_MAPPING             (MEM_V10, 2_SENSORS),
                NV_PROV_MAPPING_NUM_PROV    (MEM_V10, 2_SENSORS),
                (u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(MEM_V10, 2_SENSORS)) - 1U),
            },
            // Data related to _3_SENSORS provider mapping
            {
                NV_VBIOS_PROV_MAPPING          (MEM_V10, 3_SENSORS),
                NV_PROV_MAPPING             (MEM_V10, 3_SENSORS),
                NV_PROV_MAPPING_NUM_PROV    (MEM_V10, 3_SENSORS),
                (u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(MEM_V10, 3_SENSORS)) - 1U),
            },
            // Data related to _5_SENSORS provider mapping
            {
                NV_VBIOS_PROV_MAPPING          (MEM_V10, 5_SENSORS),
                NV_PROV_MAPPING             (MEM_V10, 5_SENSORS),
                NV_PROV_MAPPING_NUM_PROV    (MEM_V10, 5_SENSORS),
                (u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(MEM_V10, 5_SENSORS)) - 1U),
            },
            // Data related to _6_SENSORS provider mapping
            {
                NV_VBIOS_PROV_MAPPING          (MEM_V10, 6_SENSORS),
                NV_PROV_MAPPING             (MEM_V10, 6_SENSORS),
                NV_PROV_MAPPING_NUM_PROV    (MEM_V10, 6_SENSORS),
                (u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(MEM_V10, 6_SENSORS)) - 1U),
            },
            // Data related to _7_SENSORS provider mapping
            {
                NV_VBIOS_PROV_MAPPING          (MEM_V10, 7_SENSORS),
                NV_PROV_MAPPING             (MEM_V10, 7_SENSORS),
                NV_PROV_MAPPING_NUM_PROV    (MEM_V10, 7_SENSORS),
                (u32)(BIT32(NV_PROV_MAPPING_NUM_PROV(MEM_V10, 7_SENSORS))
                 - 1U),
            },
        }
    };
	nvgpu_pmu_dbg(g, " ");
	therm_device_table_ptr = (const u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_PERF_TOKEN),
			THERMAL_DEVICE_TABLE);
	if (therm_device_table_ptr == NULL) {
		status = -EINVAL;
		goto done;
	}
	(void)memcpy((u8 *)&therm_device_table_header, therm_device_table_ptr,
		VBIOS_THERM_DEVICE_2X_HEADER_SIZE_04);
	if (therm_device_table_header.version !=
			VBIOS_THERM_DEVICE_VERSION_2X) {
		status = -EINVAL;
		goto done;
	}
	if (therm_device_table_header.header_size <
			VBIOS_THERM_DEVICE_2X_HEADER_SIZE_04) {
		status = -EINVAL;
		goto done;
	}
    pDevices3x = &pthermdeviceobjs->devices3x;
	pDevices3x->typeHAL = therm_device_table_header.type_hal;
    // TODO: remove below hardcode value once implement API to use therm_device_table_header flags as detection.
	pDevices3x->bHwTempSim = 0x1;
	pDevices3x->bWatchdogTimer = 0x1;
	pDevices3x->watchdogTimerMs = therm_device_table_header.watchdog_timer_ms;
	pDevices3x->consistencyCheckerThreshold = (s32)therm_device_table_header.consistency_checker_threshold;
	curr_therm_device_table_ptr = (therm_device_table_ptr +
		VBIOS_THERM_DEVICE_2X_HEADER_SIZE_04);
	for (index = 0; index < therm_device_table_header.num_table_entries;
		index++) {
		therm_device_table_entry = (const struct therm_device_2x_entry *)
			(curr_therm_device_table_ptr +
				(therm_device_table_header.table_entry_size * index));
		(void)memset(&therm_device_data, 0, sizeof(therm_device_data));
		error_temp = therm_device_table_entry->error_temp;
		param0 = therm_device_table_entry->param0;
		param1 = therm_device_table_entry->param1;
        pMapping = NULL;

        class_id = devinitThermDevice2xClassConvertToInternal(therm_device_table_entry->class_id);
        if (class_id == NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_INVALID) {
            // Invalid entry, skip, but does not break the loop
			invalidEntryCnt++;
            continue;
        }

        switch (class_id) {
            case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GTZ_V10:
            {
                u8 mode;
                u32 temp_update_period_gtz;
                u32 sensor_mask;
                therm_device_data.gtz.super.hwIdx = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_HW_INDEX); // Extract bits 0-4 from param0
                mode = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_FUNCTIONAL_MODE); // Extract bits 5-7 from param0
                therm_device_data.gtz.super.mode = devinitThermDevice2xGpuTzModeConvertToInternal(mode);
                therm_device_data.gtz.super.bHsOffset = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_HOTSPOT_OFFSET); // Extract bits 8 from param0
                therm_device_data.gtz.super.bMunged = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_MUNGED); // Extract bit 9 from param0
                therm_device_data.gtz.super.iirFilter.bEnabled = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_IIR_FILTER); // Extract bit 10 from param0
                therm_device_data.gtz.super.iirFilter.length = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_IIR_LENGTH); // Extract bits 14:11 from param0
                // Extract update period from param0 and check for overflow
                temp_update_period_gtz = BIOS_GET_FIELD(u32, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_IIR_PERIOD_UTILSCLK); // Extract bits 30:15 from param0
                /* MISRA C-2012 Rule 10.3 & CERT INT31-C deviated: Justified cast from u32 to u16.
                 * The value extracted using the VBIOS mask and shift is checked against U16_MAX
                 * before casting to prevent overflow for GPU_TZ_IIR_PERIOD_UTILSCLK. */
                if (temp_update_period_gtz > U16_MAX) {
                    nvgpu_err(g, "GPU GTZ IIR update period value 0x%x exceeds U16_MAX", temp_update_period_gtz);
                    temp_update_period_gtz = U16_MAX;
                }
                therm_device_data.gtz.super.iirFilter.updatePeriodUtilsClock = (u16)temp_update_period_gtz;
                // Use NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_GTZ_SENSOR_IDX mask with explicit unsigned type
                sensor_mask = (param1 & ((u32)0xFFFFFFFFU)) >> NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_GTZ_SENSOR_IDX_SHIFT;
                therm_device_data.gtz.super.sensorMask = devinitThermDevice2xGpuTzSensorMaskConvertToInternal(g, (u8)index, sensor_mask);
                pMapping = &provMappingGpuTz;
                break;
            }
            case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_STZ_V10:
            {
                u8 mode;
                u32 temp_update_period;
                u32 sensor_mask;
                // Using BIOS_GET_FIELD with MASK and SHIFT macros for STZ parameters
                therm_device_data.stz.super.hwIdx = BIOS_GET_FIELD(u8, param0,
                                                                NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_HW_INDEX);
                mode = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_FUNCTIONAL_MODE); // Extract bits 5-7 from param0
                therm_device_data.stz.super.mode = devinitThermDevice2xGpuTzModeConvertToInternal(mode);
                therm_device_data.stz.super.bHsOffset = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_HOTSPOT_OFFSET); // Extract bits 8 from param0
                therm_device_data.stz.super.bMunged = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_MUNGED); // Extract bit 9 from param0
                therm_device_data.stz.super.iirFilter.bEnabled = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_IIR_FILTER); // Extract bit 10 from param0
                therm_device_data.stz.super.iirFilter.length = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_IIR_LENGTH); // Extract bits 14:11 from param0
                // Extract update period from param0 and check for overflow
                temp_update_period = BIOS_GET_FIELD(u32, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TZ_IIR_PERIOD_UTILSCLK); // Extract bits 30:15 from param0
                /* MISRA C-2012 Rule 10.3 & CERT INT31-C deviated: Justified cast from u32 to u16.
                 * The value extracted using the VBIOS mask and shift is checked against U16_MAX
                 * before casting to prevent overflow for GPU_TZ_IIR_PERIOD_UTILSCLK. */
                if (temp_update_period > U16_MAX) {
                    nvgpu_err(g, "GPU TZ IIR update period value 0x%x exceeds U16_MAX", temp_update_period);
                    temp_update_period = U16_MAX;
                }
                therm_device_data.stz.super.iirFilter.updatePeriodUtilsClock = (u16)temp_update_period;
                // Use NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_STZ_SENSOR_IDX mask with explicit unsigned type
                sensor_mask = (param1 & ((u32)0x3FFFFU)) >> NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM1_GPU_STZ_SENSOR_IDX_SHIFT;
                therm_device_data.stz.super.sensorMask = devinitThermDevice2xGpuTzSensorMaskConvertToInternal(g, (u8)index, sensor_mask);

                // Skip entries with invalid sensor masks for thermal zones
                if (therm_device_data.stz.super.sensorMask == 0) {
                    nvgpu_err(g, "Skipping thermal device entry %d with invalid sensor mask (param1=0x%x)", index, param1);
                    invalidEntryCnt++;
                    continue;
                }

                pMapping = &provMappingGpuTz;
                break;
            }
            case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_TSENSE_V10:
            {
                u8 location = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_TSENSE_LOCATION); // Extract location from param0
                therm_device_data.tsense.location = devinitThermDevice2xGpuTsenseLocationConvertToInternal (location);
                pMapping = &provMappingGpuTsense;
                break;
            }
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10:
			{
                u8 location = BIOS_GET_FIELD(u8, param0, NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_GPU_MINI_TSENSE_LOCATION); // Extract location from param0
                therm_device_data.miniTsense.location = devinitThermDevice2xGpuMiniTsenseLocationConvertToInternal(location);
			    pMapping = &provMappingGpuMiniTsense;
			    break;
            }
		case NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_V10:
			{
                const u32 mask_sensor_type = (u32)NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_MEM_SENSOR_TYPE_MASK;
                const u32 shift_sensor_type = (u32)NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_MEM_SENSOR_TYPE_SHIFT;
                const u32 mask_refresh = (u32)NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_MEM_REFRESH_PERIOD_LIMIT_MS_MASK;
                const u32 shift_refresh = (u32)NV_VBIOS_THERM_DEVICE_2X_ENTRY_PARAM0_MEM_REFRESH_PERIOD_LIMIT_MS_SHIFT;
                // Extract sensor type from param0
                u32 temp_sensor_type = (param0 & mask_sensor_type) >> shift_sensor_type;
                /* MISRA C-2012 Rule 10.3 deviated: Justified cast from u32 to u8.
                 * The value extracted using the VBIOS mask and shift is known to fit
                 * within a u8 for this specific field (MEM_SENSOR_TYPE). */
                u8 sensor_type = (u8)temp_sensor_type;
                // Extract refresh period limit from param0
                u32 temp_refresh = (param0 & mask_refresh) >> shift_refresh;
                therm_device_data.mem.sensorType = devinitThermDevice2xMemSensorTypeConvertToInternal(sensor_type);
                /* MISRA C-2012 Rule 10.3 & CERT INT31-C deviated: Justified cast from u32 to u16.
                 * The value extracted using the VBIOS mask and shift is known to fit
                 * within a u16 for this specific field (MEM_REFRESH_PERIOD_LIMIT_MS). */
                if (temp_refresh > U16_MAX) {
                    nvgpu_err(g, "Refresh period limit value 0x%x exceeds U16_MAX", temp_refresh);
                    temp_refresh = U16_MAX;
                }
                therm_device_data.mem.refreshPeriodLimitMs = (u16)temp_refresh;
    			pMapping = &provMappingMem;
                break;
            }
            default:
            {
            nvgpu_err(g,
				"Unknown thermal device class index - %x, class - %x",
				index, class_id);
            invalidEntryCnt++;
                break;
            }
        }
		therm_device_data.obj.type = class_id;
		therm_device_data.obj.idx = (u8)index;
		therm_device_data.v3x.errorTemp = (s32)error_temp;
        status = devinitThermDevice2xHalInfoGet(g, pDevices3x->typeHAL, index, &therm_device_data.v3x.devType,
                                                &therm_device_data.v3x.bI2csExposure);

        if (status != 0) {
            nvgpu_err(g, "devinitThermDevice2xHalInfoGet() failed for index %d", index);
            goto done;
        }

        status = devinitThermDevice2xProviderMappingInfoGet(pMapping,
                                                            therm_device_table_entry->provider_mapping,
                                                            &therm_device_data.v3x.provMapping,
                                                            &therm_device_data.v3x.numProv,
                                                            &therm_device_data.v3x.provSupportMask);

        if (status != 0) {
            nvgpu_err(g, "devinitThermDevice2xProviderMappingInfoGet() failed for index %d", index);
            goto done;
        }

        if (pMapping == NULL) {
            nvgpu_err(g, "Provider mapping is NULL for class_id 0x%x", class_id);
            goto done;
        }

        // Initialize board object data
        therm_device_data.obj.type = class_id;
        therm_device_data.obj.idx = index;

        therm_device_data.v3x.errorTemp = therm_device_table_entry->error_temp;

        obj_tmp = therm_device_construct(g, (void *)&therm_device_data);

        if (obj_tmp == NULL) {
            nvgpu_err(g, "unable to create thermal device for %d type %d",
                      index, therm_device_data.obj.type);
            status = -ENOMEM;
            goto done;
        }

        status = boardobjgrp_objinsert(&pthermdeviceobjs->super.super,
                                       (struct pmu_board_obj *)(void *)obj_tmp, (u8)index);

        if (status != 0) {
            nvgpu_err(g, "unable to insert thermal device boardobj for %d", index);
            status = -ENOMEM;
            goto done;
        }
	}
done:
	nvgpu_pmu_dbg(g, " done status %x", status);
	return status;
}

static s32 _therm_device_pmudata_instget(struct gk20a *g,
			struct nv_pmu_boardobjgrp *pmuboardobjgrp,
			struct nv_pmu_boardobj **pmu_obj,
			u16 idx)
{
	struct nv_pmu_therm_therm_device_boardobj_grp_set *pgrp_set =
		(struct nv_pmu_therm_therm_device_boardobj_grp_set *)
		pmuboardobjgrp;
	nvgpu_pmu_dbg(g, " ");
	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
			pgrp_set->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		return -EINVAL;
	}
	*pmu_obj = (struct nv_pmu_boardobj *)(void *)
		&pgrp_set->objects[idx].data;
	nvgpu_pmu_dbg(g, " Done");
	return 0;
}

static s32 therm_device_init_objs(struct gk20a *g)
{
	void *temp_ptr = NULL;
	/* Check if therm_deviceobjs is already initialized */
	if(g->pmu->therm_pmu->therm_deviceobjs != NULL) {
		nvgpu_err(g, "therm_deviceobjs already initialized");
		return 0;
	}
	/* Allocate memory for therm_deviceobjs */
	temp_ptr = nvgpu_kzalloc_impl(g, sizeof(struct therm_devices), NVGPU_GET_IP);
	if(temp_ptr == NULL) {
		nvgpu_err(g, "error allocating memory for therm_deviceobjs");
		return -ENOMEM;
	}
	/* Assign the allocated memory to therm_deviceobjs */
	g->pmu->therm_pmu->therm_deviceobjs = temp_ptr;
	return 0;
}

s32 therm_device_sw_setup(struct gk20a *g)
{
	s32 status = 0;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct therm_devices *pthermdeviceobjs = NULL;
	struct boardobjgrp_e32 *pboardobjgrp_e32 = NULL;

	//nvgpu_pmu_dbg(g, " ");

	/* Check if therm_deviceobjs is initialized */
	if (g->pmu->therm_pmu->therm_deviceobjs == NULL) {
		nvgpu_err(g, "therm_deviceobjs not initialized, re-allocate memory for therm_deviceobjs");
		status = therm_device_init_objs(g);
		if (status != 0) {
			nvgpu_err(g, "error initializing therm_deviceobjs");
			status = -EINVAL;
			goto done;
		}
	}

	/* If already constructed, do not re-construct (suspend/resume pattern) */
	pboardobjgrp = &g->pmu->therm_pmu->therm_deviceobjs->super.super;
	if (pboardobjgrp->bconstructed) {
		nvgpu_pmu_dbg(g, "therm device boardobjgrp already constructed, skipping reinit");
		return 0;
	}

	nvgpu_pmu_dbg(g, "Constructing therm device boardobjgrp for first time");

	/* Initialize board object group mask */
	pboardobjgrp_e32 = &g->pmu->therm_pmu->therm_deviceobjs->super;
	status = nvgpu_boardobjgrpmask_init(&(&pboardobjgrp_e32->mask)->super, CTRL_BOARDOBJGRP_E32_MAX_OBJECTS, NULL);
	if (status != 0) {
		nvgpu_err(g, "error initializing boardobjgrp mask");
		status = -EINVAL;
		goto done;
	}

	/* Construct the board object group */
	status = nvgpu_boardobjgrp_construct_e32(g, pboardobjgrp_e32);
	if (status != 0) {
		nvgpu_err(g, "error creating boardobjgrp for therm devices, status - 0x%x",
		         status);
		status = -EINVAL;
		goto done;
	}

	pboardobjgrp = &g->pmu->therm_pmu->therm_deviceobjs->super.super;
	pthermdeviceobjs = g->pmu->therm_pmu->therm_deviceobjs;

	/* Set the thermal devices class type to 3X */
	pboardobjgrp->classType = NV2080_CTRL_THERMAL_THERM_CHANNELS_CLASS_TYPE_3X;

	/* Override the Interfaces */
	pboardobjgrp->pmudatainstget = _therm_device_pmudata_instget;
    pboardobjgrp->pmudatainit = therm_device_pmudatainit;
	// Initialize the thermal device table based on therm device objects
	status = devinit_get_therm_device_table(g, pthermdeviceobjs);
	if (status != 0) {
		goto done;
	}
	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, THERM, THERM_DEVICE);
	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			therm, THERM, therm_device, THERM_DEVICE);
	if (status != 0) {
		nvgpu_err(g,
			  "error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
			  status);
		goto done;
	}
done:
	nvgpu_pmu_dbg(g, " done status %x", status);
	return status;
}

s32 therm_device_pmu_setup(struct gk20a *g)
{
	int status = 0;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_pmu_dbg(g, " ");
	if (g == NULL) {
		return -EINVAL;
	}
	if (g->pmu == NULL || g->pmu->therm_pmu == NULL) {
		nvgpu_err(g, "PMU or therm_pmu not initialized");
		return -EINVAL;
	}

	if (!BOARDOBJGRP_IS_EMPTY(
			&g->pmu->therm_pmu->therm_deviceobjs->super.super)) {
		pboardobjgrp = &g->pmu->therm_pmu->therm_deviceobjs->super.super;
		status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);
		if (status != 0) {
			nvgpu_err(g, "error initializing PMU for therm devices");
			goto exit;
		}
	}
exit:
	return status;
}
