/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_THERM_THRMCHANNEL_H
#define NVGPU_THERM_THRMCHANNEL_H
#include <nvgpu/bios.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/boardobjgrp_e255.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <common/pmu/boardobj/boardobj.h>
#include <nvgpu/pmu/therm.h>
#include <nvgpu/pmu/pmuif/therm_inf.h>
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_DEBUG_V10_PROV_LIST__SIZE 0xAU

// Temperature bounds in PMU format (temp_celsius = temp_raw / 256)
#define PMU_TEMP_MIN_40C    (-0x2800)  /* -40°C in PMU format */
#define PMU_TEMP_MAX_150C   (0x9600)   /* 150°C in PMU format */

// Forward declarations
typedef struct therm_channel_3x THERM_CHANNEL_3X;
typedef struct therm_channels_3x THERM_CHANNELS_3X;
typedef struct THERM_CHANNELS_MODEL_10 THERM_CHANNELS_MODEL_10;
typedef struct BOARDOBJGRP_INTERFACE_VIRTUAL_TABLE BOARDOBJGRP_INTERFACE_VIRTUAL_TABLE;
typedef struct BOARDOBJGRP_INTERFACE BOARDOBJGRP_INTERFACE;
typedef struct BOARDOBJGRP_IFACE_MODEL BOARDOBJGRP_IFACE_MODEL;

typedef struct
{
	    bool bSupported;
	    bool bEnabled;
	    s32 targetTemp;
} THERM_CHANNEL_TEMP_SIM;

struct therm_channel {
	struct pmu_board_obj super;
	u8 chType;
	u8 relloc;
	u8 tgtGPU;
	u8 flags;
	bool bScaling;
	s16 scaling;
	s16 offsetSW;
	s16 offsetHW;
	bool bBounds;
	s32 temp_min;
	s32 temp_max;
	THERM_CHANNEL_TEMP_SIM tempSim;
};

struct therm_channels {
	struct boardobjgrp_e255 super;
	// Primary channel index for each NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE
	u8 prichIdx[NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_MAX_COUNT];
};

// Define THERM_CHANNELS_MODEL_10 struct first, before it's used in therm_channel_3x.h
struct THERM_CHANNELS_MODEL_10
{
	struct therm_channels super;
	BOARDOBJGRP_IFACE_MODEL_10 model10;
};

struct therm_channels_3x
{
	THERM_CHANNELS_MODEL_10 super;
	bool bSnapshot;
	u8 typeHAL;
	u16 pmuMonSamplingNormalMs;
	u16 pmuMonSamplingSleepMs;
};

// Define the therm_channel_3x structure
struct therm_channel_3x
{
	struct therm_channel super;  // Should inherit from therm_channel, not THERM_CHANNELS_MODEL_10
	bool bPublicVisibility;
	bool bVisOverrideSupported;
};

struct therm_channel_device {
	struct therm_channel super;
	u8 therm_dev_idx;
	u8 therm_dev_prov_idx;
};

typedef struct u64_aligned32 {
	u32 lo;
	u32 hi;
} u64_aligned32;

struct therm_channel_pmu_board_obj {
	u8 type;
	u64_aligned32 timestamp;
};

struct therm_channel_get_status {
	struct therm_channel_pmu_board_obj super;
	s32 curr_temp;
	// Reserved for padding
	u8 rsvd[0x41];
};

/* PMU temperature data structure - 4 possible 16-byte data slots */
typedef struct therm_channel_get_status_pmu_board_obj {
	union {
	    /* Raw buffer view for debugging */
	    u8 raw_data[64];
	    /* Array of 4 possible 16-byte data slots */
	    struct {
	        u32 type;                    /* Type field (should be 2 for valid data) */
	        u64_aligned32 timestamp;     /* 64-bit timestamp */
	        u16 curr_temp;               /* Temperature value */
	        u8 reserved[2];              /* Reserved/padding */
	    } __attribute__((packed)) data_slots[4];
	};
} therm_channel_get_status_pmu_board_obj;

/*!
 * Header for @ref RM_PMU_THERM_THERM_CHANNEL_BOARDOBJ_SET.
 */
typedef struct
{
	/*!
	 * Super class. This should always be the first member!
	 */
	struct boardobjgrp_e255 super;
	/*!
	 * When set to NV_TRUE, RM Control NV2080_CTRL_CMD_THERMAL_CHANNEL_GET_STATUS
	 * returns snapshot temperature. Every GET_STATUS will trigger the snapshot
	 * and then return the temperature.
	 *
	 * When set to NV_FALSE, RM Control NV2080_CTRL_CMD_THERMAL_CHANNEL_GET_STATUS
	 * returns current temperature from sensors without taking snapshot.
	 *
	 * This field is used by class type _3X
	 */
	bool bSnapshot;
} RM_PMU_THERM_THERM_CHANNEL_BOARDOBJGRP_SET_HEADER;

/*!
 * Structure for describing a Thermal Channel on the board, as described in the
 * Thermal Channel Table.
 */
typedef struct
{
	/*!
	 * Super class. This should always be the first member!
	 */
	struct nv_pmu_boardobj super;
	/*!
	 * Temperature scaling and offset:
	 *
	 * Scaling and offset feature will be active when bScaling is set to NV_TRUE.
	 * Channel uses NvSFXP8_8 format for scaling and offset.
	 */
	u8      bScaling;   // 4 bytes (bool + padding)
	u32      scaling;    // 4 bytes scaling value
	u32      bBounds;    // 4 bytes (bool + padding)
	u32      temp_min;   // 4 bytes (-40°C = 0xffffd800)
	u32      temp_max;   // 4 bytes (150°C = 0x00009600)
	/*!
	 * Temp Simulation Data Structure
	 */
	u32      tempsim_bSupported;  // 4 bytes (bool + padding)
	u32      tempsim_bEnabled;    // 4 bytes (bool + padding)
	u32      tempsim_targetTemp;  // 4 bytes
	/*!
	 * Channel Type as NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_<xyz>
	 */
	u32      chType;             // 4 bytes (u8 + padding)
	u32      thermDevIdx;        // 4 bytes (u8 + padding)
	u32      thermDevProvIdx;    // 4 bytes (u8 + padding)
} RM_PMU_THERM_THERM_CHANNEL_BOARDOBJ_SET;

/*!
 * Structure of static information specific to THERM_CHANNEL_MODEL_10.
 */
typedef struct
{
	/*!
	 * THERM_CHANEL Super class. This should always be the first member!
	 */
	RM_PMU_THERM_THERM_CHANNEL_BOARDOBJ_SET super;
	/*!
	 * Interface for the 1.0 version of the BOARDOBJ model
	 */
	NV_PMU_BOARDOBJ_IFACE_MODEL_10_THERM          model10;
} NV_PMU_THERM_THERM_CHANNEL_MODEL_10_BOARDOBJ_SET;

/*!
 * Structure of static information specific to THERM_CHANNEL_DEVICE.
 */
typedef struct
{
	/*!
	 * THERM_CHANNEL_MODEL_10 Super class. This should always be
	 * the first member!
	 */
	NV_PMU_THERM_THERM_CHANNEL_MODEL_10_BOARDOBJ_SET super;
	/*!
	 * Index into the Thermal Device Table for the THERM_DEVICE from which this
	 * THERM_CHANNEL should query temperature value.
	 */
	u8    thermDevIdx;
	/*!
	 * Provider index to query temperature value.
	 */
	u8    thermDevProvIdx;
} NV_PMU_THERM_THERM_CHANNEL_DEVICE_BOARDOBJ_SET;

/*!
 * Structure of static information specific to THERM_CHANNEL_3X.
 */
typedef struct
{
	/*!
	 * THERM_CHANNEL_MODEL_10 Super class. This should always be
	 * the first member!
	 */
	NV_PMU_THERM_THERM_CHANNEL_MODEL_10_BOARDOBJ_SET super;
	/*!
	 * Defines whether this channel visible to public In-Band as well as
	 * Out-Of-Band Interfaces.
	 */
	u32 bPublicVisibility;  // 4 bytes (bool + padding)
} RM_PMU_THERM_THERM_CHANNEL_3X_BOARDOBJ_SET;

/*!
 * Structure of static information specific to THERM_CHANNEL_PROVIDER_V10.
 */
typedef struct
{
	/*!
	 * THERM_CHANNEL_3X Super class. This should always be
	 * the first member!
	 */
	RM_PMU_THERM_THERM_CHANNEL_3X_BOARDOBJ_SET super;
	/*!
	 * Public visibility flag
	 */
	u32 bPublicVisibility;           // 4 bytes (bool + padding)
	/*!
	 * Reserved for future use.
	 */
	u32 rsvd[6];
} RM_PMU_THERM_THERM_CHANNEL_PROVIDER_V10_BOARDOBJ_SET;

typedef struct NV2080_CTRL_THERMAL_CHANNEL_INFO {
	/*!
	 * NV2080_CTRL_BOARDOBJ super class.  Must always be first object in
	 * structure.
	 */
	u8 type;
	/*!
	 * @ref NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_<xyz>.
	 */
	u8                 chType;
	/*!
	 * @ref NV2080_CTRL_THERMAL_THERM_CHANNEL_REL_LOC_<xyz>.
	 */
	u8                 relLoc;
	/*!
	 * @ref NV2080_CTRL_THERMAL_THERM_CHANNEL_TGT_<xyz>.
	 */
	u8                 tgtGPU;
	/*!
	 * Temperature Bounds:
	 *
	 * Bounding feature will be active when bBounds is set to NV_TRUE.
	 * minTemp acts as FLOOR for channel temperature.
	 * maxTemp acts as CEIL for channel temperature.
	 */
	u8               bBounds;
	s32               minTemp;
	s32               maxTemp;
	/*!
	 * Temperature scaling and offset:
	 *
	 * Scaling and offset feature will be active when bScaling is set to NV_TRUE.
	 * Channel uses SFXP24_8 format for scaling and NvTemp format for offset.
	 */
	u8                 bScaling;
	s32                scaling;
	s32                offsetSw;
	/*!
	 * HW Temperature offset.
	 */
	s32                offsetHw;
	/*!
	 * Temp sim capability of the channel.
	 */
	u8               bIsTempSimSupported;
	/*!
	 * Temp sim capability of the channel.
	 */
	u8               bIsTempSimEnabled;
	/*!
	 * @ref NV2080_CTRL_THERMAL_THERM_CHANNEL_FLAGS_CHANNEL_<xyz>.
	 */
	u8                 flags;
	/*!
	 * Type specific information.
	 *
	 * @ref super.type
	 * @ref NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_<xyz>.
	 */
	u32 rsvd[1];
} NV2080_CTRL_THERMAL_CHANNEL_INFO;


struct raw_thermal_entry {
	u32 type;         /* 0x00: Channel type (02=provider, 03=debug) */
	u8  padding1[8];  /* 0x04-0x0B: Various fields we don't need */
	u32 temp_value;   /* 0x0C: Temperature value in PMU format */
	u8  padding2[48]; /* 0x10-0x3F: Rest of 64-byte entry */
} __packed;

struct debug_channel_raw_temp {
	u32 type;         /* 0x00: Channel type (should be 0x03) */
	u32 reserved1;    /* 0x04: Reserved */
	u64 timestamp;    /* 0x08: Timestamp */
	u32 reserved2;    /* 0x10: Reserved */
	u32 flags;        /* 0x14: Flags field */
	u32 temp_array[8]; /* 0x18-0x37: Array of temperature values */
	u8  padding[8];   /* 0x38-0x3F: Padding to 64 bytes */
} __packed;

/* Extended header structure to account for the full buffer header */
struct thermal_pmu_buffer_header {
	struct nv_pmu_boardobjgrp_super super;  /* Standard PMU boardobj header */
	u8  reserved[42];                       /* Padding to reach 0x30 where thermal data starts */
} __packed;

/* Structure to represent thermal channel data in the PMU buffer */
struct thermal_channel_pmu_entry {
	u32 type;                    /* offset 0x00: Channel type (02=provider, 03=aggregation/debug) */
	u32 reserved1;               /* offset 0x04: Reserved/padding */
	u64 timestamp;               /* offset 0x08: PMU timestamp */
	u32 temp_value;              /* offset 0x10: Temperature value (PMU format) */
	u32 reserved2;               /* offset 0x14: Reserved/padding */
	u8  padding[48];             /* offset 0x18: Padding to make total size 64 bytes */
} __packed;

s32 thermChannelIfaceModel10PmuDataInit_SUPER
(
	struct gk20a *g,
	struct pmu_board_obj *obj,
	struct nv_pmu_boardobj *pmu_obj
);
s32 boardObjIfaceModel10PmuDataInit_SUPER
(
	struct gk20a *g,
	struct pmu_board_obj *obj,
	struct nv_pmu_boardobj *pmu_obj
);
int thermChannelIfaceModel10PmuDataInit_3X(struct gk20a *g,
			struct pmu_board_obj *obj,
			struct nv_pmu_boardobj *pmu_obj);
s32 thermChannelIfaceModel10
(
	struct gk20a *g,
	struct pmu_board_obj *obj,
	struct nv_pmu_boardobj *pmu_obj
);
struct therm_channel *construct_channel_device(struct gk20a *g,
			void *pargs, size_t pargs_size, u8 class_id);
int therm_channel_sw_setup(struct gk20a *g);
int therm_channel_pmu_setup(struct gk20a *g);
/* Add function prototype for thermal HAL ops initialization */
void therm_channel_init_ops(struct gk20a *g);
s32 thermChannelIfaceModel10Construct_3X(struct gk20a *g,
		struct pmu_board_obj **ppboardobj, size_t size, void *pArgs);
#endif /* NVGPU_THERM_THRMCHANNEL_H */
