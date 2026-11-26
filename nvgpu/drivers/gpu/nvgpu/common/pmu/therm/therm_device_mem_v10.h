/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _THERM_DEVICE_MEM_V10_H_
#define _THERM_DEVICE_MEM_V10_H_
//#include <stdint.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bios.h>             // bios_bit struct and others...
#include <nvgpu/string.h>           // for nvgpu_memcpy.
#include <nvgpu/pmu/super_surface.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <common/pmu/boardobj/boardobj.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/boardobjgrpmask.h>
#include <nvgpu/pmu/therm.h>
#include "thrm.h"
#include "therm_device_3x.h"
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include "common/pmu/clk/ucode_clk_inf.h"
#include "common/pmu/clk/clk_vin.h"
#include "common/pmu/clk/clk.h"
#include "include/nvgpu/list.h"
#include "common/pmu/super_surface/super_surface_priv.h"
/* ------------------------ Includes --------------------------------------- */
/* ------------------------ Macros ----------------------------------------- */
/* ------------------------ Datatypes -------------------------------------- */
typedef struct THERM_DEVICE_MEM_V10 THERM_DEVICE_MEM_V10;
struct THERM_DEVICE_MEM_V10 {
    /*!
     * THERM_DEVICE_3X super class. This should always be the first member!
     */
    THERM_DEVICE_3X super;
    /*!
     * Sensor type NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_<xyz>
     */
    u8  sensorType;
    /*!
     * Worst case sensor refresh period in milli-seconds
     */
    u16 refreshPeriodLimitMs;
};
/* ------------------------ Function Prototypes ---------------------------- */
//BoardObjIfaceModel10Construct thermDeviceIfaceModel10Construct_MEM_V10;
s32 thermDeviceIfaceModel10Construct_MEM_V10
(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pArgs
);
s32 thermDeviceConstructSanityChecks_MEM_V10
(
    struct gk20a *g,
    THERM_DEVICE *pDevice
);
#endif // _THERM_DEVICE_MEM_V10_H_
