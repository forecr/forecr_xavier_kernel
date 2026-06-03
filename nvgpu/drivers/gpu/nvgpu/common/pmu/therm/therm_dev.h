/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_THERM_THRMDEV_H
#define NVGPU_THERM_THRMDEV_H

#include <nvgpu/types.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/pmu/pmuif/therm_inf.h>
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GTZ_V10                                     0x14U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_STZ_V10                                     0x15U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_TSENSE_V10                                  0x16U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10                             0x17U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_UNUSED_0                                        0x18U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_GDDRX_SW_V10                                0x19U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_V10                                         0x1AU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_BOARD_I2C_V10                                   0x1BU
//#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_HBM_SW_V10                                  NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_V10
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_TZ_V10                                      0xFCU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_3X                                              0xFDU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MODEL_10                                        0xFEU

struct nvgpu_pmu_therm;

/* Forward declarations */
struct therm_devices;
struct therm_device;
struct therm_device_3x;
struct THERM_DEVICES_3X;
struct THERM_DEVICE_MODEL_10;
struct NV_PMU_BOARDOBJ_IFACE_MODEL;
struct NV_PMU_BOARDOBJ_IFACE_MODEL_10_THERM;
struct NV2080_CTRL_BOARDOBJ;
struct NV2080_CTRL_BOARDOBJ_INFO_IFACE_MODEL_10;
struct nv_pmu_boardobj;
struct NV_PMU_BOARDOBJ_STATUS;

/* Typedefs for backward compatibility - only define if not already defined */
#ifndef THERM_DEVICES_TYPEDEF
#define THERM_DEVICES_TYPEDEF
typedef struct therm_devices THERM_DEVICES;
#endif

#ifndef THERM_DEVICE_TYPEDEF  
#define THERM_DEVICE_TYPEDEF
typedef struct therm_device THERM_DEVICE;
#endif

#ifndef THERM_DEVICE_3X_TYPEDEF
#define THERM_DEVICE_3X_TYPEDEF
typedef struct therm_device_3x THERM_DEVICE_3X;
#endif

#ifndef THERM_DEVICES_3X_TYPEDEF
#define THERM_DEVICES_3X_TYPEDEF
typedef struct THERM_DEVICES_3X THERM_DEVICES_3X;
#endif

#ifndef THERM_DEVICE_MODEL_10_TYPEDEF
#define THERM_DEVICE_MODEL_10_TYPEDEF
typedef struct THERM_DEVICE_MODEL_10 THERM_DEVICE_MODEL_10;
#endif

/* Thermal device base structure */
struct therm_device {
	struct pmu_board_obj super;
	u8 numProv;       /* Number of providers */
	u8 idx;           /* Device index */
	s32 (*stateInit)(struct gk20a *g, struct therm_device *pDevice);
	u8 (*provNumGet)(struct gk20a *g, struct therm_device *pDevice);
};

//typedef S32 ThermDevicesStateInit(struct gk20a *g, struct therm_device *pDevice, struct THERM_DEVICES *pDevices);

struct THERM_DEVICES
{
    struct boardobjgrp_e32 super;
	// TODO: Add stateInit function pointer if necessary. Remove if no longer needed.
    //ThermDevicesStateInit *stateInit;
};

struct THERM_DEVICES_MODEL_10
{

    struct THERM_DEVICES              super;
    struct boardobjgrp_iface_model_10 model10;
};

struct THERM_DEVICES_3X
{
    //TODO:THERM_DEVICES_MODEL_10 super class.
    // Base class - must be the first member
    struct THERM_DEVICES_MODEL_10 base;
    /*!
     * Specifies the list of enumerants to use when interpreting the
     * device table entries.
     */
    u8   typeHAL;
    /*!
     * HW TempSim Support
     */
    bool bHwTempSim;
    /*!
     * Watchdog timer for Tsense
     */
    bool bWatchdogTimer;
    u16  watchdogTimerMs;
    /*!
     * Consistency checker support and temperature threshold in NvTemp
     */
    bool bConsistencyChecker;
    s32  consistencyCheckerThreshold;
    /*!
     * Mask of active GPCs (0 - Disable, 1 - Enable)
     */
    u32  activeGpcMask;
};

struct therm_devices {
	struct boardobjgrp_e32 super;
    struct THERM_DEVICES_3X devices3x;
};

int therm_device_sw_setup(struct gk20a *g);
int therm_device_pmu_setup(struct gk20a *g);
bool therm_device_idx_is_valid(struct nvgpu_pmu_therm *therm_pmu, u16 idx);

#endif /* NVGPU_THERM_DEV_H */
