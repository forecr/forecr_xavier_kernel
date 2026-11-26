/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _THERM_DEVICE_GPU_MINI_TSENSE_V10_H_
#define _THERM_DEVICE_GPU_MINI_TSENSE_V10_H_
//#include <stdint.h>
#include <nvgpu/gk20a.h>
#include "therm_device_3x.h"  /* Include this for therm_device_3x structure */
#include "therm_dev.h"
#include <nvgpu/pmu/therm.h>
/* Define class IDs and other constants */
#ifndef NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10 0x18
#endif
/* Forward declare structure */
struct THERM_DEVICE_GPU_MINI_TSENSE_V10;
typedef struct THERM_DEVICE_GPU_MINI_TSENSE_V10 THERM_DEVICE_GPU_MINI_TSENSE_V10;
/*!
 * Extends THERM_DEVICE_3X providing attributes specific to
 * THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10.
 */
struct THERM_DEVICE_GPU_MINI_TSENSE_V10
{
    /*!
     * therm_device_3x super class. This should always be the first member!
     */
    struct therm_device_3x super;
    /*!
     * Sensor location within GPU
     */
    u8 location;
};
/* Function Prototypes */
s32 thermDeviceIfaceModel10Construct_GPU_MINI_TSENSE_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
);
s32 thermDeviceConstructSanityChecks_GPU_MINI_TSENSE_V10
(
    struct gk20a *g,
    struct therm_device *pDevice
);
#endif // _THERM_DEVICE_GPU_MINI_TSENSE_V10_H_
