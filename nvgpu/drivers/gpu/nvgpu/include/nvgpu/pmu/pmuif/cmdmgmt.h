/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMUIF_CMDMGMT_H
#define NVGPU_PMUIF_CMDMGMT_H

#include <nvgpu/types.h>
#include "cmn.h"

/* PMU RPC ID handled by PMU CMDMGMT unit */
#define NV_PMU_RPC_ID_CMDMGMT_DRIVER_REGISTER_EVENT     0x02

#define NV_PMU_DRIVER_REGISTER_EVENT_REGISTER_START         0x00
#define NV_PMU_DRIVER_REGISTER_EVENT_REGISTERING_FAILED     0x01
#define NV_PMU_DRIVER_REGISTER_EVENT_REGISTER_COMPLETE      0x02
#define NV_PMU_DRIVER_REGISTER_EVENT_UNREGISTER_START       0x03
#define NV_PMU_DRIVER_REGISTER_EVENT_UNREGISTER_COMPLETE    0x04

/*!
 * Data layout of NV2080_CTRL_BOARDOBJGRP_MASK child class
 * capable of storing 255 bits indexed between 0..254.
 * NV2080_CTRL_BOARDOBJGRP_MASK_E255
 */
typedef struct {
	u32 super;
	u32 pData[7];
} pmu_boardobjgrp_classes_mask;

/*!
 * The payload for a DRIVER_REGISTER_EVENT RPC with
 * registerEvent field set to NV_PMU_DRIVER_REGISTER_EVENT_REGISTER_COMPLETE
 */
typedef struct {
    /*!
     * Mask of BOARDOBJGRP classes that have had their data populated
     */
	pmu_boardobjgrp_classes_mask populatedBoardObjGrpClasses;
} pmu_driver_register_event_payload_register_complete;

/*!
 * The payload for any type of DRIVER_REGISTER_EVENT RPC
 */
typedef union {
	pmu_driver_register_event_payload_register_complete registerComplete;
} pmu_driver_register_event_payload;

/*!
 * Defines the structure that holds data used to execute DRIVER_REGISTER_EVENT RPC.
 */
struct pmu_rpc_struct_driver_register_event {
    /*!
     * [IN/OUT] Must be first field in RPC structure.
     */
	struct nv_pmu_rpc_header hdr;
    /*!
     * [IN] Enumeration value for the event to trigger.
     */
	u8 registerEvent;
    /*!
     * [IN] Event-specific payload data.
     */
	pmu_driver_register_event_payload payload;
    /*!
     * [NONE] Must be last field in RPC structure.
     * Used as variable size scrach space on NVGPU managed DMEM heap for this RPC.
     */
	u32 scratch[1];
};

/* PMU Driver Register Event type */
enum driver_register_type {
	register_start = 0,
	registering_failed,
	register_complete,
	unregister_start,
	unregister_complete
};

#endif /* NVGPU_PMUIF_CMDMGMT_H */
