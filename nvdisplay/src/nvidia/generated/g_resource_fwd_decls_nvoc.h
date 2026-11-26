
#ifndef _G_RESOURCE_FWD_DECLS_NVOC_H_
#define _G_RESOURCE_FWD_DECLS_NVOC_H_

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
 * SPDX-FileCopyrightText: Copyright (c) 2021-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "g_resource_fwd_decls_nvoc.h"

#ifndef RESOURCE_FWD_DECLS_H
#define RESOURCE_FWD_DECLS_H

#include "nvtypes.h"
#include "nvoc/prelude.h"
#include "nvoc/object.h"
#include "rmconfig.h"

// Base classes
struct ChannelDescendant;

#ifndef __nvoc_class_id_ChannelDescendant
#define __nvoc_class_id_ChannelDescendant 0x43d7c4u
typedef struct ChannelDescendant ChannelDescendant;
#endif /* __nvoc_class_id_ChannelDescendant */


struct DispChannel;

#ifndef __nvoc_class_id_DispChannel
#define __nvoc_class_id_DispChannel 0xbd2ff3u
typedef struct DispChannel DispChannel;
#endif /* __nvoc_class_id_DispChannel */


struct GpuResource;

#ifndef __nvoc_class_id_GpuResource
#define __nvoc_class_id_GpuResource 0x5d5d9fu
typedef struct GpuResource GpuResource;
#endif /* __nvoc_class_id_GpuResource */


struct INotifier;

#ifndef __nvoc_class_id_INotifier
#define __nvoc_class_id_INotifier 0xf8f965u
typedef struct INotifier INotifier;
#endif /* __nvoc_class_id_INotifier */


struct Memory;

#ifndef __nvoc_class_id_Memory
#define __nvoc_class_id_Memory 0x4789f2u
typedef struct Memory Memory;
#endif /* __nvoc_class_id_Memory */


struct Notifier;

#ifndef __nvoc_class_id_Notifier
#define __nvoc_class_id_Notifier 0xa8683bu
typedef struct Notifier Notifier;
#endif /* __nvoc_class_id_Notifier */


struct NotifShare;

#ifndef __nvoc_class_id_NotifShare
#define __nvoc_class_id_NotifShare 0xd5f150u
typedef struct NotifShare NotifShare;
#endif /* __nvoc_class_id_NotifShare */


struct Resource;

#ifndef __nvoc_class_id_Resource
#define __nvoc_class_id_Resource 0xbe8545u
typedef struct Resource Resource;
#endif /* __nvoc_class_id_Resource */


struct RmResource;

#ifndef __nvoc_class_id_RmResource
#define __nvoc_class_id_RmResource 0x03610du
typedef struct RmResource RmResource;
#endif /* __nvoc_class_id_RmResource */


struct RmResourceCommon;

#ifndef __nvoc_class_id_RmResourceCommon
#define __nvoc_class_id_RmResourceCommon 0x8ef259u
typedef struct RmResourceCommon RmResourceCommon;
#endif /* __nvoc_class_id_RmResourceCommon */


struct RsResource;

#ifndef __nvoc_class_id_RsResource
#define __nvoc_class_id_RsResource 0xd551cbu
typedef struct RsResource RsResource;
#endif /* __nvoc_class_id_RsResource */


struct RsShared;

#ifndef __nvoc_class_id_RsShared
#define __nvoc_class_id_RsShared 0x830542u
typedef struct RsShared RsShared;
#endif /* __nvoc_class_id_RsShared */



// Classes disabled in orin but required forward declarations to build.
struct HostVgpuDeviceApi;

#ifndef __nvoc_class_id_HostVgpuDeviceApi
#define __nvoc_class_id_HostVgpuDeviceApi 0x4c4173u
typedef struct HostVgpuDeviceApi HostVgpuDeviceApi;
#endif /* __nvoc_class_id_HostVgpuDeviceApi */

 // also used by open rm
struct MpsApi;

#ifndef __nvoc_class_id_MpsApi
#define __nvoc_class_id_MpsApi 0x22ce42u
typedef struct MpsApi MpsApi;
#endif /* __nvoc_class_id_MpsApi */


struct MIGConfigSession;

#ifndef __nvoc_class_id_MIGConfigSession
#define __nvoc_class_id_MIGConfigSession 0x36a941u
typedef struct MIGConfigSession MIGConfigSession;
#endif /* __nvoc_class_id_MIGConfigSession */


struct FmSessionApi;

#ifndef __nvoc_class_id_FmSessionApi
#define __nvoc_class_id_FmSessionApi 0xdfbd08u
typedef struct FmSessionApi FmSessionApi;
#endif /* __nvoc_class_id_FmSessionApi */


struct MIGMonitorSession;

#ifndef __nvoc_class_id_MIGMonitorSession
#define __nvoc_class_id_MIGMonitorSession 0x29e15cu
typedef struct MIGMonitorSession MIGMonitorSession;
#endif /* __nvoc_class_id_MIGMonitorSession */


struct TimerApi;

#ifndef __nvoc_class_id_TimerApi
#define __nvoc_class_id_TimerApi 0xb13ac4u
typedef struct TimerApi TimerApi;
#endif /* __nvoc_class_id_TimerApi */


struct KernelSMDebuggerSession;

#ifndef __nvoc_class_id_KernelSMDebuggerSession
#define __nvoc_class_id_KernelSMDebuggerSession 0x4adc81u
typedef struct KernelSMDebuggerSession KernelSMDebuggerSession;
#endif /* __nvoc_class_id_KernelSMDebuggerSession */



// NVOC only expand macros inside a class. Use the stub class

// Private field names are wrapped in PRIVATE_FIELD, which does nothing for
// the matching C source file, but causes diagnostics to be issued if another
// source file references the field.
#ifdef NVOC_RESOURCE_FWD_DECLS_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif


// Metadata with per-class RTTI
struct NVOC_METADATA__NVOCFwdDeclHack;

struct RmClientResource;

#ifndef __nvoc_class_id_RmClientResource
#define __nvoc_class_id_RmClientResource 0x37a701u
typedef struct RmClientResource RmClientResource;
#endif /* __nvoc_class_id_RmClientResource */

struct Device;

#ifndef __nvoc_class_id_Device
#define __nvoc_class_id_Device 0xe0ac20u
typedef struct Device Device;
#endif /* __nvoc_class_id_Device */

struct Hdacodec;

#ifndef __nvoc_class_id_Hdacodec
#define __nvoc_class_id_Hdacodec 0xf59a20u
typedef struct Hdacodec Hdacodec;
#endif /* __nvoc_class_id_Hdacodec */

struct Subdevice;

#ifndef __nvoc_class_id_Subdevice
#define __nvoc_class_id_Subdevice 0x4b01b3u
typedef struct Subdevice Subdevice;
#endif /* __nvoc_class_id_Subdevice */

struct SystemMemory;

#ifndef __nvoc_class_id_SystemMemory
#define __nvoc_class_id_SystemMemory 0x007a98u
typedef struct SystemMemory SystemMemory;
#endif /* __nvoc_class_id_SystemMemory */

struct OsDescMemory;

#ifndef __nvoc_class_id_OsDescMemory
#define __nvoc_class_id_OsDescMemory 0xb3dacdu
typedef struct OsDescMemory OsDescMemory;
#endif /* __nvoc_class_id_OsDescMemory */

struct SyncpointMemory;

#ifndef __nvoc_class_id_SyncpointMemory
#define __nvoc_class_id_SyncpointMemory 0x529defu
typedef struct SyncpointMemory SyncpointMemory;
#endif /* __nvoc_class_id_SyncpointMemory */

struct DispSfUser;

#ifndef __nvoc_class_id_DispSfUser
#define __nvoc_class_id_DispSfUser 0xba7439u
typedef struct DispSfUser DispSfUser;
#endif /* __nvoc_class_id_DispSfUser */

struct NvDispApi;

#ifndef __nvoc_class_id_NvDispApi
#define __nvoc_class_id_NvDispApi 0x36aa0bu
typedef struct NvDispApi NvDispApi;
#endif /* __nvoc_class_id_NvDispApi */

struct DispSwObj;

#ifndef __nvoc_class_id_DispSwObj
#define __nvoc_class_id_DispSwObj 0x6aa5e2u
typedef struct DispSwObj DispSwObj;
#endif /* __nvoc_class_id_DispSwObj */

struct DispCommon;

#ifndef __nvoc_class_id_DispCommon
#define __nvoc_class_id_DispCommon 0x41f4f2u
typedef struct DispCommon DispCommon;
#endif /* __nvoc_class_id_DispCommon */

struct DispChannelPio;

#ifndef __nvoc_class_id_DispChannelPio
#define __nvoc_class_id_DispChannelPio 0x10dec3u
typedef struct DispChannelPio DispChannelPio;
#endif /* __nvoc_class_id_DispChannelPio */

struct DispChannelDma;

#ifndef __nvoc_class_id_DispChannelDma
#define __nvoc_class_id_DispChannelDma 0xfe3d2eu
typedef struct DispChannelDma DispChannelDma;
#endif /* __nvoc_class_id_DispChannelDma */

struct DispCapabilities;

#ifndef __nvoc_class_id_DispCapabilities
#define __nvoc_class_id_DispCapabilities 0x99db3eu
typedef struct DispCapabilities DispCapabilities;
#endif /* __nvoc_class_id_DispCapabilities */

struct ContextDma;

#ifndef __nvoc_class_id_ContextDma
#define __nvoc_class_id_ContextDma 0x88441bu
typedef struct ContextDma ContextDma;
#endif /* __nvoc_class_id_ContextDma */

struct EventApi;

#ifndef __nvoc_class_id_EventApi
#define __nvoc_class_id_EventApi 0x854293u
typedef struct EventApi EventApi;
#endif /* __nvoc_class_id_EventApi */

struct LockStressObject;

#ifndef __nvoc_class_id_LockStressObject
#define __nvoc_class_id_LockStressObject 0xecce10u
typedef struct LockStressObject LockStressObject;
#endif /* __nvoc_class_id_LockStressObject */

struct LockTestRelaxedDupObject;

#ifndef __nvoc_class_id_LockTestRelaxedDupObject
#define __nvoc_class_id_LockTestRelaxedDupObject 0x19e861u
typedef struct LockTestRelaxedDupObject LockTestRelaxedDupObject;
#endif /* __nvoc_class_id_LockTestRelaxedDupObject */



struct NVOCFwdDeclHack {

    // Metadata starts with RTTI structure.
    union {
         const struct NVOC_METADATA__NVOCFwdDeclHack *__nvoc_metadata_ptr;
         const struct NVOC_RTTI *__nvoc_rtti;
    };

    // Ancestor object pointers for `staticCast` feature
    struct NVOCFwdDeclHack *__nvoc_pbase_NVOCFwdDeclHack;    // nvocfwddeclhack

    // Data members
    struct RmClientResource *PRIVATE_FIELD(RmClientResource_NV01_ROOT);
    struct RmClientResource *PRIVATE_FIELD(RmClientResource_NV01_ROOT_NON_PRIV);
    struct RmClientResource *PRIVATE_FIELD(RmClientResource_NV01_ROOT_CLIENT);
    struct Device *PRIVATE_FIELD(Device_NV01_DEVICE_0);
    struct Hdacodec *PRIVATE_FIELD(Hdacodec_GF100_HDACODEC);
    struct Subdevice *PRIVATE_FIELD(Subdevice_NV20_SUBDEVICE_0);
    struct SystemMemory *PRIVATE_FIELD(SystemMemory_NV01_MEMORY_SYSTEM);
    struct OsDescMemory *PRIVATE_FIELD(OsDescMemory_NV01_MEMORY_SYSTEM_OS_DESCRIPTOR);
    struct SyncpointMemory *PRIVATE_FIELD(SyncpointMemory_NV01_MEMORY_SYNCPOINT);
    struct DispSfUser *PRIVATE_FIELD(DispSfUser_NVC671_DISP_SF_USER);
    struct DispSfUser *PRIVATE_FIELD(DispSfUser_NVC971_DISP_SF_USER);
    struct DispSfUser *PRIVATE_FIELD(DispSfUser_NVCC71_DISP_SF_USER);
    struct NvDispApi *PRIVATE_FIELD(NvDispApi_NVC670_DISPLAY);
    struct NvDispApi *PRIVATE_FIELD(NvDispApi_NVC970_DISPLAY);
    struct NvDispApi *PRIVATE_FIELD(NvDispApi_NVCC70_DISPLAY);
    struct DispSwObj *PRIVATE_FIELD(DispSwObj_NVC372_DISPLAY_SW);
    struct DispCommon *PRIVATE_FIELD(DispCommon_NV04_DISPLAY_COMMON);
    struct DispChannelPio *PRIVATE_FIELD(DispChannelPio_NVC67A_CURSOR_IMM_CHANNEL_PIO);
    struct DispChannelPio *PRIVATE_FIELD(DispChannelPio_NVC97A_CURSOR_IMM_CHANNEL_PIO);
    struct DispChannelPio *PRIVATE_FIELD(DispChannelPio_NVCC7A_CURSOR_IMM_CHANNEL_PIO);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC67B_WINDOW_IMM_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC67D_CORE_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC77F_ANY_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC67E_WINDOW_CHANNEL_DMA);
    struct DispCapabilities *PRIVATE_FIELD(DispCapabilities_NVC673_DISP_CAPABILITIES);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC97B_WINDOW_IMM_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC97D_CORE_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVC97E_WINDOW_CHANNEL_DMA);
    struct DispCapabilities *PRIVATE_FIELD(DispCapabilities_NVC973_DISP_CAPABILITIES);
    struct DispCapabilities *PRIVATE_FIELD(DispCapabilities_NVCC73_DISP_CAPABILITIES);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVCC7B_WINDOW_IMM_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVCC7D_CORE_CHANNEL_DMA);
    struct DispChannelDma *PRIVATE_FIELD(DispChannelDma_NVCC7E_WINDOW_CHANNEL_DMA);
    struct ContextDma *PRIVATE_FIELD(ContextDma_NV01_CONTEXT_DMA);
    struct EventApi *PRIVATE_FIELD(EventApi_NV01_EVENT);
    struct EventApi *PRIVATE_FIELD(EventApi_NV01_EVENT_OS_EVENT);
    struct EventApi *PRIVATE_FIELD(EventApi_NV01_EVENT_KERNEL_CALLBACK);
    struct EventApi *PRIVATE_FIELD(EventApi_NV01_EVENT_KERNEL_CALLBACK_EX);
    struct LockStressObject *PRIVATE_FIELD(LockStressObject_LOCK_STRESS_OBJECT);
    struct LockTestRelaxedDupObject *PRIVATE_FIELD(LockTestRelaxedDupObject_LOCK_TEST_RELAXED_DUP_OBJECT);
};


// Metadata with per-class RTTI
struct NVOC_METADATA__NVOCFwdDeclHack {
    const struct NVOC_RTTI rtti;
};

#ifndef __nvoc_class_id_NVOCFwdDeclHack
#define __nvoc_class_id_NVOCFwdDeclHack 0x0d01f5u
typedef struct NVOCFwdDeclHack NVOCFwdDeclHack;
#endif /* __nvoc_class_id_NVOCFwdDeclHack */

// Casting support
extern const struct NVOC_CLASS_DEF __nvoc_class_def_NVOCFwdDeclHack;

#define __staticCast_NVOCFwdDeclHack(pThis) \
    ((pThis)->__nvoc_pbase_NVOCFwdDeclHack)

#ifdef __nvoc_resource_fwd_decls_h_disabled
#define __dynamicCast_NVOCFwdDeclHack(pThis) ((NVOCFwdDeclHack*) NULL)
#else //__nvoc_resource_fwd_decls_h_disabled
#define __dynamicCast_NVOCFwdDeclHack(pThis) \
    ((NVOCFwdDeclHack*) __nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(NVOCFwdDeclHack)))
#endif //__nvoc_resource_fwd_decls_h_disabled

NV_STATUS __nvoc_objCreateDynamic_NVOCFwdDeclHack(NVOCFwdDeclHack**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_NVOCFwdDeclHack(NVOCFwdDeclHack**, Dynamic*, NvU32);
#define __objCreate_NVOCFwdDeclHack(__nvoc_ppNewObj, __nvoc_pParent, __nvoc_createFlags) \
    __nvoc_objCreate_NVOCFwdDeclHack((__nvoc_ppNewObj), staticCast((__nvoc_pParent), Dynamic), (__nvoc_createFlags))

// Virtual method declarations and/or inline definitions
// Exported method declarations and/or inline definitions
// HAL method declarations without bodies
// Inline HAL method definitions
// Static dispatch method declarations
// Static inline method definitions
#undef PRIVATE_FIELD


#endif // RESOURCE_FWD_DECLS_H


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _G_RESOURCE_FWD_DECLS_NVOC_H_
