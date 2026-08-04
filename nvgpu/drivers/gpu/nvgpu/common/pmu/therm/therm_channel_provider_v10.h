/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef _THERM_CHANNEL_PROVIDER_V10_H_
#define _THERM_CHANNEL_PROVIDER_V10_H_
// Include base headers first
#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <common/pmu/boardobj/boardobj.h>
#include "ucode_therm_inf.h"
#include <nvgpu/kmem.h>
#include <nvgpu/boardobjgrp_e32.h>
#include "therm_channel.h"
// Forward declarations for structures
struct gk20a;
struct pmu_board_obj;
struct therm_device;
// Forward declare the provider structure
typedef struct therm_channel_provider_v10 THERM_CHANNEL_PROVIDER_V10;
// Define a simplified provider structure to break circular dependencies
struct therm_channel_provider_v10
{
    struct therm_channel_3x  super;
    // Thermal Device pointer to query temperature value
    u8  thermDevIdx;
    // Thermal Device provider index to query temperature value
    u8 thermDevProvIdx;
};

// Function prototypes
s32 thermChannelIfaceModel10Construct_PROVIDER_V10(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u16 size,
    void *pArgs
);
// TODO:Necessary macros
//#define thermChannelTempValueGet_PROVIDER_V10(_pChannel, _pNvTe(thermChannelTempValueGetSnapshot_PROVIDER_V10((_pChannel), NV_FALSE, (_pNvTemp)))
#endif // _THERM_CHANNEL_PROVIDER_V10_H_
