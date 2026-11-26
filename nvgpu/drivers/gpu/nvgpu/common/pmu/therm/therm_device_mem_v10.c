// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/string.h>
#include "therm_device_mem_v10.h"

s32 thermDeviceIfaceModel10Construct_MEM_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
)
{
    struct THERM_DEVICE_MEM_V10 *pDevMem   = NULL;
    s32                status    = 0;
    //BOARDOBJ_IFACE_MODEL_10 *pModel10  = NULL;
    // Ensure that pArgs is not NULL
    if (pArgs == NULL) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_MEM_V10_exit;
    }

    if (thermDeviceConstructSanityChecks_MEM_V10(g, (THERM_DEVICE *)pArgs) != 0) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_MEM_V10_exit;
    }

    // Allocate memory for the MEM device
    pDevMem = nvgpu_kzalloc(g, size);
    if (pDevMem == NULL) {
        status = -ENOMEM;
        goto thermDeviceIfaceModel10Construct_MEM_V10_exit;
    }

    nvgpu_memcpy((u8 *)pDevMem, (u8 *)pArgs, size);
thermDeviceIfaceModel10Construct_MEM_V10_exit:
    if (status != 0 && pDevMem != NULL) {
        nvgpu_kfree(g, pDevMem);
    }
    *ppBoardObj = (struct pmu_board_obj *)pDevMem;
    return status;
}
/* ------------------------ Private Static Functions ----------------------- */
/*!
 * Sanity checks for THERM_DEVICE_MEM_V10 object.
 *
 * @copydoc ThermDeviceConstructSanityChecks
 */
s32 thermDeviceConstructSanityChecks_MEM_V10
(
    struct gk20a *g,
    THERM_DEVICE *pDevice
)
{
    THERM_DEVICE_MEM_V10 *pDevMem = (THERM_DEVICE_MEM_V10 *) pDevice;
    s32                    status  = 0;

    (void)g; /* Suppress unused parameter warning */
    // MEM sensorType should be within max supported value
    if (pDevMem->sensorType >= NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE__COUNT)
    {
        status = -EINVAL;
        goto _thermDeviceConstructSanityChecks_MEM_V10_exit;
    }
    // MEM sensorType should be within max supported value
    if (pDevMem->refreshPeriodLimitMs >= NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_REFRESH_PERIOD_LIMIT_MAX_MS)
    {
        status = -EINVAL;
        goto _thermDeviceConstructSanityChecks_MEM_V10_exit;
    }
_thermDeviceConstructSanityChecks_MEM_V10_exit:
    return status;
}
