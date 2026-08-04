
#ifndef _G_CLIENT_RESOURCE_NVOC_H_
#define _G_CLIENT_RESOURCE_NVOC_H_

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
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "g_client_resource_nvoc.h"


#ifndef _CLIENT_RESOURCE_H_
#define _CLIENT_RESOURCE_H_

#include "resserv/resserv.h"
#include "nvoc/prelude.h"
#include "resserv/rs_client.h"
#include "rmapi/resource.h"
#include "rmapi/event.h"
#include "rmapi/control.h"

#include "ctrl/ctrl0000/ctrl0000gpu.h"
#include "ctrl/ctrl0000/ctrl0000gpuacct.h"
#include "ctrl/ctrl0000/ctrl0000gsync.h"
#include "ctrl/ctrl0000/ctrl0000diag.h"
#include "ctrl/ctrl0000/ctrl0000event.h"
#include "ctrl/ctrl0000/ctrl0000nvd.h"
#include "ctrl/ctrl0000/ctrl0000proc.h"
#include "ctrl/ctrl0000/ctrl0000syncgpuboost.h"
#include "ctrl/ctrl0000/ctrl0000vgpu.h"
#include "ctrl/ctrl0000/ctrl0000client.h"

/* include appropriate os-specific command header */
#if defined(NV_UNIX) || defined(NV_QNX)
#include "ctrl/ctrl0000/ctrl0000unix.h"
#endif


// Private field names are wrapped in PRIVATE_FIELD, which does nothing for
// the matching C source file, but causes diagnostics to be issued if another
// source file references the field.
#ifdef NVOC_CLIENT_RESOURCE_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif


// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__RmClientResource;
struct NVOC_METADATA__RsClientResource;
struct NVOC_METADATA__RmResourceCommon;
struct NVOC_METADATA__Notifier;
struct NVOC_VTABLE__RmClientResource;


struct RmClientResource {

    // Metadata starts with RTTI structure.
    union {
         const struct NVOC_METADATA__RmClientResource *__nvoc_metadata_ptr;
         const struct NVOC_RTTI *__nvoc_rtti;
    };

    // Parent (i.e. superclass or base class) objects
    struct RsClientResource __nvoc_base_RsClientResource;
    struct RmResourceCommon __nvoc_base_RmResourceCommon;
    struct Notifier __nvoc_base_Notifier;

    // Ancestor object pointers for `staticCast` feature
    struct Object *__nvoc_pbase_Object;    // obj super^3
    struct RsResource *__nvoc_pbase_RsResource;    // res super^2
    struct RsClientResource *__nvoc_pbase_RsClientResource;    // clientres super
    struct RmResourceCommon *__nvoc_pbase_RmResourceCommon;    // rmrescmn super
    struct INotifier *__nvoc_pbase_INotifier;    // inotify super^2
    struct Notifier *__nvoc_pbase_Notifier;    // notify super
    struct RmClientResource *__nvoc_pbase_RmClientResource;    // clires

    // Vtable with 56 per-object function pointers
    NV_STATUS (*__cliresCtrlCmdSystemGetCpuInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_CPU_INFO_PARAMS *);  // exported (id=0x102)
    NV_STATUS (*__cliresCtrlCmdSystemGetFeatures__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_FEATURES_PARAMS *);  // exported (id=0x1f0)
    NV_STATUS (*__cliresCtrlCmdSystemGetBuildVersionV2__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_BUILD_VERSION_V2_PARAMS *);  // exported (id=0x13e)
    NV_STATUS (*__cliresCtrlCmdSystemGetLockTimes__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_LOCK_TIMES_PARAMS *);  // exported (id=0x109)
    NV_STATUS (*__cliresCtrlCmdSystemGetClassList__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_CLASSLIST_PARAMS *);  // exported (id=0x108)
    NV_STATUS (*__cliresCtrlCmdSystemNotifyEvent__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_NOTIFY_EVENT_PARAMS *);  // exported (id=0x110)
    NV_STATUS (*__cliresCtrlCmdSystemDebugCtrlRmMsg__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_DEBUG_RMMSG_CTRL_PARAMS *);  // exported (id=0x121)
    NV_STATUS (*__cliresCtrlCmdSystemGetVgxSystemInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_VGX_SYSTEM_INFO_PARAMS *);  // exported (id=0x133)
    NV_STATUS (*__cliresCtrlCmdSystemGetPrivilegedStatus__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_PRIVILEGED_STATUS_PARAMS *);  // exported (id=0x135)
    NV_STATUS (*__cliresCtrlCmdSystemGetFabricStatus__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_FABRIC_STATUS_PARAMS *);  // exported (id=0x136)
    NV_STATUS (*__cliresCtrlCmdSystemGetRmInstanceId__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_RM_INSTANCE_ID_PARAMS *);  // exported (id=0x139)
    NV_STATUS (*__cliresCtrlCmdSystemGetClientDatabaseInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_CLIENT_DATABASE_INFO_PARAMS *);  // exported (id=0x13d)
    NV_STATUS (*__cliresCtrlCmdSystemRmctrlCacheModeCtrl__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_RMCTRL_CACHE_MODE_CTRL_PARAMS *);  // exported (id=0x13f)
    NV_STATUS (*__cliresCtrlCmdClientGetAddrSpaceType__)(struct RmClientResource * /*this*/, NV0000_CTRL_CLIENT_GET_ADDR_SPACE_TYPE_PARAMS *);  // exported (id=0xd01)
    NV_STATUS (*__cliresCtrlCmdClientGetHandleInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_CLIENT_GET_HANDLE_INFO_PARAMS *);  // exported (id=0xd02)
    NV_STATUS (*__cliresCtrlCmdClientGetAccessRights__)(struct RmClientResource * /*this*/, NV0000_CTRL_CLIENT_GET_ACCESS_RIGHTS_PARAMS *);  // exported (id=0xd03)
    NV_STATUS (*__cliresCtrlCmdClientSetInheritedSharePolicy__)(struct RmClientResource * /*this*/, NV0000_CTRL_CLIENT_SET_INHERITED_SHARE_POLICY_PARAMS *);  // exported (id=0xd04)
    NV_STATUS (*__cliresCtrlCmdClientShareObject__)(struct RmClientResource * /*this*/, NV0000_CTRL_CLIENT_SHARE_OBJECT_PARAMS *);  // exported (id=0xd06)
    NV_STATUS (*__cliresCtrlCmdClientGetChildHandle__)(struct RmClientResource * /*this*/, NV0000_CTRL_CMD_CLIENT_GET_CHILD_HANDLE_PARAMS *);  // exported (id=0xd05)
    NV_STATUS (*__cliresCtrlCmdObjectsAreDuplicates__)(struct RmClientResource * /*this*/, NV0000_CTRL_CLIENT_OBJECTS_ARE_DUPLICATES_PARAMS *);  // exported (id=0xd07)
    NV_STATUS (*__cliresCtrlCmdGpuGetAttachedIds__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_ATTACHED_IDS_PARAMS *);  // exported (id=0x201)
    NV_STATUS (*__cliresCtrlCmdGpuGetIdInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_ID_INFO_PARAMS *);  // exported (id=0x202)
    NV_STATUS (*__cliresCtrlCmdGpuGetIdInfoV2__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS *);  // exported (id=0x205)
    NV_STATUS (*__cliresCtrlCmdGpuGetInitStatus__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_INIT_STATUS_PARAMS *);  // exported (id=0x203)
    NV_STATUS (*__cliresCtrlCmdGpuGetDeviceIds__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_DEVICE_IDS_PARAMS *);  // exported (id=0x204)
    NV_STATUS (*__cliresCtrlCmdGpuGetActiveDeviceIds__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_ACTIVE_DEVICE_IDS_PARAMS *);  // exported (id=0x288)
    NV_STATUS (*__cliresCtrlCmdGpuGetProbedIds__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_PROBED_IDS_PARAMS *);  // exported (id=0x214)
    NV_STATUS (*__cliresCtrlCmdGpuAttachIds__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_ATTACH_IDS_PARAMS *);  // exported (id=0x215)
    NV_STATUS (*__cliresCtrlCmdGpuAsyncAttachId__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_ASYNC_ATTACH_ID_PARAMS *);  // exported (id=0x289)
    NV_STATUS (*__cliresCtrlCmdGpuWaitAttachId__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_WAIT_ATTACH_ID_PARAMS *);  // exported (id=0x290)
    NV_STATUS (*__cliresCtrlCmdGpuDetachIds__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_DETACH_IDS_PARAMS *);  // exported (id=0x216)
    NV_STATUS (*__cliresCtrlCmdGpuGetPciInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_PCI_INFO_PARAMS *);  // exported (id=0x21b)
    NV_STATUS (*__cliresCtrlCmdGpuGetUuidInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_UUID_INFO_PARAMS *);  // exported (id=0x274)
    NV_STATUS (*__cliresCtrlCmdGpuGetUuidFromGpuId__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_UUID_FROM_GPU_ID_PARAMS *);  // exported (id=0x275)
    NV_STATUS (*__cliresCtrlCmdGpuModifyGpuDrainState__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_MODIFY_DRAIN_STATE_PARAMS *);  // exported (id=0x278)
    NV_STATUS (*__cliresCtrlCmdGpuQueryGpuDrainState__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_QUERY_DRAIN_STATE_PARAMS *);  // exported (id=0x279)
    NV_STATUS (*__cliresCtrlCmdGpuGetMemOpEnable__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_GET_MEMOP_ENABLE_PARAMS *);  // exported (id=0x27b)
    NV_STATUS (*__cliresCtrlCmdGpuDisableNvlinkInit__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_DISABLE_NVLINK_INIT_PARAMS *);  // exported (id=0x281)
    NV_STATUS (*__cliresCtrlCmdLegacyConfig__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_LEGACY_CONFIG_PARAMS *);  // exported (id=0x282)
    NV_STATUS (*__cliresCtrlCmdPushUcodeImage__)(struct RmClientResource * /*this*/, NV0000_CTRL_GPU_PUSH_UCODE_IMAGE_PARAMS *);  // exported (id=0x285)
    NV_STATUS (*__cliresCtrlCmdSystemGetVrrCookiePresent__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_GET_VRR_COOKIE_PRESENT_PARAMS *);  // exported (id=0x107)
    NV_STATUS (*__cliresCtrlCmdGsyncGetAttachedIds__)(struct RmClientResource * /*this*/, NV0000_CTRL_GSYNC_GET_ATTACHED_IDS_PARAMS *);  // exported (id=0x301)
    NV_STATUS (*__cliresCtrlCmdGsyncGetIdInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_GSYNC_GET_ID_INFO_PARAMS *);  // exported (id=0x302)
    NV_STATUS (*__cliresCtrlCmdEventSetNotification__)(struct RmClientResource * /*this*/, NV0000_CTRL_EVENT_SET_NOTIFICATION_PARAMS *);  // exported (id=0x501)
    NV_STATUS (*__cliresCtrlCmdEventGetSystemEventData__)(struct RmClientResource * /*this*/, NV0000_CTRL_GET_SYSTEM_EVENT_DATA_PARAMS *);  // exported (id=0x502)
    NV_STATUS (*__cliresCtrlCmdOsUnixExportObjectToFd__)(struct RmClientResource * /*this*/, NV0000_CTRL_OS_UNIX_EXPORT_OBJECT_TO_FD_PARAMS *);  // exported (id=0x3d05)
    NV_STATUS (*__cliresCtrlCmdOsUnixImportObjectFromFd__)(struct RmClientResource * /*this*/, NV0000_CTRL_OS_UNIX_IMPORT_OBJECT_FROM_FD_PARAMS *);  // exported (id=0x3d06)
    NV_STATUS (*__cliresCtrlCmdOsUnixGetExportObjectInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_OS_UNIX_GET_EXPORT_OBJECT_INFO_PARAMS *);  // exported (id=0x3d08)
    NV_STATUS (*__cliresCtrlCmdOsUnixCreateExportObjectFd__)(struct RmClientResource * /*this*/, NV0000_CTRL_OS_UNIX_CREATE_EXPORT_OBJECT_FD_PARAMS *);  // exported (id=0x3d0a)
    NV_STATUS (*__cliresCtrlCmdOsUnixExportObjectsToFd__)(struct RmClientResource * /*this*/, NV0000_CTRL_OS_UNIX_EXPORT_OBJECTS_TO_FD_PARAMS *);  // exported (id=0x3d0b)
    NV_STATUS (*__cliresCtrlCmdOsUnixImportObjectsFromFd__)(struct RmClientResource * /*this*/, NV0000_CTRL_OS_UNIX_IMPORT_OBJECTS_FROM_FD_PARAMS *);  // exported (id=0x3d0c)
    NV_STATUS (*__cliresCtrlCmdOsUnixFlushUserCache__)(struct RmClientResource * /*this*/, NV0000_CTRL_OS_UNIX_FLUSH_USER_CACHE_PARAMS *);  // exported (id=0x3d02)
    NV_STATUS (*__cliresCtrlCmdSetSubProcessID__)(struct RmClientResource * /*this*/, NV0000_CTRL_SET_SUB_PROCESS_ID_PARAMS *);  // exported (id=0x901)
    NV_STATUS (*__cliresCtrlCmdDisableSubProcessUserdIsolation__)(struct RmClientResource * /*this*/, NV0000_CTRL_DISABLE_SUB_PROCESS_USERD_ISOLATION_PARAMS *);  // exported (id=0x902)
    NV_STATUS (*__cliresCtrlCmdSystemNVPCFGetPowerModeInfo__)(struct RmClientResource * /*this*/, NV0000_CTRL_SYSTEM_NVPCF_GET_POWER_MODE_INFO_PARAMS *);  // exported (id=0x13b)
    NV_STATUS (*__cliresCtrlCmdSystemSyncExternalFabricMgmt__)(struct RmClientResource * /*this*/, NV0000_CTRL_CMD_SYSTEM_SYNC_EXTERNAL_FABRIC_MGMT_PARAMS *);  // exported (id=0x13c)
};


// Vtable with 23 per-class function pointers
struct NVOC_VTABLE__RmClientResource {
    NvBool (*__cliresAccessCallback__)(struct RmClientResource * /*this*/, struct RsClient *, void *, RsAccessRight);  // virtual override (res) base (clientres)
    NvBool (*__cliresShareCallback__)(struct RmClientResource * /*this*/, struct RsClient *, struct RsResourceRef *, RS_SHARE_POLICY *);  // virtual override (res) base (clientres)
    NV_STATUS (*__cliresControl_Prologue__)(struct RmClientResource * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual override (res) base (clientres)
    void (*__cliresControl_Epilogue__)(struct RmClientResource * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual override (res) base (clientres)
    NvBool (*__cliresCanCopy__)(struct RmClientResource * /*this*/);  // virtual inherited (res) base (clientres)
    NV_STATUS (*__cliresIsDuplicate__)(struct RmClientResource * /*this*/, NvHandle, NvBool *);  // virtual inherited (res) base (clientres)
    void (*__cliresPreDestruct__)(struct RmClientResource * /*this*/);  // virtual inherited (res) base (clientres)
    NV_STATUS (*__cliresControl__)(struct RmClientResource * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (res) base (clientres)
    NV_STATUS (*__cliresControlFilter__)(struct RmClientResource * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (res) base (clientres)
    NV_STATUS (*__cliresControlSerialization_Prologue__)(struct RmClientResource * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (res) base (clientres)
    void (*__cliresControlSerialization_Epilogue__)(struct RmClientResource * /*this*/, struct CALL_CONTEXT *, struct RS_RES_CONTROL_PARAMS_INTERNAL *);  // virtual inherited (res) base (clientres)
    NV_STATUS (*__cliresMap__)(struct RmClientResource * /*this*/, struct CALL_CONTEXT *, RS_CPU_MAP_PARAMS *, RsCpuMapping *);  // virtual inherited (res) base (clientres)
    NV_STATUS (*__cliresUnmap__)(struct RmClientResource * /*this*/, struct CALL_CONTEXT *, RsCpuMapping *);  // virtual inherited (res) base (clientres)
    NvBool (*__cliresIsPartialUnmapSupported__)(struct RmClientResource * /*this*/);  // inline virtual inherited (res) base (clientres) body
    NV_STATUS (*__cliresMapTo__)(struct RmClientResource * /*this*/, RS_RES_MAP_TO_PARAMS *);  // virtual inherited (res) base (clientres)
    NV_STATUS (*__cliresUnmapFrom__)(struct RmClientResource * /*this*/, RS_RES_UNMAP_FROM_PARAMS *);  // virtual inherited (res) base (clientres)
    NvU32 (*__cliresGetRefCount__)(struct RmClientResource * /*this*/);  // virtual inherited (res) base (clientres)
    void (*__cliresAddAdditionalDependants__)(struct RsClient *, struct RmClientResource * /*this*/, RsResourceRef *);  // virtual inherited (res) base (clientres)
    PEVENTNOTIFICATION * (*__cliresGetNotificationListPtr__)(struct RmClientResource * /*this*/);  // virtual inherited (notify) base (notify)
    struct NotifShare * (*__cliresGetNotificationShare__)(struct RmClientResource * /*this*/);  // virtual inherited (notify) base (notify)
    void (*__cliresSetNotificationShare__)(struct RmClientResource * /*this*/, struct NotifShare *);  // virtual inherited (notify) base (notify)
    NV_STATUS (*__cliresUnregisterEvent__)(struct RmClientResource * /*this*/, NvHandle, NvHandle, NvHandle, NvHandle);  // virtual inherited (notify) base (notify)
    NV_STATUS (*__cliresGetOrAllocNotifShare__)(struct RmClientResource * /*this*/, NvHandle, NvHandle, struct NotifShare **);  // virtual inherited (notify) base (notify)
};

// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__RmClientResource {
    const struct NVOC_RTTI rtti;
    const struct NVOC_METADATA__RsClientResource metadata__RsClientResource;
    const struct NVOC_METADATA__RmResourceCommon metadata__RmResourceCommon;
    const struct NVOC_METADATA__Notifier metadata__Notifier;
    const struct NVOC_VTABLE__RmClientResource vtable;
};

#ifndef __NVOC_CLASS_RmClientResource_TYPEDEF__
#define __NVOC_CLASS_RmClientResource_TYPEDEF__
typedef struct RmClientResource RmClientResource;
#endif /* __NVOC_CLASS_RmClientResource_TYPEDEF__ */

#ifndef __nvoc_class_id_RmClientResource
#define __nvoc_class_id_RmClientResource 0x37a701
#endif /* __nvoc_class_id_RmClientResource */

// Casting support
extern const struct NVOC_CLASS_DEF __nvoc_class_def_RmClientResource;

#define __staticCast_RmClientResource(pThis) \
    ((pThis)->__nvoc_pbase_RmClientResource)

#ifdef __nvoc_client_resource_h_disabled
#define __dynamicCast_RmClientResource(pThis) ((RmClientResource*) NULL)
#else //__nvoc_client_resource_h_disabled
#define __dynamicCast_RmClientResource(pThis) \
    ((RmClientResource*) __nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(RmClientResource)))
#endif //__nvoc_client_resource_h_disabled

NV_STATUS __nvoc_objCreateDynamic_RmClientResource(RmClientResource**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_RmClientResource(RmClientResource**, Dynamic*, NvU32, struct CALL_CONTEXT *arg_pCallContext, struct RS_RES_ALLOC_PARAMS_INTERNAL *arg_pParams);
#define __objCreate_RmClientResource(ppNewObj, pParent, createFlags, arg_pCallContext, arg_pParams) \
    __nvoc_objCreate_RmClientResource((ppNewObj), staticCast((pParent), Dynamic), (createFlags), arg_pCallContext, arg_pParams)


// Wrapper macros
#define cliresAccessCallback_FNPTR(pRmCliRes) pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresAccessCallback__
#define cliresAccessCallback(pRmCliRes, pInvokingClient, pAllocParams, accessRight) cliresAccessCallback_DISPATCH(pRmCliRes, pInvokingClient, pAllocParams, accessRight)
#define cliresShareCallback_FNPTR(pRmCliRes) pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresShareCallback__
#define cliresShareCallback(pRmCliRes, pInvokingClient, pParentRef, pSharePolicy) cliresShareCallback_DISPATCH(pRmCliRes, pInvokingClient, pParentRef, pSharePolicy)
#define cliresControl_Prologue_FNPTR(pRmCliRes) pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresControl_Prologue__
#define cliresControl_Prologue(pRmCliRes, pCallContext, pParams) cliresControl_Prologue_DISPATCH(pRmCliRes, pCallContext, pParams)
#define cliresControl_Epilogue_FNPTR(pRmCliRes) pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresControl_Epilogue__
#define cliresControl_Epilogue(pRmCliRes, pCallContext, pParams) cliresControl_Epilogue_DISPATCH(pRmCliRes, pCallContext, pParams)
#define cliresCtrlCmdSystemGetCpuInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetCpuInfo__
#define cliresCtrlCmdSystemGetCpuInfo(pRmCliRes, pCpuInfoParams) cliresCtrlCmdSystemGetCpuInfo_DISPATCH(pRmCliRes, pCpuInfoParams)
#define cliresCtrlCmdSystemGetFeatures_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetFeatures__
#define cliresCtrlCmdSystemGetFeatures(pRmCliRes, pParams) cliresCtrlCmdSystemGetFeatures_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemGetBuildVersionV2_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetBuildVersionV2__
#define cliresCtrlCmdSystemGetBuildVersionV2(pRmCliRes, pParams) cliresCtrlCmdSystemGetBuildVersionV2_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemGetLockTimes_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetLockTimes__
#define cliresCtrlCmdSystemGetLockTimes(pRmCliRes, pParams) cliresCtrlCmdSystemGetLockTimes_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemGetClassList_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetClassList__
#define cliresCtrlCmdSystemGetClassList(pRmCliRes, pParams) cliresCtrlCmdSystemGetClassList_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemNotifyEvent_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemNotifyEvent__
#define cliresCtrlCmdSystemNotifyEvent(pRmCliRes, pParams) cliresCtrlCmdSystemNotifyEvent_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemDebugCtrlRmMsg_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemDebugCtrlRmMsg__
#define cliresCtrlCmdSystemDebugCtrlRmMsg(pRmCliRes, pParams) cliresCtrlCmdSystemDebugCtrlRmMsg_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemGetVgxSystemInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetVgxSystemInfo__
#define cliresCtrlCmdSystemGetVgxSystemInfo(pRmCliRes, pParams) cliresCtrlCmdSystemGetVgxSystemInfo_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemGetPrivilegedStatus_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetPrivilegedStatus__
#define cliresCtrlCmdSystemGetPrivilegedStatus(pRmCliRes, pParams) cliresCtrlCmdSystemGetPrivilegedStatus_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemGetFabricStatus_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetFabricStatus__
#define cliresCtrlCmdSystemGetFabricStatus(pRmCliRes, pParams) cliresCtrlCmdSystemGetFabricStatus_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemGetRmInstanceId_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetRmInstanceId__
#define cliresCtrlCmdSystemGetRmInstanceId(pRmCliRes, pRmInstanceIdParams) cliresCtrlCmdSystemGetRmInstanceId_DISPATCH(pRmCliRes, pRmInstanceIdParams)
#define cliresCtrlCmdSystemGetClientDatabaseInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetClientDatabaseInfo__
#define cliresCtrlCmdSystemGetClientDatabaseInfo(pRmCliRes, pParams) cliresCtrlCmdSystemGetClientDatabaseInfo_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemRmctrlCacheModeCtrl_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemRmctrlCacheModeCtrl__
#define cliresCtrlCmdSystemRmctrlCacheModeCtrl(pRmCliRes, pParams) cliresCtrlCmdSystemRmctrlCacheModeCtrl_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdClientGetAddrSpaceType_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdClientGetAddrSpaceType__
#define cliresCtrlCmdClientGetAddrSpaceType(pRmCliRes, pParams) cliresCtrlCmdClientGetAddrSpaceType_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdClientGetHandleInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdClientGetHandleInfo__
#define cliresCtrlCmdClientGetHandleInfo(pRmCliRes, pParams) cliresCtrlCmdClientGetHandleInfo_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdClientGetAccessRights_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdClientGetAccessRights__
#define cliresCtrlCmdClientGetAccessRights(pRmCliRes, pParams) cliresCtrlCmdClientGetAccessRights_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdClientSetInheritedSharePolicy_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdClientSetInheritedSharePolicy__
#define cliresCtrlCmdClientSetInheritedSharePolicy(pRmCliRes, pParams) cliresCtrlCmdClientSetInheritedSharePolicy_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdClientShareObject_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdClientShareObject__
#define cliresCtrlCmdClientShareObject(pRmCliRes, pParams) cliresCtrlCmdClientShareObject_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdClientGetChildHandle_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdClientGetChildHandle__
#define cliresCtrlCmdClientGetChildHandle(pRmCliRes, pParams) cliresCtrlCmdClientGetChildHandle_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdObjectsAreDuplicates_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdObjectsAreDuplicates__
#define cliresCtrlCmdObjectsAreDuplicates(pRmCliRes, pParams) cliresCtrlCmdObjectsAreDuplicates_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdGpuGetAttachedIds_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetAttachedIds__
#define cliresCtrlCmdGpuGetAttachedIds(pRmCliRes, pGpuAttachedIds) cliresCtrlCmdGpuGetAttachedIds_DISPATCH(pRmCliRes, pGpuAttachedIds)
#define cliresCtrlCmdGpuGetIdInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetIdInfo__
#define cliresCtrlCmdGpuGetIdInfo(pRmCliRes, pGpuIdInfoParams) cliresCtrlCmdGpuGetIdInfo_DISPATCH(pRmCliRes, pGpuIdInfoParams)
#define cliresCtrlCmdGpuGetIdInfoV2_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetIdInfoV2__
#define cliresCtrlCmdGpuGetIdInfoV2(pRmCliRes, pGpuIdInfoParams) cliresCtrlCmdGpuGetIdInfoV2_DISPATCH(pRmCliRes, pGpuIdInfoParams)
#define cliresCtrlCmdGpuGetInitStatus_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetInitStatus__
#define cliresCtrlCmdGpuGetInitStatus(pRmCliRes, pGpuInitStatusParams) cliresCtrlCmdGpuGetInitStatus_DISPATCH(pRmCliRes, pGpuInitStatusParams)
#define cliresCtrlCmdGpuGetDeviceIds_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetDeviceIds__
#define cliresCtrlCmdGpuGetDeviceIds(pRmCliRes, pDeviceIdsParams) cliresCtrlCmdGpuGetDeviceIds_DISPATCH(pRmCliRes, pDeviceIdsParams)
#define cliresCtrlCmdGpuGetActiveDeviceIds_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetActiveDeviceIds__
#define cliresCtrlCmdGpuGetActiveDeviceIds(pRmCliRes, pActiveDeviceIdsParams) cliresCtrlCmdGpuGetActiveDeviceIds_DISPATCH(pRmCliRes, pActiveDeviceIdsParams)
#define cliresCtrlCmdGpuGetProbedIds_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetProbedIds__
#define cliresCtrlCmdGpuGetProbedIds(pRmCliRes, pGpuProbedIds) cliresCtrlCmdGpuGetProbedIds_DISPATCH(pRmCliRes, pGpuProbedIds)
#define cliresCtrlCmdGpuAttachIds_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuAttachIds__
#define cliresCtrlCmdGpuAttachIds(pRmCliRes, pGpuAttachIds) cliresCtrlCmdGpuAttachIds_DISPATCH(pRmCliRes, pGpuAttachIds)
#define cliresCtrlCmdGpuAsyncAttachId_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuAsyncAttachId__
#define cliresCtrlCmdGpuAsyncAttachId(pRmCliRes, pAsyncAttachIdParams) cliresCtrlCmdGpuAsyncAttachId_DISPATCH(pRmCliRes, pAsyncAttachIdParams)
#define cliresCtrlCmdGpuWaitAttachId_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuWaitAttachId__
#define cliresCtrlCmdGpuWaitAttachId(pRmCliRes, pWaitAttachIdParams) cliresCtrlCmdGpuWaitAttachId_DISPATCH(pRmCliRes, pWaitAttachIdParams)
#define cliresCtrlCmdGpuDetachIds_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuDetachIds__
#define cliresCtrlCmdGpuDetachIds(pRmCliRes, pGpuDetachIds) cliresCtrlCmdGpuDetachIds_DISPATCH(pRmCliRes, pGpuDetachIds)
#define cliresCtrlCmdGpuGetPciInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetPciInfo__
#define cliresCtrlCmdGpuGetPciInfo(pRmCliRes, pPciInfoParams) cliresCtrlCmdGpuGetPciInfo_DISPATCH(pRmCliRes, pPciInfoParams)
#define cliresCtrlCmdGpuGetUuidInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetUuidInfo__
#define cliresCtrlCmdGpuGetUuidInfo(pRmCliRes, pParams) cliresCtrlCmdGpuGetUuidInfo_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdGpuGetUuidFromGpuId_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetUuidFromGpuId__
#define cliresCtrlCmdGpuGetUuidFromGpuId(pRmCliRes, pParams) cliresCtrlCmdGpuGetUuidFromGpuId_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdGpuModifyGpuDrainState_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuModifyGpuDrainState__
#define cliresCtrlCmdGpuModifyGpuDrainState(pRmCliRes, pParams) cliresCtrlCmdGpuModifyGpuDrainState_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdGpuQueryGpuDrainState_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuQueryGpuDrainState__
#define cliresCtrlCmdGpuQueryGpuDrainState(pRmCliRes, pParams) cliresCtrlCmdGpuQueryGpuDrainState_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdGpuGetMemOpEnable_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuGetMemOpEnable__
#define cliresCtrlCmdGpuGetMemOpEnable(pRmCliRes, pMemOpEnableParams) cliresCtrlCmdGpuGetMemOpEnable_DISPATCH(pRmCliRes, pMemOpEnableParams)
#define cliresCtrlCmdGpuDisableNvlinkInit_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGpuDisableNvlinkInit__
#define cliresCtrlCmdGpuDisableNvlinkInit(pRmCliRes, pParams) cliresCtrlCmdGpuDisableNvlinkInit_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdLegacyConfig_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdLegacyConfig__
#define cliresCtrlCmdLegacyConfig(pRmCliRes, pParams) cliresCtrlCmdLegacyConfig_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdPushUcodeImage_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdPushUcodeImage__
#define cliresCtrlCmdPushUcodeImage(pRmCliRes, pParams) cliresCtrlCmdPushUcodeImage_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemGetVrrCookiePresent_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemGetVrrCookiePresent__
#define cliresCtrlCmdSystemGetVrrCookiePresent(pRmCliRes, pParams) cliresCtrlCmdSystemGetVrrCookiePresent_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdGsyncGetAttachedIds_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGsyncGetAttachedIds__
#define cliresCtrlCmdGsyncGetAttachedIds(pRmCliRes, pGsyncAttachedIds) cliresCtrlCmdGsyncGetAttachedIds_DISPATCH(pRmCliRes, pGsyncAttachedIds)
#define cliresCtrlCmdGsyncGetIdInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdGsyncGetIdInfo__
#define cliresCtrlCmdGsyncGetIdInfo(pRmCliRes, pGsyncIdInfoParams) cliresCtrlCmdGsyncGetIdInfo_DISPATCH(pRmCliRes, pGsyncIdInfoParams)
#define cliresCtrlCmdEventSetNotification_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdEventSetNotification__
#define cliresCtrlCmdEventSetNotification(pRmCliRes, pEventSetNotificationParams) cliresCtrlCmdEventSetNotification_DISPATCH(pRmCliRes, pEventSetNotificationParams)
#define cliresCtrlCmdEventGetSystemEventData_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdEventGetSystemEventData__
#define cliresCtrlCmdEventGetSystemEventData(pRmCliRes, pSystemEventDataParams) cliresCtrlCmdEventGetSystemEventData_DISPATCH(pRmCliRes, pSystemEventDataParams)
#define cliresCtrlCmdOsUnixExportObjectToFd_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdOsUnixExportObjectToFd__
#define cliresCtrlCmdOsUnixExportObjectToFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixExportObjectToFd_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdOsUnixImportObjectFromFd_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdOsUnixImportObjectFromFd__
#define cliresCtrlCmdOsUnixImportObjectFromFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixImportObjectFromFd_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdOsUnixGetExportObjectInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdOsUnixGetExportObjectInfo__
#define cliresCtrlCmdOsUnixGetExportObjectInfo(pRmCliRes, pParams) cliresCtrlCmdOsUnixGetExportObjectInfo_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdOsUnixCreateExportObjectFd_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdOsUnixCreateExportObjectFd__
#define cliresCtrlCmdOsUnixCreateExportObjectFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixCreateExportObjectFd_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdOsUnixExportObjectsToFd_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdOsUnixExportObjectsToFd__
#define cliresCtrlCmdOsUnixExportObjectsToFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixExportObjectsToFd_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdOsUnixImportObjectsFromFd_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdOsUnixImportObjectsFromFd__
#define cliresCtrlCmdOsUnixImportObjectsFromFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixImportObjectsFromFd_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdOsUnixFlushUserCache_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdOsUnixFlushUserCache__
#define cliresCtrlCmdOsUnixFlushUserCache(pRmCliRes, pAddressSpaceParams) cliresCtrlCmdOsUnixFlushUserCache_DISPATCH(pRmCliRes, pAddressSpaceParams)
#define cliresCtrlCmdSetSubProcessID_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSetSubProcessID__
#define cliresCtrlCmdSetSubProcessID(pRmCliRes, pParams) cliresCtrlCmdSetSubProcessID_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdDisableSubProcessUserdIsolation_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdDisableSubProcessUserdIsolation__
#define cliresCtrlCmdDisableSubProcessUserdIsolation(pRmCliRes, pParams) cliresCtrlCmdDisableSubProcessUserdIsolation_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemNVPCFGetPowerModeInfo_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemNVPCFGetPowerModeInfo__
#define cliresCtrlCmdSystemNVPCFGetPowerModeInfo(pRmCliRes, pParams) cliresCtrlCmdSystemNVPCFGetPowerModeInfo_DISPATCH(pRmCliRes, pParams)
#define cliresCtrlCmdSystemSyncExternalFabricMgmt_FNPTR(pRmCliRes) pRmCliRes->__cliresCtrlCmdSystemSyncExternalFabricMgmt__
#define cliresCtrlCmdSystemSyncExternalFabricMgmt(pRmCliRes, pExtFabricMgmtParams) cliresCtrlCmdSystemSyncExternalFabricMgmt_DISPATCH(pRmCliRes, pExtFabricMgmtParams)
#define cliresCanCopy_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resCanCopy__
#define cliresCanCopy(pResource) cliresCanCopy_DISPATCH(pResource)
#define cliresIsDuplicate_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resIsDuplicate__
#define cliresIsDuplicate(pResource, hMemory, pDuplicate) cliresIsDuplicate_DISPATCH(pResource, hMemory, pDuplicate)
#define cliresPreDestruct_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resPreDestruct__
#define cliresPreDestruct(pResource) cliresPreDestruct_DISPATCH(pResource)
#define cliresControl_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resControl__
#define cliresControl(pResource, pCallContext, pParams) cliresControl_DISPATCH(pResource, pCallContext, pParams)
#define cliresControlFilter_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resControlFilter__
#define cliresControlFilter(pResource, pCallContext, pParams) cliresControlFilter_DISPATCH(pResource, pCallContext, pParams)
#define cliresControlSerialization_Prologue_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resControlSerialization_Prologue__
#define cliresControlSerialization_Prologue(pResource, pCallContext, pParams) cliresControlSerialization_Prologue_DISPATCH(pResource, pCallContext, pParams)
#define cliresControlSerialization_Epilogue_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resControlSerialization_Epilogue__
#define cliresControlSerialization_Epilogue(pResource, pCallContext, pParams) cliresControlSerialization_Epilogue_DISPATCH(pResource, pCallContext, pParams)
#define cliresMap_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resMap__
#define cliresMap(pResource, pCallContext, pParams, pCpuMapping) cliresMap_DISPATCH(pResource, pCallContext, pParams, pCpuMapping)
#define cliresUnmap_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resUnmap__
#define cliresUnmap(pResource, pCallContext, pCpuMapping) cliresUnmap_DISPATCH(pResource, pCallContext, pCpuMapping)
#define cliresIsPartialUnmapSupported_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resIsPartialUnmapSupported__
#define cliresIsPartialUnmapSupported(pResource) cliresIsPartialUnmapSupported_DISPATCH(pResource)
#define cliresMapTo_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resMapTo__
#define cliresMapTo(pResource, pParams) cliresMapTo_DISPATCH(pResource, pParams)
#define cliresUnmapFrom_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resUnmapFrom__
#define cliresUnmapFrom(pResource, pParams) cliresUnmapFrom_DISPATCH(pResource, pParams)
#define cliresGetRefCount_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resGetRefCount__
#define cliresGetRefCount(pResource) cliresGetRefCount_DISPATCH(pResource)
#define cliresAddAdditionalDependants_FNPTR(pResource) pResource->__nvoc_base_RsClientResource.__nvoc_base_RsResource.__nvoc_metadata_ptr->vtable.__resAddAdditionalDependants__
#define cliresAddAdditionalDependants(pClient, pResource, pReference) cliresAddAdditionalDependants_DISPATCH(pClient, pResource, pReference)
#define cliresGetNotificationListPtr_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifyGetNotificationListPtr__
#define cliresGetNotificationListPtr(pNotifier) cliresGetNotificationListPtr_DISPATCH(pNotifier)
#define cliresGetNotificationShare_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifyGetNotificationShare__
#define cliresGetNotificationShare(pNotifier) cliresGetNotificationShare_DISPATCH(pNotifier)
#define cliresSetNotificationShare_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifySetNotificationShare__
#define cliresSetNotificationShare(pNotifier, pNotifShare) cliresSetNotificationShare_DISPATCH(pNotifier, pNotifShare)
#define cliresUnregisterEvent_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifyUnregisterEvent__
#define cliresUnregisterEvent(pNotifier, hNotifierClient, hNotifierResource, hEventClient, hEvent) cliresUnregisterEvent_DISPATCH(pNotifier, hNotifierClient, hNotifierResource, hEventClient, hEvent)
#define cliresGetOrAllocNotifShare_FNPTR(pNotifier) pNotifier->__nvoc_base_Notifier.__nvoc_metadata_ptr->vtable.__notifyGetOrAllocNotifShare__
#define cliresGetOrAllocNotifShare(pNotifier, hNotifierClient, hNotifierResource, ppNotifShare) cliresGetOrAllocNotifShare_DISPATCH(pNotifier, hNotifierClient, hNotifierResource, ppNotifShare)

// Dispatch functions
static inline NvBool cliresAccessCallback_DISPATCH(struct RmClientResource *pRmCliRes, struct RsClient *pInvokingClient, void *pAllocParams, RsAccessRight accessRight) {
    return pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresAccessCallback__(pRmCliRes, pInvokingClient, pAllocParams, accessRight);
}

static inline NvBool cliresShareCallback_DISPATCH(struct RmClientResource *pRmCliRes, struct RsClient *pInvokingClient, struct RsResourceRef *pParentRef, RS_SHARE_POLICY *pSharePolicy) {
    return pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresShareCallback__(pRmCliRes, pInvokingClient, pParentRef, pSharePolicy);
}

static inline NV_STATUS cliresControl_Prologue_DISPATCH(struct RmClientResource *pRmCliRes, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    return pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresControl_Prologue__(pRmCliRes, pCallContext, pParams);
}

static inline void cliresControl_Epilogue_DISPATCH(struct RmClientResource *pRmCliRes, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresControl_Epilogue__(pRmCliRes, pCallContext, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetCpuInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CPU_INFO_PARAMS *pCpuInfoParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetCpuInfo__(pRmCliRes, pCpuInfoParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetFeatures_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_FEATURES_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetFeatures__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetBuildVersionV2_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_BUILD_VERSION_V2_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetBuildVersionV2__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetLockTimes_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_LOCK_TIMES_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetLockTimes__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetClassList_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CLASSLIST_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetClassList__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemNotifyEvent_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_NOTIFY_EVENT_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemNotifyEvent__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemDebugCtrlRmMsg_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_DEBUG_RMMSG_CTRL_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemDebugCtrlRmMsg__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetVgxSystemInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_VGX_SYSTEM_INFO_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetVgxSystemInfo__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetPrivilegedStatus_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_PRIVILEGED_STATUS_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetPrivilegedStatus__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetFabricStatus_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_FABRIC_STATUS_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetFabricStatus__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetRmInstanceId_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_RM_INSTANCE_ID_PARAMS *pRmInstanceIdParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetRmInstanceId__(pRmCliRes, pRmInstanceIdParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetClientDatabaseInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CLIENT_DATABASE_INFO_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetClientDatabaseInfo__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemRmctrlCacheModeCtrl_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_RMCTRL_CACHE_MODE_CTRL_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemRmctrlCacheModeCtrl__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdClientGetAddrSpaceType_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_ADDR_SPACE_TYPE_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdClientGetAddrSpaceType__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdClientGetHandleInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_HANDLE_INFO_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdClientGetHandleInfo__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdClientGetAccessRights_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_ACCESS_RIGHTS_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdClientGetAccessRights__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdClientSetInheritedSharePolicy_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_SET_INHERITED_SHARE_POLICY_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdClientSetInheritedSharePolicy__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdClientShareObject_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_SHARE_OBJECT_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdClientShareObject__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdClientGetChildHandle_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_CMD_CLIENT_GET_CHILD_HANDLE_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdClientGetChildHandle__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdObjectsAreDuplicates_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_OBJECTS_ARE_DUPLICATES_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdObjectsAreDuplicates__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdGpuGetAttachedIds_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ATTACHED_IDS_PARAMS *pGpuAttachedIds) {
    return pRmCliRes->__cliresCtrlCmdGpuGetAttachedIds__(pRmCliRes, pGpuAttachedIds);
}

static inline NV_STATUS cliresCtrlCmdGpuGetIdInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ID_INFO_PARAMS *pGpuIdInfoParams) {
    return pRmCliRes->__cliresCtrlCmdGpuGetIdInfo__(pRmCliRes, pGpuIdInfoParams);
}

static inline NV_STATUS cliresCtrlCmdGpuGetIdInfoV2_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS *pGpuIdInfoParams) {
    return pRmCliRes->__cliresCtrlCmdGpuGetIdInfoV2__(pRmCliRes, pGpuIdInfoParams);
}

static inline NV_STATUS cliresCtrlCmdGpuGetInitStatus_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_INIT_STATUS_PARAMS *pGpuInitStatusParams) {
    return pRmCliRes->__cliresCtrlCmdGpuGetInitStatus__(pRmCliRes, pGpuInitStatusParams);
}

static inline NV_STATUS cliresCtrlCmdGpuGetDeviceIds_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_DEVICE_IDS_PARAMS *pDeviceIdsParams) {
    return pRmCliRes->__cliresCtrlCmdGpuGetDeviceIds__(pRmCliRes, pDeviceIdsParams);
}

static inline NV_STATUS cliresCtrlCmdGpuGetActiveDeviceIds_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ACTIVE_DEVICE_IDS_PARAMS *pActiveDeviceIdsParams) {
    return pRmCliRes->__cliresCtrlCmdGpuGetActiveDeviceIds__(pRmCliRes, pActiveDeviceIdsParams);
}

static inline NV_STATUS cliresCtrlCmdGpuGetProbedIds_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_PROBED_IDS_PARAMS *pGpuProbedIds) {
    return pRmCliRes->__cliresCtrlCmdGpuGetProbedIds__(pRmCliRes, pGpuProbedIds);
}

static inline NV_STATUS cliresCtrlCmdGpuAttachIds_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_ATTACH_IDS_PARAMS *pGpuAttachIds) {
    return pRmCliRes->__cliresCtrlCmdGpuAttachIds__(pRmCliRes, pGpuAttachIds);
}

static inline NV_STATUS cliresCtrlCmdGpuAsyncAttachId_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_ASYNC_ATTACH_ID_PARAMS *pAsyncAttachIdParams) {
    return pRmCliRes->__cliresCtrlCmdGpuAsyncAttachId__(pRmCliRes, pAsyncAttachIdParams);
}

static inline NV_STATUS cliresCtrlCmdGpuWaitAttachId_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_WAIT_ATTACH_ID_PARAMS *pWaitAttachIdParams) {
    return pRmCliRes->__cliresCtrlCmdGpuWaitAttachId__(pRmCliRes, pWaitAttachIdParams);
}

static inline NV_STATUS cliresCtrlCmdGpuDetachIds_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_DETACH_IDS_PARAMS *pGpuDetachIds) {
    return pRmCliRes->__cliresCtrlCmdGpuDetachIds__(pRmCliRes, pGpuDetachIds);
}

static inline NV_STATUS cliresCtrlCmdGpuGetPciInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_PCI_INFO_PARAMS *pPciInfoParams) {
    return pRmCliRes->__cliresCtrlCmdGpuGetPciInfo__(pRmCliRes, pPciInfoParams);
}

static inline NV_STATUS cliresCtrlCmdGpuGetUuidInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_UUID_INFO_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdGpuGetUuidInfo__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdGpuGetUuidFromGpuId_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_UUID_FROM_GPU_ID_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdGpuGetUuidFromGpuId__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdGpuModifyGpuDrainState_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_MODIFY_DRAIN_STATE_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdGpuModifyGpuDrainState__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdGpuQueryGpuDrainState_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_QUERY_DRAIN_STATE_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdGpuQueryGpuDrainState__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdGpuGetMemOpEnable_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_MEMOP_ENABLE_PARAMS *pMemOpEnableParams) {
    return pRmCliRes->__cliresCtrlCmdGpuGetMemOpEnable__(pRmCliRes, pMemOpEnableParams);
}

static inline NV_STATUS cliresCtrlCmdGpuDisableNvlinkInit_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_DISABLE_NVLINK_INIT_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdGpuDisableNvlinkInit__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdLegacyConfig_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_LEGACY_CONFIG_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdLegacyConfig__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdPushUcodeImage_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_PUSH_UCODE_IMAGE_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdPushUcodeImage__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemGetVrrCookiePresent_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_VRR_COOKIE_PRESENT_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemGetVrrCookiePresent__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdGsyncGetAttachedIds_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GSYNC_GET_ATTACHED_IDS_PARAMS *pGsyncAttachedIds) {
    return pRmCliRes->__cliresCtrlCmdGsyncGetAttachedIds__(pRmCliRes, pGsyncAttachedIds);
}

static inline NV_STATUS cliresCtrlCmdGsyncGetIdInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GSYNC_GET_ID_INFO_PARAMS *pGsyncIdInfoParams) {
    return pRmCliRes->__cliresCtrlCmdGsyncGetIdInfo__(pRmCliRes, pGsyncIdInfoParams);
}

static inline NV_STATUS cliresCtrlCmdEventSetNotification_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_EVENT_SET_NOTIFICATION_PARAMS *pEventSetNotificationParams) {
    return pRmCliRes->__cliresCtrlCmdEventSetNotification__(pRmCliRes, pEventSetNotificationParams);
}

static inline NV_STATUS cliresCtrlCmdEventGetSystemEventData_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_GET_SYSTEM_EVENT_DATA_PARAMS *pSystemEventDataParams) {
    return pRmCliRes->__cliresCtrlCmdEventGetSystemEventData__(pRmCliRes, pSystemEventDataParams);
}

static inline NV_STATUS cliresCtrlCmdOsUnixExportObjectToFd_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_EXPORT_OBJECT_TO_FD_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdOsUnixExportObjectToFd__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdOsUnixImportObjectFromFd_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_IMPORT_OBJECT_FROM_FD_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdOsUnixImportObjectFromFd__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdOsUnixGetExportObjectInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_GET_EXPORT_OBJECT_INFO_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdOsUnixGetExportObjectInfo__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdOsUnixCreateExportObjectFd_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_CREATE_EXPORT_OBJECT_FD_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdOsUnixCreateExportObjectFd__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdOsUnixExportObjectsToFd_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_EXPORT_OBJECTS_TO_FD_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdOsUnixExportObjectsToFd__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdOsUnixImportObjectsFromFd_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_IMPORT_OBJECTS_FROM_FD_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdOsUnixImportObjectsFromFd__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdOsUnixFlushUserCache_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_FLUSH_USER_CACHE_PARAMS *pAddressSpaceParams) {
    return pRmCliRes->__cliresCtrlCmdOsUnixFlushUserCache__(pRmCliRes, pAddressSpaceParams);
}

static inline NV_STATUS cliresCtrlCmdSetSubProcessID_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SET_SUB_PROCESS_ID_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSetSubProcessID__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdDisableSubProcessUserdIsolation_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_DISABLE_SUB_PROCESS_USERD_ISOLATION_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdDisableSubProcessUserdIsolation__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemNVPCFGetPowerModeInfo_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_NVPCF_GET_POWER_MODE_INFO_PARAMS *pParams) {
    return pRmCliRes->__cliresCtrlCmdSystemNVPCFGetPowerModeInfo__(pRmCliRes, pParams);
}

static inline NV_STATUS cliresCtrlCmdSystemSyncExternalFabricMgmt_DISPATCH(struct RmClientResource *pRmCliRes, NV0000_CTRL_CMD_SYSTEM_SYNC_EXTERNAL_FABRIC_MGMT_PARAMS *pExtFabricMgmtParams) {
    return pRmCliRes->__cliresCtrlCmdSystemSyncExternalFabricMgmt__(pRmCliRes, pExtFabricMgmtParams);
}

static inline NvBool cliresCanCopy_DISPATCH(struct RmClientResource *pResource) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresCanCopy__(pResource);
}

static inline NV_STATUS cliresIsDuplicate_DISPATCH(struct RmClientResource *pResource, NvHandle hMemory, NvBool *pDuplicate) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresIsDuplicate__(pResource, hMemory, pDuplicate);
}

static inline void cliresPreDestruct_DISPATCH(struct RmClientResource *pResource) {
    pResource->__nvoc_metadata_ptr->vtable.__cliresPreDestruct__(pResource);
}

static inline NV_STATUS cliresControl_DISPATCH(struct RmClientResource *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresControl__(pResource, pCallContext, pParams);
}

static inline NV_STATUS cliresControlFilter_DISPATCH(struct RmClientResource *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresControlFilter__(pResource, pCallContext, pParams);
}

static inline NV_STATUS cliresControlSerialization_Prologue_DISPATCH(struct RmClientResource *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresControlSerialization_Prologue__(pResource, pCallContext, pParams);
}

static inline void cliresControlSerialization_Epilogue_DISPATCH(struct RmClientResource *pResource, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams) {
    pResource->__nvoc_metadata_ptr->vtable.__cliresControlSerialization_Epilogue__(pResource, pCallContext, pParams);
}

static inline NV_STATUS cliresMap_DISPATCH(struct RmClientResource *pResource, struct CALL_CONTEXT *pCallContext, RS_CPU_MAP_PARAMS *pParams, RsCpuMapping *pCpuMapping) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresMap__(pResource, pCallContext, pParams, pCpuMapping);
}

static inline NV_STATUS cliresUnmap_DISPATCH(struct RmClientResource *pResource, struct CALL_CONTEXT *pCallContext, RsCpuMapping *pCpuMapping) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresUnmap__(pResource, pCallContext, pCpuMapping);
}

static inline NvBool cliresIsPartialUnmapSupported_DISPATCH(struct RmClientResource *pResource) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresIsPartialUnmapSupported__(pResource);
}

static inline NV_STATUS cliresMapTo_DISPATCH(struct RmClientResource *pResource, RS_RES_MAP_TO_PARAMS *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresMapTo__(pResource, pParams);
}

static inline NV_STATUS cliresUnmapFrom_DISPATCH(struct RmClientResource *pResource, RS_RES_UNMAP_FROM_PARAMS *pParams) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresUnmapFrom__(pResource, pParams);
}

static inline NvU32 cliresGetRefCount_DISPATCH(struct RmClientResource *pResource) {
    return pResource->__nvoc_metadata_ptr->vtable.__cliresGetRefCount__(pResource);
}

static inline void cliresAddAdditionalDependants_DISPATCH(struct RsClient *pClient, struct RmClientResource *pResource, RsResourceRef *pReference) {
    pResource->__nvoc_metadata_ptr->vtable.__cliresAddAdditionalDependants__(pClient, pResource, pReference);
}

static inline PEVENTNOTIFICATION * cliresGetNotificationListPtr_DISPATCH(struct RmClientResource *pNotifier) {
    return pNotifier->__nvoc_metadata_ptr->vtable.__cliresGetNotificationListPtr__(pNotifier);
}

static inline struct NotifShare * cliresGetNotificationShare_DISPATCH(struct RmClientResource *pNotifier) {
    return pNotifier->__nvoc_metadata_ptr->vtable.__cliresGetNotificationShare__(pNotifier);
}

static inline void cliresSetNotificationShare_DISPATCH(struct RmClientResource *pNotifier, struct NotifShare *pNotifShare) {
    pNotifier->__nvoc_metadata_ptr->vtable.__cliresSetNotificationShare__(pNotifier, pNotifShare);
}

static inline NV_STATUS cliresUnregisterEvent_DISPATCH(struct RmClientResource *pNotifier, NvHandle hNotifierClient, NvHandle hNotifierResource, NvHandle hEventClient, NvHandle hEvent) {
    return pNotifier->__nvoc_metadata_ptr->vtable.__cliresUnregisterEvent__(pNotifier, hNotifierClient, hNotifierResource, hEventClient, hEvent);
}

static inline NV_STATUS cliresGetOrAllocNotifShare_DISPATCH(struct RmClientResource *pNotifier, NvHandle hNotifierClient, NvHandle hNotifierResource, struct NotifShare **ppNotifShare) {
    return pNotifier->__nvoc_metadata_ptr->vtable.__cliresGetOrAllocNotifShare__(pNotifier, hNotifierClient, hNotifierResource, ppNotifShare);
}

NvBool cliresAccessCallback_IMPL(struct RmClientResource *pRmCliRes, struct RsClient *pInvokingClient, void *pAllocParams, RsAccessRight accessRight);

NvBool cliresShareCallback_IMPL(struct RmClientResource *pRmCliRes, struct RsClient *pInvokingClient, struct RsResourceRef *pParentRef, RS_SHARE_POLICY *pSharePolicy);

NV_STATUS cliresControl_Prologue_IMPL(struct RmClientResource *pRmCliRes, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams);

void cliresControl_Epilogue_IMPL(struct RmClientResource *pRmCliRes, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams);

NV_STATUS cliresCtrlCmdSystemGetCpuInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CPU_INFO_PARAMS *pCpuInfoParams);

NV_STATUS cliresCtrlCmdSystemGetFeatures_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_FEATURES_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemGetBuildVersionV2_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_BUILD_VERSION_V2_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemGetLockTimes_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_LOCK_TIMES_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemGetClassList_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CLASSLIST_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemNotifyEvent_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_NOTIFY_EVENT_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemDebugCtrlRmMsg_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_DEBUG_RMMSG_CTRL_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemGetVgxSystemInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_VGX_SYSTEM_INFO_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemGetPrivilegedStatus_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_PRIVILEGED_STATUS_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemGetFabricStatus_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_FABRIC_STATUS_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemGetRmInstanceId_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_RM_INSTANCE_ID_PARAMS *pRmInstanceIdParams);

NV_STATUS cliresCtrlCmdSystemGetClientDatabaseInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CLIENT_DATABASE_INFO_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemRmctrlCacheModeCtrl_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_RMCTRL_CACHE_MODE_CTRL_PARAMS *pParams);

NV_STATUS cliresCtrlCmdClientGetAddrSpaceType_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_ADDR_SPACE_TYPE_PARAMS *pParams);

NV_STATUS cliresCtrlCmdClientGetHandleInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_HANDLE_INFO_PARAMS *pParams);

NV_STATUS cliresCtrlCmdClientGetAccessRights_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_ACCESS_RIGHTS_PARAMS *pParams);

NV_STATUS cliresCtrlCmdClientSetInheritedSharePolicy_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_SET_INHERITED_SHARE_POLICY_PARAMS *pParams);

NV_STATUS cliresCtrlCmdClientShareObject_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_SHARE_OBJECT_PARAMS *pParams);

NV_STATUS cliresCtrlCmdClientGetChildHandle_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CMD_CLIENT_GET_CHILD_HANDLE_PARAMS *pParams);

NV_STATUS cliresCtrlCmdObjectsAreDuplicates_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_OBJECTS_ARE_DUPLICATES_PARAMS *pParams);

NV_STATUS cliresCtrlCmdGpuGetAttachedIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ATTACHED_IDS_PARAMS *pGpuAttachedIds);

NV_STATUS cliresCtrlCmdGpuGetIdInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ID_INFO_PARAMS *pGpuIdInfoParams);

NV_STATUS cliresCtrlCmdGpuGetIdInfoV2_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS *pGpuIdInfoParams);

NV_STATUS cliresCtrlCmdGpuGetInitStatus_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_INIT_STATUS_PARAMS *pGpuInitStatusParams);

NV_STATUS cliresCtrlCmdGpuGetDeviceIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_DEVICE_IDS_PARAMS *pDeviceIdsParams);

NV_STATUS cliresCtrlCmdGpuGetActiveDeviceIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ACTIVE_DEVICE_IDS_PARAMS *pActiveDeviceIdsParams);

NV_STATUS cliresCtrlCmdGpuGetProbedIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_PROBED_IDS_PARAMS *pGpuProbedIds);

NV_STATUS cliresCtrlCmdGpuAttachIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_ATTACH_IDS_PARAMS *pGpuAttachIds);

NV_STATUS cliresCtrlCmdGpuAsyncAttachId_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_ASYNC_ATTACH_ID_PARAMS *pAsyncAttachIdParams);

NV_STATUS cliresCtrlCmdGpuWaitAttachId_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_WAIT_ATTACH_ID_PARAMS *pWaitAttachIdParams);

NV_STATUS cliresCtrlCmdGpuDetachIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_DETACH_IDS_PARAMS *pGpuDetachIds);

NV_STATUS cliresCtrlCmdGpuGetPciInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_PCI_INFO_PARAMS *pPciInfoParams);

NV_STATUS cliresCtrlCmdGpuGetUuidInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_UUID_INFO_PARAMS *pParams);

NV_STATUS cliresCtrlCmdGpuGetUuidFromGpuId_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_UUID_FROM_GPU_ID_PARAMS *pParams);

NV_STATUS cliresCtrlCmdGpuModifyGpuDrainState_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_MODIFY_DRAIN_STATE_PARAMS *pParams);

NV_STATUS cliresCtrlCmdGpuQueryGpuDrainState_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_QUERY_DRAIN_STATE_PARAMS *pParams);

NV_STATUS cliresCtrlCmdGpuGetMemOpEnable_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_MEMOP_ENABLE_PARAMS *pMemOpEnableParams);

NV_STATUS cliresCtrlCmdGpuDisableNvlinkInit_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_DISABLE_NVLINK_INIT_PARAMS *pParams);

NV_STATUS cliresCtrlCmdLegacyConfig_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_LEGACY_CONFIG_PARAMS *pParams);

NV_STATUS cliresCtrlCmdPushUcodeImage_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_PUSH_UCODE_IMAGE_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemGetVrrCookiePresent_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_VRR_COOKIE_PRESENT_PARAMS *pParams);

NV_STATUS cliresCtrlCmdGsyncGetAttachedIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GSYNC_GET_ATTACHED_IDS_PARAMS *pGsyncAttachedIds);

NV_STATUS cliresCtrlCmdGsyncGetIdInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GSYNC_GET_ID_INFO_PARAMS *pGsyncIdInfoParams);

NV_STATUS cliresCtrlCmdEventSetNotification_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_EVENT_SET_NOTIFICATION_PARAMS *pEventSetNotificationParams);

NV_STATUS cliresCtrlCmdEventGetSystemEventData_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GET_SYSTEM_EVENT_DATA_PARAMS *pSystemEventDataParams);

NV_STATUS cliresCtrlCmdOsUnixExportObjectToFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_EXPORT_OBJECT_TO_FD_PARAMS *pParams);

NV_STATUS cliresCtrlCmdOsUnixImportObjectFromFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_IMPORT_OBJECT_FROM_FD_PARAMS *pParams);

NV_STATUS cliresCtrlCmdOsUnixGetExportObjectInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_GET_EXPORT_OBJECT_INFO_PARAMS *pParams);

NV_STATUS cliresCtrlCmdOsUnixCreateExportObjectFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_CREATE_EXPORT_OBJECT_FD_PARAMS *pParams);

NV_STATUS cliresCtrlCmdOsUnixExportObjectsToFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_EXPORT_OBJECTS_TO_FD_PARAMS *pParams);

NV_STATUS cliresCtrlCmdOsUnixImportObjectsFromFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_IMPORT_OBJECTS_FROM_FD_PARAMS *pParams);

NV_STATUS cliresCtrlCmdOsUnixFlushUserCache_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_FLUSH_USER_CACHE_PARAMS *pAddressSpaceParams);

NV_STATUS cliresCtrlCmdSetSubProcessID_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SET_SUB_PROCESS_ID_PARAMS *pParams);

NV_STATUS cliresCtrlCmdDisableSubProcessUserdIsolation_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_DISABLE_SUB_PROCESS_USERD_ISOLATION_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemNVPCFGetPowerModeInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_NVPCF_GET_POWER_MODE_INFO_PARAMS *pParams);

NV_STATUS cliresCtrlCmdSystemSyncExternalFabricMgmt_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CMD_SYSTEM_SYNC_EXTERNAL_FABRIC_MGMT_PARAMS *pExtFabricMgmtParams);

NV_STATUS cliresConstruct_IMPL(struct RmClientResource *arg_pRmCliRes, struct CALL_CONTEXT *arg_pCallContext, struct RS_RES_ALLOC_PARAMS_INTERNAL *arg_pParams);

#define __nvoc_cliresConstruct(arg_pRmCliRes, arg_pCallContext, arg_pParams) cliresConstruct_IMPL(arg_pRmCliRes, arg_pCallContext, arg_pParams)
void cliresDestruct_IMPL(struct RmClientResource *pRmCliRes);

#define __nvoc_cliresDestruct(pRmCliRes) cliresDestruct_IMPL(pRmCliRes)
#undef PRIVATE_FIELD


NV_STATUS CliGetSystemP2pCaps(NvU32 *gpuIds,
                              NvU32 gpuCount,
                              NvU32 *p2pCaps,
                              NvU32 *p2pOptimalReadCEs,
                              NvU32 *p2pOptimalWriteCEs,
                              NvU8 *p2pCapsStatus,
                              NvU32 *pBusPeerIds,
                              NvU32 *pBusEgmPeerIds);

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _G_CLIENT_RESOURCE_NVOC_H_
