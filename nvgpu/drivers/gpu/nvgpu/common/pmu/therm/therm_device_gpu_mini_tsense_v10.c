// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

/* ------------------------ Includes --------------------------------------- */
#include <nvgpu/string.h>
#include "therm_device_gpu_mini_tsense_v10.h"
/* ------------------------ Public Class Interfaces ------------------------ */
/*!
 * Construct new THERM_DEVICE_GPU_MINI_TSENSE_V10 object.
 *
 * @copydoc BoardObjIfaceModel10Construct
 */
s32 thermDeviceIfaceModel10Construct_GPU_MINI_TSENSE_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
)
{
    s32 status = 0;
    struct THERM_DEVICE_GPU_MINI_TSENSE_V10 *pDevMiniTsense = NULL;
    u8 objType = 0;
    // Ensure that pArgs is not NULL
    if (pArgs == NULL || ppBoardObj == NULL) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_MINI_TSENSE_V10_exit;
    }
    objType = pmu_board_obj_get_type(pArgs);
    if (objType != NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_MINI_TSENSE_V10_exit;
    }
    // Sanity checks before constructing _GPU_MINI_TSENSE_V10

        status = thermDeviceConstructSanityChecks_GPU_MINI_TSENSE_V10(g, (THERM_DEVICE *)pArgs);
        if (status != 0) {
            goto thermDeviceIfaceModel10Construct_GPU_MINI_TSENSE_V10_exit;
        }
    // Allocate memory for the THERM_DEVICE_GPU_MINI_TSENSE_V10 object
    pDevMiniTsense = (THERM_DEVICE_GPU_MINI_TSENSE_V10 *) nvgpu_kzalloc(g, size);
    if (pDevMiniTsense == NULL) {
        status = -ENOMEM;
        goto thermDeviceIfaceModel10Construct_GPU_MINI_TSENSE_V10_exit;
    }
    nvgpu_memcpy((u8 *)pDevMiniTsense, (u8 *)pArgs, size);
    status = pmu_board_obj_construct_super(g, &pDevMiniTsense->super.super, pArgs);
    if (status != 0) {
        nvgpu_kfree(g, pDevMiniTsense);
    }

thermDeviceIfaceModel10Construct_GPU_MINI_TSENSE_V10_exit:
    *ppBoardObj = (struct pmu_board_obj *)pDevMiniTsense;
    if (status != 0 && pDevMiniTsense != NULL) {
        nvgpu_kfree(g, pDevMiniTsense);
    }
    return status;
}
/* ------------------------ Private Static Functions ----------------------- */
/*!
 * Sanity checks for THERM_DEVICE_GPU_MINI_TSENSE_V10 object.
 *
 * @copydoc ThermDeviceConstructSanityChecks
 */
s32 thermDeviceConstructSanityChecks_GPU_MINI_TSENSE_V10
(
    struct gk20a *g,
    THERM_DEVICE *pDevice
)
{
    THERM_DEVICE_GPU_MINI_TSENSE_V10 *pDevMiniTsense = (THERM_DEVICE_GPU_MINI_TSENSE_V10 *) pDevice;
    s32 status = 0;

    (void)g; /* Suppress unused parameter warning */
    // Tsense location should be within max supported value
    if (pDevMiniTsense->location >= NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_LOC__COUNT)
    {
        status = -EINVAL;
    }
    return status;
}
