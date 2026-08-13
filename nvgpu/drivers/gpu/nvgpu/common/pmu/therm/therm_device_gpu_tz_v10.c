// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/string.h>
#include "therm_device_gpu_tz_v10.h"

s32 thermDeviceIfaceModel10Construct_GPU_TZ_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32       size,
    void       *pArgs
)
{
    struct THERM_DEVICE_GPU_TZ_V10 *pDevTz    = NULL;
    s32                status    = 0;
    u8 objType = 0;
    // Ensure that pArgs is not NULL
    if (pArgs == NULL || ppBoardObj == NULL)
    {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_TZ_V10_exit;
    }
    // Verify object type
    objType = pmu_board_obj_get_type(pArgs);
    if (objType != NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_TZ_V10) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_TZ_V10_exit;
    }
    // Sanity checks before constructing _GPU_TZ_V10
    status = thermDeviceConstructSanityChecks_GPU_TZ_V10(g, (THERM_DEVICE *)pArgs);
    if (status != 0) {
        goto thermDeviceIfaceModel10Construct_GPU_TZ_V10_exit;
    }
    // Allocate memory for the TZ device
    pDevTz = nvgpu_kzalloc(g, size);
    if (pDevTz == NULL) {
        status = -ENOMEM;
        goto thermDeviceIfaceModel10Construct_GPU_TZ_V10_exit;
    }
    // Copy data from input args
    nvgpu_memcpy((u8 *)pDevTz, (u8 *)pArgs, size);
    // Initialize the base object
    status = pmu_board_obj_construct_super(g, &pDevTz->super.super, pArgs);
    if (status != 0) {
        goto thermDeviceIfaceModel10Construct_GPU_TZ_V10_exit;
    }
    // Override interfaces if needed
    //pDevTz->super.super.super.stateInit = _thermDeviceStateInit_GPU_TZ_V10;
    // Set the output parameter
    *ppBoardObj = (struct pmu_board_obj *)pDevTz;
thermDeviceIfaceModel10Construct_GPU_TZ_V10_exit:
    // Clean up on error
    if (status != 0 && pDevTz != NULL) {
        nvgpu_kfree(g, pDevTz);
        pDevTz = NULL;
    }
    return status;
}
/*!
 * Sanity checks for THERM_DEVICE_GPU_TZ_V10 object.
 *
 * @copydoc ThermDeviceConstructSanityChecks
 */
s32 thermDeviceConstructSanityChecks_GPU_TZ_V10
(
    struct gk20a *g,
    THERM_DEVICE *pDevice
)
{
    THERM_DEVICE_GPU_TZ_V10 *pDevTz = (THERM_DEVICE_GPU_TZ_V10 *) pDevice;
    u32                status = 0;

    (void)g; /* Suppress unused parameter warning */

    // Thermal zone should have a valid mode
    if (pDevTz->mode == NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_INVALID)
    {
        status = -EINVAL;
        goto thermDeviceConstructSanityChecks_GPU_TZ_V10_exit;
    }
    // Thermal zone should have a at least one input sensor
    if (pDevTz->sensorMask == 0)
    {
        status = -EINVAL;
        goto thermDeviceConstructSanityChecks_GPU_TZ_V10_exit;
    }
    // IIR filter sanity checks
    if (pDevTz->iirFilter.bEnabled)
    {
        // IIR filter length should not be zero
        if (pDevTz->iirFilter.length == 0)
        {
            status = -EINVAL;
            goto thermDeviceConstructSanityChecks_GPU_TZ_V10_exit;
        }
        // IIR filter update period should not be zero
        if (pDevTz->iirFilter.updatePeriodUtilsClock == 0)
        {
            status = -EINVAL;
            goto thermDeviceConstructSanityChecks_GPU_TZ_V10_exit;
        }
    }
    else
    {
        // IIR filter length should be zero
        if (pDevTz->iirFilter.length != 0)
        {
            status = -EINVAL;
            goto thermDeviceConstructSanityChecks_GPU_TZ_V10_exit;
        }
        // IIR filter update period should be zero
        if (pDevTz->iirFilter.updatePeriodUtilsClock != 0)
        {
            status = -EINVAL;
            goto thermDeviceConstructSanityChecks_GPU_TZ_V10_exit;
        }
    }

thermDeviceConstructSanityChecks_GPU_TZ_V10_exit:
    return status;
}
