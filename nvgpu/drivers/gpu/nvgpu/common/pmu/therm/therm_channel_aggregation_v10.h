/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef _THERM_CHANNEL_AGGREGATION_V10_H_
#define _THERM_CHANNEL_AGGREGATION_V10_H_
// Include base headers first
#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <common/pmu/boardobj/boardobj.h>
#include "ucode_therm_inf.h"
#include <nvgpu/kmem.h>
#include <nvgpu/boardobjgrp_e32.h>
#include "therm_channel.h"
typedef struct therm_channel_aggregation_v10 THERM_CHANNEL_AGGREGATION_V10;

/*!
 * Extends THERM_CHANNEL_3X providing attributes specific to
 * THERM_CHANNEL_CLASS_AGGREGATION_V10.
 *
 * AGGREGATION_V10 channel takes temperature from multiple adjacent
 * providers of one or more thermal devices and finds max or average
 * of these temperatures.
 */
struct therm_channel_aggregation_v10
{
    /*!
     * THERM_CHANNEL_3X super class. This should always be
     * the first member!
     */
    struct therm_channel_3x super;

    /*!
     * Thermal Device index to query temperature value.
     */
    u8             thermDevIdx;

    /*!
     * AGGREGATION_V10 channel fetches temperatures from a list of consecutive
     * providers. List is specified by Provider IDX start and Provider IDX end.
     */
    u8             thermDevProvIdxStart;
    u8             thermDevProvIdxEnd;

    /*!
     * Index pointing to next aggregation channel.
     */
    u8             thermChannelIdxNext;

    /*!
     * AGGREGATION_V10 modes of operation identified by
     * NV2080_CTRL_THERMAL_THERM_CHANNEL_AGGREGATION_MODE_<xyz>
     *
     * All channels linked through "thermChannelIdxNext" should have same
     * operation mode.
     */
    u8             mode;
};

s32 thermChannelIfaceModel10Construct_AGGREGATION_V10(struct gk20a *g, struct pmu_board_obj **ppBoardObj, u16 size, void *pArgs);

#endif // _THERM_CHANNEL_AGGREGATION_V10_H_
