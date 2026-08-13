// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include "therm_channel_provider_v10.h"

s32 thermChannelIfaceModel10Construct_PROVIDER_V10
(
    struct gk20a     *g,
    struct pmu_board_obj  **ppBoardObj,
    u16        size,
    void       *pArgs
)
{
    s32 status        = 0;
    struct therm_channel_provider_v10 *pChTemp = (struct therm_channel_provider_v10 *) pArgs;
    struct therm_channel_provider_v10 *pChProvider10 = NULL;

    if (pArgs == NULL) {
        status = -EINVAL;
        goto thermChannelIfaceModel10Construct_PROVIDER_V10_exit;
    }
    pChProvider10 = (struct therm_channel_provider_v10 *) nvgpu_kzalloc(g, size);
    if (pChProvider10 == NULL) {
        status = -ENOMEM;
        goto thermChannelIfaceModel10Construct_PROVIDER_V10_exit;
    }
    status = pmu_board_obj_construct_super(g, (struct pmu_board_obj *)(void *)pChProvider10, pArgs);
    if (status != 0) {
        nvgpu_err(g, "failed to construct provider channel is %d", status);
        nvgpu_kfree(g, pChProvider10);
        *ppBoardObj = NULL;
        goto thermChannelIfaceModel10Construct_PROVIDER_V10_exit;
    }

    *ppBoardObj = (struct pmu_board_obj *)(void *)pChProvider10;
    //TODO:Implement API for GET IFACE_MODEL_10 pointer from BOARDOBJ
    //pModel10 = boardObjIfaceModel10FromBoardObjGet(
    //                    &pChProvider10->super.super.super.super);
    // Override Boardobj Model 10 interfaces
    //pModel10->super.getInfo = _thermChannelIfaceModel10GetInfo_PROVIDER_V10;
    //pModel10->pmuDataInit   = _thermChannelIfaceModel10PmuDataInit_PROVIDER_V10;

    // CRITICAL FIX: Copy base thermal channel data from input
    pChProvider10->super.super.bScaling            = pChTemp->super.super.bScaling;
    pChProvider10->super.super.scaling             = pChTemp->super.super.scaling;
    pChProvider10->super.super.offsetSW            = pChTemp->super.super.offsetSW;
    pChProvider10->super.super.bBounds             = pChTemp->super.super.bBounds;
    pChProvider10->super.super.temp_min            = pChTemp->super.super.temp_min;
    pChProvider10->super.super.temp_max            = pChTemp->super.super.temp_max;
    pChProvider10->super.super.tempSim.bSupported  = pChTemp->super.super.tempSim.bSupported;
    pChProvider10->super.super.tempSim.bEnabled    = pChTemp->super.super.tempSim.bEnabled;
    pChProvider10->super.super.tempSim.targetTemp  = pChTemp->super.super.tempSim.targetTemp;
    pChProvider10->super.super.chType              = pChTemp->super.super.chType;

    // Set THERM_CHANNEL_PROVIDER_V10 specific parameters.
    pChProvider10->thermDevIdx = pChTemp->thermDevIdx;
    pChProvider10->thermDevProvIdx = pChTemp->thermDevProvIdx;
    pChProvider10->super.bPublicVisibility = pChTemp->super.bPublicVisibility;
    pChProvider10->super.bVisOverrideSupported = pChTemp->super.bVisOverrideSupported;

thermChannelIfaceModel10Construct_PROVIDER_V10_exit:

    return status;
}
