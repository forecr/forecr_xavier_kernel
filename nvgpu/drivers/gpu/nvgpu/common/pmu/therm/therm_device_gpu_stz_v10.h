/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _THERM_DEVICE_GPU_STZ_V10_H_
#define _THERM_DEVICE_GPU_STZ_V10_H_
//#include <stdint.h>
#include <nvgpu/gk20a.h>
#include "therm_device_3x.h"
#include "therm_device_gpu_tz_v10.h"
/* Define class IDs and other constants */
#ifndef NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_STZ_V10
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_STZ_V10 0x15
#endif
/* Forward declarations */
struct THERM_DEVICE_GPU_STZ_V10;
typedef struct THERM_DEVICE_GPU_STZ_V10 THERM_DEVICE_GPU_STZ_V10;
/*!
 * Extends THERM_DEVICE_GPU_TZ_V10 providing attributes specific to
 * THERM_DEVICE_CLASS_GPU_STZ_V10.
 */
struct THERM_DEVICE_GPU_STZ_V10
{
    /*!
     * THERM_DEVICE_GPU_TZ_V10 super class. This should always be the first member!
     */
    struct THERM_DEVICE_GPU_TZ_V10 super;
};
/* Function Prototypes */
s32 thermDeviceIfaceModel10Construct_GPU_STZ_V10(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
);
#endif // _THERM_DEVICE_GPU_STZ_V10_H_
