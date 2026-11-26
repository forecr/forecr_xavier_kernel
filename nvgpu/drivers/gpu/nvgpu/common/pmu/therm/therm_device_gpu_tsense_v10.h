/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _THERM_DEVICE_GPU_TSENSE_V10_H_
#define _THERM_DEVICE_GPU_TSENSE_V10_H_
//#include <stdint.h>
#include <nvgpu/gk20a.h>
#include "therm_device_3x.h"
#include "therm_dev.h"
#include <nvgpu/pmu/therm.h>
/* Forward declarations */
struct THERM_DEVICE_GPU_TSENSE_V10;
typedef struct THERM_DEVICE_GPU_TSENSE_V10 THERM_DEVICE_GPU_TSENSE_V10;

typedef struct
{
    /*!
     * Hotspot offset for given provider
     */
    s32  hsOffset;
} THERM_DEVICE_GPU_TSENSE_PROV;
/*!
 * Extends THERM_DEVICE_3X providing device-specific attributes for TSENSE V10
 */
struct THERM_DEVICE_GPU_TSENSE_V10
{
    struct therm_device_3x super;
    /*!
     * Sensor location within GPU
     */
    u8 location;
    /*!
     * Per provider information for Tsense.
     */
    THERM_DEVICE_GPU_TSENSE_PROV provider[NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV__NUM_PROVS];
};
/* Function Prototypes */
s32 thermDeviceIfaceModel10Construct_GPU_TSENSE_V10(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
);
s32 thermDeviceConstructSanityChecks_GPU_TSENSE_V10(
    struct gk20a *g,
    THERM_DEVICE *pDevice
);
#endif // _THERM_DEVICE_GPU_TSENSE_V10_H_
