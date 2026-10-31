// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include "therm_channel_estimation_v10.h"
s32 thermChannelIfaceModel10Construct_ESTIMATION_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u16 size,
    void *pArgs
)
{
    struct therm_channel_estimation_v10 *pChTemp         = (struct therm_channel_estimation_v10 *) pArgs;
    struct therm_channel_estimation_v10 *pChEstimation10 = NULL;
    s32                     status          = 0;
    if (pArgs == NULL) {
        nvgpu_err(g, "pArgs is NULL - %s", __func__);
        status = -EINVAL;
        goto thermChannelIfaceModel10Construct_ESTIMATION_V10_exit;
    }
    pChEstimation10 = (struct therm_channel_estimation_v10 *) nvgpu_kzalloc(g, size);
    if (pChEstimation10 == NULL) {
        nvgpu_err(g, "pChEstimation10 is NULL - %s", __func__);
        status = -ENOMEM;
        goto thermChannelIfaceModel10Construct_ESTIMATION_V10_exit;
    }
    status = pmu_board_obj_construct_super(g, (struct pmu_board_obj *)(void *)pChEstimation10, pArgs);
    if (status != 0) {
        nvgpu_err(g, "failed to construct estimation channel is %d", status);
        nvgpu_kfree(g, pChEstimation10);
        goto thermChannelIfaceModel10Construct_ESTIMATION_V10_exit;
    }
    *ppBoardObj = (struct pmu_board_obj *)(void *)pChEstimation10;
    // Override Boardobj Model 10 interfaces
    // Override Boardobj Model 10 interfaces
    // pModel10->super.getInfo = _thermChannelIfaceModel10GetInfo_ESTIMATION_V10;
    // pModel10->pmuDataInit   = _thermChannelIfaceModel10PmuDataInit_ESTIMATION_V10;
    // Override super-class interfaces
    // pChEstimation10->super.super.super.stateInit = _thermChannelStateInit_ESTIMATION_V10;
    // Set THERM_CHANNEL_ESTIMATION_V10 specific parameters.
    pChEstimation10->thermChannelIdx1 = pChTemp->thermChannelIdx1;
    pChEstimation10->thermChannelIdx2 = pChTemp->thermChannelIdx2;
    pChEstimation10->mode             = pChTemp->mode;
thermChannelIfaceModel10Construct_ESTIMATION_V10_exit:
    return status;
}
