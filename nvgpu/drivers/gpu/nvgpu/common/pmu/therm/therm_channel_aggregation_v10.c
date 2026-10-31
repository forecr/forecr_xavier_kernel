// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include "therm_channel_aggregation_v10.h"
/*!
 * Construct a new THERM THERM_CHANNEL_AGGREGATION_V10 object.
 *
 * @copydoc BoardObjIfaceModel10Construct
 */
s32 thermChannelIfaceModel10Construct_AGGREGATION_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u16 size,
    void *pArgs
)
{
    struct therm_channel_aggregation_v10 *pChTemp          = (struct therm_channel_aggregation_v10 *) pArgs;
    struct therm_channel_aggregation_v10 *pChAggregation10 = NULL;
    s32                      status           = 0;
    if (pArgs == NULL) {
        nvgpu_err(g, "pArgs is NULL - %s", __func__);
        status = -EINVAL;
        goto thermChannelIfaceModel10Construct_AGGREGATION_V10_exit;
    }
    pChAggregation10 = (struct therm_channel_aggregation_v10 *) nvgpu_kzalloc(g, size);
    if (pChAggregation10 == NULL) {
        nvgpu_err(g, "pChAggregation10 is NULL - %s", __func__);
        status = -ENOMEM;
        goto thermChannelIfaceModel10Construct_AGGREGATION_V10_exit;
    }

    status = pmu_board_obj_construct_super(g, (struct pmu_board_obj *)(void *)pChAggregation10, pArgs);
    if (status != 0) {
        nvgpu_err(g, "failed to construct aggregation channel is %d", status);
        nvgpu_kfree(g, pChAggregation10);
        goto thermChannelIfaceModel10Construct_AGGREGATION_V10_exit;
    }
    *ppBoardObj = (struct pmu_board_obj *)(void *)pChAggregation10;
    // TODO: implement Model 10 interfaces
    // Override Boardobj Model 10 interfaces
    // pModel10->super.getInfo = _thermChannelIfaceModel10GetInfo_AGGREGATION_V10;
    // pModel10->pmuDataInit   = _thermChannelIfaceModel10PmuDataInit_AGGREGATION_V10;
    // Override super-class interfaces
    // pChAggregation10->super.super.super.stateInit = _thermChannelStateInit_AGGREGATION_V10;
    // Set THERM_CHANNEL_AGGREGATION_V10 specific parameters.
    pChAggregation10->thermDevIdx          = pChTemp->thermDevIdx;
    pChAggregation10->thermDevProvIdxStart = pChTemp->thermDevProvIdxStart;
    pChAggregation10->thermDevProvIdxEnd   = pChTemp->thermDevProvIdxEnd;
    pChAggregation10->thermChannelIdxNext  = pChTemp->thermChannelIdxNext;
    pChAggregation10->mode                 = pChTemp->mode;
thermChannelIfaceModel10Construct_AGGREGATION_V10_exit:
    return status;
}
