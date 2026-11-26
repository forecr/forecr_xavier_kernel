// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/string.h>
#include "therm_device_gpu_tsense_v10.h"
#include <nvgpu/pmu/therm.h>

/*!
 * Construct new THERM_DEVICE_GPU_TSENSE_V10 object.
 *
 * @copydoc BoardObjIfaceModel10Construct
 */
s32 thermDeviceIfaceModel10Construct_GPU_TSENSE_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
)
{
    struct THERM_DEVICE_GPU_TSENSE_V10 *pDevTsense = NULL;
    struct THERM_DEVICE_GPU_TSENSE_V10 *pDevTemp   = (struct THERM_DEVICE_GPU_TSENSE_V10 *) pArgs;
    s32                    status     = 0;
    // Ensure that pArgs is not NULL
    if (pArgs == NULL) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_TSENSE_V10_exit;
    }
    // Sanity checks before constructing _GPU_TSENSE_V10
    if (thermDeviceConstructSanityChecks_GPU_TSENSE_V10(g, (THERM_DEVICE *)pArgs) != 0) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_TSENSE_V10_exit;
    }
    pDevTsense = nvgpu_kzalloc(g, size);

    if (pDevTsense == NULL) {
        status = -ENOMEM;
        goto thermDeviceIfaceModel10Construct_GPU_TSENSE_V10_exit;
    }

    nvgpu_memcpy((u8 *)pDevTsense, (u8 *)pArgs, size);

    status = pmu_board_obj_construct_super(g, &pDevTsense->super.super, pArgs);
    if (status != 0) {
        nvgpu_err(g, "pmu_board_obj_construct_super failed for GPU_TSENSE_V10");
        goto thermDeviceIfaceModel10Construct_GPU_TSENSE_V10_exit;
    }

    if (pDevTsense == NULL) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_TSENSE_V10_exit;
    }
    pDevTsense->location = pDevTemp->location;
thermDeviceIfaceModel10Construct_GPU_TSENSE_V10_exit:
    if (status != 0 && pDevTsense != NULL) {
        nvgpu_kfree(g, pDevTsense);
    }
    *ppBoardObj = (struct pmu_board_obj *)pDevTsense;
    return status;
}
/* ------------------------ Private Static Functions ----------------------- */
/*!
 * Sanity checks for THERM_DEVICE_GPU_TSENSE_V10 object.
 *
 * @copydoc ThermDeviceConstructSanityChecks
 */
s32 thermDeviceConstructSanityChecks_GPU_TSENSE_V10
(
    struct gk20a *g,
    THERM_DEVICE *pDevice
)
{
    struct THERM_DEVICE_GPU_TSENSE_V10 *pDevTsense = (struct THERM_DEVICE_GPU_TSENSE_V10 *) pDevice;
    s32                    status     = 0;

    (void)g; /* Suppress unused parameter warning */

    // Tsense location should be within max supported value
    if (pDevTsense->location >= NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC__COUNT)
    {
        status = -EINVAL;
        goto thermDeviceConstructSanityChecks_GPU_TSENSE_V10_exit;
    }
thermDeviceConstructSanityChecks_GPU_TSENSE_V10_exit:
    return status;
}
