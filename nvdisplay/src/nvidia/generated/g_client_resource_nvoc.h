
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
#include "rmapi/event_api.h"
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

#ifndef __nvoc_class_id_RmClientResource
#define __nvoc_class_id_RmClientResource 0x37a701u
typedef struct RmClientResource RmClientResource;
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

NV_STATUS __nvoc_objCreate_RmClientResource(RmClientResource**, Dynamic*, NvU32, struct CALL_CONTEXT *pCallContext, struct RS_RES_ALLOC_PARAMS_INTERNAL *pParams);
#define __objCreate_RmClientResource(__nvoc_ppNewObj, __nvoc_pParent, __nvoc_createFlags, pCallContext, pParams) \
    __nvoc_objCreate_RmClientResource((__nvoc_ppNewObj), staticCast((__nvoc_pParent), Dynamic), (__nvoc_createFlags), pCallContext, pParams)


// Wrapper macros for implementation functions
NV_STATUS cliresConstruct_IMPL(struct RmClientResource *pRmCliRes, struct CALL_CONTEXT *pCallContext, struct RS_RES_ALLOC_PARAMS_INTERNAL *pParams);
#define __nvoc_cliresConstruct(pRmCliRes, pCallContext, pParams) cliresConstruct_IMPL(pRmCliRes, pCallContext, pParams)

void cliresDestruct_IMPL(struct RmClientResource *pRmCliRes);
#define __nvoc_cliresDestruct(pRmCliRes) cliresDestruct_IMPL(pRmCliRes)

NV_STATUS cliresCtrlCmdSystemGetCpuInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CPU_INFO_PARAMS *pCpuInfoParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetCpuInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CPU_INFO_PARAMS *pCpuInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetCpuInfo(pRmCliRes, pCpuInfoParams) cliresCtrlCmdSystemGetCpuInfo_IMPL(pRmCliRes, pCpuInfoParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetFeatures_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_FEATURES_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetFeatures(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_FEATURES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetFeatures(pRmCliRes, pParams) cliresCtrlCmdSystemGetFeatures_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetBuildVersionV2_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_BUILD_VERSION_V2_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetBuildVersionV2(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_BUILD_VERSION_V2_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetBuildVersionV2(pRmCliRes, pParams) cliresCtrlCmdSystemGetBuildVersionV2_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetLockTimes_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_LOCK_TIMES_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetLockTimes(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_LOCK_TIMES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetLockTimes(pRmCliRes, pParams) cliresCtrlCmdSystemGetLockTimes_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetClassList_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CLASSLIST_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetClassList(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CLASSLIST_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetClassList(pRmCliRes, pParams) cliresCtrlCmdSystemGetClassList_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemNotifyEvent_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_NOTIFY_EVENT_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemNotifyEvent(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_NOTIFY_EVENT_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemNotifyEvent(pRmCliRes, pParams) cliresCtrlCmdSystemNotifyEvent_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemDebugCtrlRmMsg_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_DEBUG_RMMSG_CTRL_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemDebugCtrlRmMsg(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_DEBUG_RMMSG_CTRL_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemDebugCtrlRmMsg(pRmCliRes, pParams) cliresCtrlCmdSystemDebugCtrlRmMsg_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetVgxSystemInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_VGX_SYSTEM_INFO_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetVgxSystemInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_VGX_SYSTEM_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetVgxSystemInfo(pRmCliRes, pParams) cliresCtrlCmdSystemGetVgxSystemInfo_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetPrivilegedStatus_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_PRIVILEGED_STATUS_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetPrivilegedStatus(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_PRIVILEGED_STATUS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetPrivilegedStatus(pRmCliRes, pParams) cliresCtrlCmdSystemGetPrivilegedStatus_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetFabricStatus_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_FABRIC_STATUS_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetFabricStatus(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_FABRIC_STATUS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetFabricStatus(pRmCliRes, pParams) cliresCtrlCmdSystemGetFabricStatus_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetRmInstanceId_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_RM_INSTANCE_ID_PARAMS *pRmInstanceIdParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetRmInstanceId(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_RM_INSTANCE_ID_PARAMS *pRmInstanceIdParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetRmInstanceId(pRmCliRes, pRmInstanceIdParams) cliresCtrlCmdSystemGetRmInstanceId_IMPL(pRmCliRes, pRmInstanceIdParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetClientDatabaseInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CLIENT_DATABASE_INFO_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetClientDatabaseInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_CLIENT_DATABASE_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetClientDatabaseInfo(pRmCliRes, pParams) cliresCtrlCmdSystemGetClientDatabaseInfo_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemRmctrlCacheModeCtrl_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_RMCTRL_CACHE_MODE_CTRL_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemRmctrlCacheModeCtrl(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_RMCTRL_CACHE_MODE_CTRL_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemRmctrlCacheModeCtrl(pRmCliRes, pParams) cliresCtrlCmdSystemRmctrlCacheModeCtrl_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdClientGetAddrSpaceType_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_ADDR_SPACE_TYPE_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdClientGetAddrSpaceType(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_ADDR_SPACE_TYPE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdClientGetAddrSpaceType(pRmCliRes, pParams) cliresCtrlCmdClientGetAddrSpaceType_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdClientGetHandleInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_HANDLE_INFO_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdClientGetHandleInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_HANDLE_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdClientGetHandleInfo(pRmCliRes, pParams) cliresCtrlCmdClientGetHandleInfo_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdClientGetAccessRights_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_ACCESS_RIGHTS_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdClientGetAccessRights(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_GET_ACCESS_RIGHTS_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdClientGetAccessRights(pRmCliRes, pParams) cliresCtrlCmdClientGetAccessRights_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdClientSetInheritedSharePolicy_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_SET_INHERITED_SHARE_POLICY_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdClientSetInheritedSharePolicy(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_SET_INHERITED_SHARE_POLICY_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdClientSetInheritedSharePolicy(pRmCliRes, pParams) cliresCtrlCmdClientSetInheritedSharePolicy_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdClientShareObject_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_SHARE_OBJECT_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdClientShareObject(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_SHARE_OBJECT_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdClientShareObject(pRmCliRes, pParams) cliresCtrlCmdClientShareObject_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdClientGetChildHandle_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CMD_CLIENT_GET_CHILD_HANDLE_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdClientGetChildHandle(struct RmClientResource *pRmCliRes, NV0000_CTRL_CMD_CLIENT_GET_CHILD_HANDLE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdClientGetChildHandle(pRmCliRes, pParams) cliresCtrlCmdClientGetChildHandle_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdObjectsAreDuplicates_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_OBJECTS_ARE_DUPLICATES_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdObjectsAreDuplicates(struct RmClientResource *pRmCliRes, NV0000_CTRL_CLIENT_OBJECTS_ARE_DUPLICATES_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdObjectsAreDuplicates(pRmCliRes, pParams) cliresCtrlCmdObjectsAreDuplicates_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetAttachedIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ATTACHED_IDS_PARAMS *pGpuAttachedIds);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetAttachedIds(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ATTACHED_IDS_PARAMS *pGpuAttachedIds) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetAttachedIds(pRmCliRes, pGpuAttachedIds) cliresCtrlCmdGpuGetAttachedIds_IMPL(pRmCliRes, pGpuAttachedIds)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetIdInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ID_INFO_PARAMS *pGpuIdInfoParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetIdInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ID_INFO_PARAMS *pGpuIdInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetIdInfo(pRmCliRes, pGpuIdInfoParams) cliresCtrlCmdGpuGetIdInfo_IMPL(pRmCliRes, pGpuIdInfoParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetIdInfoV2_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS *pGpuIdInfoParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetIdInfoV2(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS *pGpuIdInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetIdInfoV2(pRmCliRes, pGpuIdInfoParams) cliresCtrlCmdGpuGetIdInfoV2_IMPL(pRmCliRes, pGpuIdInfoParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetInitStatus_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_INIT_STATUS_PARAMS *pGpuInitStatusParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetInitStatus(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_INIT_STATUS_PARAMS *pGpuInitStatusParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetInitStatus(pRmCliRes, pGpuInitStatusParams) cliresCtrlCmdGpuGetInitStatus_IMPL(pRmCliRes, pGpuInitStatusParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetDeviceIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_DEVICE_IDS_PARAMS *pDeviceIdsParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetDeviceIds(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_DEVICE_IDS_PARAMS *pDeviceIdsParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetDeviceIds(pRmCliRes, pDeviceIdsParams) cliresCtrlCmdGpuGetDeviceIds_IMPL(pRmCliRes, pDeviceIdsParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetActiveDeviceIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ACTIVE_DEVICE_IDS_PARAMS *pActiveDeviceIdsParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetActiveDeviceIds(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_ACTIVE_DEVICE_IDS_PARAMS *pActiveDeviceIdsParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetActiveDeviceIds(pRmCliRes, pActiveDeviceIdsParams) cliresCtrlCmdGpuGetActiveDeviceIds_IMPL(pRmCliRes, pActiveDeviceIdsParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetProbedIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_PROBED_IDS_PARAMS *pGpuProbedIds);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetProbedIds(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_PROBED_IDS_PARAMS *pGpuProbedIds) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetProbedIds(pRmCliRes, pGpuProbedIds) cliresCtrlCmdGpuGetProbedIds_IMPL(pRmCliRes, pGpuProbedIds)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuAttachIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_ATTACH_IDS_PARAMS *pGpuAttachIds);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuAttachIds(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_ATTACH_IDS_PARAMS *pGpuAttachIds) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuAttachIds(pRmCliRes, pGpuAttachIds) cliresCtrlCmdGpuAttachIds_IMPL(pRmCliRes, pGpuAttachIds)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuAsyncAttachId_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_ASYNC_ATTACH_ID_PARAMS *pAsyncAttachIdParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuAsyncAttachId(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_ASYNC_ATTACH_ID_PARAMS *pAsyncAttachIdParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuAsyncAttachId(pRmCliRes, pAsyncAttachIdParams) cliresCtrlCmdGpuAsyncAttachId_IMPL(pRmCliRes, pAsyncAttachIdParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuWaitAttachId_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_WAIT_ATTACH_ID_PARAMS *pWaitAttachIdParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuWaitAttachId(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_WAIT_ATTACH_ID_PARAMS *pWaitAttachIdParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuWaitAttachId(pRmCliRes, pWaitAttachIdParams) cliresCtrlCmdGpuWaitAttachId_IMPL(pRmCliRes, pWaitAttachIdParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuDetachIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_DETACH_IDS_PARAMS *pGpuDetachIds);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuDetachIds(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_DETACH_IDS_PARAMS *pGpuDetachIds) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuDetachIds(pRmCliRes, pGpuDetachIds) cliresCtrlCmdGpuDetachIds_IMPL(pRmCliRes, pGpuDetachIds)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetPciInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_PCI_INFO_PARAMS *pPciInfoParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetPciInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_PCI_INFO_PARAMS *pPciInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetPciInfo(pRmCliRes, pPciInfoParams) cliresCtrlCmdGpuGetPciInfo_IMPL(pRmCliRes, pPciInfoParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetUuidInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_UUID_INFO_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetUuidInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_UUID_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetUuidInfo(pRmCliRes, pParams) cliresCtrlCmdGpuGetUuidInfo_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetUuidFromGpuId_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_UUID_FROM_GPU_ID_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetUuidFromGpuId(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_UUID_FROM_GPU_ID_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetUuidFromGpuId(pRmCliRes, pParams) cliresCtrlCmdGpuGetUuidFromGpuId_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuModifyGpuDrainState_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_MODIFY_DRAIN_STATE_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuModifyGpuDrainState(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_MODIFY_DRAIN_STATE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuModifyGpuDrainState(pRmCliRes, pParams) cliresCtrlCmdGpuModifyGpuDrainState_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuQueryGpuDrainState_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_QUERY_DRAIN_STATE_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuQueryGpuDrainState(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_QUERY_DRAIN_STATE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuQueryGpuDrainState(pRmCliRes, pParams) cliresCtrlCmdGpuQueryGpuDrainState_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuGetMemOpEnable_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_MEMOP_ENABLE_PARAMS *pMemOpEnableParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuGetMemOpEnable(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_GET_MEMOP_ENABLE_PARAMS *pMemOpEnableParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuGetMemOpEnable(pRmCliRes, pMemOpEnableParams) cliresCtrlCmdGpuGetMemOpEnable_IMPL(pRmCliRes, pMemOpEnableParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGpuDisableNvlinkInit_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_DISABLE_NVLINK_INIT_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGpuDisableNvlinkInit(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_DISABLE_NVLINK_INIT_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGpuDisableNvlinkInit(pRmCliRes, pParams) cliresCtrlCmdGpuDisableNvlinkInit_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdPushUcodeImage_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_PUSH_UCODE_IMAGE_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdPushUcodeImage(struct RmClientResource *pRmCliRes, NV0000_CTRL_GPU_PUSH_UCODE_IMAGE_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdPushUcodeImage(pRmCliRes, pParams) cliresCtrlCmdPushUcodeImage_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemGetVrrCookiePresent_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_VRR_COOKIE_PRESENT_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemGetVrrCookiePresent(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_GET_VRR_COOKIE_PRESENT_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemGetVrrCookiePresent(pRmCliRes, pParams) cliresCtrlCmdSystemGetVrrCookiePresent_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGsyncGetAttachedIds_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GSYNC_GET_ATTACHED_IDS_PARAMS *pGsyncAttachedIds);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGsyncGetAttachedIds(struct RmClientResource *pRmCliRes, NV0000_CTRL_GSYNC_GET_ATTACHED_IDS_PARAMS *pGsyncAttachedIds) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGsyncGetAttachedIds(pRmCliRes, pGsyncAttachedIds) cliresCtrlCmdGsyncGetAttachedIds_IMPL(pRmCliRes, pGsyncAttachedIds)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdGsyncGetIdInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GSYNC_GET_ID_INFO_PARAMS *pGsyncIdInfoParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdGsyncGetIdInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_GSYNC_GET_ID_INFO_PARAMS *pGsyncIdInfoParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdGsyncGetIdInfo(pRmCliRes, pGsyncIdInfoParams) cliresCtrlCmdGsyncGetIdInfo_IMPL(pRmCliRes, pGsyncIdInfoParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdEventSetNotification_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_EVENT_SET_NOTIFICATION_PARAMS *pEventSetNotificationParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdEventSetNotification(struct RmClientResource *pRmCliRes, NV0000_CTRL_EVENT_SET_NOTIFICATION_PARAMS *pEventSetNotificationParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdEventSetNotification(pRmCliRes, pEventSetNotificationParams) cliresCtrlCmdEventSetNotification_IMPL(pRmCliRes, pEventSetNotificationParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdEventGetSystemEventData_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_GET_SYSTEM_EVENT_DATA_PARAMS *pSystemEventDataParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdEventGetSystemEventData(struct RmClientResource *pRmCliRes, NV0000_CTRL_GET_SYSTEM_EVENT_DATA_PARAMS *pSystemEventDataParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdEventGetSystemEventData(pRmCliRes, pSystemEventDataParams) cliresCtrlCmdEventGetSystemEventData_IMPL(pRmCliRes, pSystemEventDataParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdOsUnixExportObjectToFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_EXPORT_OBJECT_TO_FD_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdOsUnixExportObjectToFd(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_EXPORT_OBJECT_TO_FD_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdOsUnixExportObjectToFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixExportObjectToFd_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdOsUnixImportObjectFromFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_IMPORT_OBJECT_FROM_FD_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdOsUnixImportObjectFromFd(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_IMPORT_OBJECT_FROM_FD_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdOsUnixImportObjectFromFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixImportObjectFromFd_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdOsUnixGetExportObjectInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_GET_EXPORT_OBJECT_INFO_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdOsUnixGetExportObjectInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_GET_EXPORT_OBJECT_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdOsUnixGetExportObjectInfo(pRmCliRes, pParams) cliresCtrlCmdOsUnixGetExportObjectInfo_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdOsUnixCreateExportObjectFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_CREATE_EXPORT_OBJECT_FD_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdOsUnixCreateExportObjectFd(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_CREATE_EXPORT_OBJECT_FD_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdOsUnixCreateExportObjectFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixCreateExportObjectFd_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdOsUnixExportObjectsToFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_EXPORT_OBJECTS_TO_FD_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdOsUnixExportObjectsToFd(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_EXPORT_OBJECTS_TO_FD_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdOsUnixExportObjectsToFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixExportObjectsToFd_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdOsUnixImportObjectsFromFd_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_IMPORT_OBJECTS_FROM_FD_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdOsUnixImportObjectsFromFd(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_IMPORT_OBJECTS_FROM_FD_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdOsUnixImportObjectsFromFd(pRmCliRes, pParams) cliresCtrlCmdOsUnixImportObjectsFromFd_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdOsUnixFlushUserCache_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_FLUSH_USER_CACHE_PARAMS *pAddressSpaceParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdOsUnixFlushUserCache(struct RmClientResource *pRmCliRes, NV0000_CTRL_OS_UNIX_FLUSH_USER_CACHE_PARAMS *pAddressSpaceParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdOsUnixFlushUserCache(pRmCliRes, pAddressSpaceParams) cliresCtrlCmdOsUnixFlushUserCache_IMPL(pRmCliRes, pAddressSpaceParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSetSubProcessID_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SET_SUB_PROCESS_ID_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSetSubProcessID(struct RmClientResource *pRmCliRes, NV0000_CTRL_SET_SUB_PROCESS_ID_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSetSubProcessID(pRmCliRes, pParams) cliresCtrlCmdSetSubProcessID_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdDisableSubProcessUserdIsolation_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_DISABLE_SUB_PROCESS_USERD_ISOLATION_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdDisableSubProcessUserdIsolation(struct RmClientResource *pRmCliRes, NV0000_CTRL_DISABLE_SUB_PROCESS_USERD_ISOLATION_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdDisableSubProcessUserdIsolation(pRmCliRes, pParams) cliresCtrlCmdDisableSubProcessUserdIsolation_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemNVPCFGetPowerModeInfo_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_NVPCF_GET_POWER_MODE_INFO_PARAMS *pParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemNVPCFGetPowerModeInfo(struct RmClientResource *pRmCliRes, NV0000_CTRL_SYSTEM_NVPCF_GET_POWER_MODE_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemNVPCFGetPowerModeInfo(pRmCliRes, pParams) cliresCtrlCmdSystemNVPCFGetPowerModeInfo_IMPL(pRmCliRes, pParams)
#endif // __nvoc_client_resource_h_disabled

NV_STATUS cliresCtrlCmdSystemSyncExternalFabricMgmt_IMPL(struct RmClientResource *pRmCliRes, NV0000_CTRL_CMD_SYSTEM_SYNC_EXTERNAL_FABRIC_MGMT_PARAMS *pExtFabricMgmtParams);
#ifdef __nvoc_client_resource_h_disabled
static inline NV_STATUS cliresCtrlCmdSystemSyncExternalFabricMgmt(struct RmClientResource *pRmCliRes, NV0000_CTRL_CMD_SYSTEM_SYNC_EXTERNAL_FABRIC_MGMT_PARAMS *pExtFabricMgmtParams) {
    NV_ASSERT_FAILED_PRECOMP("RmClientResource was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_client_resource_h_disabled
#define cliresCtrlCmdSystemSyncExternalFabricMgmt(pRmCliRes, pExtFabricMgmtParams) cliresCtrlCmdSystemSyncExternalFabricMgmt_IMPL(pRmCliRes, pExtFabricMgmtParams)
#endif // __nvoc_client_resource_h_disabled


// Wrapper macros for halified functions
#define cliresAccessCallback_FNPTR(pRmCliRes) pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresAccessCallback__
#define cliresAccessCallback(pRmCliRes, pInvokingClient, pAllocParams, accessRight) cliresAccessCallback_DISPATCH(pRmCliRes, pInvokingClient, pAllocParams, accessRight)
#define cliresShareCallback_FNPTR(pRmCliRes) pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresShareCallback__
#define cliresShareCallback(pRmCliRes, pInvokingClient, pParentRef, pSharePolicy) cliresShareCallback_DISPATCH(pRmCliRes, pInvokingClient, pParentRef, pSharePolicy)
#define cliresControl_Prologue_FNPTR(pRmCliRes) pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresControl_Prologue__
#define cliresControl_Prologue(pRmCliRes, pCallContext, pParams) cliresControl_Prologue_DISPATCH(pRmCliRes, pCallContext, pParams)
#define cliresControl_Epilogue_FNPTR(pRmCliRes) pRmCliRes->__nvoc_metadata_ptr->vtable.__cliresControl_Epilogue__
#define cliresControl_Epilogue(pRmCliRes, pCallContext, pParams) cliresControl_Epilogue_DISPATCH(pRmCliRes, pCallContext, pParams)
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

// Virtual method declarations and/or inline definitions
NvBool cliresAccessCallback_IMPL(struct RmClientResource *pRmCliRes, struct RsClient *pInvokingClient, void *pAllocParams, RsAccessRight accessRight);

NvBool cliresShareCallback_IMPL(struct RmClientResource *pRmCliRes, struct RsClient *pInvokingClient, struct RsResourceRef *pParentRef, RS_SHARE_POLICY *pSharePolicy);

NV_STATUS cliresControl_Prologue_IMPL(struct RmClientResource *pRmCliRes, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams);

void cliresControl_Epilogue_IMPL(struct RmClientResource *pRmCliRes, struct CALL_CONTEXT *pCallContext, struct RS_RES_CONTROL_PARAMS_INTERNAL *pParams);

// Exported method declarations and/or inline definitions
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

// HAL method declarations without bodies
// Inline HAL method definitions
// Static dispatch method declarations
// Static inline method definitions
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
