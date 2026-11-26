/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
/*!
 * @file  therm_device_3x.c
 * @brief Thermal Device class type 3X
 */

/* ------------------------ Includes --------------------------------------- */
#include "therm_device_3x.h"
#include <nvgpu/kmem.h>
#include <nvgpu/string.h>

/* ------------------------ Public Class Interfaces ------------------------ */

/*!
 * Base constructor for thermal device types
 *
 * @param g         Pointer to GPU object
 * @param ppBoardObj Pointer to board object pointer
 * @param size      Size of the object to allocate
 * @param pargs     Pointer to arguments
 *
 * @return 0 on success, error code otherwise
 */
s32 thermDeviceIfaceModel10Construct_3X(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pargs
)
{
    struct therm_device_3x *pDevice = NULL;
    s32 status = 0;

    /* Ensure that pargs is not NULL */
    if (pargs == NULL) {
        status = -EINVAL;
        goto exit;
    }

    /* Allocate memory for the device */
    pDevice = nvgpu_kzalloc(g, size);
    if (pDevice == NULL) {
        status = -ENOMEM;
        goto exit;
    }

    /* Copy the input arguments to pDevice */
    nvgpu_memcpy((u8 *)pDevice, (u8 *)pargs, size);

    /* Initialize the device */
    status = therm_device_3x_init(g, pDevice);
    if (status != 0) {
        goto exit;
    }

exit:
    if (status != 0 && pDevice != NULL) {
        nvgpu_kfree(g, pDevice);
        pDevice = NULL;
    }
    *ppBoardObj = (struct pmu_board_obj *)pDevice;
    return status;
}

/*!
 * Initialize a THERM_DEVICE_3X structure
 *
 * @param g        Pointer to GPU object
 * @param pDevice  Pointer to the device to initialize
 *
 * @return 0 on success, error code otherwise
 */
s32 therm_device_3x_init(struct gk20a *g, struct therm_device_3x *pDevice)
{
    s32 status = 0;

    if (pDevice == NULL) {
        return -EINVAL;
    }

    /* Validate device parameters */
    status = therm_device_3x_validate(g, pDevice);
    if (status != 0) {
        goto exit;
    }

    /* Initialize device-specific fields if needed */
    /* Additional initialization can be added here */

exit:
    return status;
}

/*!
 * Convert device type to string representation
 *
 * @param devType  Device type
 *
 * @return String representation of the device type
 */
const char *therm_device_3x_type_to_string(u8 devType)
{
    switch (devType) {
    case NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_NOT_ASSIGNED:
        return "NOT_ASSIGNED";
    case NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_GPU_OFFSET_MAX:
        return "GPU_OFFSET_MAX";
    case NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_GPU_AVG:
        return "GPU_AVG";
    default:
        return "UNKNOWN";
    }
}

/*!
 * Validate device parameters
 *
 * @param g        Pointer to GPU object
 * @param pDevice  Pointer to the device to validate
 *
 * @return 0 on success, error code otherwise
 */
s32 therm_device_3x_validate(struct gk20a *g, struct therm_device_3x *pDevice)
{
    (void)g;

    if (pDevice == NULL) {
        return -EINVAL;
    }

    /* Validate provider mapping */
    if (pDevice->numProv == 0 && pDevice->provSupportMask != 0) {
        return -EINVAL;
    }

    /* Additional validation can be added here */
    return 0;
}
