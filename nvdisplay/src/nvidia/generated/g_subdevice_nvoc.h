
#ifndef _G_SUBDEVICE_NVOC_H_
#define _G_SUBDEVICE_NVOC_H_

// Version of generated metadata structures
#ifdef NVOC_METADATA_VERSION
#undef NVOC_METADATA_VERSION
#endif
#define NVOC_METADATA_VERSION 2

#include "nvoc/runtime.h"
#include "nvoc/rtti.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SPDX-FileCopyrightText: Copyright (c) 1993-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#pragma once
#include "g_subdevice_nvoc.h"

#ifndef _SUBDEVICE_H_
#define _SUBDEVICE_H_

#include "resserv/resserv.h"
#include "nvoc/prelude.h"
#include "resserv/rs_resource.h"
#include "gpu/gpu_resource.h"
#include "rmapi/event.h"
#include "containers/btree.h"
#include "nvoc/utility.h"
#include "gpu/gpu_halspec.h"

#include "class/cl2080.h"
#include "ctrl/ctrl2080.h" // rmcontrol parameters

typedef struct TMR_EVENT TMR_EVENT;


struct Device;

#ifndef __NVOC_CLASS_Device_TYPEDEF__
#define __NVOC_CLASS_Device_TYPEDEF__
typedef struct Device Device;
#endif /* __NVOC_CLASS_Device_TYPEDEF__ */

#ifndef __nvoc_class_id_Device
#define __nvoc_class_id_Device 0xe0ac20
#endif /* __nvoc_class_id_Device */



struct OBJGPU;

#ifndef __NVOC_CLASS_OBJGPU_TYPEDEF__
#define __NVOC_CLASS_OBJGPU_TYPEDEF__
typedef struct OBJGPU OBJGPU;
#endif /* __NVOC_CLASS_OBJGPU_TYPEDEF__ */

#ifndef __nvoc_class_id_OBJGPU
#define __nvoc_class_id_OBJGPU 0x7ef3cb
#endif /* __nvoc_class_id_OBJGPU */



struct Memory;

#ifndef __NVOC_CLASS_Memory_TYPEDEF__
#define __NVOC_CLASS_Memory_TYPEDEF__
typedef struct Memory Memory;
#endif /* __NVOC_CLASS_Memory_TYPEDEF__ */

#ifndef __nvoc_class_id_Memory
#define __nvoc_class_id_Memory 0x4789f2
#endif /* __nvoc_class_id_Memory */



struct P2PApi;

#ifndef __NVOC_CLASS_P2PApi_TYPEDEF__
#define __NVOC_CLASS_P2PApi_TYPEDEF__
typedef struct P2PApi P2PApi;
#endif /* __NVOC_CLASS_P2PApi_TYPEDEF__ */

#ifndef __nvoc_class_id_P2PApi
#define __nvoc_class_id_P2PApi 0x3982b7
#endif /* __nvoc_class_id_P2PApi */



/**
 * A subdevice represents a single GPU within a device. Subdevice provide
 * unicast semantics; that is, operations involving a subdevice are applied to
 * the associated GPU only.
 */

// Private field names are wrapped in PRIVATE_FIELD, which does nothing for
// the matching C source file, but causes diagnostics to be issued if another
// source file references the field.
#ifdef NVOC_SUBDEVICE_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif


// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__Subdevice;
struct NVOC_METADATA__GpuResource;
struct NVOC_METADATA__Notifier;
struct NVOC_VTABLE__Subdevice;


struct Subdevice {

    // Metadata starts with RTTI structure.
    union {
         const struct NVOC_METADATA__Subdevice *__nvoc_metadata_ptr;
         const struct NVOC_RTTI *__nvoc_rtti;
    };

    // Parent (i.e. superclass or base class) objects
    struct GpuResource __nvoc_base_GpuResource;
    struct Notifier __nvoc_base_Notifier;

    // Ancestor object pointers for `staticCast` feature
    struct Object *__nvoc_pbase_Object;    // obj super^4
    struct RsResource *__nvoc_pbase_RsResource;    // res super^3
    struct RmResourceCommon *__nvoc_pbase_RmResourceCommon;    // rmrescmn super^3
    struct RmResource *__nvoc_pbase_RmResource;    // rmres super^2
    struct GpuResource *__nvoc_pbase_GpuResource;    // gpures super
    struct INotifier *__nvoc_pbase_INotifier;    // inotify super^2
    struct Notifier *__nvoc_pbase_Notifier;    // notify super
    struct Subdevice *__nvoc_pbase_Subdevice;    // subdevice

    // Vtable with 107 per-object function pointers
    NV_STATUS (*__subdeviceCtrlCmdGpuGetCachedInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *);  // exported (id=0x20800182)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetInfoV2__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *);  // exported (id=0x20800102)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetIpVersion__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_IP_VERSION_PARAMS *);  // exported (id=0x2080014d)
    NV_STATUS (*__subdeviceCtrlCmdGpuSetOptimusInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_OPTIMUS_INFO_PARAMS *);  // exported (id=0x2080014c)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetNameString__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_NAME_STRING_PARAMS *);  // exported (id=0x20800110)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetShortNameString__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_SHORT_NAME_STRING_PARAMS *);  // exported (id=0x20800111)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetSdm__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_SDM_PARAMS *);  // exported (id=0x20800118)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetSimulationInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_SIMULATION_INFO_PARAMS *);  // exported (id=0x20800119)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetEngines__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_ENGINES_PARAMS *);  // exported (id=0x20800123)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetEnginesV2__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_ENGINES_V2_PARAMS *);  // exported (id=0x20800170)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetEngineClasslist__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_ENGINE_CLASSLIST_PARAMS *);  // exported (id=0x20800124)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetEnginePartnerList__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_ENGINE_PARTNERLIST_PARAMS *);  // exported (id=0x20800147)
    NV_STATUS (*__subdeviceCtrlCmdGpuQueryMode__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_QUERY_MODE_PARAMS *);  // exported (id=0x20800128)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetChipDetails__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_CHIP_DETAILS_PARAMS *);  // exported (id=0x208001a4)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetOEMBoardInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_OEM_BOARD_INFO_PARAMS *);  // exported (id=0x2080013f)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetOEMInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_OEM_INFO_PARAMS *);  // exported (id=0x20800169)
    NV_STATUS (*__subdeviceCtrlCmdGpuHandleGpuSR__)(struct Subdevice * /*this*/);  // exported (id=0x20800167)
    NV_STATUS (*__subdeviceCtrlCmdGpuInitializeCtx__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_INITIALIZE_CTX_PARAMS *);  // exported (id=0x2080012d)
    NV_STATUS (*__subdeviceCtrlCmdGpuPromoteCtx__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_PROMOTE_CTX_PARAMS *);  // exported (id=0x2080012b)
    NV_STATUS (*__subdeviceCtrlCmdGpuEvictCtx__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_EVICT_CTX_PARAMS *);  // exported (id=0x2080012c)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetId__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_ID_PARAMS *);  // exported (id=0x20800142)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetGidInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_GID_INFO_PARAMS *);  // exported (id=0x2080014a)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetPids__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_PIDS_PARAMS *);  // exported (id=0x2080018d)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetPidInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_PID_INFO_PARAMS *);  // exported (id=0x2080018e)
    NV_STATUS (*__subdeviceCtrlCmdGpuQueryFunctionStatus__)(struct Subdevice * /*this*/, NV2080_CTRL_CMD_GPU_QUERY_FUNCTION_STATUS_PARAMS *);  // exported (id=0x20800173)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetFirstAsyncCEIdx__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_FIRST_ASYNC_CE_IDX_PARAMS *);  // exported (id=0x208001e6)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetVmmuSegmentSize__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_VMMU_SEGMENT_SIZE_PARAMS *);  // exported (id=0x2080017e)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetMaxSupportedPageSize__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_MAX_SUPPORTED_PAGE_SIZE_PARAMS *);  // exported (id=0x20800188)
    NV_STATUS (*__subdeviceCtrlCmdGpuHandleVfPriFault__)(struct Subdevice * /*this*/, NV2080_CTRL_CMD_GPU_HANDLE_VF_PRI_FAULT_PARAMS *);  // exported (id=0x20800192)
    NV_STATUS (*__subdeviceCtrlCmdValidateMemMapRequest__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_VALIDATE_MEM_MAP_REQUEST_PARAMS *);  // exported (id=0x20800198)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetGfid__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_GFID_PARAMS *);  // exported (id=0x20800196)
    NV_STATUS (*__subdeviceCtrlCmdUpdateGfidP2pCapability__)(struct Subdevice * /*this*/, NV2080_CTRL_CMD_GPU_UPDATE_GFID_P2P_CAPABILITY_PARAMS *);  // exported (id=0x20800197)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetEngineLoadTimes__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_ENGINE_LOAD_TIMES_PARAMS *);  // exported (id=0x2080019b)
    NV_STATUS (*__subdeviceCtrlCmdGpuMarkDeviceForReset__)(struct Subdevice * /*this*/);  // exported (id=0x208001a9)
    NV_STATUS (*__subdeviceCtrlCmdGpuUnmarkDeviceForReset__)(struct Subdevice * /*this*/);  // exported (id=0x208001aa)
    NV_STATUS (*__subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset__)(struct Subdevice * /*this*/);  // exported (id=0x208001ac)
    NV_STATUS (*__subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset__)(struct Subdevice * /*this*/);  // exported (id=0x208001ad)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetResetStatus__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_RESET_STATUS_PARAMS *);  // exported (id=0x208001ab)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetDrainAndResetStatus__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_DRAIN_AND_RESET_STATUS_PARAMS *);  // exported (id=0x208001ae)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetConstructedFalconInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_CONSTRUCTED_FALCON_INFO_PARAMS *);  // exported (id=0x208001b0)
    NV_STATUS (*__subdeviceCtrlGpuGetFipsStatus__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_FIPS_STATUS_PARAMS *);  // exported (id=0x208001e4)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetVfCaps__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_VF_CAPS_PARAMS *);  // exported (id=0x208001b1)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetRecoveryAction__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_GET_RECOVERY_ACTION_PARAMS *);  // exported (id=0x208001b2)
    NV_STATUS (*__subdeviceCtrlCmdGpuRpcGspTest__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_RPC_GSP_TEST_PARAMS *);  // exported (id=0x208001e8)
    NV_STATUS (*__subdeviceCtrlCmdGpuRpcGspQuerySizes__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_RPC_GSP_QUERY_SIZES_PARAMS *);  // exported (id=0x208001e9)
    NV_STATUS (*__subdeviceCtrlCmdRusdGetSupportedFeatures__)(struct Subdevice * /*this*/, NV2080_CTRL_RUSD_GET_SUPPORTED_FEATURES_PARAMS *);  // exported (id=0x208001ea)
    NV_STATUS (*__subdeviceCtrlCmdRusdSetFeatures__)(struct Subdevice * /*this*/, NV2080_CTRL_GPU_RUSD_SET_FEATURES_PARAMS *);  // exported (id=0x208001eb)
    NV_STATUS (*__subdeviceCtrlCmdEventSetTrigger__)(struct Subdevice * /*this*/);  // exported (id=0x20800302)
    NV_STATUS (*__subdeviceCtrlCmdEventSetTriggerFifo__)(struct Subdevice * /*this*/, NV2080_CTRL_EVENT_SET_TRIGGER_FIFO_PARAMS *);  // exported (id=0x20800308)
    NV_STATUS (*__subdeviceCtrlCmdEventSetNotification__)(struct Subdevice * /*this*/, NV2080_CTRL_EVENT_SET_NOTIFICATION_PARAMS *);  // exported (id=0x20800301)
    NV_STATUS (*__subdeviceCtrlCmdEventSetMemoryNotifies__)(struct Subdevice * /*this*/, NV2080_CTRL_EVENT_SET_MEMORY_NOTIFIES_PARAMS *);  // exported (id=0x20800303)
    NV_STATUS (*__subdeviceCtrlCmdEventSetSemaphoreMemory__)(struct Subdevice * /*this*/, NV2080_CTRL_EVENT_SET_SEMAPHORE_MEMORY_PARAMS *);  // exported (id=0x20800304)
    NV_STATUS (*__subdeviceCtrlCmdEventSetSemaMemValidation__)(struct Subdevice * /*this*/, NV2080_CTRL_EVENT_SET_SEMA_MEM_VALIDATION_PARAMS *);  // exported (id=0x20800306)
    NV_STATUS (*__subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf__)(struct Subdevice * /*this*/, NV2080_CTRL_EVENT_RATS_GSP_TRACE_BIND_EVTBUF_PARAMS *);  // exported (id=0x2080030a)
    NV_STATUS (*__subdeviceCtrlCmdTimerCancel__)(struct Subdevice * /*this*/);  // exported (id=0x20800402)
    NV_STATUS (*__subdeviceCtrlCmdTimerSchedule__)(struct Subdevice * /*this*/, NV2080_CTRL_CMD_TIMER_SCHEDULE_PARAMS *);  // exported (id=0x20800401)
    NV_STATUS (*__subdeviceCtrlCmdTimerGetTime__)(struct Subdevice * /*this*/, NV2080_CTRL_TIMER_GET_TIME_PARAMS *);  // exported (id=0x20800403)
    NV_STATUS (*__subdeviceCtrlCmdTimerGetRegisterOffset__)(struct Subdevice * /*this*/, NV2080_CTRL_TIMER_GET_REGISTER_OFFSET_PARAMS *);  // exported (id=0x20800404)
    NV_STATUS (*__subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_TIMER_GET_GPU_CPU_TIME_CORRELATION_INFO_PARAMS *);  // exported (id=0x20800406)
    NV_STATUS (*__subdeviceCtrlCmdEccGetClientExposedCounters__)(struct Subdevice * /*this*/, NV2080_CTRL_ECC_GET_CLIENT_EXPOSED_COUNTERS_PARAMS *);  // exported (id=0x20803400)
    NV_STATUS (*__subdeviceCtrlCmdEccGetVolatileCounts__)(struct Subdevice * /*this*/, NV2080_CTRL_ECC_GET_VOLATILE_COUNTS_PARAMS *);  // exported (id=0x20803401)
    NV_STATUS (*__subdeviceCtrlCmdEccInjectError__)(struct Subdevice * /*this*/, NV2080_CTRL_ECC_INJECT_ERROR_PARAMS *);  // exported (id=0x20803403)
    NV_STATUS (*__subdeviceCtrlCmdEccGetRepairStatus__)(struct Subdevice * /*this*/, NV2080_CTRL_ECC_GET_REPAIR_STATUS_PARAMS *);  // exported (id=0x20803404)
    NV_STATUS (*__subdeviceCtrlCmdEccInjectionSupported__)(struct Subdevice * /*this*/, NV2080_CTRL_ECC_INJECTION_SUPPORTED_PARAMS *);  // exported (id=0x20803405)
    NV_STATUS (*__subdeviceCtrlCmdGspGetFeatures__)(struct Subdevice * /*this*/, NV2080_CTRL_GSP_GET_FEATURES_PARAMS *);  // halified (singleton optimized) exported (id=0x20803601) body
    NV_STATUS (*__subdeviceCtrlCmdGspGetRmHeapStats__)(struct Subdevice * /*this*/, NV2080_CTRL_GSP_GET_RM_HEAP_STATS_PARAMS *);  // exported (id=0x20803602)
    NV_STATUS (*__subdeviceCtrlCmdGpuGetVgpuHeapStats__)(struct Subdevice * /*this*/, NV2080_CTRL_CMD_GSP_GET_VGPU_HEAP_STATS_PARAMS *);  // exported (id=0x20803603)
    NV_STATUS (*__subdeviceCtrlCmdLibosGetHeapStats__)(struct Subdevice * /*this*/, NV2080_CTRL_CMD_GSP_GET_LIBOS_HEAP_STATS_PARAMS *);  // exported (id=0x20803604)
    NV_STATUS (*__subdeviceCtrlCmdOsUnixGc6BlockerRefCnt__)(struct Subdevice * /*this*/, NV2080_CTRL_OS_UNIX_GC6_BLOCKER_REFCNT_PARAMS *);  // exported (id=0x20803d01)
    NV_STATUS (*__subdeviceCtrlCmdOsUnixAllowDisallowGcoff__)(struct Subdevice * /*this*/, NV2080_CTRL_OS_UNIX_ALLOW_DISALLOW_GCOFF_PARAMS *);  // exported (id=0x20803d02)
    NV_STATUS (*__subdeviceCtrlCmdOsUnixAudioDynamicPower__)(struct Subdevice * /*this*/, NV2080_CTRL_OS_UNIX_AUDIO_DYNAMIC_POWER_PARAMS *);  // exported (id=0x20803d03)
    NV_STATUS (*__subdeviceCtrlCmdDisplayGetIpVersion__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_DISPLAY_GET_IP_VERSION_PARAMS *);  // exported (id=0x20800a4b)
    NV_STATUS (*__subdeviceCtrlCmdDisplayGetStaticInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_DISPLAY_GET_STATIC_INFO_PARAMS *);  // exported (id=0x20800a01)
    NV_STATUS (*__subdeviceCtrlCmdDisplaySetChannelPushbuffer__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_DISPLAY_CHANNEL_PUSHBUFFER_PARAMS *);  // exported (id=0x20800a58)
    NV_STATUS (*__subdeviceCtrlCmdDisplayWriteInstMem__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_DISPLAY_WRITE_INST_MEM_PARAMS *);  // exported (id=0x20800a49)
    NV_STATUS (*__subdeviceCtrlCmdDisplaySetupRgLineIntr__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_DISPLAY_SETUP_RG_LINE_INTR_PARAMS *);  // exported (id=0x20800a4d)
    NV_STATUS (*__subdeviceCtrlCmdDisplaySetImportedImpData__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_DISPLAY_SET_IMP_INIT_INFO_PARAMS *);  // exported (id=0x20800a54)
    NV_STATUS (*__subdeviceCtrlCmdDisplayGetDisplayMask__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_DISPLAY_GET_ACTIVE_DISPLAY_DEVICES_PARAMS *);  // exported (id=0x20800a5d)
    NV_STATUS (*__subdeviceCtrlCmdDisplayPinsetsToLockpins__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_DISP_PINSETS_TO_LOCKPINS_PARAMS *);  // exported (id=0x20800adc)
    NV_STATUS (*__subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_DISP_SET_SLI_LINK_GPIO_SW_CONTROL_PARAMS *);  // exported (id=0x20800ade)
    NV_STATUS (*__subdeviceCtrlCmdInternalGpioProgramDirection__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_DIRECTION_PARAMS *);  // exported (id=0x20802300)
    NV_STATUS (*__subdeviceCtrlCmdInternalGpioProgramOutput__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_OUTPUT_PARAMS *);  // exported (id=0x20802301)
    NV_STATUS (*__subdeviceCtrlCmdInternalGpioReadInput__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GPIO_READ_INPUT_PARAMS *);  // exported (id=0x20802302)
    NV_STATUS (*__subdeviceCtrlCmdInternalGpioActivateHwFunction__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GPIO_ACTIVATE_HW_FUNCTION_PARAMS *);  // exported (id=0x20802303)
    NV_STATUS (*__subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated__)(struct Subdevice * /*this*/);  // exported (id=0x20800af0)
    NV_STATUS (*__subdeviceCtrlCmdInternalDisplayPreModeSet__)(struct Subdevice * /*this*/);  // exported (id=0x20800af1)
    NV_STATUS (*__subdeviceCtrlCmdInternalDisplayPostModeSet__)(struct Subdevice * /*this*/);  // exported (id=0x20800af2)
    NV_STATUS (*__subdeviceCtrlCmdInternalGetChipInfo__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GPU_GET_CHIP_INFO_PARAMS *);  // exported (id=0x20800a36)
    NV_STATUS (*__subdeviceCtrlCmdInternalGetUserRegisterAccessMap__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GPU_GET_USER_REGISTER_ACCESS_MAP_PARAMS *);  // exported (id=0x20800a41)
    NV_STATUS (*__subdeviceCtrlCmdInternalGetDeviceInfoTable__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GET_DEVICE_INFO_TABLE_PARAMS *);  // exported (id=0x20800a40)
    NV_STATUS (*__subdeviceCtrlCmdInternalRecoverAllComputeContexts__)(struct Subdevice * /*this*/);  // exported (id=0x20800a4a)
    NV_STATUS (*__subdeviceCtrlCmdInternalGetSmcMode__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GPU_GET_SMC_MODE_PARAMS *);  // exported (id=0x20800a4c)
    NV_STATUS (*__subdeviceCtrlCmdIsEgpuBridge__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GET_EGPU_BRIDGE_INFO_PARAMS *);  // exported (id=0x20800a55)
    NV_STATUS (*__subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GPU_GET_GSP_RM_FREE_HEAP_PARAMS *);  // exported (id=0x20800aeb)
    NV_STATUS (*__subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_VMMU_GET_SPA_FOR_GPA_ENTRIES_PARAMS *);  // exported (id=0x20800a57)
    NV_STATUS (*__subdeviceCtrlCmdInternalGcxEntryPrerequisite__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GCX_ENTRY_PREREQUISITE_PARAMS *);  // exported (id=0x2080a7d7)
    NV_STATUS (*__subdeviceCtrlCmdInternalSetP2pCaps__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_SET_P2P_CAPS_PARAMS *);  // exported (id=0x20800ab5)
    NV_STATUS (*__subdeviceCtrlCmdInternalRemoveP2pCaps__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_REMOVE_P2P_CAPS_PARAMS *);  // exported (id=0x20800ab6)
    NV_STATUS (*__subdeviceCtrlCmdInternalGetPcieP2pCaps__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GET_PCIE_P2P_CAPS_PARAMS *);  // exported (id=0x20800ab8)
    NV_STATUS (*__subdeviceCtrlCmdInternalPostInitBrightcStateLoad__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_INIT_BRIGHTC_STATE_LOAD_PARAMS *);  // exported (id=0x20800ac6)
    NV_STATUS (*__subdeviceCtrlCmdInternalSetStaticEdidData__)(struct Subdevice * /*this*/, NV2080_CTRL_CMD_INTERNAL_SET_STATIC_EDID_DATA_PARAMS *);  // exported (id=0x20800adf)
    NV_STATUS (*__subdeviceCtrlCmdInternalDetectHsVideoBridge__)(struct Subdevice * /*this*/);  // exported (id=0x20800add)
    NV_STATUS (*__subdeviceCtrlCmdInternalInitUserSharedData__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_INIT_USER_SHARED_DATA_PARAMS *);  // exported (id=0x20800afe)
    NV_STATUS (*__subdeviceCtrlCmdInternalUserSharedDataSetDataPoll__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_USER_SHARED_DATA_SET_DATA_POLL_PARAMS *);  // exported (id=0x20800aff)
    NV_STATUS (*__subdeviceCtrlCmdInternalControlGspTrace__)(struct Subdevice * /*this*/, NV2080_CTRL_CMD_INTERNAL_CONTROL_GSP_TRACE_PARAMS *);  // exported (id=0x208001e3)
    NV_STATUS (*__subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_GPU_CLIENT_LOW_POWER_MODE_ENTER_PARAMS *);  // exported (id=0x20800ae9)
    NV_STATUS (*__subdeviceCtrlCmdInternalLogOobXid__)(struct Subdevice * /*this*/, NV2080_CTRL_INTERNAL_LOG_OOB_XID_PARAMS *);  // exported (id=0x20800a56)

    // Data members
    NvU32 deviceInst;
    NvU32 subDeviceInst;
    struct Device *pDevice;
    NvBool bMaxGrTickFreqRequested;
    NvU64 P2PfbMappedBytes;
    NvU32 notifyActions[198];
    NvHandle hNotifierMemory;
    struct Memory *pNotifierMemory;
    NvHandle hSemMemory;
    NvU32 videoStream4KCount;
    NvU32 videoStreamHDCount;
    NvU32 videoStreamSDCount;
    NvU32 videoStreamLinearCount;
    NvU32 ofaCount;
    NvBool bGpuDebugModeEnabled;
    NvBool bRcWatchdogEnableRequested;
    NvBool bRcWatchdogDisableRequested;
    NvBool bRcWatchdogSoftDisableRequested;
    NvBool bReservePerfMon;
    NvU32 perfBoostIndex;
    NvU32 perfBoostHighRefCount;
    NvU32 perfBoostLowRefCount;
    NvBool perfBoostEntryExists;
    NvBool bLockedClockModeRequested;
    NvU32 bNvlinkErrorInjectionModeRequested;
    NvBool bSchedPolicySet;
    NvBool bGcoffDisallowed;
    NvBool bUpdateTGP;
    TMR_EVENT *pTimerEvent;
};


// Vtable with 30 per-class function pointers
struct NVOC_VTABLE__Subdevice {
    void (*__subdevicePreDestruct__)(struct Subdevice * /*this*/);  // virtual override (res) base (gpures)
    NV_STATUS (*__subdeviceInternalControlForward__)(struct Subdevice * /*this*/, NvU32, void *, NvU32);  // virtual override (gpures) base (gpures)
    NV_STATUS (*__subdeviceControl__)(struct Subdevice * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (gpures) base (gpures)
    NV_STATUS (*__subdeviceMap__)(struct Subdevice * /*this*/, struct CALL_CONTEXT *, struct RS_CPU_MAP_PARAMS *, struct RsCpuMapping *);  // virtual inherited (gpures) base (gpures)
    NV_STATUS (*__subdeviceUnmap__)(struct Subdevice * /*this*/, struct CALL_CONTEXT *, struct RsCpuMapping *);  // virtual inherited (gpures) base (gpures)
    NvBool (*__subdeviceShareCallback__)(struct Subdevice * /*this*/, struct RsClient *, struct RsResourceRef *, RS_SHARE_POLICY *);  // virtual inherited (gpures) base (gpures)
    NV_STATUS (*__subdeviceGetRegBaseOffsetAndSize__)(struct Subdevice * /*this*/, struct OBJGPU *, NvU32 *, NvU32 *);  // virtual inherited (gpures) base (gpures)
    NV_STATUS (*__subdeviceGetMapAddrSpace__)(struct Subdevice * /*this*/, struct CALL_CONTEXT *, NvU32, NV_ADDRESS_SPACE *);  // virtual inherited (gpures) base (gpures)
    NvHandle (*__subdeviceGetInternalObjectHandle__)(struct Subdevice * /*this*/);  // virtual inherited (gpures) base (gpures)
    NvBool (*__subdeviceAccessCallback__)(struct Subdevice * /*this*/, struct RsClient *, void *, RsAccessRight);  // virtual inherited (rmres) base (gpures)
    NV_STATUS (*__subdeviceGetMemInterMapParams__)(struct Subdevice * /*this*/, RMRES_MEM_INTER_MAP_PARAMS *);  // virtual inherited (rmres) base (gpures)
    NV_STATUS (*__subdeviceCheckMemInterUnmap__)(struct Subdevice * /*this*/, NvBool);  // virtual inherited (rmres) base (gpures)
    NV_STATUS (*__subdeviceGetMemoryMappingDescriptor__)(struct Subdevice * /*this*/, struct MEMORY_DESCRIPTOR **);  // virtual inherited (rmres) base (gpures)
    NV_STATUS (*__subdeviceControlSerialization_Prologue__)(struct Subdevice * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (rmres) base (gpures)
    void (*__subdeviceControlSerialization_Epilogue__)(struct Subdevice * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (rmres) base (gpures)
    NV_STATUS (*__subdeviceControl_Prologue__)(struct Subdevice * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (rmres) base (gpures)
    void (*__subdeviceControl_Epilogue__)(struct Subdevice * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (rmres) base (gpures)
    NvBool (*__subdeviceCanCopy__)(struct Subdevice * /*this*/);  // virtual inherited (res) base (gpures)
    NV_STATUS (*__subdeviceIsDuplicate__)(struct Subdevice * /*this*/, NvHandle, NvBool *);  // virtual inherited (res) base (gpures)
    NV_STATUS (*__subdeviceControlFilter__)(struct Subdevice * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (res) base (gpures)
    NvBool (*__subdeviceIsPartialUnmapSupported__)(struct Subdevice * /*this*/);  // inline virtual inherited (res) base (gpures) body
    NV_STATUS (*__subdeviceMapTo__)(struct Subdevice * /*this*/, RS_RES_MAP_TO_PARAMS *);  // virtual inherited (res) base (gpures)
    NV_STATUS (*__subdeviceUnmapFrom__)(struct Subdevice * /*this*/, RS_RES_UNMAP_FROM_PARAMS *);  // virtual inherited (res) base (gpures)
    NvU32 (*__subdeviceGetRefCount__)(struct Subdevice * /*this*/);  // virtual inherited (res) base (gpures)
    void (*__subdeviceAddAdditionalDependants__)(struct RsClient *, struct Subdevice * /*this*/, RsResourceRef *);  // virtual inherited (res) base (gpures)
    PEVENTNOTIFICATION * (*__subdeviceGetNotificationListPtr__)(struct Subdevice * /*this*/);  // virtual inherited (notify) base (notify)
    struct NotifShare * (*__subdeviceGetNotificationShare__)(struct Subdevice * /*this*/);  // virtual inherited (notify) base (notify)
    void (*__subdeviceSetNotificationShare__)(struct Subdevice * /*this*/, struct NotifShare *);  // virtual inherited (notify) base (notify)
    NV_STATUS (*__subdeviceUnregisterEvent__)(struct Subdevice * /*this*/, NvHandle, NvHandle, NvHandle, NvHandle);  // virtual inherited (notify) base (notify)
    NV_STATUS (*__subdeviceGetOrAllocNotifShare__)(struct Subdevice * /*this*/, NvHandle, NvHandle, struct NotifShare **);  // virtual inherited (notify) base (notify)
};

// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__Subdevice {
    const struct NVOC_RTTI rtti;
    const struct NVOC_METADATA__GpuResource metadata__GpuResource;
    const struct NVOC_METADATA__Notifier metadata__Notifier;
    const struct NVOC_VTABLE__Subdevice vtable;
};

#ifndef __NVOC_CLASS_Subdevice_TYPEDEF__
#define __NVOC_CLASS_Subdevice_TYPEDEF__
typedef struct Subdevice Subdevice;
#endif /* __NVOC_CLASS_Subdevice_TYPEDEF__ */

#ifndef __nvoc_class_id_Subdevice
#define __nvoc_class_id_Subdevice 0x4b01b3
#endif /* __nvoc_class_id_Subdevice */

// Casting support
extern const struct NVOC_CLASS_DEF __nvoc_class_def_Subdevice;

#define __staticCast_Subdevice(pThis) \
    ((pThis)->__nvoc_pbase_Subdevice)

#ifdef __nvoc_subdevice_h_disabled
#define __dynamicCast_Subdevice(pThis) ((Subdevice*) NULL)
#else //__nvoc_subdevice_h_disabled
#define __dynamicCast_Subdevice(pThis) \
    ((Subdevice*) __nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(Subdevice)))
#endif //__nvoc_subdevice_h_disabled

NV_STATUS __nvoc_objCreateDynamic_Subdevice(Subdevice**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_Subdevice(Subdevice**, Dynamic*, NvU32, struct CALL_CONTEXT *arg_pCallContext, struct RS_RES_ALLOC_PARAMS_INTERNAL *arg_pParams);
#define __objCreate_Subdevice(ppNewObj, pParent, createFlags, arg_pCallContext, arg_pParams) \
    __nvoc_objCreate_Subdevice((ppNewObj), staticCast((pParent), Dynamic), (createFlags), arg_pCallContext, arg_pParams)


// Wrapper macros
#define subdevicePreDestruct_FNPTR(pResource) pResource->__nvoc_metadata_ptr->vtable.__subdevicePreDestruct__
#define subdevicePreDestruct(pResource) subdevicePreDestruct_DISPATCH(pResource)
#define subdeviceInternalControlForward_FNPTR(pSubdevice) pSubdevice->__nvoc_metadata_ptr->vtable.__subdeviceInternalControlForward__
#define subdeviceInternalControlForward(pSubdevice, command, pParams, size) subdeviceInternalControlForward_DISPATCH(pSubdevice, command, pParams, size)
#define subdeviceCtrlCmdGpuGetCachedInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetCachedInfo__
#define subdeviceCtrlCmdGpuGetCachedInfo(pSubdevice, pGpuInfoParams) subdeviceCtrlCmdGpuGetCachedInfo_DISPATCH(pSubdevice, pGpuInfoParams)
#define subdeviceCtrlCmdGpuGetInfoV2_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetInfoV2__
#define subdeviceCtrlCmdGpuGetInfoV2(pSubdevice, pGpuInfoParams) subdeviceCtrlCmdGpuGetInfoV2_DISPATCH(pSubdevice, pGpuInfoParams)
#define subdeviceCtrlCmdGpuGetIpVersion_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetIpVersion__
#define subdeviceCtrlCmdGpuGetIpVersion(pSubdevice, pGpuIpVersionParams) subdeviceCtrlCmdGpuGetIpVersion_DISPATCH(pSubdevice, pGpuIpVersionParams)
#define subdeviceCtrlCmdGpuSetOptimusInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuSetOptimusInfo__
#define subdeviceCtrlCmdGpuSetOptimusInfo(pSubdevice, pGpuOptimusInfoParams) subdeviceCtrlCmdGpuSetOptimusInfo_DISPATCH(pSubdevice, pGpuOptimusInfoParams)
#define subdeviceCtrlCmdGpuGetNameString_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetNameString__
#define subdeviceCtrlCmdGpuGetNameString(pSubdevice, pNameStringParams) subdeviceCtrlCmdGpuGetNameString_DISPATCH(pSubdevice, pNameStringParams)
#define subdeviceCtrlCmdGpuGetShortNameString_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetShortNameString__
#define subdeviceCtrlCmdGpuGetShortNameString(pSubdevice, pShortNameStringParams) subdeviceCtrlCmdGpuGetShortNameString_DISPATCH(pSubdevice, pShortNameStringParams)
#define subdeviceCtrlCmdGpuGetSdm_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetSdm__
#define subdeviceCtrlCmdGpuGetSdm(pSubdevice, pSdmParams) subdeviceCtrlCmdGpuGetSdm_DISPATCH(pSubdevice, pSdmParams)
#define subdeviceCtrlCmdGpuGetSimulationInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetSimulationInfo__
#define subdeviceCtrlCmdGpuGetSimulationInfo(pSubdevice, pGpuSimulationInfoParams) subdeviceCtrlCmdGpuGetSimulationInfo_DISPATCH(pSubdevice, pGpuSimulationInfoParams)
#define subdeviceCtrlCmdGpuGetEngines_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetEngines__
#define subdeviceCtrlCmdGpuGetEngines(pSubdevice, pParams) subdeviceCtrlCmdGpuGetEngines_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetEnginesV2_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetEnginesV2__
#define subdeviceCtrlCmdGpuGetEnginesV2(pSubdevice, pEngineParams) subdeviceCtrlCmdGpuGetEnginesV2_DISPATCH(pSubdevice, pEngineParams)
#define subdeviceCtrlCmdGpuGetEngineClasslist_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetEngineClasslist__
#define subdeviceCtrlCmdGpuGetEngineClasslist(pSubdevice, pClassParams) subdeviceCtrlCmdGpuGetEngineClasslist_DISPATCH(pSubdevice, pClassParams)
#define subdeviceCtrlCmdGpuGetEnginePartnerList_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetEnginePartnerList__
#define subdeviceCtrlCmdGpuGetEnginePartnerList(pSubdevice, pPartnerListParams) subdeviceCtrlCmdGpuGetEnginePartnerList_DISPATCH(pSubdevice, pPartnerListParams)
#define subdeviceCtrlCmdGpuQueryMode_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuQueryMode__
#define subdeviceCtrlCmdGpuQueryMode(pSubdevice, pQueryMode) subdeviceCtrlCmdGpuQueryMode_DISPATCH(pSubdevice, pQueryMode)
#define subdeviceCtrlCmdGpuGetChipDetails_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetChipDetails__
#define subdeviceCtrlCmdGpuGetChipDetails(pSubdevice, pParams) subdeviceCtrlCmdGpuGetChipDetails_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetOEMBoardInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetOEMBoardInfo__
#define subdeviceCtrlCmdGpuGetOEMBoardInfo(pSubdevice, pBoardInfo) subdeviceCtrlCmdGpuGetOEMBoardInfo_DISPATCH(pSubdevice, pBoardInfo)
#define subdeviceCtrlCmdGpuGetOEMInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetOEMInfo__
#define subdeviceCtrlCmdGpuGetOEMInfo(pSubdevice, pOemInfo) subdeviceCtrlCmdGpuGetOEMInfo_DISPATCH(pSubdevice, pOemInfo)
#define subdeviceCtrlCmdGpuHandleGpuSR_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuHandleGpuSR__
#define subdeviceCtrlCmdGpuHandleGpuSR(pSubdevice) subdeviceCtrlCmdGpuHandleGpuSR_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdGpuInitializeCtx_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuInitializeCtx__
#define subdeviceCtrlCmdGpuInitializeCtx(pSubdevice, pInitializeCtxParams) subdeviceCtrlCmdGpuInitializeCtx_DISPATCH(pSubdevice, pInitializeCtxParams)
#define subdeviceCtrlCmdGpuPromoteCtx_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuPromoteCtx__
#define subdeviceCtrlCmdGpuPromoteCtx(pSubdevice, pPromoteCtxParams) subdeviceCtrlCmdGpuPromoteCtx_DISPATCH(pSubdevice, pPromoteCtxParams)
#define subdeviceCtrlCmdGpuEvictCtx_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuEvictCtx__
#define subdeviceCtrlCmdGpuEvictCtx(pSubdevice, pEvictCtxParams) subdeviceCtrlCmdGpuEvictCtx_DISPATCH(pSubdevice, pEvictCtxParams)
#define subdeviceCtrlCmdGpuGetId_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetId__
#define subdeviceCtrlCmdGpuGetId(pSubdevice, pIdParams) subdeviceCtrlCmdGpuGetId_DISPATCH(pSubdevice, pIdParams)
#define subdeviceCtrlCmdGpuGetGidInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetGidInfo__
#define subdeviceCtrlCmdGpuGetGidInfo(pSubdevice, pGidInfoParams) subdeviceCtrlCmdGpuGetGidInfo_DISPATCH(pSubdevice, pGidInfoParams)
#define subdeviceCtrlCmdGpuGetPids_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetPids__
#define subdeviceCtrlCmdGpuGetPids(pSubdevice, pGetPidsParams) subdeviceCtrlCmdGpuGetPids_DISPATCH(pSubdevice, pGetPidsParams)
#define subdeviceCtrlCmdGpuGetPidInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetPidInfo__
#define subdeviceCtrlCmdGpuGetPidInfo(pSubdevice, pGetPidInfoParams) subdeviceCtrlCmdGpuGetPidInfo_DISPATCH(pSubdevice, pGetPidInfoParams)
#define subdeviceCtrlCmdGpuQueryFunctionStatus_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuQueryFunctionStatus__
#define subdeviceCtrlCmdGpuQueryFunctionStatus(pSubdevice, pParams) subdeviceCtrlCmdGpuQueryFunctionStatus_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetFirstAsyncCEIdx_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetFirstAsyncCEIdx__
#define subdeviceCtrlCmdGpuGetFirstAsyncCEIdx(pSubdevice, pParams) subdeviceCtrlCmdGpuGetFirstAsyncCEIdx_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetVmmuSegmentSize_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetVmmuSegmentSize__
#define subdeviceCtrlCmdGpuGetVmmuSegmentSize(pSubdevice, pParams) subdeviceCtrlCmdGpuGetVmmuSegmentSize_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetMaxSupportedPageSize_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetMaxSupportedPageSize__
#define subdeviceCtrlCmdGpuGetMaxSupportedPageSize(pSubdevice, pParams) subdeviceCtrlCmdGpuGetMaxSupportedPageSize_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuHandleVfPriFault_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuHandleVfPriFault__
#define subdeviceCtrlCmdGpuHandleVfPriFault(pSubdevice, pParams) subdeviceCtrlCmdGpuHandleVfPriFault_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdValidateMemMapRequest_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdValidateMemMapRequest__
#define subdeviceCtrlCmdValidateMemMapRequest(pSubdevice, pParams) subdeviceCtrlCmdValidateMemMapRequest_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetGfid_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetGfid__
#define subdeviceCtrlCmdGpuGetGfid(pSubdevice, pParams) subdeviceCtrlCmdGpuGetGfid_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdUpdateGfidP2pCapability_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdUpdateGfidP2pCapability__
#define subdeviceCtrlCmdUpdateGfidP2pCapability(pSubdevice, pParams) subdeviceCtrlCmdUpdateGfidP2pCapability_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetEngineLoadTimes_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetEngineLoadTimes__
#define subdeviceCtrlCmdGpuGetEngineLoadTimes(pSubdevice, pParams) subdeviceCtrlCmdGpuGetEngineLoadTimes_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuMarkDeviceForReset_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuMarkDeviceForReset__
#define subdeviceCtrlCmdGpuMarkDeviceForReset(pSubdevice) subdeviceCtrlCmdGpuMarkDeviceForReset_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdGpuUnmarkDeviceForReset_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuUnmarkDeviceForReset__
#define subdeviceCtrlCmdGpuUnmarkDeviceForReset(pSubdevice) subdeviceCtrlCmdGpuUnmarkDeviceForReset_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset__
#define subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset(pSubdevice) subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset__
#define subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset(pSubdevice) subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdGpuGetResetStatus_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetResetStatus__
#define subdeviceCtrlCmdGpuGetResetStatus(pSubdevice, pParams) subdeviceCtrlCmdGpuGetResetStatus_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetDrainAndResetStatus_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetDrainAndResetStatus__
#define subdeviceCtrlCmdGpuGetDrainAndResetStatus(pSubdevice, pParams) subdeviceCtrlCmdGpuGetDrainAndResetStatus_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetConstructedFalconInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetConstructedFalconInfo__
#define subdeviceCtrlCmdGpuGetConstructedFalconInfo(pSubdevice, pParams) subdeviceCtrlCmdGpuGetConstructedFalconInfo_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlGpuGetFipsStatus_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlGpuGetFipsStatus__
#define subdeviceCtrlGpuGetFipsStatus(pSubdevice, pParams) subdeviceCtrlGpuGetFipsStatus_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetVfCaps_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetVfCaps__
#define subdeviceCtrlCmdGpuGetVfCaps(pSubdevice, pParams) subdeviceCtrlCmdGpuGetVfCaps_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuGetRecoveryAction_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetRecoveryAction__
#define subdeviceCtrlCmdGpuGetRecoveryAction(pSubdevice, pParams) subdeviceCtrlCmdGpuGetRecoveryAction_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuRpcGspTest_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuRpcGspTest__
#define subdeviceCtrlCmdGpuRpcGspTest(pSubdevice, pParams) subdeviceCtrlCmdGpuRpcGspTest_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGpuRpcGspQuerySizes_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuRpcGspQuerySizes__
#define subdeviceCtrlCmdGpuRpcGspQuerySizes(pSubdevice, pParams) subdeviceCtrlCmdGpuRpcGspQuerySizes_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdRusdGetSupportedFeatures_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdRusdGetSupportedFeatures__
#define subdeviceCtrlCmdRusdGetSupportedFeatures(pSubdevice, pParams) subdeviceCtrlCmdRusdGetSupportedFeatures_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdRusdSetFeatures_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdRusdSetFeatures__
#define subdeviceCtrlCmdRusdSetFeatures(pSubdevice, pParams) subdeviceCtrlCmdRusdSetFeatures_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdEventSetTrigger_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEventSetTrigger__
#define subdeviceCtrlCmdEventSetTrigger(pSubdevice) subdeviceCtrlCmdEventSetTrigger_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdEventSetTriggerFifo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEventSetTriggerFifo__
#define subdeviceCtrlCmdEventSetTriggerFifo(pSubdevice, pTriggerFifoParams) subdeviceCtrlCmdEventSetTriggerFifo_DISPATCH(pSubdevice, pTriggerFifoParams)
#define subdeviceCtrlCmdEventSetNotification_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEventSetNotification__
#define subdeviceCtrlCmdEventSetNotification(pSubdevice, pSetEventParams) subdeviceCtrlCmdEventSetNotification_DISPATCH(pSubdevice, pSetEventParams)
#define subdeviceCtrlCmdEventSetMemoryNotifies_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEventSetMemoryNotifies__
#define subdeviceCtrlCmdEventSetMemoryNotifies(pSubdevice, pSetMemoryNotifiesParams) subdeviceCtrlCmdEventSetMemoryNotifies_DISPATCH(pSubdevice, pSetMemoryNotifiesParams)
#define subdeviceCtrlCmdEventSetSemaphoreMemory_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEventSetSemaphoreMemory__
#define subdeviceCtrlCmdEventSetSemaphoreMemory(pSubdevice, pSetSemMemoryParams) subdeviceCtrlCmdEventSetSemaphoreMemory_DISPATCH(pSubdevice, pSetSemMemoryParams)
#define subdeviceCtrlCmdEventSetSemaMemValidation_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEventSetSemaMemValidation__
#define subdeviceCtrlCmdEventSetSemaMemValidation(pSubdevice, pSetSemaMemValidationParams) subdeviceCtrlCmdEventSetSemaMemValidation_DISPATCH(pSubdevice, pSetSemaMemValidationParams)
#define subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf__
#define subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf(pSubdevice, pBindParams) subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf_DISPATCH(pSubdevice, pBindParams)
#define subdeviceCtrlCmdTimerCancel_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdTimerCancel__
#define subdeviceCtrlCmdTimerCancel(pSubdevice) subdeviceCtrlCmdTimerCancel_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdTimerSchedule_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdTimerSchedule__
#define subdeviceCtrlCmdTimerSchedule(pSubdevice, pParams) subdeviceCtrlCmdTimerSchedule_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdTimerGetTime_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdTimerGetTime__
#define subdeviceCtrlCmdTimerGetTime(pSubdevice, pParams) subdeviceCtrlCmdTimerGetTime_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdTimerGetRegisterOffset_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdTimerGetRegisterOffset__
#define subdeviceCtrlCmdTimerGetRegisterOffset(pSubdevice, pTimerRegOffsetParams) subdeviceCtrlCmdTimerGetRegisterOffset_DISPATCH(pSubdevice, pTimerRegOffsetParams)
#define subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo__
#define subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo(pSubdevice, pParams) subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdEccGetClientExposedCounters_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEccGetClientExposedCounters__
#define subdeviceCtrlCmdEccGetClientExposedCounters(pSubdevice, pParams) subdeviceCtrlCmdEccGetClientExposedCounters_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdEccGetVolatileCounts_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEccGetVolatileCounts__
#define subdeviceCtrlCmdEccGetVolatileCounts(pSubdevice, pParams) subdeviceCtrlCmdEccGetVolatileCounts_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdEccInjectError_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEccInjectError__
#define subdeviceCtrlCmdEccInjectError(pSubdevice, pParams) subdeviceCtrlCmdEccInjectError_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdEccGetRepairStatus_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEccGetRepairStatus__
#define subdeviceCtrlCmdEccGetRepairStatus(pSubdevice, pParams) subdeviceCtrlCmdEccGetRepairStatus_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdEccInjectionSupported_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdEccInjectionSupported__
#define subdeviceCtrlCmdEccInjectionSupported(pSubdevice, pParams) subdeviceCtrlCmdEccInjectionSupported_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdGspGetFeatures_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGspGetFeatures__
#define subdeviceCtrlCmdGspGetFeatures(pSubdevice, pGspFeaturesParams) subdeviceCtrlCmdGspGetFeatures_DISPATCH(pSubdevice, pGspFeaturesParams)
#define subdeviceCtrlCmdGspGetFeatures_HAL(pSubdevice, pGspFeaturesParams) subdeviceCtrlCmdGspGetFeatures_DISPATCH(pSubdevice, pGspFeaturesParams)
#define subdeviceCtrlCmdGspGetRmHeapStats_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGspGetRmHeapStats__
#define subdeviceCtrlCmdGspGetRmHeapStats(pSubdevice, pGspRmHeapStatsParams) subdeviceCtrlCmdGspGetRmHeapStats_DISPATCH(pSubdevice, pGspRmHeapStatsParams)
#define subdeviceCtrlCmdGpuGetVgpuHeapStats_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdGpuGetVgpuHeapStats__
#define subdeviceCtrlCmdGpuGetVgpuHeapStats(pSubdevice, pParams) subdeviceCtrlCmdGpuGetVgpuHeapStats_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdLibosGetHeapStats_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdLibosGetHeapStats__
#define subdeviceCtrlCmdLibosGetHeapStats(pSubdevice, pGspLibosHeapStatsParams) subdeviceCtrlCmdLibosGetHeapStats_DISPATCH(pSubdevice, pGspLibosHeapStatsParams)
#define subdeviceCtrlCmdOsUnixGc6BlockerRefCnt_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdOsUnixGc6BlockerRefCnt__
#define subdeviceCtrlCmdOsUnixGc6BlockerRefCnt(pSubdevice, pParams) subdeviceCtrlCmdOsUnixGc6BlockerRefCnt_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdOsUnixAllowDisallowGcoff_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdOsUnixAllowDisallowGcoff__
#define subdeviceCtrlCmdOsUnixAllowDisallowGcoff(pSubdevice, pParams) subdeviceCtrlCmdOsUnixAllowDisallowGcoff_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdOsUnixAudioDynamicPower_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdOsUnixAudioDynamicPower__
#define subdeviceCtrlCmdOsUnixAudioDynamicPower(pSubdevice, pParams) subdeviceCtrlCmdOsUnixAudioDynamicPower_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdDisplayGetIpVersion_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdDisplayGetIpVersion__
#define subdeviceCtrlCmdDisplayGetIpVersion(pSubdevice, pParams) subdeviceCtrlCmdDisplayGetIpVersion_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdDisplayGetStaticInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdDisplayGetStaticInfo__
#define subdeviceCtrlCmdDisplayGetStaticInfo(pSubdevice, pParams) subdeviceCtrlCmdDisplayGetStaticInfo_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdDisplaySetChannelPushbuffer_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdDisplaySetChannelPushbuffer__
#define subdeviceCtrlCmdDisplaySetChannelPushbuffer(pSubdevice, pParams) subdeviceCtrlCmdDisplaySetChannelPushbuffer_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdDisplayWriteInstMem_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdDisplayWriteInstMem__
#define subdeviceCtrlCmdDisplayWriteInstMem(pSubdevice, pParams) subdeviceCtrlCmdDisplayWriteInstMem_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdDisplaySetupRgLineIntr_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdDisplaySetupRgLineIntr__
#define subdeviceCtrlCmdDisplaySetupRgLineIntr(pSubdevice, pParams) subdeviceCtrlCmdDisplaySetupRgLineIntr_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdDisplaySetImportedImpData_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdDisplaySetImportedImpData__
#define subdeviceCtrlCmdDisplaySetImportedImpData(pSubdevice, pParams) subdeviceCtrlCmdDisplaySetImportedImpData_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdDisplayGetDisplayMask_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdDisplayGetDisplayMask__
#define subdeviceCtrlCmdDisplayGetDisplayMask(pSubdevice, pParams) subdeviceCtrlCmdDisplayGetDisplayMask_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdDisplayPinsetsToLockpins_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdDisplayPinsetsToLockpins__
#define subdeviceCtrlCmdDisplayPinsetsToLockpins(pSubdevice, pParams) subdeviceCtrlCmdDisplayPinsetsToLockpins_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl__
#define subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl(pSubdevice, pParams) subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGpioProgramDirection_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGpioProgramDirection__
#define subdeviceCtrlCmdInternalGpioProgramDirection(pSubdevice, pParams) subdeviceCtrlCmdInternalGpioProgramDirection_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGpioProgramOutput_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGpioProgramOutput__
#define subdeviceCtrlCmdInternalGpioProgramOutput(pSubdevice, pParams) subdeviceCtrlCmdInternalGpioProgramOutput_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGpioReadInput_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGpioReadInput__
#define subdeviceCtrlCmdInternalGpioReadInput(pSubdevice, pParams) subdeviceCtrlCmdInternalGpioReadInput_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGpioActivateHwFunction_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGpioActivateHwFunction__
#define subdeviceCtrlCmdInternalGpioActivateHwFunction(pSubdevice, pParams) subdeviceCtrlCmdInternalGpioActivateHwFunction_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated__
#define subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated(pSubdevice) subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdInternalDisplayPreModeSet_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalDisplayPreModeSet__
#define subdeviceCtrlCmdInternalDisplayPreModeSet(pSubdevice) subdeviceCtrlCmdInternalDisplayPreModeSet_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdInternalDisplayPostModeSet_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalDisplayPostModeSet__
#define subdeviceCtrlCmdInternalDisplayPostModeSet(pSubdevice) subdeviceCtrlCmdInternalDisplayPostModeSet_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdInternalGetChipInfo_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGetChipInfo__
#define subdeviceCtrlCmdInternalGetChipInfo(pSubdevice, pParams) subdeviceCtrlCmdInternalGetChipInfo_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGetUserRegisterAccessMap_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGetUserRegisterAccessMap__
#define subdeviceCtrlCmdInternalGetUserRegisterAccessMap(pSubdevice, pParams) subdeviceCtrlCmdInternalGetUserRegisterAccessMap_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGetDeviceInfoTable_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGetDeviceInfoTable__
#define subdeviceCtrlCmdInternalGetDeviceInfoTable(pSubdevice, pParams) subdeviceCtrlCmdInternalGetDeviceInfoTable_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalRecoverAllComputeContexts_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalRecoverAllComputeContexts__
#define subdeviceCtrlCmdInternalRecoverAllComputeContexts(pSubdevice) subdeviceCtrlCmdInternalRecoverAllComputeContexts_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdInternalGetSmcMode_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGetSmcMode__
#define subdeviceCtrlCmdInternalGetSmcMode(pSubdevice, pParams) subdeviceCtrlCmdInternalGetSmcMode_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdIsEgpuBridge_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdIsEgpuBridge__
#define subdeviceCtrlCmdIsEgpuBridge(pSubdevice, pParams) subdeviceCtrlCmdIsEgpuBridge_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap__
#define subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap(pSubdevice, pParams) subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries__
#define subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries(pSubdevice, pParams) subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGcxEntryPrerequisite_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGcxEntryPrerequisite__
#define subdeviceCtrlCmdInternalGcxEntryPrerequisite(pSubdevice, pGcxEntryPrerequisite) subdeviceCtrlCmdInternalGcxEntryPrerequisite_DISPATCH(pSubdevice, pGcxEntryPrerequisite)
#define subdeviceCtrlCmdInternalSetP2pCaps_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalSetP2pCaps__
#define subdeviceCtrlCmdInternalSetP2pCaps(pSubdevice, pParams) subdeviceCtrlCmdInternalSetP2pCaps_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalRemoveP2pCaps_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalRemoveP2pCaps__
#define subdeviceCtrlCmdInternalRemoveP2pCaps(pSubdevice, pParams) subdeviceCtrlCmdInternalRemoveP2pCaps_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGetPcieP2pCaps_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGetPcieP2pCaps__
#define subdeviceCtrlCmdInternalGetPcieP2pCaps(pSubdevice, pParams) subdeviceCtrlCmdInternalGetPcieP2pCaps_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalPostInitBrightcStateLoad_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalPostInitBrightcStateLoad__
#define subdeviceCtrlCmdInternalPostInitBrightcStateLoad(pSubdevice, pParams) subdeviceCtrlCmdInternalPostInitBrightcStateLoad_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalSetStaticEdidData_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalSetStaticEdidData__
#define subdeviceCtrlCmdInternalSetStaticEdidData(pSubdevice, pParams) subdeviceCtrlCmdInternalSetStaticEdidData_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalDetectHsVideoBridge_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalDetectHsVideoBridge__
#define subdeviceCtrlCmdInternalDetectHsVideoBridge(pSubdevice) subdeviceCtrlCmdInternalDetectHsVideoBridge_DISPATCH(pSubdevice)
#define subdeviceCtrlCmdInternalInitUserSharedData_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalInitUserSharedData__
#define subdeviceCtrlCmdInternalInitUserSharedData(pSubdevice, pParams) subdeviceCtrlCmdInternalInitUserSharedData_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalUserSharedDataSetDataPoll_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalUserSharedDataSetDataPoll__
#define subdeviceCtrlCmdInternalUserSharedDataSetDataPoll(pSubdevice, pParams) subdeviceCtrlCmdInternalUserSharedDataSetDataPoll_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalControlGspTrace_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalControlGspTrace__
#define subdeviceCtrlCmdInternalControlGspTrace(pSubdevice, pParams) subdeviceCtrlCmdInternalControlGspTrace_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter__
#define subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter(pSubdevice, pParams) subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter_DISPATCH(pSubdevice, pParams)
#define subdeviceCtrlCmdInternalLogOobXid_FNPTR(pSubdevice) pSubdevice->__subdeviceCtrlCmdInternalLogOobXid__
#define subdeviceCtrlCmdInternalLogOobXid(pSubdevice, pParams) subdeviceCtrlCmdInternalLogOobXid_DISPATCH(pSubdevice, pParams)
#define subdeviceControl_FNPTR(pGpuResource) pGpuResource->__nvoc_base_GpuResource.__nvoc_metadata_ptr->vtable.__gpuresControl__
#define subdeviceControl(pGpuResource, pCallContext, pParams) subdeviceControl_DISPATCH(pGpuResource, pCallContext, pParams)
#define subdeviceMap_FNPTR(pGpuResource) pGpuResource->__nvoc_base_GpuResource.__nvoc_metadata_ptr->vtable.__gpuresMap__
#define subdeviceMap(pGpuResource, pCallContext, pParams, pCpuMapping) subdeviceMap_DISPATCH(pGpuResource, pCallContext, pParams, pCpuMapping)
#define subdeviceUnmap_FNPTR(pGpuResource) pGpuResource->__nvoc_base_GpuResource.__nvoc_metadata_ptr->vtable.__gpuresUnmap__
#define subdeviceUnmap(pGpuResource, pCallContext, pCpuMapping) subdeviceUnmap_DISPATCH(pGpuResource, pCallContext, pCpuMapping)
#define subdeviceShareCallback_FNPTR(pGpuResource) pGpuResource->__nvoc_base_GpuResource.__nvoc_metadata_ptr->vtable.__gpuresShareCallback__
#define subdeviceShareCallback(pGpuResource, pInvokingClient, pParentRef, pSharePolicy) subdeviceShareCallback_DISPATCH(pGpuResource, pInvokingClient, pParentRef, pSharePolicy)
#define subdeviceGetRegBaseOffsetAndSize_FNPTR(pGpuResource) pGpuResource->__nvoc_base_GpuResource.__nvoc_metadata_ptr->vtable.__gpuresGetRegBaseOffsetAndSize__
#define subdeviceGetRegBaseOffsetAndSize(pGpuResource, pGpu, pOffset, pSize) subdeviceGetRegBaseOffsetAndSize_DISPATCH(pGpuResource, pGpu, pOffset, pSize)
#define subdeviceGetMapAddrSpace_FNPTR(pGpuResource) pGpuResource->__nvoc_base_GpuResource.__nvoc_metadata_ptr->vtable.__gpuresGetMapAddrSpace__
#define subdeviceGetMapAddrSpace(pGpuResource, pCallContext, mapFlags, pAddrSpace) subdeviceGetMapAddrSpace_DISPATCH(pGpuResource, pCallContext, mapFlags, pAddrSpace)
#define subdeviceGetInternalObjectHandle_FNPTR(pGpuResource) pGpuResource->__nvoc_base_GpuResource.__nvoc_metadata_ptr->vtable.__gpuresGetInternalObjectHandle__
#define subdeviceGetInternalObjectHandle(pGpuResource) subdeviceGetInternalObjectHandle_DISPATCH(pGpuResource)
#define subdeviceAccessCallback_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_metadata_ptr->vtable.__rmresAccessCallback__
#define subdeviceAccessCallback(pResource, pInvokingClient, pAllocParams, accessRight) subdeviceAccessCallback_DISPATCH(pResource, pInvokingClient, pAllocParams, accessRight)
#define subdeviceGetMemInterMapParams_FNPTR(pRmResource) pRmResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_metadata_ptr->vtable.__rmresGetMemInterMapParams__
#define subdeviceGetMemInterMapParams(pRmResource, pParams) subdeviceGetMemInterMapParams_DISPATCH(pRmResource, pParams)
#define subdeviceCheckMemInterUnmap_FNPTR(pRmResource) pRmResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_metadata_ptr->vtable.__rmresCheckMemInterUnmap__
#define subdeviceCheckMemInterUnmap(pRmResource, bSubdeviceHandleProvided) subdeviceCheckMemInterUnmap_DISPATCH(pRmResource, bSubdeviceHandleProvided)
#define subdeviceGetMemoryMappingDescriptor_FNPTR(pRmResource) pRmResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_metadata_ptr->vtable.__rmresGetMemoryMappingDescriptor__
#define subdeviceGetMemoryMappingDescriptor(pRmResource, ppMemDesc) subdeviceGetMemoryMappingDescriptor_DISPATCH(pRmResource, ppMemDesc)
#define subdeviceControlSerialization_Prologue_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_metadata_ptr->vtable.__rmresControlSerialization_Prologue__
#define subdeviceControlSerialization_Prologue(pResource, pCallContext, pParams) subdeviceControlSerialization_Prologue_DISPATCH(pResource, pCallContext, pParams)
#define subdeviceControlSerialization_Epilogue_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_metadata_ptr->vtable.__rmresControlSerialization_Epilogue__
#define subdeviceControlSerialization_Epilogue(pResource, pCallContext, pParams) subdeviceControlSerialization_Epilogue_DISPATCH(pResource, pCallContext, pParams)
#define subdeviceControl_Prologue_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_metadata_ptr->vtable.__rmresControl_Prologue__
#define subdeviceControl_Prologue(pResource, pCallContext, pParams) subdeviceControl_Prologue_DISPATCH(pResource, pCallContext, pParams)
#define subdeviceControl_Epilogue_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_metadata_ptr->vtable.__rmresControl_Epilogue__
#define subdeviceControl_Epilogue(pResource, pCallContext, pParams) subdeviceControl_Epilogue_DISPATCH(pResource, pCallContext, pParams)
#define subdeviceCanCopy_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resCanCopy__
#define subdeviceCanCopy(pResource) subdeviceCanCopy_DISPATCH(pResource)
#define subdeviceIsDuplicate_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resIsDuplicate__
#define subdeviceIsDuplicate(pResource, hMemory, pDuplicate) subdeviceIsDuplicate_DISPATCH(pResource, hMemory, pDuplicate)
#define subdeviceControlFilter_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resControlFilter__
#define subdeviceControlFilter(pResource, pCallContext, pParams) subdeviceControlFilter_DISPATCH(pResource, pCallContext, pParams)
#define subdeviceIsPartialUnmapSupported_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resIsPartialUnmapSupported__
#define subdeviceIsPartialUnmapSupported(pResource) subdeviceIsPartialUnmapSupported_DISPATCH(pResource)
#define subdeviceMapTo_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resMapTo__
#define subdeviceMapTo(pResource, pParams) subdeviceMapTo_DISPATCH(pResource, pParams)
#define subdeviceUnmapFrom_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resUnmapFrom__
#define subdeviceUnmapFrom(pResource, pParams) subdeviceUnmapFrom_DISPATCH(pResource, pParams)
#define subdeviceGetRefCount_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resGetRefCount__
#define subdeviceGetRefCount(pResource) subdeviceGetRefCount_DISPATCH(pResource)
#define subdeviceAddAdditionalDependants_FNPTR(pResource) pResource->__nvoc_base_GpuResource.__nvoc_base_RmResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resAddAdditionalDependants__
#define subdeviceAddAdditionalDependants(pClient, pResource, pReference) subdeviceAddAdditionalDependants_DISPATCH(pClient, pResource, pReference)
#define subdeviceGetNotificationListPtr_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifyGetNotificationListPtr__
#define subdeviceGetNotificationListPtr(pNotifier) subdeviceGetNotificationListPtr_DISPATCH(pNotifier)
#define subdeviceGetNotificationShare_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifyGetNotificationShare__
#define subdeviceGetNotificationShare(pNotifier) subdeviceGetNotificationShare_DISPATCH(pNotifier)
#define subdeviceSetNotificationShare_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifySetNotificationShare__
#define subdeviceSetNotificationShare(pNotifier, pNotifShare) subdeviceSetNotificationShare_DISPATCH(pNotifier, pNotifShare)
#define subdeviceUnregisterEvent_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifyUnregisterEvent__
#define subdeviceUnregisterEvent(pNotifier, hNotifierClient, hNotifierResource, hEventClient, hEvent) subdeviceUnregisterEvent_DISPATCH(pNotifier, hNotifierClient, hNotifierResource, hEventClient, hEvent)
#define subdeviceGetOrAllocNotifShare_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifyGetOrAllocNotifShare__
#define subdeviceGetOrAllocNotifShare(pNotifier, hNotifierClient, hNotifierResource, ppNotifShare) subdeviceGetOrAllocNotifShare_DISPATCH(pNotifier, hNotifierClient, hNotifierResource, ppNotifShare)

// Dispatch functions
static inline void subdevicePreDestruct_DISPATCH(struct Subdevice *pResource) {
    pResource->__nvoc_metadata_ptr->vtable.__subdevicePreDestruct__(pResource);
}

static inline NV_STATUS subdeviceInternalControlForward_DISPATCH(struct Subdevice *pSubdevice, NvU32 command, void *pParams, NvU32 size) {
    return pSubdevice->__nvoc_metadata_ptr->vtable.__subdeviceInternalControlForward__(pSubdevice, command, pParams, size);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetCachedInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetCachedInfo__(pSubdevice, pGpuInfoParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetInfoV2_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetInfoV2__(pSubdevice, pGpuInfoParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetIpVersion_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_IP_VERSION_PARAMS *pGpuIpVersionParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetIpVersion__(pSubdevice, pGpuIpVersionParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuSetOptimusInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_OPTIMUS_INFO_PARAMS *pGpuOptimusInfoParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuSetOptimusInfo__(pSubdevice, pGpuOptimusInfoParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetNameString_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_NAME_STRING_PARAMS *pNameStringParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetNameString__(pSubdevice, pNameStringParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetShortNameString_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SHORT_NAME_STRING_PARAMS *pShortNameStringParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetShortNameString__(pSubdevice, pShortNameStringParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetSdm_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SDM_PARAMS *pSdmParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetSdm__(pSubdevice, pSdmParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetSimulationInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SIMULATION_INFO_PARAMS *pGpuSimulationInfoParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetSimulationInfo__(pSubdevice, pGpuSimulationInfoParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetEngines_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINES_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetEngines__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetEnginesV2_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINES_V2_PARAMS *pEngineParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetEnginesV2__(pSubdevice, pEngineParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetEngineClasslist_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_CLASSLIST_PARAMS *pClassParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetEngineClasslist__(pSubdevice, pClassParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetEnginePartnerList_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_PARTNERLIST_PARAMS *pPartnerListParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetEnginePartnerList__(pSubdevice, pPartnerListParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuQueryMode_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_QUERY_MODE_PARAMS *pQueryMode) {
    return pSubdevice->__subdeviceCtrlCmdGpuQueryMode__(pSubdevice, pQueryMode);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetChipDetails_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_CHIP_DETAILS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetChipDetails__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetOEMBoardInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_OEM_BOARD_INFO_PARAMS *pBoardInfo) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetOEMBoardInfo__(pSubdevice, pBoardInfo);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetOEMInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_OEM_INFO_PARAMS *pOemInfo) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetOEMInfo__(pSubdevice, pOemInfo);
}

static inline NV_STATUS subdeviceCtrlCmdGpuHandleGpuSR_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdGpuHandleGpuSR__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdGpuInitializeCtx_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_INITIALIZE_CTX_PARAMS *pInitializeCtxParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuInitializeCtx__(pSubdevice, pInitializeCtxParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuPromoteCtx_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_PROMOTE_CTX_PARAMS *pPromoteCtxParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuPromoteCtx__(pSubdevice, pPromoteCtxParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuEvictCtx_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_EVICT_CTX_PARAMS *pEvictCtxParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuEvictCtx__(pSubdevice, pEvictCtxParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetId_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ID_PARAMS *pIdParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetId__(pSubdevice, pIdParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetGidInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_GID_INFO_PARAMS *pGidInfoParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetGidInfo__(pSubdevice, pGidInfoParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetPids_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_PIDS_PARAMS *pGetPidsParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetPids__(pSubdevice, pGetPidsParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetPidInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_PID_INFO_PARAMS *pGetPidInfoParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetPidInfo__(pSubdevice, pGetPidInfoParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuQueryFunctionStatus_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_QUERY_FUNCTION_STATUS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuQueryFunctionStatus__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetFirstAsyncCEIdx_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_FIRST_ASYNC_CE_IDX_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetFirstAsyncCEIdx__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetVmmuSegmentSize_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_VMMU_SEGMENT_SIZE_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetVmmuSegmentSize__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetMaxSupportedPageSize_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_MAX_SUPPORTED_PAGE_SIZE_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetMaxSupportedPageSize__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuHandleVfPriFault_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_HANDLE_VF_PRI_FAULT_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuHandleVfPriFault__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdValidateMemMapRequest_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_VALIDATE_MEM_MAP_REQUEST_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdValidateMemMapRequest__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetGfid_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_GFID_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetGfid__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdUpdateGfidP2pCapability_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_UPDATE_GFID_P2P_CAPABILITY_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdUpdateGfidP2pCapability__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetEngineLoadTimes_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_LOAD_TIMES_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetEngineLoadTimes__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuMarkDeviceForReset_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdGpuMarkDeviceForReset__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdGpuUnmarkDeviceForReset_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdGpuUnmarkDeviceForReset__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetResetStatus_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_RESET_STATUS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetResetStatus__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetDrainAndResetStatus_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_DRAIN_AND_RESET_STATUS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetDrainAndResetStatus__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetConstructedFalconInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_CONSTRUCTED_FALCON_INFO_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetConstructedFalconInfo__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlGpuGetFipsStatus_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_FIPS_STATUS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlGpuGetFipsStatus__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetVfCaps_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_VF_CAPS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetVfCaps__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetRecoveryAction_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_RECOVERY_ACTION_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetRecoveryAction__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuRpcGspTest_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RPC_GSP_TEST_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuRpcGspTest__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuRpcGspQuerySizes_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RPC_GSP_QUERY_SIZES_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuRpcGspQuerySizes__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdRusdGetSupportedFeatures_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_RUSD_GET_SUPPORTED_FEATURES_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdRusdGetSupportedFeatures__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdRusdSetFeatures_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RUSD_SET_FEATURES_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdRusdSetFeatures__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdEventSetTrigger_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdEventSetTrigger__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdEventSetTriggerFifo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_TRIGGER_FIFO_PARAMS *pTriggerFifoParams) {
    return pSubdevice->__subdeviceCtrlCmdEventSetTriggerFifo__(pSubdevice, pTriggerFifoParams);
}

static inline NV_STATUS subdeviceCtrlCmdEventSetNotification_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_NOTIFICATION_PARAMS *pSetEventParams) {
    return pSubdevice->__subdeviceCtrlCmdEventSetNotification__(pSubdevice, pSetEventParams);
}

static inline NV_STATUS subdeviceCtrlCmdEventSetMemoryNotifies_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_MEMORY_NOTIFIES_PARAMS *pSetMemoryNotifiesParams) {
    return pSubdevice->__subdeviceCtrlCmdEventSetMemoryNotifies__(pSubdevice, pSetMemoryNotifiesParams);
}

static inline NV_STATUS subdeviceCtrlCmdEventSetSemaphoreMemory_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_SEMAPHORE_MEMORY_PARAMS *pSetSemMemoryParams) {
    return pSubdevice->__subdeviceCtrlCmdEventSetSemaphoreMemory__(pSubdevice, pSetSemMemoryParams);
}

static inline NV_STATUS subdeviceCtrlCmdEventSetSemaMemValidation_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_SEMA_MEM_VALIDATION_PARAMS *pSetSemaMemValidationParams) {
    return pSubdevice->__subdeviceCtrlCmdEventSetSemaMemValidation__(pSubdevice, pSetSemaMemValidationParams);
}

static inline NV_STATUS subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_RATS_GSP_TRACE_BIND_EVTBUF_PARAMS *pBindParams) {
    return pSubdevice->__subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf__(pSubdevice, pBindParams);
}

static inline NV_STATUS subdeviceCtrlCmdTimerCancel_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdTimerCancel__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdTimerSchedule_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_TIMER_SCHEDULE_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdTimerSchedule__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdTimerGetTime_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_TIME_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdTimerGetTime__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdTimerGetRegisterOffset_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_REGISTER_OFFSET_PARAMS *pTimerRegOffsetParams) {
    return pSubdevice->__subdeviceCtrlCmdTimerGetRegisterOffset__(pSubdevice, pTimerRegOffsetParams);
}

static inline NV_STATUS subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_GPU_CPU_TIME_CORRELATION_INFO_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdEccGetClientExposedCounters_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_CLIENT_EXPOSED_COUNTERS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdEccGetClientExposedCounters__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdEccGetVolatileCounts_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_VOLATILE_COUNTS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdEccGetVolatileCounts__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdEccInjectError_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_INJECT_ERROR_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdEccInjectError__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdEccGetRepairStatus_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_REPAIR_STATUS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdEccGetRepairStatus__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdEccInjectionSupported_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_INJECTION_SUPPORTED_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdEccInjectionSupported__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdGspGetFeatures_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GSP_GET_FEATURES_PARAMS *pGspFeaturesParams) {
    return pSubdevice->__subdeviceCtrlCmdGspGetFeatures__(pSubdevice, pGspFeaturesParams);
}

static inline NV_STATUS subdeviceCtrlCmdGspGetRmHeapStats_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_GSP_GET_RM_HEAP_STATS_PARAMS *pGspRmHeapStatsParams) {
    return pSubdevice->__subdeviceCtrlCmdGspGetRmHeapStats__(pSubdevice, pGspRmHeapStatsParams);
}

static inline NV_STATUS subdeviceCtrlCmdGpuGetVgpuHeapStats_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_VGPU_HEAP_STATS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdGpuGetVgpuHeapStats__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdLibosGetHeapStats_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_LIBOS_HEAP_STATS_PARAMS *pGspLibosHeapStatsParams) {
    return pSubdevice->__subdeviceCtrlCmdLibosGetHeapStats__(pSubdevice, pGspLibosHeapStatsParams);
}

static inline NV_STATUS subdeviceCtrlCmdOsUnixGc6BlockerRefCnt_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_GC6_BLOCKER_REFCNT_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdOsUnixGc6BlockerRefCnt__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdOsUnixAllowDisallowGcoff_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_ALLOW_DISALLOW_GCOFF_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdOsUnixAllowDisallowGcoff__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdOsUnixAudioDynamicPower_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_AUDIO_DYNAMIC_POWER_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdOsUnixAudioDynamicPower__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdDisplayGetIpVersion_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_IP_VERSION_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdDisplayGetIpVersion__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdDisplayGetStaticInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_STATIC_INFO_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdDisplayGetStaticInfo__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdDisplaySetChannelPushbuffer_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_CHANNEL_PUSHBUFFER_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdDisplaySetChannelPushbuffer__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdDisplayWriteInstMem_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_WRITE_INST_MEM_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdDisplayWriteInstMem__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdDisplaySetupRgLineIntr_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_SETUP_RG_LINE_INTR_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdDisplaySetupRgLineIntr__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdDisplaySetImportedImpData_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_SET_IMP_INIT_INFO_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdDisplaySetImportedImpData__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdDisplayGetDisplayMask_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_ACTIVE_DISPLAY_DEVICES_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdDisplayGetDisplayMask__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdDisplayPinsetsToLockpins_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISP_PINSETS_TO_LOCKPINS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdDisplayPinsetsToLockpins__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISP_SET_SLI_LINK_GPIO_SW_CONTROL_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGpioProgramDirection_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_DIRECTION_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGpioProgramDirection__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGpioProgramOutput_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_OUTPUT_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGpioProgramOutput__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGpioReadInput_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_READ_INPUT_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGpioReadInput__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGpioActivateHwFunction_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_ACTIVATE_HW_FUNCTION_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGpioActivateHwFunction__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdInternalDisplayPreModeSet_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdInternalDisplayPreModeSet__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdInternalDisplayPostModeSet_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdInternalDisplayPostModeSet__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGetChipInfo_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_CHIP_INFO_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGetChipInfo__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGetUserRegisterAccessMap_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_USER_REGISTER_ACCESS_MAP_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGetUserRegisterAccessMap__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGetDeviceInfoTable_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_DEVICE_INFO_TABLE_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGetDeviceInfoTable__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalRecoverAllComputeContexts_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdInternalRecoverAllComputeContexts__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGetSmcMode_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_SMC_MODE_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGetSmcMode__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdIsEgpuBridge_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_EGPU_BRIDGE_INFO_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdIsEgpuBridge__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_GSP_RM_FREE_HEAP_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_VMMU_GET_SPA_FOR_GPA_ENTRIES_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGcxEntryPrerequisite_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GCX_ENTRY_PREREQUISITE_PARAMS *pGcxEntryPrerequisite) {
    return pSubdevice->__subdeviceCtrlCmdInternalGcxEntryPrerequisite__(pSubdevice, pGcxEntryPrerequisite);
}

static inline NV_STATUS subdeviceCtrlCmdInternalSetP2pCaps_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_SET_P2P_CAPS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalSetP2pCaps__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalRemoveP2pCaps_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_REMOVE_P2P_CAPS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalRemoveP2pCaps__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGetPcieP2pCaps_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_PCIE_P2P_CAPS_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGetPcieP2pCaps__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalPostInitBrightcStateLoad_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_INIT_BRIGHTC_STATE_LOAD_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalPostInitBrightcStateLoad__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalSetStaticEdidData_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_INTERNAL_SET_STATIC_EDID_DATA_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalSetStaticEdidData__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalDetectHsVideoBridge_DISPATCH(struct Subdevice *pSubdevice) {
    return pSubdevice->__subdeviceCtrlCmdInternalDetectHsVideoBridge__(pSubdevice);
}

static inline NV_STATUS subdeviceCtrlCmdInternalInitUserSharedData_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_INIT_USER_SHARED_DATA_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalInitUserSharedData__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalUserSharedDataSetDataPoll_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_USER_SHARED_DATA_SET_DATA_POLL_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalUserSharedDataSetDataPoll__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalControlGspTrace_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_INTERNAL_CONTROL_GSP_TRACE_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalControlGspTrace__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_CLIENT_LOW_POWER_MODE_ENTER_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceCtrlCmdInternalLogOobXid_DISPATCH(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_LOG_OOB_XID_PARAMS *pParams) {
    return pSubdevice->__subdeviceCtrlCmdInternalLogOobXid__(pSubdevice, pParams);
}

static inline NV_STATUS subdeviceControl_DISPATCH(struct Subdevice *pGpuResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    return pGpuResource->__nvoc_metadata_ptr->vtable.__subdeviceControl__(pGpuResource, pCallContext, pParams);
}

static inline NV_STATUS subdeviceMap_DISPATCH(struct Subdevice *pGpuResource, struct CALL_CONTEXT *pCallContext, struct RS_CPU_MAP_PARAMS *pParams, struct RsCpuMapping *pCpuMapping) {
    return pGpuResource->__nvoc_metadata_ptr->vtable.__subdeviceMap__(pGpuResource, pCallContext, pParams, pCpuMapping);
}

static inline NV_STATUS subdeviceUnmap_DISPATCH(struct Subdevice *pGpuResource, struct CALL_CONTEXT *pCallContext, struct RsCpuMapping *pCpuMapping) {
    return pGpuResource->__nvoc_metadata_ptr->vtable.__subdeviceUnmap__(pGpuResource, pCallContext, pCpuMapping);
}

static inline NvBool subdeviceShareCallback_DISPATCH(struct Subdevice *pGpuResource, struct RsClient *pInvokingClient, struct RsResourceRef *pParentRef, RS_SHARE_POLICY *pSharePolicy) {
    return pGpuResource->__nvoc_metadata_ptr->vtable.__subdeviceShareCallback__(pGpuResource, pInvokingClient, pParentRef, pSharePolicy);
}

static inline NV_STATUS subdeviceGetRegBaseOffsetAndSize_DISPATCH(struct Subdevice *pGpuResource, struct OBJGPU *pGpu, NvU32 *pOffset, NvU32 *pSize) {
    return pGpuResource->__nvoc_metadata_ptr->vtable.__subdeviceGetRegBaseOffsetAndSize__(pGpuResource, pGpu, pOffset, pSize);
}

static inline NV_STATUS subdeviceGetMapAddrSpace_DISPATCH(struct Subdevice *pGpuResource, struct CALL_CONTEXT *pCallContext, NvU32 mapFlags, NV_ADDRESS_SPACE *pAddrSpace) {
    return pGpuResource->__nvoc_metadata_ptr->vtable.__subdeviceGetMapAddrSpace__(pGpuResource, pCallContext, mapFlags, pAddrSpace);
}

static inline NvHandle subdeviceGetInternalObjectHandle_DISPATCH(struct Subdevice *pGpuResource) {
    return pGpuResource->__nvoc_metadata_ptr->vtable.__subdeviceGetInternalObjectHandle__(pGpuResource);
}

static inline NvBool subdeviceAccessCallback_DISPATCH(struct Subdevice *pResource, struct RsClient *pInvokingClient, void *pAllocParams, RsAccessRight accessRight) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceAccessCallback__(pResource, pInvokingClient, pAllocParams, accessRight);
}

static inline NV_STATUS subdeviceGetMemInterMapParams_DISPATCH(struct Subdevice *pRmResource, RMRES_MEM_INTER_MAP_PARAMS *pParams) {
    return pRmResource->__nvoc_metadata_ptr->vtable.__subdeviceGetMemInterMapParams__(pRmResource, pParams);
}

static inline NV_STATUS subdeviceCheckMemInterUnmap_DISPATCH(struct Subdevice *pRmResource, NvBool bSubdeviceHandleProvided) {
    return pRmResource->__nvoc_metadata_ptr->vtable.__subdeviceCheckMemInterUnmap__(pRmResource, bSubdeviceHandleProvided);
}

static inline NV_STATUS subdeviceGetMemoryMappingDescriptor_DISPATCH(struct Subdevice *pRmResource, struct MEMORY_DESCRIPTOR **ppMemDesc) {
    return pRmResource->__nvoc_metadata_ptr->vtable.__subdeviceGetMemoryMappingDescriptor__(pRmResource, ppMemDesc);
}

static inline NV_STATUS subdeviceControlSerialization_Prologue_DISPATCH(struct Subdevice *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceControlSerialization_Prologue__(pResource, pCallContext, pParams);
}

static inline void subdeviceControlSerialization_Epilogue_DISPATCH(struct Subdevice *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    pResource->__nvoc_metadata_ptr->vtable.__subdeviceControlSerialization_Epilogue__(pResource, pCallContext, pParams);
}

static inline NV_STATUS subdeviceControl_Prologue_DISPATCH(struct Subdevice *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceControl_Prologue__(pResource, pCallContext, pParams);
}

static inline void subdeviceControl_Epilogue_DISPATCH(struct Subdevice *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    pResource->__nvoc_metadata_ptr->vtable.__subdeviceControl_Epilogue__(pResource, pCallContext, pParams);
}

static inline NvBool subdeviceCanCopy_DISPATCH(struct Subdevice *pResource) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceCanCopy__(pResource);
}

static inline NV_STATUS subdeviceIsDuplicate_DISPATCH(struct Subdevice *pResource, NvHandle hMemory, NvBool *pDuplicate) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceIsDuplicate__(pResource, hMemory, pDuplicate);
}

static inline NV_STATUS subdeviceControlFilter_DISPATCH(struct Subdevice *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceControlFilter__(pResource, pCallContext, pParams);
}

static inline NvBool subdeviceIsPartialUnmapSupported_DISPATCH(struct Subdevice *pResource) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceIsPartialUnmapSupported__(pResource);
}

static inline NV_STATUS subdeviceMapTo_DISPATCH(struct Subdevice *pResource, RS_RES_MAP_TO_PARAMS *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceMapTo__(pResource, pParams);
}

static inline NV_STATUS subdeviceUnmapFrom_DISPATCH(struct Subdevice *pResource, RS_RES_UNMAP_FROM_PARAMS *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceUnmapFrom__(pResource, pParams);
}

static inline NvU32 subdeviceGetRefCount_DISPATCH(struct Subdevice *pResource) {
    return pResource->__nvoc_metadata_ptr->vtable.__subdeviceGetRefCount__(pResource);
}

static inline void subdeviceAddAdditionalDependants_DISPATCH(struct RsClient *pClient, struct Subdevice *pResource, RsResourceRef *pReference) {
    pResource->__nvoc_metadata_ptr->vtable.__subdeviceAddAdditionalDependants__(pClient, pResource, pReference);
}

static inline PEVENTNOTIFICATION * subdeviceGetNotificationListPtr_DISPATCH(struct Subdevice *pNotifier) {
    return pNotifier->__nvoc_metadata_ptr->vtable.__subdeviceGetNotificationListPtr__(pNotifier);
}

static inline struct NotifShare * subdeviceGetNotificationShare_DISPATCH(struct Subdevice *pNotifier) {
    return pNotifier->__nvoc_metadata_ptr->vtable.__subdeviceGetNotificationShare__(pNotifier);
}

static inline void subdeviceSetNotificationShare_DISPATCH(struct Subdevice *pNotifier, struct NotifShare *pNotifShare) {
    pNotifier->__nvoc_metadata_ptr->vtable.__subdeviceSetNotificationShare__(pNotifier, pNotifShare);
}

static inline NV_STATUS subdeviceUnregisterEvent_DISPATCH(struct Subdevice *pNotifier, NvHandle hNotifierClient, NvHandle hNotifierResource, NvHandle hEventClient, NvHandle hEvent) {
    return pNotifier->__nvoc_metadata_ptr->vtable.__subdeviceUnregisterEvent__(pNotifier, hNotifierClient, hNotifierResource, hEventClient, hEvent);
}

static inline NV_STATUS subdeviceGetOrAllocNotifShare_DISPATCH(struct Subdevice *pNotifier, NvHandle hNotifierClient, NvHandle hNotifierResource, struct NotifShare **ppNotifShare) {
    return pNotifier->__nvoc_metadata_ptr->vtable.__subdeviceGetOrAllocNotifShare__(pNotifier, hNotifierClient, hNotifierResource, ppNotifShare);
}

void subdevicePreDestruct_IMPL(struct Subdevice *pResource);

NV_STATUS subdeviceInternalControlForward_IMPL(struct Subdevice *pSubdevice, NvU32 command, void *pParams, NvU32 size);

NV_STATUS subdeviceCtrlCmdGpuGetCachedInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams);

NV_STATUS subdeviceCtrlCmdGpuGetInfoV2_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams);

NV_STATUS subdeviceCtrlCmdGpuGetIpVersion_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_IP_VERSION_PARAMS *pGpuIpVersionParams);

NV_STATUS subdeviceCtrlCmdGpuSetOptimusInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_OPTIMUS_INFO_PARAMS *pGpuOptimusInfoParams);

NV_STATUS subdeviceCtrlCmdGpuGetNameString_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_NAME_STRING_PARAMS *pNameStringParams);

NV_STATUS subdeviceCtrlCmdGpuGetShortNameString_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SHORT_NAME_STRING_PARAMS *pShortNameStringParams);

NV_STATUS subdeviceCtrlCmdGpuGetSdm_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SDM_PARAMS *pSdmParams);

NV_STATUS subdeviceCtrlCmdGpuGetSimulationInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SIMULATION_INFO_PARAMS *pGpuSimulationInfoParams);

NV_STATUS subdeviceCtrlCmdGpuGetEngines_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINES_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetEnginesV2_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINES_V2_PARAMS *pEngineParams);

NV_STATUS subdeviceCtrlCmdGpuGetEngineClasslist_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_CLASSLIST_PARAMS *pClassParams);

NV_STATUS subdeviceCtrlCmdGpuGetEnginePartnerList_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_PARTNERLIST_PARAMS *pPartnerListParams);

NV_STATUS subdeviceCtrlCmdGpuQueryMode_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_QUERY_MODE_PARAMS *pQueryMode);

NV_STATUS subdeviceCtrlCmdGpuGetChipDetails_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_CHIP_DETAILS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetOEMBoardInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_OEM_BOARD_INFO_PARAMS *pBoardInfo);

NV_STATUS subdeviceCtrlCmdGpuGetOEMInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_OEM_INFO_PARAMS *pOemInfo);

NV_STATUS subdeviceCtrlCmdGpuHandleGpuSR_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdGpuInitializeCtx_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_INITIALIZE_CTX_PARAMS *pInitializeCtxParams);

NV_STATUS subdeviceCtrlCmdGpuPromoteCtx_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_PROMOTE_CTX_PARAMS *pPromoteCtxParams);

NV_STATUS subdeviceCtrlCmdGpuEvictCtx_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_EVICT_CTX_PARAMS *pEvictCtxParams);

NV_STATUS subdeviceCtrlCmdGpuGetId_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ID_PARAMS *pIdParams);

NV_STATUS subdeviceCtrlCmdGpuGetGidInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_GID_INFO_PARAMS *pGidInfoParams);

NV_STATUS subdeviceCtrlCmdGpuGetPids_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_PIDS_PARAMS *pGetPidsParams);

NV_STATUS subdeviceCtrlCmdGpuGetPidInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_PID_INFO_PARAMS *pGetPidInfoParams);

NV_STATUS subdeviceCtrlCmdGpuQueryFunctionStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_QUERY_FUNCTION_STATUS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetFirstAsyncCEIdx_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_FIRST_ASYNC_CE_IDX_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetVmmuSegmentSize_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_VMMU_SEGMENT_SIZE_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetMaxSupportedPageSize_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_MAX_SUPPORTED_PAGE_SIZE_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuHandleVfPriFault_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_HANDLE_VF_PRI_FAULT_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdValidateMemMapRequest_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_VALIDATE_MEM_MAP_REQUEST_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetGfid_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_GFID_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdUpdateGfidP2pCapability_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_UPDATE_GFID_P2P_CAPABILITY_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetEngineLoadTimes_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_LOAD_TIMES_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuMarkDeviceForReset_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdGpuUnmarkDeviceForReset_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdGpuGetResetStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_RESET_STATUS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetDrainAndResetStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_DRAIN_AND_RESET_STATUS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetConstructedFalconInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_CONSTRUCTED_FALCON_INFO_PARAMS *pParams);

NV_STATUS subdeviceCtrlGpuGetFipsStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_FIPS_STATUS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetVfCaps_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_VF_CAPS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetRecoveryAction_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_RECOVERY_ACTION_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuRpcGspTest_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RPC_GSP_TEST_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuRpcGspQuerySizes_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RPC_GSP_QUERY_SIZES_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdRusdGetSupportedFeatures_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_RUSD_GET_SUPPORTED_FEATURES_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdRusdSetFeatures_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RUSD_SET_FEATURES_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdEventSetTrigger_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdEventSetTriggerFifo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_TRIGGER_FIFO_PARAMS *pTriggerFifoParams);

NV_STATUS subdeviceCtrlCmdEventSetNotification_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_NOTIFICATION_PARAMS *pSetEventParams);

NV_STATUS subdeviceCtrlCmdEventSetMemoryNotifies_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_MEMORY_NOTIFIES_PARAMS *pSetMemoryNotifiesParams);

NV_STATUS subdeviceCtrlCmdEventSetSemaphoreMemory_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_SEMAPHORE_MEMORY_PARAMS *pSetSemMemoryParams);

NV_STATUS subdeviceCtrlCmdEventSetSemaMemValidation_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_SEMA_MEM_VALIDATION_PARAMS *pSetSemaMemValidationParams);

NV_STATUS subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_RATS_GSP_TRACE_BIND_EVTBUF_PARAMS *pBindParams);

NV_STATUS subdeviceCtrlCmdTimerCancel_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdTimerSchedule_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_TIMER_SCHEDULE_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdTimerGetTime_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_TIME_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdTimerGetRegisterOffset_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_REGISTER_OFFSET_PARAMS *pTimerRegOffsetParams);

NV_STATUS subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_GPU_CPU_TIME_CORRELATION_INFO_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdEccGetClientExposedCounters_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_CLIENT_EXPOSED_COUNTERS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdEccGetVolatileCounts_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_VOLATILE_COUNTS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdEccInjectError_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_INJECT_ERROR_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdEccGetRepairStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_REPAIR_STATUS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdEccInjectionSupported_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_INJECTION_SUPPORTED_PARAMS *pParams);

static inline NV_STATUS subdeviceCtrlCmdGspGetFeatures_92bfc3(struct Subdevice *pSubdevice, NV2080_CTRL_GSP_GET_FEATURES_PARAMS *pGspFeaturesParams) {
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}

NV_STATUS subdeviceCtrlCmdGspGetRmHeapStats_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GSP_GET_RM_HEAP_STATS_PARAMS *pGspRmHeapStatsParams);

NV_STATUS subdeviceCtrlCmdGpuGetVgpuHeapStats_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_VGPU_HEAP_STATS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdLibosGetHeapStats_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_LIBOS_HEAP_STATS_PARAMS *pGspLibosHeapStatsParams);

NV_STATUS subdeviceCtrlCmdOsUnixGc6BlockerRefCnt_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_GC6_BLOCKER_REFCNT_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdOsUnixAllowDisallowGcoff_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_ALLOW_DISALLOW_GCOFF_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdOsUnixAudioDynamicPower_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_AUDIO_DYNAMIC_POWER_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdDisplayGetIpVersion_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_IP_VERSION_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdDisplayGetStaticInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_STATIC_INFO_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdDisplaySetChannelPushbuffer_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_CHANNEL_PUSHBUFFER_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdDisplayWriteInstMem_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_WRITE_INST_MEM_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdDisplaySetupRgLineIntr_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_SETUP_RG_LINE_INTR_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdDisplaySetImportedImpData_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_SET_IMP_INIT_INFO_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdDisplayGetDisplayMask_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_ACTIVE_DISPLAY_DEVICES_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdDisplayPinsetsToLockpins_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISP_PINSETS_TO_LOCKPINS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISP_SET_SLI_LINK_GPIO_SW_CONTROL_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGpioProgramDirection_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_DIRECTION_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGpioProgramOutput_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_OUTPUT_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGpioReadInput_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_READ_INPUT_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGpioActivateHwFunction_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_ACTIVATE_HW_FUNCTION_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdInternalDisplayPreModeSet_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdInternalDisplayPostModeSet_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdInternalGetChipInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_CHIP_INFO_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGetUserRegisterAccessMap_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_USER_REGISTER_ACCESS_MAP_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGetDeviceInfoTable_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_DEVICE_INFO_TABLE_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalRecoverAllComputeContexts_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdInternalGetSmcMode_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_SMC_MODE_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdIsEgpuBridge_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_EGPU_BRIDGE_INFO_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_GSP_RM_FREE_HEAP_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_VMMU_GET_SPA_FOR_GPA_ENTRIES_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGcxEntryPrerequisite_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GCX_ENTRY_PREREQUISITE_PARAMS *pGcxEntryPrerequisite);

NV_STATUS subdeviceCtrlCmdInternalSetP2pCaps_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_SET_P2P_CAPS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalRemoveP2pCaps_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_REMOVE_P2P_CAPS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGetPcieP2pCaps_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_PCIE_P2P_CAPS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalPostInitBrightcStateLoad_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_INIT_BRIGHTC_STATE_LOAD_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalSetStaticEdidData_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_INTERNAL_SET_STATIC_EDID_DATA_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalDetectHsVideoBridge_IMPL(struct Subdevice *pSubdevice);

NV_STATUS subdeviceCtrlCmdInternalInitUserSharedData_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_INIT_USER_SHARED_DATA_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalUserSharedDataSetDataPoll_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_USER_SHARED_DATA_SET_DATA_POLL_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalControlGspTrace_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_INTERNAL_CONTROL_GSP_TRACE_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_CLIENT_LOW_POWER_MODE_ENTER_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdInternalLogOobXid_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_LOG_OOB_XID_PARAMS *pParams);

static inline NV_STATUS subdeviceSetPerfmonReservation(struct Subdevice *pSubdevice, NvBool bReservation, NvBool bClientHandlesGrGating, NvBool bRmHandlesIdleSlow) {
    return NV_OK;
}

static inline NV_STATUS subdeviceUnsetDynamicBoostLimit(struct Subdevice *pSubdevice) {
    return NV_OK;
}

static inline NV_STATUS subdeviceReleaseVideoStreams(struct Subdevice *pSubdevice) {
    return NV_OK;
}

static inline void subdeviceRestoreLockedClock(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext) {
    return;
}

static inline void subdeviceRestoreVF(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext) {
    return;
}

static inline void subdeviceReleaseNvlinkErrorInjectionMode(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext) {
    return;
}

static inline void subdeviceRestoreGrTickFreq(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext) {
    return;
}

static inline void subdeviceRestoreWatchdog(struct Subdevice *pSubdevice) {
    return;
}

NV_STATUS subdeviceConstruct_IMPL(struct Subdevice *arg_pResource, struct CALL_CONTEXT *arg_pCallContext, struct RS_RES_ALLOC_PARAMS_INTERNAL *arg_pParams);

#define __nvoc_subdeviceConstruct(arg_pResource, arg_pCallContext, arg_pParams) subdeviceConstruct_IMPL(arg_pResource, arg_pCallContext, arg_pParams)
void subdeviceDestruct_IMPL(struct Subdevice *pResource);

#define __nvoc_subdeviceDestruct(pResource) subdeviceDestruct_IMPL(pResource)
void subdeviceUnsetGpuDebugMode_IMPL(struct Subdevice *pSubdevice);

#ifdef __nvoc_subdevice_h_disabled
static inline void subdeviceUnsetGpuDebugMode(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
}
#else //__nvoc_subdevice_h_disabled
#define subdeviceUnsetGpuDebugMode(pSubdevice) subdeviceUnsetGpuDebugMode_IMPL(pSubdevice)
#endif //__nvoc_subdevice_h_disabled

void subdeviceReleaseComputeModeReservation_IMPL(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext);

#ifdef __nvoc_subdevice_h_disabled
static inline void subdeviceReleaseComputeModeReservation(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
}
#else //__nvoc_subdevice_h_disabled
#define subdeviceReleaseComputeModeReservation(pSubdevice, pCallContext) subdeviceReleaseComputeModeReservation_IMPL(pSubdevice, pCallContext)
#endif //__nvoc_subdevice_h_disabled

NV_STATUS subdeviceGetByHandle_IMPL(struct RsClient *pClient, NvHandle hSubdevice, struct Subdevice **ppSubdevice);

#define subdeviceGetByHandle(pClient, hSubdevice, ppSubdevice) subdeviceGetByHandle_IMPL(pClient, hSubdevice, ppSubdevice)
NV_STATUS subdeviceGetByGpu_IMPL(struct RsClient *pClient, struct OBJGPU *pGpu, struct Subdevice **ppSubdevice);

#define subdeviceGetByGpu(pClient, pGpu, ppSubdevice) subdeviceGetByGpu_IMPL(pClient, pGpu, ppSubdevice)
NV_STATUS subdeviceGetByDeviceAndGpu_IMPL(struct RsClient *pClient, struct Device *pDevice, struct OBJGPU *pGpu, struct Subdevice **ppSubdevice);

#define subdeviceGetByDeviceAndGpu(pClient, pDevice, pGpu, ppSubdevice) subdeviceGetByDeviceAndGpu_IMPL(pClient, pDevice, pGpu, ppSubdevice)
NV_STATUS subdeviceGetByInstance_IMPL(struct RsClient *pClient, NvHandle hDevice, NvU32 subDeviceInst, struct Subdevice **ppSubdevice);

#define subdeviceGetByInstance(pClient, hDevice, subDeviceInst, ppSubdevice) subdeviceGetByInstance_IMPL(pClient, hDevice, subDeviceInst, ppSubdevice)
#undef PRIVATE_FIELD


#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _G_SUBDEVICE_NVOC_H_
