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
#include "ucode_therm_inf.h"

// Forward declarations
typedef struct therm_channel_3x THERM_CHANNEL_3X;
// Typedefs for convenience
typedef struct therm_channels_3x THERM_CHANNELS_3X;
typedef struct THERM_CHANNELS_MODEL_10 THERM_CHANNELS_MODEL_10;
typedef struct BOARDOBJGRP_INTERFACE_VIRTUAL_TABLE BOARDOBJGRP_INTERFACE_VIRTUAL_TABLE;
typedef struct BOARDOBJGRP_INTERFACE BOARDOBJGRP_INTERFACE;
typedef struct BOARDOBJGRP_IFACE_MODEL BOARDOBJGRP_IFACE_MODEL;
typedef struct BOARDOBJGRP_IFACE_MODEL_10 BOARDOBJGRP_IFACE_MODEL_10;

typedef struct
{
        bool  bSupported;
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
	//TODO: add the necessary function pointer interfaces such as temValueGet, SimIsSupport and etc with MODEL 10.
};
struct therm_channels {
	struct boardobjgrp_e255 super;
	// Primary channel index for each NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE
	u8 prichIdx[NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_MAX_COUNT];
};

struct BOARDOBJGRP_INTERFACE_VIRTUAL_TABLE
{
    s16   offset;
};

struct BOARDOBJGRP_INTERFACE
{
    const BOARDOBJGRP_INTERFACE_VIRTUAL_TABLE *pVirtualTable;
};

struct BOARDOBJGRP_IFACE_MODEL
{
    /*!
     * Super class. Must be first member.
     */
    struct BOARDOBJGRP_INTERFACE super;


    //TODO implement the necessary APIs only for MODEL 10
    //BoardObjGrpIfaceModelGetInfo                *getInfo;
    //BoardObjGrpIfaceModelGetClientInfo          *getClientInfo;
    //BoardObjGrpIfaceModelGetControl             *getControl;
    //BoardObjGrpIfaceModelGetClientControl       *getClientControl;
    //BoardObjGrpIfaceModelSetControl             *setControl;
    //BoardObjGrpIfaceModelSetClientControl       *setClientControl;
    //BoardObjGrpIfaceModelGetStatus              *getStatus;
    //BoardObjGrpIfaceModelGetClientStatus        *getClientStatus;
    //BoardObjGrpIfaceModelGetReadings            *getReadings;
    //BoardObjGrpIfaceModelGetClientReadings      *getClientReadings;

    // Virtual RMCTRL interfaces which client *MUST* implement.
    //BoardObjGrpIfaceModelInfoInstGet            *infoInstGet;
    //BoardObjGrpIfaceModelClientInfoInstGet      *clientInfoInstGet;
    //BoardObjGrpIfaceModelStatusInstGet          *statusInstGet;
    //BoardObjGrpIfaceModelClientStatusInstGet    *clientStatusInstGet;
    //BoardObjGrpIfaceModelControlInstGet         *controlInstGet;
    //BoardObjGrpIfaceModelClientControlInstGet   *clientControlInstGet;
    //BoardObjGrpIfaceModelReadingsInstGet        *readingsInstGet;
    //BoardObjGrpIfaceModelClientReadingsInstGet  *clientReadingsInstGet;
};


struct BOARDOBJGRP_IFACE_MODEL_10
{

    BOARDOBJGRP_IFACE_MODEL super;
    bool                              bCopyIn;
    bool                              bHdrExport;

    /*!
     * Information describing the PMU interfaces for this BOARDOBJGRP.
     */
    //BOARDOBJGRP_IFACE_MODEL_10_PMU                      pmu;

    // PMU interfaces
    //BoardObjGrpIfaceModel10PmuInitHandle           *pmuInitHandle;
    //BoardObjGrpIfaceModel10PmuDeinitHandle         *pmuDeinitHandle;
    //BoardObjGrpIfaceModel10PmuHdrDataInit          *pmuHdrDataInit;
    //BoardObjGrpIfaceModel10PmuDataInit             *pmuDataInit;
    //BoardObjGrpIfaceModel10PmuStatusInit           *pmuStatusInit;
    //BoardObjGrpIfaceModel10PmuSet                  *pmuSet;
    //BoardObjGrpIfaceModel10PmuGetStatus            *pmuGetStatus;

    // RMCTRL interfaces
    //BoardObjGrpIfaceModel10CopyStatus               *copyStatus;

    // Virtual RMCTRL interfaces which client *MUST* implement.
    //BoardObjGrpIfaceModel10PmuDataInstGet          *pmuDataInstGet;
    //BoardObjGrpIfaceModel10PmuStatusInstGet        *pmuStatusInstGet;
    //BoardObjGrpIfaceModel10HdrDataExport           *hdrDataExport;
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
    struct THERM_CHANNELS_MODEL_10 super;
    bool bPublicVisibility;
    bool bVisOverrideSupported;
};

struct therm_channel_device {
	struct therm_channel super;
	u8 therm_dev_idx;
	u8 therm_dev_prov_idx;
};

struct therm_channel_get_status {
	struct pmu_board_obj super;
	u32 curr_temp;
};

int therm_channel_sw_setup(struct gk20a *g);
int therm_channel_pmu_setup(struct gk20a *g);

#endif /* NVGPU_THERM_THRMCHANNEL_H */
