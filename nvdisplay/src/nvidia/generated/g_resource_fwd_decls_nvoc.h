#ifndef _G_RESOURCE_FWD_DECLS_NVOC_H_
#define _G_RESOURCE_FWD_DECLS_NVOC_H_
#include "nvoc/runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SPDX-FileCopyrightText: Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "g_resource_fwd_decls_nvoc.h"

#ifndef RESOURCE_FWD_DECLS_H
#define RESOURCE_FWD_DECLS_H

#include "nvtypes.h"
#include "nvoc/prelude.h"
#include "nvoc/object.h"
#include "rmconfig.h"

// Base classes
struct ChannelDescendant;

#ifndef __NVOC_CLASS_ChannelDescendant_TYPEDEF__
#define __NVOC_CLASS_ChannelDescendant_TYPEDEF__
typedef struct ChannelDescendant ChannelDescendant;
#endif /* __NVOC_CLASS_ChannelDescendant_TYPEDEF__ */

#ifndef __nvoc_class_id_ChannelDescendant
#define __nvoc_class_id_ChannelDescendant 0x43d7c4
#endif /* __nvoc_class_id_ChannelDescendant */


struct DispChannel;

#ifndef __NVOC_CLASS_DispChannel_TYPEDEF__
#define __NVOC_CLASS_DispChannel_TYPEDEF__
typedef struct DispChannel DispChannel;
#endif /* __NVOC_CLASS_DispChannel_TYPEDEF__ */

#ifndef __nvoc_class_id_DispChannel
#define __nvoc_class_id_DispChannel 0xbd2ff3
#endif /* __nvoc_class_id_DispChannel */


struct GpuResource;

#ifndef __NVOC_CLASS_GpuResource_TYPEDEF__
#define __NVOC_CLASS_GpuResource_TYPEDEF__
typedef struct GpuResource GpuResource;
#endif /* __NVOC_CLASS_GpuResource_TYPEDEF__ */

#ifndef __nvoc_class_id_GpuResource
#define __nvoc_class_id_GpuResource 0x5d5d9f
#endif /* __nvoc_class_id_GpuResource */


struct INotifier;

#ifndef __NVOC_CLASS_INotifier_TYPEDEF__
#define __NVOC_CLASS_INotifier_TYPEDEF__
typedef struct INotifier INotifier;
#endif /* __NVOC_CLASS_INotifier_TYPEDEF__ */

#ifndef __nvoc_class_id_INotifier
#define __nvoc_class_id_INotifier 0xf8f965
#endif /* __nvoc_class_id_INotifier */


struct Memory;

#ifndef __NVOC_CLASS_Memory_TYPEDEF__
#define __NVOC_CLASS_Memory_TYPEDEF__
typedef struct Memory Memory;
#endif /* __NVOC_CLASS_Memory_TYPEDEF__ */

#ifndef __nvoc_class_id_Memory
#define __nvoc_class_id_Memory 0x4789f2
#endif /* __nvoc_class_id_Memory */


struct Notifier;

#ifndef __NVOC_CLASS_Notifier_TYPEDEF__
#define __NVOC_CLASS_Notifier_TYPEDEF__
typedef struct Notifier Notifier;
#endif /* __NVOC_CLASS_Notifier_TYPEDEF__ */

#ifndef __nvoc_class_id_Notifier
#define __nvoc_class_id_Notifier 0xa8683b
#endif /* __nvoc_class_id_Notifier */


struct NotifShare;

#ifndef __NVOC_CLASS_NotifShare_TYPEDEF__
#define __NVOC_CLASS_NotifShare_TYPEDEF__
typedef struct NotifShare NotifShare;
#endif /* __NVOC_CLASS_NotifShare_TYPEDEF__ */

#ifndef __nvoc_class_id_NotifShare
#define __nvoc_class_id_NotifShare 0xd5f150
#endif /* __nvoc_class_id_NotifShare */


struct Resource;

#ifndef __NVOC_CLASS_Resource_TYPEDEF__
#define __NVOC_CLASS_Resource_TYPEDEF__
typedef struct Resource Resource;
#endif /* __NVOC_CLASS_Resource_TYPEDEF__ */

#ifndef __nvoc_class_id_Resource
#define __nvoc_class_id_Resource 0xbe8545
#endif /* __nvoc_class_id_Resource */


struct RmResource;

#ifndef __NVOC_CLASS_RmResource_TYPEDEF__
#define __NVOC_CLASS_RmResource_TYPEDEF__
typedef struct RmResource RmResource;
#endif /* __NVOC_CLASS_RmResource_TYPEDEF__ */

#ifndef __nvoc_class_id_RmResource
#define __nvoc_class_id_RmResource 0x03610d
#endif /* __nvoc_class_id_RmResource */


struct RmResourceCommon;

#ifndef __NVOC_CLASS_RmResourceCommon_TYPEDEF__
#define __NVOC_CLASS_RmResourceCommon_TYPEDEF__
typedef struct RmResourceCommon RmResourceCommon;
#endif /* __NVOC_CLASS_RmResourceCommon_TYPEDEF__ */

#ifndef __nvoc_class_id_RmResourceCommon
#define __nvoc_class_id_RmResourceCommon 0x8ef259
#endif /* __nvoc_class_id_RmResourceCommon */


struct RsResource;

#ifndef __NVOC_CLASS_RsResource_TYPEDEF__
#define __NVOC_CLASS_RsResource_TYPEDEF__
typedef struct RsResource RsResource;
#endif /* __NVOC_CLASS_RsResource_TYPEDEF__ */

#ifndef __nvoc_class_id_RsResource
#define __nvoc_class_id_RsResource 0xd551cb
#endif /* __nvoc_class_id_RsResource */


struct RsShared;

#ifndef __NVOC_CLASS_RsShared_TYPEDEF__
#define __NVOC_CLASS_RsShared_TYPEDEF__
typedef struct RsShared RsShared;
#endif /* __NVOC_CLASS_RsShared_TYPEDEF__ */

#ifndef __nvoc_class_id_RsShared
#define __nvoc_class_id_RsShared 0x830542
#endif /* __nvoc_class_id_RsShared */



// Classes disabled in orin but required forward declarations to build.
struct HostVgpuDeviceApi;

#ifndef __NVOC_CLASS_HostVgpuDeviceApi_TYPEDEF__
#define __NVOC_CLASS_HostVgpuDeviceApi_TYPEDEF__
typedef struct HostVgpuDeviceApi HostVgpuDeviceApi;
#endif /* __NVOC_CLASS_HostVgpuDeviceApi_TYPEDEF__ */

#ifndef __nvoc_class_id_HostVgpuDeviceApi
#define __nvoc_class_id_HostVgpuDeviceApi 0x4c4173
#endif /* __nvoc_class_id_HostVgpuDeviceApi */

 // also used by open rm
struct MpsApi;

#ifndef __NVOC_CLASS_MpsApi_TYPEDEF__
#define __NVOC_CLASS_MpsApi_TYPEDEF__
typedef struct MpsApi MpsApi;
#endif /* __NVOC_CLASS_MpsApi_TYPEDEF__ */

#ifndef __nvoc_class_id_MpsApi
#define __nvoc_class_id_MpsApi 0x22ce42
#endif /* __nvoc_class_id_MpsApi */


struct MIGConfigSession;

#ifndef __NVOC_CLASS_MIGConfigSession_TYPEDEF__
#define __NVOC_CLASS_MIGConfigSession_TYPEDEF__
typedef struct MIGConfigSession MIGConfigSession;
#endif /* __NVOC_CLASS_MIGConfigSession_TYPEDEF__ */

#ifndef __nvoc_class_id_MIGConfigSession
#define __nvoc_class_id_MIGConfigSession 0x36a941
#endif /* __nvoc_class_id_MIGConfigSession */


struct FmSessionApi;

#ifndef __NVOC_CLASS_FmSessionApi_TYPEDEF__
#define __NVOC_CLASS_FmSessionApi_TYPEDEF__
typedef struct FmSessionApi FmSessionApi;
#endif /* __NVOC_CLASS_FmSessionApi_TYPEDEF__ */

#ifndef __nvoc_class_id_FmSessionApi
#define __nvoc_class_id_FmSessionApi 0xdfbd08
#endif /* __nvoc_class_id_FmSessionApi */


struct MIGMonitorSession;

#ifndef __NVOC_CLASS_MIGMonitorSession_TYPEDEF__
#define __NVOC_CLASS_MIGMonitorSession_TYPEDEF__
typedef struct MIGMonitorSession MIGMonitorSession;
#endif /* __NVOC_CLASS_MIGMonitorSession_TYPEDEF__ */

#ifndef __nvoc_class_id_MIGMonitorSession
#define __nvoc_class_id_MIGMonitorSession 0x29e15c
#endif /* __nvoc_class_id_MIGMonitorSession */


struct TimerApi;

#ifndef __NVOC_CLASS_TimerApi_TYPEDEF__
#define __NVOC_CLASS_TimerApi_TYPEDEF__
typedef struct TimerApi TimerApi;
#endif /* __NVOC_CLASS_TimerApi_TYPEDEF__ */

#ifndef __nvoc_class_id_TimerApi
#define __nvoc_class_id_TimerApi 0xb13ac4
#endif /* __nvoc_class_id_TimerApi */


struct KernelSMDebuggerSession;

#ifndef __NVOC_CLASS_KernelSMDebuggerSession_TYPEDEF__
#define __NVOC_CLASS_KernelSMDebuggerSession_TYPEDEF__
typedef struct KernelSMDebuggerSession KernelSMDebuggerSession;
#endif /* __NVOC_CLASS_KernelSMDebuggerSession_TYPEDEF__ */

#ifndef __nvoc_class_id_KernelSMDebuggerSession
#define __nvoc_class_id_KernelSMDebuggerSession 0x4adc81
#endif /* __nvoc_class_id_KernelSMDebuggerSession */



// NVOC only expand macros inside a class. Use the stub class
#ifdef NVOC_RESOURCE_FWD_DECLS_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif
struct RmClientResource;

#ifndef __NVOC_CLASS_RmClientResource_TYPEDEF__
#define __NVOC_CLASS_RmClientResource_TYPEDEF__
typedef struct RmClientResource RmClientResource;
#endif /* __NVOC_CLASS_RmClientResource_TYPEDEF__ */

#ifndef __nvoc_class_id_RmClientResource
#define __nvoc_class_id_RmClientResource 0x37a701
#endif /* __nvoc_class_id_RmClientResource */

struct GpuManagementApi;

#ifndef __NVOC_CLASS_GpuManagementApi_TYPEDEF__
#define __NVOC_CLASS_GpuManagementApi_TYPEDEF__
typedef struct GpuManagementApi GpuManagementApi;
#endif /* __NVOC_CLASS_GpuManagementApi_TYPEDEF__ */

#ifndef __nvoc_class_id_GpuManagementApi
#define __nvoc_class_id_GpuManagementApi 0x376305
#endif /* __nvoc_class_id_GpuManagementApi */

struct Device;

#ifndef __NVOC_CLASS_Device_TYPEDEF__
#define __NVOC_CLASS_Device_TYPEDEF__
typedef struct Device Device;
#endif /* __NVOC_CLASS_Device_TYPEDEF__ */

#ifndef __nvoc_class_id_Device
#define __nvoc_class_id_Device 0xe0ac20
#endif /* __nvoc_class_id_Device */

struct Hdacodec;

#ifndef __NVOC_CLASS_Hdacodec_TYPEDEF__
#define __NVOC_CLASS_Hdacodec_TYPEDEF__
typedef struct Hdacodec Hdacodec;
#endif /* __NVOC_CLASS_Hdacodec_TYPEDEF__ */

#ifndef __nvoc_class_id_Hdacodec
#define __nvoc_class_id_Hdacodec 0xf59a20
#endif /* __nvoc_class_id_Hdacodec */

struct Subdevice;

#ifndef __NVOC_CLASS_Subdevice_TYPEDEF__
#define __NVOC_CLASS_Subdevice_TYPEDEF__
typedef struct Subdevice Subdevice;
#endif /* __NVOC_CLASS_Subdevice_TYPEDEF__ */

#ifndef __nvoc_class_id_Subdevice
#define __nvoc_class_id_Subdevice 0x4b01b3
#endif /* __nvoc_class_id_Subdevice */

struct BinaryApi;

#ifndef __NVOC_CLASS_BinaryApi_TYPEDEF__
#define __NVOC_CLASS_BinaryApi_TYPEDEF__
typedef struct BinaryApi BinaryApi;
#endif /* __NVOC_CLASS_BinaryApi_TYPEDEF__ */

#ifndef __nvoc_class_id_BinaryApi
#define __nvoc_class_id_BinaryApi 0xb7a47c
#endif /* __nvoc_class_id_BinaryApi */

struct BinaryApiPrivileged;

#ifndef __NVOC_CLASS_BinaryApiPrivileged_TYPEDEF__
#define __NVOC_CLASS_BinaryApiPrivileged_TYPEDEF__
typedef struct BinaryApiPrivileged BinaryApiPrivileged;
#endif /* __NVOC_CLASS_BinaryApiPrivileged_TYPEDEF__ */

#ifndef __nvoc_class_id_BinaryApiPrivileged
#define __nvoc_class_id_BinaryApiPrivileged 0x1c0579
#endif /* __nvoc_class_id_BinaryApiPrivileged */

struct SystemMemory;

#ifndef __NVOC_CLASS_SystemMemory_TYPEDEF__
#define __NVOC_CLASS_SystemMemory_TYPEDEF__
typedef struct SystemMemory SystemMemory;
#endif /* __NVOC_CLASS_SystemMemory_TYPEDEF__ */

#ifndef __nvoc_class_id_SystemMemory
#define __nvoc_class_id_SystemMemory 0x007a98
#endif /* __nvoc_class_id_SystemMemory */

struct OsDescMemory;

#ifndef __NVOC_CLASS_OsDescMemory_TYPEDEF__
#define __NVOC_CLASS_OsDescMemory_TYPEDEF__
typedef struct OsDescMemory OsDescMemory;
#endif /* __NVOC_CLASS_OsDescMemory_TYPEDEF__ */

#ifndef __nvoc_class_id_OsDescMemory
#define __nvoc_class_id_OsDescMemory 0xb3dacd
#endif /* __nvoc_class_id_OsDescMemory */

struct SyncpointMemory;

#ifndef __NVOC_CLASS_SyncpointMemory_TYPEDEF__
#define __NVOC_CLASS_SyncpointMemory_TYPEDEF__
typedef struct SyncpointMemory SyncpointMemory;
#endif /* __NVOC_CLASS_SyncpointMemory_TYPEDEF__ */

#ifndef __nvoc_class_id_SyncpointMemory
#define __nvoc_class_id_SyncpointMemory 0x529def
#endif /* __nvoc_class_id_SyncpointMemory */

struct ConsoleMemory;

#ifndef __NVOC_CLASS_ConsoleMemory_TYPEDEF__
#define __NVOC_CLASS_ConsoleMemory_TYPEDEF__
typedef struct ConsoleMemory ConsoleMemory;
#endif /* __NVOC_CLASS_ConsoleMemory_TYPEDEF__ */

#ifndef __nvoc_class_id_ConsoleMemory
#define __nvoc_class_id_ConsoleMemory 0xaac69e
#endif /* __nvoc_class_id_ConsoleMemory */

struct DispSfUser;

#ifndef __NVOC_CLASS_DispSfUser_TYPEDEF__
#define __NVOC_CLASS_DispSfUser_TYPEDEF__
typedef struct DispSfUser DispSfUser;
#endif /* __NVOC_CLASS_DispSfUser_TYPEDEF__ */

#ifndef __nvoc_class_id_DispSfUser
#define __nvoc_class_id_DispSfUser 0xba7439
#endif /* __nvoc_class_id_DispSfUser */

struct NvDispApi;

#ifndef __NVOC_CLASS_NvDispApi_TYPEDEF__
#define __NVOC_CLASS_NvDispApi_TYPEDEF__
typedef struct NvDispApi NvDispApi;
#endif /* __NVOC_CLASS_NvDispApi_TYPEDEF__ */

#ifndef __nvoc_class_id_NvDispApi
#define __nvoc_class_id_NvDispApi 0x36aa0b
#endif /* __nvoc_class_id_NvDispApi */

struct DispSwObj;

#ifndef __NVOC_CLASS_DispSwObj_TYPEDEF__
#define __NVOC_CLASS_DispSwObj_TYPEDEF__
typedef struct DispSwObj DispSwObj;
#endif /* __NVOC_CLASS_DispSwObj_TYPEDEF__ */

#ifndef __nvoc_class_id_DispSwObj
#define __nvoc_class_id_DispSwObj 0x6aa5e2
#endif /* __nvoc_class_id_DispSwObj */

struct DispCommon;

#ifndef __NVOC_CLASS_DispCommon_TYPEDEF__
#define __NVOC_CLASS_DispCommon_TYPEDEF__
typedef struct DispCommon DispCommon;
#endif /* __NVOC_CLASS_DispCommon_TYPEDEF__ */

#ifndef __nvoc_class_id_DispCommon
#define __nvoc_class_id_DispCommon 0x41f4f2
#endif /* __nvoc_class_id_DispCommon */

struct DispChannelPio;

#ifndef __NVOC_CLASS_DispChannelPio_TYPEDEF__
#define __NVOC_CLASS_DispChannelPio_TYPEDEF__
typedef struct DispChannelPio DispChannelPio;
#endif /* __NVOC_CLASS_DispChannelPio_TYPEDEF__ */

#ifndef __nvoc_class_id_DispChannelPio
#define __nvoc_class_id_DispChannelPio 0x10dec3
#endif /* __nvoc_class_id_DispChannelPio */

struct DispChannelDma;

#ifndef __NVOC_CLASS_DispChannelDma_TYPEDEF__
#define __NVOC_CLASS_DispChannelDma_TYPEDEF__
typedef struct DispChannelDma DispChannelDma;
#endif /* __NVOC_CLASS_DispChannelDma_TYPEDEF__ */

#ifndef __nvoc_class_id_DispChannelDma
#define __nvoc_class_id_DispChannelDma 0xfe3d2e
#endif /* __nvoc_class_id_DispChannelDma */

struct DispCapabilities;

#ifndef __NVOC_CLASS_DispCapabilities_TYPEDEF__
#define __NVOC_CLASS_DispCapabilities_TYPEDEF__
typedef struct DispCapabilities DispCapabilities;
#endif /* __NVOC_CLASS_DispCapabilities_TYPEDEF__ */

#ifndef __nvoc_class_id_DispCapabilities
#define __nvoc_class_id_DispCapabilities 0x99db3e
#endif /* __nvoc_class_id_DispCapabilities */

struct ContextDma;

#ifndef __NVOC_CLASS_ContextDma_TYPEDEF__
#define __NVOC_CLASS_ContextDma_TYPEDEF__
typedef struct ContextDma ContextDma;
#endif /* __NVOC_CLASS_ContextDma_TYPEDEF__ */

#ifndef __nvoc_class_id_ContextDma
#define __nvoc_class_id_ContextDma 0x88441b
#endif /* __nvoc_class_id_ContextDma */

struct Event;

#ifndef __NVOC_CLASS_Event_TYPEDEF__
#define __NVOC_CLASS_Event_TYPEDEF__
typedef struct Event Event;
#endif /* __NVOC_CLASS_Event_TYPEDEF__ */

#ifndef __nvoc_class_id_Event
#define __nvoc_class_id_Event 0xa4ecfc
#endif /* __nvoc_class_id_Event */


struct NVOCFwdDeclHack {
    const struct NVOC_RTTI *__nvoc_rtti;
    struct NVOCFwdDeclHack *__nvoc_pbase_NVOCFwdDeclHack;
    struct RmClientResource *PRIVATE_FIELD(RmClientResource_NV01_ROOT);
    struct RmClientResource *PRIVATE_FIELD(RmClientResource_NV01_ROOT_NON_PRIV);
    struct RmClientResource *PRIVATE_FIELD(RmClientResource_NV01_ROOT_CLIENT);
    struct GpuManagementApi *PRIVATE_FIELD(GpuManagementApi_NV0020_GPU_MANAGEMENT);
    struct Device *PRIVATE_FIELD(Device_NV01_DEVICE_0);
    struct Hdacodec *PRIVATE_FIELD(Hdacodec_GF100_HDACODEC);
    struct Subdevice *PRIVATE_FIELD(Subdevice_NV20_SUBDEVICE_0);
    struct BinaryApi *PRIVATE_FIELD(BinaryApi_NV2081_BINAPI);
    struct BinaryApiPrivileged *PRIVATE_FIELD(BinaryApiPrivileged_NV2082_BINAPI_PRIVILEGED);
    struct SystemMemory *PRIVATE_FIELD(SystemMemory_NV01_MEMORY_SYSTEM);
    struct OsDescMemory *PRIVATE_FIELD(OsDescMemory_NV01_MEMORY_SYSTEM_OS_DESCRIPTOR);
    struct SyncpointMemory *PRIVATE_FIELD(SyncpointMemory_NV01_MEMORY_SYNCPOINT);
    struct ConsoleMemory *PRIVATE_FIELD(ConsoleMemory_NV01_MEMORY_FRAMEBUFFER_CONSOLE);
    struct DispSfUser *PRIVATE_FIELD(DispSfUser_NVC671_DISP_SF_USER);
    struct NvDispApi *PRIVATE_FIELD(NvDispApi_NVC670_DISPLAY);
    struct DispSwObj *PRIVATE_FIELD(DispSwObj_NVC372_DISPLAY_SW);
    struct DispCommon *PRIVATE_FIELD(DispCommon_NV04_DISPLAY_COMMON);
    struct DispChannelPio *PRIVATE_FIELD(DispChannelPio_NVC67A_CURSOR_IMM_CHANNEL_PIO);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC67B_WINDOW_IMM_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC67D_CORE_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC77F_ANY_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC67E_WINDOW_CHANNEL_DMA);
    struct DispCapabilities *PRIVATE_FIELD(DispCapabilities_NVC673_DISP_CAPABILITIES);
    struct ContextDma *PRIVATE_FIELD(ContextDma_NV01_CONTEXT_DMA);
    struct Event *PRIVATE_FIELD(Event_NV01_EVENT);
    struct Event *PRIVATE_FIELD(Event_NV01_EVENT_OS_EVENT);
    struct Event *PRIVATE_FIELD(Event_NV01_EVENT_KERNEL_CALLBACK);
    struct Event *PRIVATE_FIELD(Event_NV01_EVENT_KERNEL_CALLBACK_EX);
};

#ifndef __NVOC_CLASS_NVOCFwdDeclHack_TYPEDEF__
#define __NVOC_CLASS_NVOCFwdDeclHack_TYPEDEF__
typedef struct NVOCFwdDeclHack NVOCFwdDeclHack;
#endif /* __NVOC_CLASS_NVOCFwdDeclHack_TYPEDEF__ */

#ifndef __nvoc_class_id_NVOCFwdDeclHack
#define __nvoc_class_id_NVOCFwdDeclHack 0x0d01f5
#endif /* __nvoc_class_id_NVOCFwdDeclHack */

extern const struct NVOC_CLASS_DEF __nvoc_class_def_NVOCFwdDeclHack;

#define __staticCast_NVOCFwdDeclHack(pThis) \
    ((pThis)->__nvoc_pbase_NVOCFwdDeclHack)

#ifdef __nvoc_resource_fwd_decls_h_disabled
#define __dynamicCast_NVOCFwdDeclHack(pThis) ((NVOCFwdDeclHack*)NULL)
#else //__nvoc_resource_fwd_decls_h_disabled
#define __dynamicCast_NVOCFwdDeclHack(pThis) \
    ((NVOCFwdDeclHack*)__nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(NVOCFwdDeclHack)))
#endif //__nvoc_resource_fwd_decls_h_disabled


NV_STATUS __nvoc_objCreateDynamic_NVOCFwdDeclHack(NVOCFwdDeclHack**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_NVOCFwdDeclHack(NVOCFwdDeclHack**, Dynamic*, NvU32);
#define __objCreate_NVOCFwdDeclHack(ppNewObj, pParent, createFlags) \
    __nvoc_objCreate_NVOCFwdDeclHack((ppNewObj), staticCast((pParent), Dynamic), (createFlags))

#undef PRIVATE_FIELD


#endif // RESOURCE_FWD_DECLS_H


#ifdef __cplusplus
} // extern "C"
#endif
#endif // _G_RESOURCE_FWD_DECLS_NVOC_H_
