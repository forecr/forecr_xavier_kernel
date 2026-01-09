// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include "therm_channel_debug_v10.h"
int thermChannelIfaceModel10Construct_DEBUG_V10(struct gk20a *g, struct pmu_board_obj **ppBoardObj, u16 size, void *pArgs)
{
    int status = 0;
    struct therm_channel_debug_v10 *pChTemp = (struct therm_channel_debug_v10 *) pArgs;
    struct therm_channel_debug_v10 *pChDebug10 = NULL;
    if (pArgs == NULL) {
        nvgpu_err(g, "pArgs is NULL - %s", __func__);
        status = -EINVAL;
        goto thermChannelIfaceModel10Construct_DEBUG_V10_exit;
    }
    // Allocate memory for the THERM_CHANNEL_DEBUG_V10 object
    pChDebug10 = (struct therm_channel_debug_v10 *) nvgpu_kzalloc(g, size);
    if (pChDebug10 == NULL) {
        nvgpu_err(g, "pChDebug10 is NULL - %s", __func__);
        status = -ENOMEM;
        goto thermChannelIfaceModel10Construct_DEBUG_V10_exit;
    }
    status = pmu_board_obj_construct_super(g, (struct pmu_board_obj *)(void *)pChDebug10, pArgs);
    if (status != 0) {
        nvgpu_err(g, "failed to construct debug channel is %d", status);
        nvgpu_kfree(g, pChDebug10);
        *ppBoardObj = NULL;
        goto thermChannelIfaceModel10Construct_DEBUG_V10_exit;
    }
    *ppBoardObj = (struct pmu_board_obj *)(void *)pChDebug10;
    // TODO: implement Model 10 interfaces
    // Override Boardobj Model 10 interfaces
    // pModel10->super.getInfo = _thermChannelIfaceModel10GetInfo_DEBUG_V10;
    // pModel10->pmuDataInit   = _thermChannelIfaceModel10PmuDataInit_DEBUG_V10;
    // pModel10->copyState     = _thermChannelIfaceModel10CopyState_DEBUG_V10;
    // Override super-class interfaces
    // pChDebug10->super.super.super.stateInit = _thermChannelStateInit_DEBUG_V10;
    // Set THERM_CHANNEL_DEBUG_V10 specific parameters.
    pChDebug10->thermDevIdx = pChTemp->thermDevIdx;
    pChDebug10->thermDevProvIdxStart = pChTemp->thermDevProvIdxStart;
    pChDebug10->thermDevProvIdxEnd = pChTemp->thermDevProvIdxEnd;

thermChannelIfaceModel10Construct_DEBUG_V10_exit:
    return status;
}
