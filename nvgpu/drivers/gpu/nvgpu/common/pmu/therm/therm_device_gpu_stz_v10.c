// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/string.h>
#include "therm_device_gpu_stz_v10.h"

/*!
 * Construct new THERM_DEVICE_GPU_STZ_V10 object.
 *
 * @copydoc BoardObjIfaceModel10Construct
 */
s32 thermDeviceIfaceModel10Construct_GPU_STZ_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
)
{
    s32 status = 0;
    struct THERM_DEVICE_GPU_STZ_V10 *pDeviceStz = NULL;
    u8 objType = 0;
    if (pArgs == NULL || ppBoardObj == NULL) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_STZ_V10_exit;
    }
    objType = pmu_board_obj_get_type(pArgs);
    if (objType != NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_STZ_V10) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_STZ_V10_exit;
    }
    // Allocate memory for the STZ device
    pDeviceStz = nvgpu_kzalloc(g, size);
    if (pDeviceStz == NULL) {
        status = -ENOMEM;
        goto thermDeviceIfaceModel10Construct_GPU_STZ_V10_exit;
    }

    nvgpu_memcpy((u8 *)pDeviceStz, (u8 *)pArgs, size);

    status = pmu_board_obj_construct_super(g, &pDeviceStz->super.super.super, pArgs);
    if (status != 0) {
        nvgpu_kfree(g, pDeviceStz);
        goto thermDeviceIfaceModel10Construct_GPU_STZ_V10_exit;
    }

    *ppBoardObj = (struct pmu_board_obj *)pDeviceStz;

thermDeviceIfaceModel10Construct_GPU_STZ_V10_exit:
    if (status != 0 && pDeviceStz != NULL) {
        nvgpu_kfree(g, pDeviceStz);
    }
    return status;
}
