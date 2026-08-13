// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/string.h>
#include "therm_device_gpu_gtz_v10.h"
/*!
 * Construct new THERM_DEVICE_GPU_GTZ_V10 object.
 *
 * @copydoc BoardObjIfaceModel10Construct
 */
s32 thermDeviceIfaceModel10Construct_GPU_GTZ_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
)
{
    s32 status = 0;
	u8 objType = pmu_board_obj_get_type(pArgs);
    struct THERM_DEVICE_GPU_GTZ_V10 *pDeviceGtz = NULL;

    if (pArgs == NULL || ppBoardObj == NULL) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_GTZ_V10_exit;
    }
    if (objType != NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GTZ_V10) {
        status = -EINVAL;
        goto thermDeviceIfaceModel10Construct_GPU_GTZ_V10_exit;
    }
    pDeviceGtz = nvgpu_kzalloc(g, size);
    if (pDeviceGtz == NULL) {
        status = -ENOMEM;
        goto thermDeviceIfaceModel10Construct_GPU_GTZ_V10_exit;
    }

    nvgpu_memcpy((u8 *)pDeviceGtz, (u8 *)pArgs, size);
    status = pmu_board_obj_construct_super(g, &pDeviceGtz->super.super.super, pArgs);
    if (status != 0) {
        nvgpu_kfree(g, pDeviceGtz);
        goto thermDeviceIfaceModel10Construct_GPU_GTZ_V10_exit;
    }
    *ppBoardObj = (struct pmu_board_obj *)pDeviceGtz;
    return 0;
thermDeviceIfaceModel10Construct_GPU_GTZ_V10_exit:
    return status;
}
