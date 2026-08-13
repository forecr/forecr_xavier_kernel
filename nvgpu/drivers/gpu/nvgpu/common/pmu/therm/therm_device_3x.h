/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_THERM_DEV_3x_H
#define NVGPU_THERM_DEV_3x_H
//#include <stdint.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <common/pmu/boardobj/boardobj.h>
/* Forward declarations of types */
struct therm_device_3x;
struct therm_device;
/* Define thermal device type constants */
#ifndef NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_NOT_ASSIGNED
#define NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_NOT_ASSIGNED 0
#endif
#ifndef NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_GPU_OFFSET_MAX
#define NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_GPU_OFFSET_MAX 1
#endif
#ifndef NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_GPU_AVG
#define NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_GPU_AVG 2
#endif
/* Base thermal device structure */
struct therm_device_3x {
	struct pmu_board_obj super;
	u8 devType;
	bool bI2csExposure;
	u8 provMapping;  /* Provider mapping ID */
	u8 numProv;  /* Device Provider mapping HAL */
	u32 provSupportMask;
	s32 errorTemp;
};
/* Type definition for backward compatibility - only define if not already defined */
#ifndef THERM_DEVICE_3X_TYPEDEF
#define THERM_DEVICE_3X_TYPEDEF
typedef struct therm_device_3x THERM_DEVICE_3X;
#endif
/* Function Prototypes */
/**
 * Base constructor for thermal device types
 */
s32 thermDeviceIfaceModel10Construct_3X(
    struct gk20a *g,
    struct pmu_board_obj **ppBoardObj,
    u32 size,
    void *pargs
);
/**
 * Initialize a THERM_DEVICE_3X structure
 *
 * @param g        Pointer to GPU object
 * @param pDevice  Pointer to the device to initialize
 *
 * @return 0 on success, error code otherwise
 */
s32 therm_device_3x_init(struct gk20a *g, struct therm_device_3x *pDevice);
/**
 * Convert device type to string representation
 *
 * @param devType  Device type
 *
 * @return String representation of the device type
 */
const char *therm_device_3x_type_to_string(u8 devType);
/**
 * Validate device parameters
 *
 * @param g        Pointer to GPU object
 * @param pDevice  Pointer to the device to validate
 *
 * @return 0 on success, error code otherwise
 */
s32 therm_device_3x_validate(struct gk20a *g, struct therm_device_3x *pDevice);
#endif /* NVGPU_THERM_DEV_3x_H */