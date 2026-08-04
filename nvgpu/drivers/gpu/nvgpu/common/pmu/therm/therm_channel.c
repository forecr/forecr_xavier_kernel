// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/string.h>
#include <nvgpu/pmu/therm.h>
#include <nvgpu/types.h>
#include "therm_dev.h"
#include "therm_channel.h"
#include "ucode_therm_inf.h"
#include "thrm.h"
#include "therm_channel_provider_v10.h"
#include "therm_channel_debug_v10.h"
#include "therm_channel_aggregation_v10.h"
#include "therm_channel_estimation_v10.h"

/*
 * Define a struct to hold different thermal channel types
 * This avoids using anonymous unions, which violate MISRA C-2012 Rule 19.2
 */
struct therm_channel_data_types {
	struct pmu_board_obj obj;
	struct therm_channel channel;
	struct therm_channel_3x v3x;
	struct therm_channel_provider_v10 provider;
	struct therm_channel_debug_v10 debug;
	struct therm_channel_aggregation_v10 aggregation;
	struct therm_channel_estimation_v10 estimation;
};

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
static s32 devinitThermChannel2xHalInfoGet
(
    u8    halId,
    u8   *pChType,
    bool *pbPublicVisibility
)
{
    s32 status = 0;
    // Define the thermChannel2xHals array within the function scope
    NV_VBIOS_THERM_CHANNEL_2X_HAL thermChannel2xHals[] =
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
	// Verify that the halId matches what's expected in the array entry
    // This is a consistency check to ensure the array is correctly initialized
    if (halId != thermChannel2xHals[halId].halId) {
        status = -EINVAL;
    }

    // Ensure that pChType and pbPublicVisibility are not NULL
    if ((pChType == NULL) || (pbPublicVisibility == NULL)) {
        status = -ENOMEM;
    }else{
        *pChType             = NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_NOT_ASSIGNED;
        *pbPublicVisibility  = 0;
    }

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
	struct therm_channel_device *ptherm_channel;
	struct nv_pmu_therm_therm_channel_device_boardobj_set *pset;
	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		nvgpu_err(g,
			"error updating pmu boardobjgrp for therm channel 0x%x",
			status);
		status = -ENOMEM;
		goto done;
	}
	pchannel = (struct therm_channel *)(void *)obj;
	pset = (struct nv_pmu_therm_therm_channel_device_boardobj_set *)
			(void *)pmu_obj;
	ptherm_channel = (struct therm_channel_device *)(void *)obj;
	// TODO: Check for more necessary fields to be set
	pset->super.scaling = pchannel->scaling;
	pset->super.offsetSW = pchannel->offsetSW;
	pset->super.offsetHW = pchannel->offsetHW;
	pset->super.temp_min = pchannel->temp_min;
	pset->super.temp_max = pchannel->temp_max;
	pset->therm_dev_idx = ptherm_channel->therm_dev_idx;
	pset->therm_dev_prov_idx = ptherm_channel->therm_dev_prov_idx;
done:
	return status;
}

static struct therm_channel *construct_channel_device(struct gk20a *g,
			void *pargs, size_t pargs_size, u8 class_id)
{
	struct pmu_board_obj *obj = NULL;
	struct therm_channel *pchannel;
	struct therm_channel_device *pchannel_device;
	int status = 0;
	u16 scale_shift = 256U;
	struct therm_channel_device *therm_device = (struct therm_channel_device*)pargs;

	/* Validate required structures are initialized */
	if ((g == NULL) || (pargs == NULL)) {
		nvgpu_err(g, "Invalid NULL parameter: g or pargs");
		return NULL;
	}

	/* Ensure pargs_size is within u16 range */
	if (pargs_size > (size_t)U16_MAX) {
		nvgpu_err(g, "pargs_size too large for u16 conversion: %zu", pargs_size);
		return NULL;
	}

	if ((g->pmu == NULL) || (g->pmu->therm_pmu == NULL)) {
		nvgpu_err(g, "PMU or therm_pmu not initialized");
		return NULL;
	}

	if ((g->pmu->therm_pmu->therm_channelobjs == NULL)) {
		nvgpu_err(g, "therm_channelobjs not initialized");
		return NULL;
	}

	if ((g->pmu->therm_pmu->therm_channelobjs->super.super.ppobjects == NULL) ||
	    (g->pmu->therm_pmu->therm_channelobjs->super.super.objslots == 0U)) {
		nvgpu_err(g, "Invalid boardobjgrp configuration in therm_channelobjs");
		return NULL;
	}

	switch (class_id) {
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEVICE:
		{
			pchannel_device = nvgpu_kzalloc_impl(g, pargs_size, NVGPU_GET_IP);
			if (pchannel_device == NULL) {
				nvgpu_err(g, "Failed to allocate memory for therm_channel_device");
				return NULL;
			}
			obj = (struct pmu_board_obj *)(void *)pchannel_device;
			status = pmu_board_obj_construct_super(g, obj, pargs);
			if (status != 0) {
				nvgpu_err(g, "Failed to construct pmu_board_obj");
				return NULL;
			}
			/* Set Super class interfaces */
			obj->pmudatainit = _therm_channel_pmudatainit_device;
			pchannel = (struct therm_channel *)(void *)obj;
			pchannel_device = (struct therm_channel_device *)(void *)obj;
			g->ops.therm.get_internal_sensor_limits(&pchannel->temp_max,
				&pchannel->temp_min);
			pchannel->scaling = (s16)scale_shift;
			// TODO: Check whether offsetSW and offsetHW are needed
			pchannel->offsetSW = 0;
			pchannel->offsetHW = 0;
			pchannel_device->therm_dev_idx = therm_device->therm_dev_idx;
			pchannel_device->therm_dev_prov_idx = therm_device->therm_dev_prov_idx;
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_PROVIDER_V10:
		{
			status = thermChannelIfaceModel10Construct_PROVIDER_V10(g, &obj, (u16)pargs_size, pargs);
			obj->pmudatainit = _therm_channel_pmudatainit_device;
			if (status != 0) {
				nvgpu_err(g, " fail to construct NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_PROVIDER_V10  %d", status);
				return NULL;
			}
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10:
		{
			status = thermChannelIfaceModel10Construct_DEBUG_V10(g, &obj, (u16)pargs_size, pargs);
			if (status != 0) {
				nvgpu_err(g, " fail to construct NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10  %d", status);
				return NULL;
			}
			obj->pmudatainit = _therm_channel_pmudatainit_device;
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_AGGREGATION_V10:
		{
			status = thermChannelIfaceModel10Construct_AGGREGATION_V10(g, &obj, (u16)pargs_size, pargs);
			if (status != 0) {
				nvgpu_err(g, " fail to construct NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_AGGREGATION_V10  %d", status);
				return NULL;
			}
			break;
		}
		case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_ESTIMATION_V10:
		{
			status = thermChannelIfaceModel10Construct_ESTIMATION_V10(g, &obj, (u16)pargs_size, pargs);
			if (status != 0) {
				nvgpu_err(g, " fail to construct NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_ESTIMATION_V10  %d", status);
				return NULL;
			}
			break;
		}
		default:
		{
			nvgpu_err(g, "Invalid therm channel device type: %d", class_id);
			break;
		}
	}
	nvgpu_log_info(g, " Done");

	// Only perform the cast if the object is not NULL
	if (obj == NULL) {
		return NULL;
	}

	/* Deviation from MISRA C:2012 Rule 11.3, Required: Casting from struct pmu_board_obj * to struct therm_channel *. */
	/* Justification: This cast implements an inheritance pattern. 'obj' points to a struct (e.g., therm_channel_device, therm_channel_provider_v10) */
	/* whose first member is guaranteed by design to be compatible with struct pmu_board_obj. The cast allows accessing the derived struct's */
	/* members safely. This is a controlled cast essential for the object-oriented design used here. */
	return (struct therm_channel *)obj;
}

static int _therm_channel_pmudata_instget(struct gk20a *g,
			struct nv_pmu_boardobjgrp *pmuboardobjgrp,
			struct nv_pmu_boardobj **pmu_obj,
			u8 idx)
{
	struct nv_pmu_therm_therm_channel_boardobj_grp_set *pgrp_set =
		(struct nv_pmu_therm_therm_channel_boardobj_grp_set *)
		pmuboardobjgrp;
	nvgpu_log_info(g, " ");
	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
			pgrp_set->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		return -EINVAL;
	}
	*pmu_obj = (struct nv_pmu_boardobj *)
		&pgrp_set->objects[idx].data.obj;
	nvgpu_log_info(g, " Done");
	return 0;
}
static int therm_channel_pmustatus_instget(struct gk20a *g,
	void *pboardobjgrppmu, struct nv_pmu_boardobj_query
	**obj_pmu_status, u8 idx)
{
	struct nv_pmu_therm_therm_channel_boardobj_grp_get_status *pmu_status =
		(struct nv_pmu_therm_therm_channel_boardobj_grp_get_status *)
		(void *)pboardobjgrppmu;
	(void)g;
	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
		pmu_status->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		return -EINVAL;
	}
	*obj_pmu_status = (struct nv_pmu_boardobj_query *)
			&pmu_status->objects[idx].data.obj;
	return 0;
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
	struct therm_channels_3x *pChannels3x = NULL;
	u8 class_id = 0;
    u8 flags    = 0;
	u8 mode     = 0;
	u8  index  = 0;
	u32  param0 = 0;
    u32  param1 = 0;
    u32  param2 = 0;
	u32  obj_index = 0;
	u32  invalidEntryCnt = 0;
	size_t therm_channel_size = 0;
	bool skip_entry = false;
	struct therm_channel_data_types *therm_channel_data = NULL;

    /*
     * Safe to cast struct therm_channels* to struct therm_channels_3x* because:
     * 1. struct therm_channels_3x has THERM_CHANNELS_MODEL_10 as its first member
     * 2. THERM_CHANNELS_MODEL_10 has struct therm_channels as its first member
     * 3. The memory layout ensures that struct therm_channels_3x is an extension
     *    of struct therm_channels with additional fields
     */
    pChannels3x = (struct therm_channels_3x *)pthermchannelobjs;
	nvgpu_log_info(g, " ");

	/* Get the thermal channel table from VBIOS */
	therm_channel_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_PERF_TOKEN),
						THERMAL_CHANNEL_TABLE);
	if (therm_channel_table_ptr == NULL) {
		nvgpu_err(g, " NULL thermal channel table ptr");
		status = -EINVAL;
		goto done;
	}

	curr_therm_channel_table_ptr = therm_channel_table_ptr;

	/* Copy and validate the header */
	nvgpu_memcpy((u8 *)&therm_channel_table_header, therm_channel_table_ptr,
	            VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09);

	if (therm_channel_table_header.version != VBIOS_THERM_CHANNEL_VERSION_2X) {
		nvgpu_err(g,"Thermal channel table version mismatch, expected %d, got %d.",
		         VBIOS_THERM_CHANNEL_VERSION_2X,
		         therm_channel_table_header.version);
		status = -EINVAL;
		goto done;
	}

	if (therm_channel_table_header.header_size < VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09) {
		nvgpu_err(g,"Thermal channel table header size invalid, expected %d, got %d.",
		         VBIOS_THERM_CHANNEL_2X_HEADER_SIZE_09,
		         therm_channel_table_header.header_size);
		status = -EINVAL;
		goto done;
	}

	/* Advance pointer past the header */
	curr_therm_channel_table_ptr += therm_channel_table_header.header_size;

	/* Store thermal monitoring parameters */
	pChannels3x->typeHAL = (u8)therm_channel_table_header.typeHAL;
	pChannels3x->pmuMonSamplingNormalMs = (u16)therm_channel_table_header.pmuMonSamplingNormalMs;
	pChannels3x->pmuMonSamplingSleepMs = (u16)therm_channel_table_header.pmuMonSamplingSleepMs;

	/* Allocate memory for the therm_channel_data structure */
	therm_channel_data = nvgpu_kzalloc_impl(g, sizeof(*therm_channel_data), NVGPU_GET_IP);
	if (therm_channel_data == NULL) {
		nvgpu_err(g, "Failed to allocate memory for therm_channel_data");
		status = -ENOMEM;
		goto done;
	}

	/* Process all entries in the thermal channel table */
	for (index = 0; index < therm_channel_table_header.num_table_entries && (status == 0); index++) {
		skip_entry = false;

		/* Get the current table entry */
		therm_channel_table_entry = (struct therm_channel_2x_entry *)
			(curr_therm_channel_table_ptr +
				(therm_channel_table_header.table_entry_size * index));

		/* Initialize the therm_channel_data structure to zero */
		(void)memset((void *)therm_channel_data, 0, sizeof(*therm_channel_data));

		/* Extract entry parameters */
		flags = therm_channel_table_entry->flags;
		param0 = therm_channel_table_entry->param0;
		param1 = therm_channel_table_entry->param1;
		param2 = therm_channel_table_entry->param2;
		class_id = devinitThermChannel2xClassConvertToInternal(therm_channel_table_entry->class_id);

		/* Process entry based on class */
		switch (class_id) {
			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_INVALID:
				invalidEntryCnt++;
				/* Safety check to prevent excessive invalid entries */
				if (invalidEntryCnt > therm_channel_table_header.num_table_entries) {
					nvgpu_err(g,"[%s]:%d. - Invalid entry count reached maximum, invalidEntryCnt is %d.\n",
					        __func__, __LINE__, invalidEntryCnt);
					status = -EINVAL;
					break; /* Exit the switch and will break the loop due to status check */
				}
				/* Skip this entry and continue with the next one */
				skip_entry = true;
				break;
			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_PROVIDER_V10:
				// Extract the device index and provider index from the param0 field
				therm_channel_data->provider.thermDevIdx =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_PROVIDER_PROV_INDEX_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_PROVIDER_PROV_INDEX_SHIFT);
				// Set the size of the therm_channel_provider_v10 structure
				therm_channel_size = sizeof(struct therm_channel_provider_v10);
				break;
			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10:
				// Extract the device index and provider index from the param0 field
				therm_channel_data->debug.thermDevIdx =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_DEBUG_DEV_INDEX_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_DEBUG_DEV_INDEX_SHIFT);
				// Extract the start and end provider indices from the param0 field
				therm_channel_data->debug.thermDevProvIdxStart =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_DEBUG_PROV_INDEX_START_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_DEBUG_PROV_INDEX_START_SHIFT);
				// Extract the end provider index from the param0 field
				therm_channel_data->debug.thermDevProvIdxEnd =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_DEBUG_PROV_INDEX_END_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_DEBUG_PROV_INDEX_END_SHIFT);
				// Set the size of the therm_channel_debug_v10 structure
				therm_channel_size = sizeof(struct therm_channel_debug_v10);
				break;

			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_AGGREGATION_V10:
				// Extract the device index and provider index from the param0 field
				therm_channel_data->aggregation.thermDevIdx =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_DEV_INDEX_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_DEV_INDEX_SHIFT);
				// Extract the start and end provider indices from the param0 field
				therm_channel_data->aggregation.thermDevProvIdxStart =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_PROV_INDEX_START_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_PROV_INDEX_START_SHIFT);
				// Extract the end provider index from the param0 field
				therm_channel_data->aggregation.thermDevProvIdxEnd =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_PROV_INDEX_END_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_PROV_INDEX_END_SHIFT);
				// Extract the next channel index from the param0 field
				therm_channel_data->aggregation.thermChannelIdxNext =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_NEXT_CHANNEL_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_AGGREGATION_NEXT_CHANNEL_SHIFT);
				// Extract the aggregation mode from the flags field
				mode = (u8)(((u8)flags & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_AGGREGATION_MODE_MASK) >>
								   NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_AGGREGATION_MODE_SHIFT);
				// Convert the aggregation mode to the internal format
				therm_channel_data->aggregation.mode = devinitThermChannel2xAggregationModeConvertToInternal(g, (u8)index, mode);
				// Set the size of the therm_channel_aggregation_v10 structure
				therm_channel_size = sizeof(struct therm_channel_aggregation_v10);
				break;

			case NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_ESTIMATION_V10:
				// Extract the first channel index from the param0 field
				therm_channel_data->estimation.thermChannelIdx1 =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_ESTIMATION_CHANNEL_1_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_ESTIMATION_CHANNEL_1_SHIFT);
				// Extract the second channel index from the param0 field
				therm_channel_data->estimation.thermChannelIdx2 =
						(u8)(((u8)param0 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_ESTIMATION_CHANNEL_2_MASK) >>
							 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM0_ESTIMATION_CHANNEL_2_SHIFT);
				// Extract the estimation mode from the flags field
				mode = (u8)(((u8)flags & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_ESTIMATION_MODE_MASK) >>
								   NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_ESTIMATION_MODE_SHIFT);
				// Convert the estimation mode to the internal format
				therm_channel_data->estimation.mode = devinitThermChannel2xEstimationModeConvertToInternal(g, (u8)index, mode);
				// Set the size of the therm_channel_estimation_v10 structure
				therm_channel_size = sizeof(struct therm_channel_estimation_v10);
				break;
			default:
				// Set the size of the therm_channel_device structure
				therm_channel_size = sizeof(struct therm_channel_device);
				break;
		}

		/* Skip the rest of the loop body if this is an invalid entry */
		if (skip_entry) {
			continue;
		}

		// Initialize data for BOARDOBJ parent class
		therm_channel_data->obj.type = class_id;
		therm_channel_data->obj.idx = index;
		// Extract the scaling flag from the flags field
		therm_channel_data->channel.bScaling = (u8)(((u8)flags & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_SCALING_MASK) >>
													NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_SCALING_SHIFT);
		// Extract the bounds flag from the flags field
		therm_channel_data->channel.bBounds = (u8)(((u8)flags & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_BOUNDS_MASK) >>
												   NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_BOUNDS_SHIFT);
		// Extract the software temperature simulation flag from the flags field
		therm_channel_data->channel.tempSim.bSupported = (u8)(((u8)flags & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_SW_TEMP_SIM_MASK) >>
													   NV_VBIOS_THERM_CHANNEL_2X_ENTRY_FLAGS_SW_TEMP_SIM_SHIFT);
		// Initialize scaling parameters from VBIOS when scaling is enabled.
		// Otherwise set scaling slope to 1.0 and offset to 0.0
		if (therm_channel_data->channel.bScaling)
		{
			therm_channel_data->channel.scaling =
					(s16)(((u32)param1 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM1_SCALING_SLOPE_MASK) >>
						  NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM1_SCALING_SLOPE_SHIFT);
			/* Using explicit cast to u32 for mask values to match param1 type and avoid using BIOS_GET_FIELD() for MIRSA violation */
			therm_channel_data->channel.offsetSW =
					(s16)((((u32)param1 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM1_SCALING_OFFSETSW_MASK) >>
					       (u32)NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM1_SCALING_OFFSETSW_SHIFT));
		}
		else
		{
			therm_channel_data->channel.scaling  = 1;
			therm_channel_data->channel.offsetSW = 0;
		}
		// Initalize bounds from VBIOS when enabled. Otherwise bounds to 0C.
		if (therm_channel_data->channel.bBounds) {
			therm_channel_data->channel.temp_min = (s32)(((u32)param2 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM2_TEMP_MIN_MASK) >>
					 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM2_TEMP_MIN_SHIFT);
			therm_channel_data->channel.temp_max = (s32)(((u32)param2 & NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM2_TEMP_MAX_MASK) >>
					 NV_VBIOS_THERM_CHANNEL_2X_ENTRY_PARAM2_TEMP_MAX_SHIFT);
		}
		else{
			therm_channel_data->channel.temp_min = 0;
			therm_channel_data->channel.temp_max = 0;
		}
		// Get channel type and visibility details from HAL
		status = devinitThermChannel2xHalInfoGet(pChannels3x->typeHAL,
											&therm_channel_data->channel.chType,
											&therm_channel_data->v3x.bPublicVisibility);
		if (status != 0) {
			nvgpu_err(g,"[%s]:%d. devinitThermChannel2xHalInfoGet() failed to get channel type and visibility details.\n",
			         __func__, __LINE__);
			break; /* Exit the loop due to error */
		}

		/* Initialize additional channel parameters */
		therm_channel_data->channel.relloc = 0;
		therm_channel_data->channel.tgtGPU = 0;
		therm_channel_data->channel.flags  = 0;

		/* Construct the channel device object */
		pChannel = construct_channel_device(g, therm_channel_data,
					(u16)therm_channel_size, class_id);
		if (pChannel == NULL) {
			nvgpu_err(g, "unable to create thermal device for %d type %d",
				index, therm_channel_data->obj.type);
			status = -ENOMEM;
			break; /* Exit the loop due to error */
		}

		/* Insert the constructed object into the board object group */
		status = boardobjgrp_objinsert(&pthermchannelobjs->super.super,
				(struct pmu_board_obj *)pChannel, (u8)obj_index);
		if (status != 0) {
			nvgpu_err(g, "unable to insert thermal device boardobj for %d", index);
			status = -ENOMEM;
			break; /* Exit the loop due to error */
		}

		/* Move to the next object index */
		++obj_index;
	}

done:
	/* Free allocated memory if it exists */
	if (therm_channel_data != NULL) {
		nvgpu_kfree(g, therm_channel_data);
	}
	nvgpu_log_info(g, " done status %x", status);
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
	if ((g->pmu->therm_pmu->therm_channelobjs == NULL)) {
		nvgpu_err(g, "therm_channelobjs not initialized, re-allocate memory for therm_channelobjs");
		status = therm_channel_init_objs(g);
		if (status != 0) {
			nvgpu_err(g, "error initializing therm_channelobjs");
			status = -EINVAL;
			goto done;
		}
	}

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

done:
	return status;
}

static int therm_channel_currtemp_update(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	struct therm_channel_get_status *therm_channel_obj;
	struct nv_pmu_therm_therm_channel_boardobj_get_status *pstatus;
	nvgpu_log_info(g, " ");
	therm_channel_obj = (struct therm_channel_get_status *)
		(void *)obj;
	pstatus = (struct nv_pmu_therm_therm_channel_boardobj_get_status *)
		(void *)pmu_obj;
	if (pstatus->super.type != therm_channel_obj->super.type) {
		nvgpu_err(g, "pmu data and boardobj type not matching");
		return -EINVAL;
	}
	therm_channel_obj->curr_temp = pstatus->current_temp;
	return 0;
}

static int therm_channel_boardobj_grp_get_status(struct gk20a *g)
{
	struct boardobjgrp *pboardobjgrp = NULL;
	struct boardobjgrpmask *pboardobjgrpmask;
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu;
	struct pmu_board_obj *obj = NULL;
	struct nv_pmu_boardobj_query *pboardobjpmustatus = NULL;
	int status;
	u8 index;
	nvgpu_log_info(g, " ");
	if (g == NULL) {
		return -EINVAL;
	}
	if (((g->pmu == NULL) || (g->pmu->therm_pmu == NULL)) ||
	    ((g->pmu->therm_pmu->therm_channelobjs == NULL))) {
		nvgpu_err(g, "PMU or therm_pmu not initialized");
		return -EINVAL;
	}
	pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;
	pboardobjgrpmask = &g->pmu->therm_pmu->therm_channelobjs->super.mask.super;
	status = pboardobjgrp->pmugetstatus(g, pboardobjgrp, pboardobjgrpmask);
	if (status != 0) {
		nvgpu_err(g, "err getting boardobjs from pmu");
		return status;
	}
	pboardobjgrppmu = pboardobjgrp->pmu.getstatus.buf;
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		status = pboardobjgrp->pmustatusinstget(g,
				(struct nv_pmu_boardobjgrp *)(void *)pboardobjgrppmu,
				&pboardobjpmustatus, index);
		if (status != 0) {
			nvgpu_err(g, "could not get status object instance");
			return status;
		}
		status = therm_channel_currtemp_update(g, obj,
				(struct nv_pmu_boardobj *)(void *)pboardobjpmustatus);
		if (status != 0) {
			nvgpu_err(g, "could not update therm_channel status");
			return status;
		}
	}
	return 0;
}

int nvgpu_pmu_therm_channel_get_curr_temp(struct gk20a *g, u32 *temp)
{
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	struct therm_channel_get_status *therm_channel_status = NULL;
	int status;
	u8 index;

	if ((g == NULL) || (temp == NULL)) {
		return -EINVAL;
	}
	if (((g->pmu == NULL) || (g->pmu->therm_pmu == NULL)) ||
	    ((g->pmu->therm_pmu->therm_channelobjs == NULL))) {
		nvgpu_err(g, "PMU or therm_pmu not initialized");
		return -EINVAL;
	}
	status = therm_channel_boardobj_grp_get_status(g);
	if (status != 0) {
		nvgpu_err(g, "therm_channel get status failed");
		return status;
	}
	pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		therm_channel_status = (struct therm_channel_get_status *)
				(void *)obj;
		if (therm_channel_status->curr_temp != 0U) {
			*temp = therm_channel_status->curr_temp;
			break;
		}
	}

	return status;
}

int therm_channel_pmu_setup(struct gk20a *g)
{
	int status = 0;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_log_info(g, " ");

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
	if (!BOARDOBJGRP_IS_EMPTY(&g->pmu->therm_pmu->therm_channelobjs->super.super)) {
		pboardobjgrp = &g->pmu->therm_pmu->therm_channelobjs->super.super;
		status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);
	}

done:
	return status;
}
