/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _THERM_DEVICE_GPU_TZ_V10_H_
#define _THERM_DEVICE_GPU_TZ_V10_H_
//#include <stdint.h>
#include <nvgpu/gk20a.h>
#include "therm_device_3x.h"
#include "therm_dev.h"
#include <nvgpu/pmu/therm.h>
/* Forward declarations */
struct THERM_DEVICE_GPU_TZ_V10;
typedef struct THERM_DEVICE_GPU_TZ_V10 THERM_DEVICE_GPU_TZ_V10;
/*!
 * IIR Filter related parameters for Thermal Zone
 */
typedef struct
{
    bool  bEnabled;
     u8   length;
     u16  updatePeriodUtilsClock;
} THERM_DEVICE_GPU_TZ_V10_IIR_FILTER;
/*!
 * Extends THERM_CHANNEL_3X providing common attributes between
 * THERM_DEVICE_CLASS_GPU_GTZ_V10 and THERM_DEVICE_CLASS_GPU_GTZ_V10
 */
struct THERM_DEVICE_GPU_TZ_V10
{
    struct therm_device_3x super;
    u8            hwIdx;
    u8            mode;
    bool          bHsOffset;
    bool          bMunged;
    u32           sensorMask;
    THERM_DEVICE_GPU_TZ_V10_IIR_FILTER iirFilter;
};
s32 thermDeviceIfaceModel10Construct_GPU_TZ_V10(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
);
s32 thermDeviceConstructSanityChecks_GPU_TZ_V10(
    struct gk20a *g,
    struct therm_device *pDevice
);
#endif // _THERM_DEVICE_GPU_TZ_V10_H_
