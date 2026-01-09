/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef _THERM_CHANNEL_DEBUG_V10_H_
#define _THERM_CHANNEL_DEBUG_V10_H_
#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <common/pmu/boardobj/boardobj.h>
#include "ucode_therm_inf.h"
#include <nvgpu/kmem.h>
#include <nvgpu/boardobjgrp_e32.h>
#include "therm_channel.h"
#include "therm_dev.h"
#include "thrm.h"
typedef struct therm_channel_debug_v10 THERM_CHANNEL_DEBUG_V10;
struct therm_channel_debug_v10
{
    struct therm_channel_3x super;
    //struct boardobjgrp_e32 super;
    /*!
     * Thermal Device pointer to query temperature value.
     */
    u8             thermDevIdx;
    /*!
     * DEBUG_V10 channel fetches temperatures from a list of consecutive
     * providers. List is specified by Provider IDX start and Provider IDX end.
     */
    u8             thermDevProvIdxStart;
    u8             thermDevProvIdxEnd;
};
//TODO: Implement BoardObjIfaceModel10Construct thermChannelIfaceModel10Construct_DEBUG_V10;
s32 thermChannelIfaceModel10Construct_DEBUG_V10(struct gk20a *g, struct pmu_board_obj **ppBoardObj, u16 size, void *pArgs);
#endif
