/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef _THERM_CHANNEL_ESTIMATION_V10_H_
#define _THERM_CHANNEL_ESTIMATION_V10_H_
#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <common/pmu/boardobj/boardobj.h>
#include "ucode_therm_inf.h"
#include <nvgpu/kmem.h>
#include <nvgpu/boardobjgrp_e32.h>
#include "therm_channel.h"
/* ------------------------ Macros ----------------------------------------- */
/* ------------------------ Datatypes -------------------------------------- */
typedef struct therm_channel_estimation_v10 THERM_CHANNEL_ESTIMATION_V10;
/*!
 * Extends THERM_CHANNEL_3X providing attributes specific to
 * THERM_CHANNEL_CLASS_ESTIMATION_V10.
 *
 * ESTIMATION_V10 channel takes temperature from two other Channels,
 * post process it and provides an estimated temperature.
 */
struct therm_channel_estimation_v10
{
    /*!
     * THERM_CHANNEL_3X super class. This should always be
     * the first member!
     */
    struct therm_channel_3x super;
    /*!
     * ESTIMATION_V10 channel fetches temperatures from two channels
     * for estimation
     */
    u32             thermChannelIdx1;
    u32             thermChannelIdx2;
    /*!
     * ESTIMATION_V10 modes of operation identified by
     * NV2080_CTRL_THERMAL_THERM_CHANNEL_ESTIMATION_MODE_<xyz>
     */
    u32             mode;
};
/* ------------------------ Function Prototypes ---------------------------- */
//BoardObjIfaceModel10Construct thermChannelIfaceModel10Construct_ESTIMATION_V10;
s32 thermChannelIfaceModel10Construct_ESTIMATION_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u16 size,
    void *pArgs
);
#endif // _THERM_CHANNEL_ESTIMATION_V10_H_
