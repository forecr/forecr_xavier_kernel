
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
#include "rmapi/event_api.h"
#include "containers/btree.h"
#include "nvoc/utility.h"
#include "gpu/gpu_halspec.h"

#include "class/cl2080.h"
#include "ctrl/ctrl2080.h" // rmcontrol parameters

typedef struct TMR_EVENT TMR_EVENT;

struct Device;

#ifndef __nvoc_class_id_Device
#define __nvoc_class_id_Device 0xe0ac20u
typedef struct Device Device;
#endif /* __nvoc_class_id_Device */


struct OBJGPU;

#ifndef __nvoc_class_id_OBJGPU
#define __nvoc_class_id_OBJGPU 0x7ef3cbu
typedef struct OBJGPU OBJGPU;
#endif /* __nvoc_class_id_OBJGPU */


struct Memory;

#ifndef __nvoc_class_id_Memory
#define __nvoc_class_id_Memory 0x4789f2u
typedef struct Memory Memory;
#endif /* __nvoc_class_id_Memory */


struct P2PApi;

#ifndef __nvoc_class_id_P2PApi
#define __nvoc_class_id_P2PApi 0x3982b7u
typedef struct P2PApi P2PApi;
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

    // Data members
    NvU32 deviceInst;
    NvU32 subDeviceInst;
    struct Device *pDevice;
    NvBool bMaxGrTickFreqRequested;
    NvU64 P2PfbMappedBytes;
    _Atomic(NvU32) notificationRefCount;
    NvU32 notifyActions[199];
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

#ifndef __nvoc_class_id_Subdevice
#define __nvoc_class_id_Subdevice 0x4b01b3u
typedef struct Subdevice Subdevice;
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

NV_STATUS __nvoc_objCreate_Subdevice(Subdevice**, Dynamic*, NvU32, struct CALL_CONTEXT *pCallContext, struct RS_RES_ALLOC_PARAMS_INTERNAL *pParams);
#define __objCreate_Subdevice(__nvoc_ppNewObj, __nvoc_pParent, __nvoc_createFlags, pCallContext, pParams) \
    __nvoc_objCreate_Subdevice((__nvoc_ppNewObj), staticCast((__nvoc_pParent), Dynamic), (__nvoc_createFlags), pCallContext, pParams)


// Wrapper macros for implementation functions
NV_STATUS subdeviceConstruct_IMPL(struct Subdevice *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_ALLOC_PARAMS_INTERNAL *pParams);
#define __nvoc_subdeviceConstruct(pResource, pCallContext, pParams) subdeviceConstruct_IMPL(pResource, pCallContext, pParams)

void subdeviceDestruct_IMPL(struct Subdevice *pResource);
#define __nvoc_subdeviceDestruct(pResource) subdeviceDestruct_IMPL(pResource)

void subdeviceUnsetGpuDebugMode_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline void subdeviceUnsetGpuDebugMode(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceUnsetGpuDebugMode(pSubdevice) subdeviceUnsetGpuDebugMode_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

void subdeviceReleaseComputeModeReservation_IMPL(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext);
#ifdef __nvoc_subdevice_h_disabled
static inline void subdeviceReleaseComputeModeReservation(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceReleaseComputeModeReservation(pSubdevice, pCallContext) subdeviceReleaseComputeModeReservation_IMPL(pSubdevice, pCallContext)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceGetByHandle_IMPL(struct RsClient *pClient, NvHandle hSubdevice, struct Subdevice **ppSubdevice);
#define subdeviceGetByHandle(pClient, hSubdevice, ppSubdevice) subdeviceGetByHandle_IMPL(pClient, hSubdevice, ppSubdevice)

NV_STATUS subdeviceGetByGpu_IMPL(struct RsClient *pClient, struct OBJGPU *pGpu, struct Subdevice **ppSubdevice);
#define subdeviceGetByGpu(pClient, pGpu, ppSubdevice) subdeviceGetByGpu_IMPL(pClient, pGpu, ppSubdevice)

NV_STATUS subdeviceGetByDeviceAndGpu_IMPL(struct RsClient *pClient, struct Device *pDevice, struct OBJGPU *pGpu, struct Subdevice **ppSubdevice);
#define subdeviceGetByDeviceAndGpu(pClient, pDevice, pGpu, ppSubdevice) subdeviceGetByDeviceAndGpu_IMPL(pClient, pDevice, pGpu, ppSubdevice)

NV_STATUS subdeviceGetByInstance_IMPL(struct RsClient *pClient, NvHandle hDevice, NvU32 subDeviceInst, struct Subdevice **ppSubdevice);
#define subdeviceGetByInstance(pClient, hDevice, subDeviceInst, ppSubdevice) subdeviceGetByInstance_IMPL(pClient, hDevice, subDeviceInst, ppSubdevice)

NV_STATUS subdeviceCtrlCmdGpuGetCachedInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetCachedInfo(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetCachedInfo(pSubdevice, pGpuInfoParams) subdeviceCtrlCmdGpuGetCachedInfo_IMPL(pSubdevice, pGpuInfoParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuForceGspUnload_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_FORCE_GSP_UNLOAD_PARAMS *pGpuInfoParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuForceGspUnload(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_FORCE_GSP_UNLOAD_PARAMS *pGpuInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuForceGspUnload(pSubdevice, pGpuInfoParams) subdeviceCtrlCmdGpuForceGspUnload_IMPL(pSubdevice, pGpuInfoParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetInfoV2_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetInfoV2(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetInfoV2(pSubdevice, pGpuInfoParams) subdeviceCtrlCmdGpuGetInfoV2_IMPL(pSubdevice, pGpuInfoParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetIpVersion_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_IP_VERSION_PARAMS *pGpuIpVersionParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetIpVersion(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_IP_VERSION_PARAMS *pGpuIpVersionParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetIpVersion(pSubdevice, pGpuIpVersionParams) subdeviceCtrlCmdGpuGetIpVersion_IMPL(pSubdevice, pGpuIpVersionParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuSetOptimusInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_OPTIMUS_INFO_PARAMS *pGpuOptimusInfoParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuSetOptimusInfo(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_OPTIMUS_INFO_PARAMS *pGpuOptimusInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuSetOptimusInfo(pSubdevice, pGpuOptimusInfoParams) subdeviceCtrlCmdGpuSetOptimusInfo_IMPL(pSubdevice, pGpuOptimusInfoParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetNameString_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_NAME_STRING_PARAMS *pNameStringParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetNameString(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_NAME_STRING_PARAMS *pNameStringParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetNameString(pSubdevice, pNameStringParams) subdeviceCtrlCmdGpuGetNameString_IMPL(pSubdevice, pNameStringParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetShortNameString_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SHORT_NAME_STRING_PARAMS *pShortNameStringParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetShortNameString(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SHORT_NAME_STRING_PARAMS *pShortNameStringParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetShortNameString(pSubdevice, pShortNameStringParams) subdeviceCtrlCmdGpuGetShortNameString_IMPL(pSubdevice, pShortNameStringParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetSdm_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SDM_PARAMS *pSdmParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetSdm(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SDM_PARAMS *pSdmParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetSdm(pSubdevice, pSdmParams) subdeviceCtrlCmdGpuGetSdm_IMPL(pSubdevice, pSdmParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetSimulationInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SIMULATION_INFO_PARAMS *pGpuSimulationInfoParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetSimulationInfo(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_SIMULATION_INFO_PARAMS *pGpuSimulationInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetSimulationInfo(pSubdevice, pGpuSimulationInfoParams) subdeviceCtrlCmdGpuGetSimulationInfo_IMPL(pSubdevice, pGpuSimulationInfoParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetEngines_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINES_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetEngines(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetEngines(pSubdevice, pParams) subdeviceCtrlCmdGpuGetEngines_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetEnginesV2_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINES_V2_PARAMS *pEngineParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetEnginesV2(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINES_V2_PARAMS *pEngineParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetEnginesV2(pSubdevice, pEngineParams) subdeviceCtrlCmdGpuGetEnginesV2_IMPL(pSubdevice, pEngineParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetEngineClasslist_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_CLASSLIST_PARAMS *pClassParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetEngineClasslist(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_CLASSLIST_PARAMS *pClassParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetEngineClasslist(pSubdevice, pClassParams) subdeviceCtrlCmdGpuGetEngineClasslist_IMPL(pSubdevice, pClassParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetEnginePartnerList_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_PARTNERLIST_PARAMS *pPartnerListParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetEnginePartnerList(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_PARTNERLIST_PARAMS *pPartnerListParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetEnginePartnerList(pSubdevice, pPartnerListParams) subdeviceCtrlCmdGpuGetEnginePartnerList_IMPL(pSubdevice, pPartnerListParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetChipDetails_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_CHIP_DETAILS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetChipDetails(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_CHIP_DETAILS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetChipDetails(pSubdevice, pParams) subdeviceCtrlCmdGpuGetChipDetails_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetOEMBoardInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_OEM_BOARD_INFO_PARAMS *pBoardInfo);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetOEMBoardInfo(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_OEM_BOARD_INFO_PARAMS *pBoardInfo) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetOEMBoardInfo(pSubdevice, pBoardInfo) subdeviceCtrlCmdGpuGetOEMBoardInfo_IMPL(pSubdevice, pBoardInfo)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetOEMInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_OEM_INFO_PARAMS *pOemInfo);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetOEMInfo(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_OEM_INFO_PARAMS *pOemInfo) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetOEMInfo(pSubdevice, pOemInfo) subdeviceCtrlCmdGpuGetOEMInfo_IMPL(pSubdevice, pOemInfo)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuHandleGpuSR_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuHandleGpuSR(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuHandleGpuSR(pSubdevice) subdeviceCtrlCmdGpuHandleGpuSR_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuInitializeCtx_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_INITIALIZE_CTX_PARAMS *pInitializeCtxParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuInitializeCtx(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_INITIALIZE_CTX_PARAMS *pInitializeCtxParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuInitializeCtx(pSubdevice, pInitializeCtxParams) subdeviceCtrlCmdGpuInitializeCtx_IMPL(pSubdevice, pInitializeCtxParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuPromoteCtx_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_PROMOTE_CTX_PARAMS *pPromoteCtxParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuPromoteCtx(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_PROMOTE_CTX_PARAMS *pPromoteCtxParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuPromoteCtx(pSubdevice, pPromoteCtxParams) subdeviceCtrlCmdGpuPromoteCtx_IMPL(pSubdevice, pPromoteCtxParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuEvictCtx_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_EVICT_CTX_PARAMS *pEvictCtxParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuEvictCtx(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_EVICT_CTX_PARAMS *pEvictCtxParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuEvictCtx(pSubdevice, pEvictCtxParams) subdeviceCtrlCmdGpuEvictCtx_IMPL(pSubdevice, pEvictCtxParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetId_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ID_PARAMS *pIdParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetId(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ID_PARAMS *pIdParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetId(pSubdevice, pIdParams) subdeviceCtrlCmdGpuGetId_IMPL(pSubdevice, pIdParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetGidInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_GID_INFO_PARAMS *pGidInfoParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetGidInfo(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_GID_INFO_PARAMS *pGidInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetGidInfo(pSubdevice, pGidInfoParams) subdeviceCtrlCmdGpuGetGidInfo_IMPL(pSubdevice, pGidInfoParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetPids_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_PIDS_PARAMS *pGetPidsParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetPids(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_PIDS_PARAMS *pGetPidsParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetPids(pSubdevice, pGetPidsParams) subdeviceCtrlCmdGpuGetPids_IMPL(pSubdevice, pGetPidsParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetPidInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_PID_INFO_PARAMS *pGetPidInfoParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetPidInfo(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_PID_INFO_PARAMS *pGetPidInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetPidInfo(pSubdevice, pGetPidInfoParams) subdeviceCtrlCmdGpuGetPidInfo_IMPL(pSubdevice, pGetPidInfoParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuQueryFunctionStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_QUERY_FUNCTION_STATUS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuQueryFunctionStatus(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_QUERY_FUNCTION_STATUS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuQueryFunctionStatus(pSubdevice, pParams) subdeviceCtrlCmdGpuQueryFunctionStatus_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetFirstAsyncCEIdx_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_FIRST_ASYNC_CE_IDX_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetFirstAsyncCEIdx(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_FIRST_ASYNC_CE_IDX_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetFirstAsyncCEIdx(pSubdevice, pParams) subdeviceCtrlCmdGpuGetFirstAsyncCEIdx_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetVmmuSegmentSize_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_VMMU_SEGMENT_SIZE_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetVmmuSegmentSize(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_VMMU_SEGMENT_SIZE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetVmmuSegmentSize(pSubdevice, pParams) subdeviceCtrlCmdGpuGetVmmuSegmentSize_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetMaxSupportedPageSize_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_MAX_SUPPORTED_PAGE_SIZE_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetMaxSupportedPageSize(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_MAX_SUPPORTED_PAGE_SIZE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetMaxSupportedPageSize(pSubdevice, pParams) subdeviceCtrlCmdGpuGetMaxSupportedPageSize_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuHandleVfPriFault_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_HANDLE_VF_PRI_FAULT_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuHandleVfPriFault(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_HANDLE_VF_PRI_FAULT_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuHandleVfPriFault(pSubdevice, pParams) subdeviceCtrlCmdGpuHandleVfPriFault_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdValidateMemMapRequest_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_VALIDATE_MEM_MAP_REQUEST_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdValidateMemMapRequest(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_VALIDATE_MEM_MAP_REQUEST_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdValidateMemMapRequest(pSubdevice, pParams) subdeviceCtrlCmdValidateMemMapRequest_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetGfid_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_GFID_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetGfid(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_GFID_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetGfid(pSubdevice, pParams) subdeviceCtrlCmdGpuGetGfid_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdUpdateGfidP2pCapability_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_UPDATE_GFID_P2P_CAPABILITY_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdUpdateGfidP2pCapability(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GPU_UPDATE_GFID_P2P_CAPABILITY_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdUpdateGfidP2pCapability(pSubdevice, pParams) subdeviceCtrlCmdUpdateGfidP2pCapability_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetEngineLoadTimes_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_LOAD_TIMES_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetEngineLoadTimes(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_ENGINE_LOAD_TIMES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetEngineLoadTimes(pSubdevice, pParams) subdeviceCtrlCmdGpuGetEngineLoadTimes_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuMarkDeviceForReset_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuMarkDeviceForReset(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuMarkDeviceForReset(pSubdevice) subdeviceCtrlCmdGpuMarkDeviceForReset_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuUnmarkDeviceForReset_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuUnmarkDeviceForReset(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuUnmarkDeviceForReset(pSubdevice) subdeviceCtrlCmdGpuUnmarkDeviceForReset_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset(pSubdevice) subdeviceCtrlCmdGpuMarkDeviceForDrainAndReset_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset(pSubdevice) subdeviceCtrlCmdGpuUnmarkDeviceForDrainAndReset_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetResetStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_RESET_STATUS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetResetStatus(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_RESET_STATUS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetResetStatus(pSubdevice, pParams) subdeviceCtrlCmdGpuGetResetStatus_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetDrainAndResetStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_DRAIN_AND_RESET_STATUS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetDrainAndResetStatus(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_DRAIN_AND_RESET_STATUS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetDrainAndResetStatus(pSubdevice, pParams) subdeviceCtrlCmdGpuGetDrainAndResetStatus_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetConstructedFalconInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_CONSTRUCTED_FALCON_INFO_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetConstructedFalconInfo(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_CONSTRUCTED_FALCON_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetConstructedFalconInfo(pSubdevice, pParams) subdeviceCtrlCmdGpuGetConstructedFalconInfo_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlGpuGetFipsStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_FIPS_STATUS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlGpuGetFipsStatus(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_FIPS_STATUS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlGpuGetFipsStatus(pSubdevice, pParams) subdeviceCtrlGpuGetFipsStatus_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetVfCaps_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_VF_CAPS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetVfCaps(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_VF_CAPS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetVfCaps(pSubdevice, pParams) subdeviceCtrlCmdGpuGetVfCaps_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetRecoveryAction_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_RECOVERY_ACTION_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetRecoveryAction(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_RECOVERY_ACTION_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetRecoveryAction(pSubdevice, pParams) subdeviceCtrlCmdGpuGetRecoveryAction_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuRpcGspTest_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RPC_GSP_TEST_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuRpcGspTest(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RPC_GSP_TEST_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuRpcGspTest(pSubdevice, pParams) subdeviceCtrlCmdGpuRpcGspTest_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuRpcGspQuerySizes_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RPC_GSP_QUERY_SIZES_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuRpcGspQuerySizes(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RPC_GSP_QUERY_SIZES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuRpcGspQuerySizes(pSubdevice, pParams) subdeviceCtrlCmdGpuRpcGspQuerySizes_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdRusdGetSupportedFeatures_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_RUSD_GET_SUPPORTED_FEATURES_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdRusdGetSupportedFeatures(struct Subdevice *pSubdevice, NV2080_CTRL_RUSD_GET_SUPPORTED_FEATURES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdRusdGetSupportedFeatures(pSubdevice, pParams) subdeviceCtrlCmdRusdGetSupportedFeatures_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdRusdSetFeatures_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RUSD_SET_FEATURES_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdRusdSetFeatures(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_RUSD_SET_FEATURES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdRusdSetFeatures(pSubdevice, pParams) subdeviceCtrlCmdRusdSetFeatures_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuErrorInjectionControl_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_ERROR_INJECTION_CONTROL_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuErrorInjectionControl(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_ERROR_INJECTION_CONTROL_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuErrorInjectionControl(pSubdevice, pParams) subdeviceCtrlCmdGpuErrorInjectionControl_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuCheckMemSubsysError_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_CHECK_MEM_SUBSYS_ERROR_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuCheckMemSubsysError(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_CHECK_MEM_SUBSYS_ERROR_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuCheckMemSubsysError(pSubdevice, pParams) subdeviceCtrlCmdGpuCheckMemSubsysError_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetDefaultTimeout_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_DEFAULT_TIMEOUT_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetDefaultTimeout(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_DEFAULT_TIMEOUT_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetDefaultTimeout(pSubdevice, pParams) subdeviceCtrlCmdGpuGetDefaultTimeout_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEventSetTrigger_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEventSetTrigger(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEventSetTrigger(pSubdevice) subdeviceCtrlCmdEventSetTrigger_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEventSetTriggerFifo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_TRIGGER_FIFO_PARAMS *pTriggerFifoParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEventSetTriggerFifo(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_TRIGGER_FIFO_PARAMS *pTriggerFifoParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEventSetTriggerFifo(pSubdevice, pTriggerFifoParams) subdeviceCtrlCmdEventSetTriggerFifo_IMPL(pSubdevice, pTriggerFifoParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEventSetNotification_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_NOTIFICATION_PARAMS *pSetEventParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEventSetNotification(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_NOTIFICATION_PARAMS *pSetEventParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEventSetNotification(pSubdevice, pSetEventParams) subdeviceCtrlCmdEventSetNotification_IMPL(pSubdevice, pSetEventParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEventSetMemoryNotifies_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_MEMORY_NOTIFIES_PARAMS *pSetMemoryNotifiesParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEventSetMemoryNotifies(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_MEMORY_NOTIFIES_PARAMS *pSetMemoryNotifiesParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEventSetMemoryNotifies(pSubdevice, pSetMemoryNotifiesParams) subdeviceCtrlCmdEventSetMemoryNotifies_IMPL(pSubdevice, pSetMemoryNotifiesParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEventSetSemaphoreMemory_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_SEMAPHORE_MEMORY_PARAMS *pSetSemMemoryParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEventSetSemaphoreMemory(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_SEMAPHORE_MEMORY_PARAMS *pSetSemMemoryParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEventSetSemaphoreMemory(pSubdevice, pSetSemMemoryParams) subdeviceCtrlCmdEventSetSemaphoreMemory_IMPL(pSubdevice, pSetSemMemoryParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEventSetSemaMemValidation_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_SEMA_MEM_VALIDATION_PARAMS *pSetSemaMemValidationParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEventSetSemaMemValidation(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_SET_SEMA_MEM_VALIDATION_PARAMS *pSetSemaMemValidationParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEventSetSemaMemValidation(pSubdevice, pSetSemaMemValidationParams) subdeviceCtrlCmdEventSetSemaMemValidation_IMPL(pSubdevice, pSetSemaMemValidationParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_RATS_GSP_TRACE_BIND_EVTBUF_PARAMS *pBindParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf(struct Subdevice *pSubdevice, NV2080_CTRL_EVENT_RATS_GSP_TRACE_BIND_EVTBUF_PARAMS *pBindParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf(pSubdevice, pBindParams) subdeviceCtrlCmdEventGspTraceRatsBindEvtbuf_IMPL(pSubdevice, pBindParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdTimerCancel_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdTimerCancel(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdTimerCancel(pSubdevice) subdeviceCtrlCmdTimerCancel_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdTimerSchedule_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_TIMER_SCHEDULE_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdTimerSchedule(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_TIMER_SCHEDULE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdTimerSchedule(pSubdevice, pParams) subdeviceCtrlCmdTimerSchedule_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdTimerGetTime_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_TIME_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdTimerGetTime(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_TIME_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdTimerGetTime(pSubdevice, pParams) subdeviceCtrlCmdTimerGetTime_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdTimerGetRegisterOffset_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_REGISTER_OFFSET_PARAMS *pTimerRegOffsetParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdTimerGetRegisterOffset(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_REGISTER_OFFSET_PARAMS *pTimerRegOffsetParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdTimerGetRegisterOffset(pSubdevice, pTimerRegOffsetParams) subdeviceCtrlCmdTimerGetRegisterOffset_IMPL(pSubdevice, pTimerRegOffsetParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_GPU_CPU_TIME_CORRELATION_INFO_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo(struct Subdevice *pSubdevice, NV2080_CTRL_TIMER_GET_GPU_CPU_TIME_CORRELATION_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo(pSubdevice, pParams) subdeviceCtrlCmdTimerGetGpuCpuTimeCorrelationInfo_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEccGetClientExposedCounters_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_CLIENT_EXPOSED_COUNTERS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEccGetClientExposedCounters(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_CLIENT_EXPOSED_COUNTERS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEccGetClientExposedCounters(pSubdevice, pParams) subdeviceCtrlCmdEccGetClientExposedCounters_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEccGetVolatileCounts_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_VOLATILE_COUNTS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEccGetVolatileCounts(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_VOLATILE_COUNTS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEccGetVolatileCounts(pSubdevice, pParams) subdeviceCtrlCmdEccGetVolatileCounts_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEccInjectError_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_INJECT_ERROR_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEccInjectError(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_INJECT_ERROR_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEccInjectError(pSubdevice, pParams) subdeviceCtrlCmdEccInjectError_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEccGetRepairStatus_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_REPAIR_STATUS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEccGetRepairStatus(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_GET_REPAIR_STATUS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEccGetRepairStatus(pSubdevice, pParams) subdeviceCtrlCmdEccGetRepairStatus_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdEccInjectionSupported_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_INJECTION_SUPPORTED_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdEccInjectionSupported(struct Subdevice *pSubdevice, NV2080_CTRL_ECC_INJECTION_SUPPORTED_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdEccInjectionSupported(pSubdevice, pParams) subdeviceCtrlCmdEccInjectionSupported_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGspGetFeatures(struct Subdevice *pSubdevice, NV2080_CTRL_GSP_GET_FEATURES_PARAMS *pGspFeaturesParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGspGetFeatures(pSubdevice, pGspFeaturesParams) subdeviceCtrlCmdGspGetFeatures_92bfc3(pSubdevice, pGspFeaturesParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGspGetRmHeapStats_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GSP_GET_RM_HEAP_STATS_PARAMS *pGspRmHeapStatsParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGspGetRmHeapStats(struct Subdevice *pSubdevice, NV2080_CTRL_GSP_GET_RM_HEAP_STATS_PARAMS *pGspRmHeapStatsParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGspGetRmHeapStats(pSubdevice, pGspRmHeapStatsParams) subdeviceCtrlCmdGspGetRmHeapStats_IMPL(pSubdevice, pGspRmHeapStatsParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGpuGetVgpuHeapStats_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_VGPU_HEAP_STATS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGpuGetVgpuHeapStats(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_VGPU_HEAP_STATS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGpuGetVgpuHeapStats(pSubdevice, pParams) subdeviceCtrlCmdGpuGetVgpuHeapStats_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdLibosGetHeapStats_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_LIBOS_HEAP_STATS_PARAMS *pGspLibosHeapStatsParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdLibosGetHeapStats(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_LIBOS_HEAP_STATS_PARAMS *pGspLibosHeapStatsParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdLibosGetHeapStats(pSubdevice, pGspLibosHeapStatsParams) subdeviceCtrlCmdLibosGetHeapStats_IMPL(pSubdevice, pGspLibosHeapStatsParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdGspGdmaFuzzTest_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GDMA_FUZZ_TEST_PARAMS *pGspGdmaFuzzTestParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdGspGdmaFuzzTest(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GDMA_FUZZ_TEST_PARAMS *pGspGdmaFuzzTestParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdGspGdmaFuzzTest(pSubdevice, pGspGdmaFuzzTestParams) subdeviceCtrlCmdGspGdmaFuzzTest_IMPL(pSubdevice, pGspGdmaFuzzTestParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdOsUnixGc6BlockerRefCnt_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_GC6_BLOCKER_REFCNT_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdOsUnixGc6BlockerRefCnt(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_GC6_BLOCKER_REFCNT_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdOsUnixGc6BlockerRefCnt(pSubdevice, pParams) subdeviceCtrlCmdOsUnixGc6BlockerRefCnt_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdOsUnixAllowDisallowGcoff_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_ALLOW_DISALLOW_GCOFF_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdOsUnixAllowDisallowGcoff(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_ALLOW_DISALLOW_GCOFF_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdOsUnixAllowDisallowGcoff(pSubdevice, pParams) subdeviceCtrlCmdOsUnixAllowDisallowGcoff_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdOsUnixAudioDynamicPower_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_AUDIO_DYNAMIC_POWER_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdOsUnixAudioDynamicPower(struct Subdevice *pSubdevice, NV2080_CTRL_OS_UNIX_AUDIO_DYNAMIC_POWER_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdOsUnixAudioDynamicPower(pSubdevice, pParams) subdeviceCtrlCmdOsUnixAudioDynamicPower_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdDisplayGetIpVersion_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_IP_VERSION_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdDisplayGetIpVersion(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_IP_VERSION_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdDisplayGetIpVersion(pSubdevice, pParams) subdeviceCtrlCmdDisplayGetIpVersion_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdDisplayGetStaticInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_STATIC_INFO_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdDisplayGetStaticInfo(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_STATIC_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdDisplayGetStaticInfo(pSubdevice, pParams) subdeviceCtrlCmdDisplayGetStaticInfo_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdDisplaySetChannelPushbuffer_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_CHANNEL_PUSHBUFFER_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdDisplaySetChannelPushbuffer(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_CHANNEL_PUSHBUFFER_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdDisplaySetChannelPushbuffer(pSubdevice, pParams) subdeviceCtrlCmdDisplaySetChannelPushbuffer_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdDisplayWriteInstMem_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_WRITE_INST_MEM_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdDisplayWriteInstMem(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_WRITE_INST_MEM_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdDisplayWriteInstMem(pSubdevice, pParams) subdeviceCtrlCmdDisplayWriteInstMem_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdDisplaySetupRgLineIntr_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_SETUP_RG_LINE_INTR_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdDisplaySetupRgLineIntr(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_SETUP_RG_LINE_INTR_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdDisplaySetupRgLineIntr(pSubdevice, pParams) subdeviceCtrlCmdDisplaySetupRgLineIntr_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdDisplaySetImportedImpData_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_SET_IMP_INIT_INFO_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdDisplaySetImportedImpData(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_SET_IMP_INIT_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdDisplaySetImportedImpData(pSubdevice, pParams) subdeviceCtrlCmdDisplaySetImportedImpData_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdDisplayGetDisplayMask_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_ACTIVE_DISPLAY_DEVICES_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdDisplayGetDisplayMask(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISPLAY_GET_ACTIVE_DISPLAY_DEVICES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdDisplayGetDisplayMask(pSubdevice, pParams) subdeviceCtrlCmdDisplayGetDisplayMask_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdDisplayPinsetsToLockpins_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISP_PINSETS_TO_LOCKPINS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdDisplayPinsetsToLockpins(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISP_PINSETS_TO_LOCKPINS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdDisplayPinsetsToLockpins(pSubdevice, pParams) subdeviceCtrlCmdDisplayPinsetsToLockpins_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISP_SET_SLI_LINK_GPIO_SW_CONTROL_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_DISP_SET_SLI_LINK_GPIO_SW_CONTROL_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl(pSubdevice, pParams) subdeviceCtrlCmdDisplaySetSliLinkGpioSwControl_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGpioProgramDirection_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_DIRECTION_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGpioProgramDirection(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_DIRECTION_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGpioProgramDirection(pSubdevice, pParams) subdeviceCtrlCmdInternalGpioProgramDirection_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGpioProgramOutput_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_OUTPUT_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGpioProgramOutput(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_PROGRAM_OUTPUT_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGpioProgramOutput(pSubdevice, pParams) subdeviceCtrlCmdInternalGpioProgramOutput_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGpioReadInput_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_READ_INPUT_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGpioReadInput(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_READ_INPUT_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGpioReadInput(pSubdevice, pParams) subdeviceCtrlCmdInternalGpioReadInput_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGpioActivateHwFunction_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_ACTIVATE_HW_FUNCTION_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGpioActivateHwFunction(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPIO_ACTIVATE_HW_FUNCTION_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGpioActivateHwFunction(pSubdevice, pParams) subdeviceCtrlCmdInternalGpioActivateHwFunction_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated(pSubdevice) subdeviceCtrlCmdInternalDisplayAcpiSubsytemActivated_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalDisplayPreModeSet_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalDisplayPreModeSet(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalDisplayPreModeSet(pSubdevice) subdeviceCtrlCmdInternalDisplayPreModeSet_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalDisplayPostModeSet_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalDisplayPostModeSet(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalDisplayPostModeSet(pSubdevice) subdeviceCtrlCmdInternalDisplayPostModeSet_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGetChipInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_CHIP_INFO_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGetChipInfo(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_CHIP_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGetChipInfo(pSubdevice, pParams) subdeviceCtrlCmdInternalGetChipInfo_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGetUserRegisterAccessMap_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_USER_REGISTER_ACCESS_MAP_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGetUserRegisterAccessMap(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_USER_REGISTER_ACCESS_MAP_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGetUserRegisterAccessMap(pSubdevice, pParams) subdeviceCtrlCmdInternalGetUserRegisterAccessMap_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGetDeviceInfoTable_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_DEVICE_INFO_TABLE_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGetDeviceInfoTable(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_DEVICE_INFO_TABLE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGetDeviceInfoTable(pSubdevice, pParams) subdeviceCtrlCmdInternalGetDeviceInfoTable_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalRecoverAllComputeContexts_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalRecoverAllComputeContexts(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalRecoverAllComputeContexts(pSubdevice) subdeviceCtrlCmdInternalRecoverAllComputeContexts_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGetSmcMode_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_SMC_MODE_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGetSmcMode(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_SMC_MODE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGetSmcMode(pSubdevice, pParams) subdeviceCtrlCmdInternalGetSmcMode_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalCheckCtsIdValid_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_CHECK_CTS_ID_VALID_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalCheckCtsIdValid(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_CHECK_CTS_ID_VALID_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalCheckCtsIdValid(pSubdevice, pParams) subdeviceCtrlCmdInternalCheckCtsIdValid_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdIsEgpuBridge_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_EGPU_BRIDGE_INFO_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdIsEgpuBridge(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_EGPU_BRIDGE_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdIsEgpuBridge(pSubdevice, pParams) subdeviceCtrlCmdIsEgpuBridge_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_GSP_RM_FREE_HEAP_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_GET_GSP_RM_FREE_HEAP_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap(pSubdevice, pParams) subdeviceCtrlCmdInternalGpuGetGspRmFreeHeap_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_VMMU_GET_SPA_FOR_GPA_ENTRIES_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_VMMU_GET_SPA_FOR_GPA_ENTRIES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries(pSubdevice, pParams) subdeviceCtrlCmdInternalVmmuGetSpaForGpaEntries_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGcxEntryPrerequisite_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GCX_ENTRY_PREREQUISITE_PARAMS *pGcxEntryPrerequisite);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGcxEntryPrerequisite(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GCX_ENTRY_PREREQUISITE_PARAMS *pGcxEntryPrerequisite) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGcxEntryPrerequisite(pSubdevice, pGcxEntryPrerequisite) subdeviceCtrlCmdInternalGcxEntryPrerequisite_IMPL(pSubdevice, pGcxEntryPrerequisite)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalSetP2pCaps_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_SET_P2P_CAPS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalSetP2pCaps(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_SET_P2P_CAPS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalSetP2pCaps(pSubdevice, pParams) subdeviceCtrlCmdInternalSetP2pCaps_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalRemoveP2pCaps_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_REMOVE_P2P_CAPS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalRemoveP2pCaps(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_REMOVE_P2P_CAPS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalRemoveP2pCaps(pSubdevice, pParams) subdeviceCtrlCmdInternalRemoveP2pCaps_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGetPcieP2pCaps_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_PCIE_P2P_CAPS_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGetPcieP2pCaps(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GET_PCIE_P2P_CAPS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGetPcieP2pCaps(pSubdevice, pParams) subdeviceCtrlCmdInternalGetPcieP2pCaps_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalPostInitBrightcStateLoad_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_INIT_BRIGHTC_STATE_LOAD_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalPostInitBrightcStateLoad(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_INIT_BRIGHTC_STATE_LOAD_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalPostInitBrightcStateLoad(pSubdevice, pParams) subdeviceCtrlCmdInternalPostInitBrightcStateLoad_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalSetStaticEdidData_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_INTERNAL_SET_STATIC_EDID_DATA_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalSetStaticEdidData(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_INTERNAL_SET_STATIC_EDID_DATA_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalSetStaticEdidData(pSubdevice, pParams) subdeviceCtrlCmdInternalSetStaticEdidData_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalDetectHsVideoBridge_IMPL(struct Subdevice *pSubdevice);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalDetectHsVideoBridge(struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalDetectHsVideoBridge(pSubdevice) subdeviceCtrlCmdInternalDetectHsVideoBridge_IMPL(pSubdevice)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalInitUserSharedData_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_INIT_USER_SHARED_DATA_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalInitUserSharedData(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_INIT_USER_SHARED_DATA_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalInitUserSharedData(pSubdevice, pParams) subdeviceCtrlCmdInternalInitUserSharedData_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalUserSharedDataSetDataPoll_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_USER_SHARED_DATA_SET_DATA_POLL_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalUserSharedDataSetDataPoll(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_USER_SHARED_DATA_SET_DATA_POLL_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalUserSharedDataSetDataPoll(pSubdevice, pParams) subdeviceCtrlCmdInternalUserSharedDataSetDataPoll_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalControlGspTrace_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_INTERNAL_CONTROL_GSP_TRACE_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalControlGspTrace(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_INTERNAL_CONTROL_GSP_TRACE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalControlGspTrace(pSubdevice, pParams) subdeviceCtrlCmdInternalControlGspTrace_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_CLIENT_LOW_POWER_MODE_ENTER_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_CLIENT_LOW_POWER_MODE_ENTER_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter(pSubdevice, pParams) subdeviceCtrlCmdInternalGpuClientLowPowerModeEnter_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled

NV_STATUS subdeviceCtrlCmdInternalLogOobXid_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_LOG_OOB_XID_PARAMS *pParams);
#ifdef __nvoc_subdevice_h_disabled
static inline NV_STATUS subdeviceCtrlCmdInternalLogOobXid(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_LOG_OOB_XID_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("Subdevice was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_subdevice_h_disabled
#define subdeviceCtrlCmdInternalLogOobXid(pSubdevice, pParams) subdeviceCtrlCmdInternalLogOobXid_IMPL(pSubdevice, pParams)
#endif // __nvoc_subdevice_h_disabled


// Wrapper macros for halified functions
#define subdevicePreDestruct_FNPTR(pResource) pResource->__nvoc_metadata_ptr->vtable.__subdevicePreDestruct__
#define subdevicePreDestruct(pResource) subdevicePreDestruct_DISPATCH(pResource)
#define subdeviceInternalControlForward_FNPTR(pSubdevice) pSubdevice->__nvoc_metadata_ptr->vtable.__subdeviceInternalControlForward__
#define subdeviceInternalControlForward(pSubdevice, command, pParams, size) subdeviceInternalControlForward_DISPATCH(pSubdevice, command, pParams, size)
#define subdeviceCtrlCmdGspGetFeatures_HAL(pSubdevice, pGspFeaturesParams) subdeviceCtrlCmdGspGetFeatures(pSubdevice, pGspFeaturesParams)
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

// Virtual method declarations and/or inline definitions
void subdevicePreDestruct_IMPL(struct Subdevice *pResource);

NV_STATUS subdeviceInternalControlForward_IMPL(struct Subdevice *pSubdevice, NvU32 command, void *pParams, NvU32 size);

// Exported method declarations and/or inline definitions
NV_STATUS subdeviceCtrlCmdGpuGetCachedInfo_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams);

NV_STATUS subdeviceCtrlCmdGpuForceGspUnload_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_FORCE_GSP_UNLOAD_PARAMS *pGpuInfoParams);

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

NV_STATUS subdeviceCtrlCmdGpuErrorInjectionControl_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_ERROR_INJECTION_CONTROL_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuCheckMemSubsysError_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_CHECK_MEM_SUBSYS_ERROR_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdGpuGetDefaultTimeout_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_DEFAULT_TIMEOUT_PARAMS *pParams);

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

NV_STATUS subdeviceCtrlCmdGspGetRmHeapStats_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_GSP_GET_RM_HEAP_STATS_PARAMS *pGspRmHeapStatsParams);

NV_STATUS subdeviceCtrlCmdGpuGetVgpuHeapStats_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_VGPU_HEAP_STATS_PARAMS *pParams);

NV_STATUS subdeviceCtrlCmdLibosGetHeapStats_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GET_LIBOS_HEAP_STATS_PARAMS *pGspLibosHeapStatsParams);

NV_STATUS subdeviceCtrlCmdGspGdmaFuzzTest_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_CMD_GSP_GDMA_FUZZ_TEST_PARAMS *pGspGdmaFuzzTestParams);

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

NV_STATUS subdeviceCtrlCmdInternalCheckCtsIdValid_IMPL(struct Subdevice *pSubdevice, NV2080_CTRL_INTERNAL_GPU_CHECK_CTS_ID_VALID_PARAMS *pParams);

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

// HAL method declarations without bodies
// Inline HAL method definitions
static inline NV_STATUS subdeviceCtrlCmdGspGetFeatures_92bfc3(struct Subdevice *pSubdevice, NV2080_CTRL_GSP_GET_FEATURES_PARAMS *pGspFeaturesParams){
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}

// Static dispatch method declarations
// Static inline method definitions
static inline NV_STATUS subdeviceSetPerfmonReservation(struct Subdevice *pSubdevice, NvBool bReservation, NvBool bClientHandlesGrGating, NvBool bRmHandlesIdleSlow){
    return NV_OK;
}

static inline NV_STATUS subdeviceUnsetDynamicBoostLimit(struct Subdevice *pSubdevice){
    return NV_OK;
}

static inline NV_STATUS subdeviceReleaseVideoStreams(struct Subdevice *pSubdevice){
    return NV_OK;
}

static inline void subdeviceRestoreLockedClock(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext){
    return;
}

static inline void subdeviceRestoreVF(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext){
    return;
}

static inline void subdeviceReleaseNvlinkErrorInjectionMode(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext){
    return;
}

static inline void subdeviceRestoreGrTickFreq(struct Subdevice *pSubdevice, struct CALL_CONTEXT *pCallContext){
    return;
}

static inline void subdeviceRestoreWatchdog(struct Subdevice *pSubdevice){
    return;
}

#undef PRIVATE_FIELD


#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _G_SUBDEVICE_NVOC_H_
