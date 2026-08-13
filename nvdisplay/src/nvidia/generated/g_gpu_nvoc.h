
#ifndef _G_GPU_NVOC_H_
#define _G_GPU_NVOC_H_

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
 * SPDX-FileCopyrightText: Copyright (c) 2004-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "g_gpu_nvoc.h"

#ifndef _OBJGPU_H_
#define _OBJGPU_H_

/*!
 * @file
 * @brief Resource Manager Defines and Structures: Defines and structures used for the GPU Object.
 */

/*!
 *
 * Forward declaration of SEQSCRIPT - here because it is used by many clients
 * and we don't want objseq.h to have to be included everywhere, so adding this
 * here.  See NVCR 12827752
 *
 */
typedef struct _SEQSCRIPT    SEQSCRIPT, *PSEQSCRIPT;

typedef struct GPUATTACHARG GPUATTACHARG;

/*
 * WARNING -- Avoid including headers in gpu.h
 *   A change in gpu.h and headers included by gpu.h triggers recompilation of most RM
 *   files in an incremental build.  We should keep the list of included header as short as
 *   possible.
 *   Especially, GPU's child module should not have its object header being included here.
 *   A child module generally includes the header of its parent. A child module header included
 *   by the parent module affects all the sibling modules.
 * */
#include "nvos.h"
#include "ctrl/ctrl0000/ctrl0000system.h"
#include "ctrl/ctrl2080/ctrl2080internal.h" // NV2080_CTRL_CMD_INTERNAL_MAX_BSPS/NVENCS
#include "ctrl/ctrl2080/ctrl2080bus.h" // NV2080_CTRL_BUS_INFO
#include "class/cl2080.h"

#include "nvlimits.h"
#include "utils/nv_enum.h"

#include "gpu/gpu_arch.h"
#include "gpu/gpu_timeout.h"
#include "gpu/gpu_access.h"
#include "gpu/gpu_shared_data_map.h"
#include "gpu/kern_gpu_power.h"

#include "platform/acpi_common.h"
#include "gpu/gpu_acpi_data.h"
#include "core/core.h"
#include "core/hal.h"
#include "core/system.h"
#include "diagnostics/traceable.h"
#include "gpu/gpu_halspec.h"
#include "gpu/gpu_resource_desc.h"
#include "gpu/gpu_uuid.h"
#include "gpu/mem_mgr/mem_desc.h"
#include "kernel/gpu/error_cont.h"
#include "kernel/gpu/gpu_engine_type.h"
#include "nvoc/utility.h"
#include "libraries/containers/vector.h"

#include "class/cl00de.h"

#include "rmapi/control.h"
#include "rmapi/event_api.h"
#include "rmapi/rmapi.h"

#include "published/nv_arch.h"

#include "g_rmconfig_util.h"      // prototypes for rmconfig utility functions, eg: rmcfg_IsGK104()

// TODO - the forward declaration of OS_GPU_INFO should be simplified
typedef struct nv_state_t OS_GPU_INFO;

struct OBJGMMU;

#ifndef __nvoc_class_id_OBJGMMU
#define __nvoc_class_id_OBJGMMU 0xd7a41du
typedef struct OBJGMMU OBJGMMU;
#endif /* __nvoc_class_id_OBJGMMU */


struct OBJGRIDDISPLAYLESS;

#ifndef __nvoc_class_id_OBJGRIDDISPLAYLESS
#define __nvoc_class_id_OBJGRIDDISPLAYLESS 0x20fd5au
typedef struct OBJGRIDDISPLAYLESS OBJGRIDDISPLAYLESS;
#endif /* __nvoc_class_id_OBJGRIDDISPLAYLESS */


struct OBJHOSTENG;

#ifndef __nvoc_class_id_OBJHOSTENG
#define __nvoc_class_id_OBJHOSTENG 0xb356e7u
typedef struct OBJHOSTENG OBJHOSTENG;
#endif /* __nvoc_class_id_OBJHOSTENG */


struct OBJPMU_CLIENT_IMPLEMENTER;

#ifndef __nvoc_class_id_OBJPMU_CLIENT_IMPLEMENTER
#define __nvoc_class_id_OBJPMU_CLIENT_IMPLEMENTER 0x88caceu
typedef struct OBJPMU_CLIENT_IMPLEMENTER OBJPMU_CLIENT_IMPLEMENTER;
#endif /* __nvoc_class_id_OBJPMU_CLIENT_IMPLEMENTER */


struct OBJINTRABLE;

#ifndef __nvoc_class_id_OBJINTRABLE
#define __nvoc_class_id_OBJINTRABLE 0x31ccb7u
typedef struct OBJINTRABLE OBJINTRABLE;
#endif /* __nvoc_class_id_OBJINTRABLE */


struct OBJVBIOS;

#ifndef __nvoc_class_id_OBJVBIOS
#define __nvoc_class_id_OBJVBIOS 0x5dc772u
typedef struct OBJVBIOS OBJVBIOS;
#endif /* __nvoc_class_id_OBJVBIOS */


struct NvDebugDump;

#ifndef __nvoc_class_id_NvDebugDump
#define __nvoc_class_id_NvDebugDump 0x7e80a2u
typedef struct NvDebugDump NvDebugDump;
#endif /* __nvoc_class_id_NvDebugDump */


struct GpuMutexMgr;

#ifndef __nvoc_class_id_GpuMutexMgr
#define __nvoc_class_id_GpuMutexMgr 0x9d93b2u
typedef struct GpuMutexMgr GpuMutexMgr;
#endif /* __nvoc_class_id_GpuMutexMgr */


struct KernelFalcon;

#ifndef __nvoc_class_id_KernelFalcon
#define __nvoc_class_id_KernelFalcon 0xb6b1afu
typedef struct KernelFalcon KernelFalcon;
#endif /* __nvoc_class_id_KernelFalcon */


struct KernelVideoEngine;

#ifndef __nvoc_class_id_KernelVideoEngine
#define __nvoc_class_id_KernelVideoEngine 0x9e2f3eu
typedef struct KernelVideoEngine KernelVideoEngine;
#endif /* __nvoc_class_id_KernelVideoEngine */


struct KernelChannel;

#ifndef __nvoc_class_id_KernelChannel
#define __nvoc_class_id_KernelChannel 0x5d8d70u
typedef struct KernelChannel KernelChannel;
#endif /* __nvoc_class_id_KernelChannel */


struct GenericKernelFalcon;

#ifndef __nvoc_class_id_GenericKernelFalcon
#define __nvoc_class_id_GenericKernelFalcon 0xabcf08u
typedef struct GenericKernelFalcon GenericKernelFalcon;
#endif /* __nvoc_class_id_GenericKernelFalcon */



struct Subdevice;

#ifndef __nvoc_class_id_Subdevice
#define __nvoc_class_id_Subdevice 0x4b01b3u
typedef struct Subdevice Subdevice;
#endif /* __nvoc_class_id_Subdevice */


struct Device;

#ifndef __nvoc_class_id_Device
#define __nvoc_class_id_Device 0xe0ac20u
typedef struct Device Device;
#endif /* __nvoc_class_id_Device */


struct RsClient;

#ifndef __nvoc_class_id_RsClient
#define __nvoc_class_id_RsClient 0x8f87e5u
typedef struct RsClient RsClient;
#endif /* __nvoc_class_id_RsClient */


struct Memory;

#ifndef __nvoc_class_id_Memory
#define __nvoc_class_id_Memory 0x4789f2u
typedef struct Memory Memory;
#endif /* __nvoc_class_id_Memory */



#ifndef PARTITIONID_INVALID
#define PARTITIONID_INVALID 0xFFFFFFFF
#endif
typedef struct MIG_INSTANCE_REF MIG_INSTANCE_REF;
typedef struct NV2080_CTRL_GPU_REG_OP NV2080_CTRL_GPU_REG_OP;
typedef struct NV2080_CTRL_BIOS_GET_SKU_INFO_PARAMS
    NV2080_CTRL_BIOS_GET_SKU_INFO_PARAMS;
typedef struct NV0080_CTRL_GPU_GET_SRIOV_CAPS_PARAMS
    NV0080_CTRL_GPU_GET_SRIOV_CAPS_PARAMS;
typedef struct NV0080_CTRL_GPU_SET_VGPU_VF_BAR1_SIZE_PARAMS
    NV0080_CTRL_GPU_SET_VGPU_VF_BAR1_SIZE_PARAMS;

typedef enum
{
    BRANDING_TYPE_NONE,
    BRANDING_TYPE_QUADRO_GENERIC,
    BRANDING_TYPE_QUADRO_AD,
    BRANDING_TYPE_NVS_NVIDIA, // "NVIDIA NVS"
    BRANDING_TYPE_VGX,
} BRANDING_TYPE;

typedef enum
{
    COMPUTE_BRANDING_TYPE_NONE,
    COMPUTE_BRANDING_TYPE_TESLA,
} COMPUTE_BRANDING_TYPE;

//
// Enum for type of Scal to retrieve in PTOP
//
typedef enum
{
    SCAL_TYPE_FBPS,
    SCAL_TYPE_TPC_PER_GPC,
    SCAL_TYPE_GPCS,
} SCAL_TYPE;

#define OOR_ARCH_DEF(x)                            \
    NV_ENUM_ENTRY(x, OOR_ARCH_X86_64,  0x00000000) \
    NV_ENUM_ENTRY(x, OOR_ARCH_PPC64LE, 0x00000001) \
    NV_ENUM_ENTRY(x, OOR_ARCH_ARM,     0x00000002) \
    NV_ENUM_ENTRY(x, OOR_ARCH_AARCH64, 0x00000003) \
    NV_ENUM_ENTRY(x, OOR_ARCH_NONE,    0x00000004)

NV_ENUM_DEF(OOR_ARCH, OOR_ARCH_DEF)

typedef struct
{
    NvU32 classId;
    NvU32 flags;
} GPUCHILDORDER;

typedef struct
{
    NvU32 classId;
    NvU32 instances;

    /*!
     * Pointer to the @ref NVOC_CLASS_INFO for the concrete class to instantiate
     * for this child.
     */
    const NVOC_CLASS_INFO *pClassInfo;
} GPUCHILDPRESENT;

/*!
 * @brief   Generates an entry for a list of @ref GPUCHILDPRESENT objects for a
 *          class of the given name
 *
 * @param[in]   _childClassName
 *  Name of the class for the entry
 * @param[in]   _instances
 *  Number of instances of the child that may be present; see
 *  @ref GPUCHILDPRESENT::instances
 *
 * @return  An entry suitable for a list of @ref GPUCHILDPRESENT for the given
 *          child of @ref OBJGPU
 */
#define GPU_CHILD_PRESENT(_childClassName, _instances) \
    GPU_CHILD_PRESENT_POLYMORPHIC(_childClassName, (_instances), _childClassName)

/*!
 * @brief   Generates an entry for a list of @ref GPUCHILDPRESENT objects that
 *          allows the @ref OBJGPU child to instantiate a sub-class of the base
 *          @ref OBJGPU child class.
 *
 * @details The intention of this macro is to allow a list of
 *          @ref GPUCHILDPRESENT to essentially state "this child should be
 *          present with this concrete class type". This allows for different
 *          @ref GPUCHILDPRESENT lists to request different classes with
 *          different behavior via sub-classes, for the same basic @ref OBJGPU
 *          child.
 *
 * @param[in]   _childClassName
 *  Name of the base class at which @ref OBJGPU points
 * @param[in]   _instances
 *  Number of instances of the child that may be present; see
 *  @ref GPUCHILDPRESENT::instances
 * @param[in]   _concreteClassName
 *   Name of the sub-class of _childClassName that should actually be
 *   instantiated
 *
 * @return  An entry suitable for a list of @ref GPUCHILDPRESENT for the given
 *          child of @ref OBJGPU with the given concrete class type.
 */
#define GPU_CHILD_PRESENT_POLYMORPHIC(_childClassName, _instances, _concreteClassName) \
    { \
        .classId = classId(_childClassName), \
        .instances = (_instances), \
        .pClassInfo = classInfo(_concreteClassName) \
    }

// GPU Child Order Flags
#define GCO_LIST_INIT            NVBIT(0)    // entry is used for init ordering (DO NOT USE)
#define GCO_LIST_LOAD            NVBIT(1)    // entry is used for load and postload ordering (DO NOT USE)
#define GCO_LIST_UNLOAD          NVBIT(2)    // entry is used for unload and preunload ordering (DO NOT USE)
#define GCO_LIST_DESTROY         NVBIT(3)    // entry is used for destroy order (DO NOT USE)
#define GCO_LIST_ALL             (GCO_LIST_INIT | GCO_LIST_LOAD | GCO_LIST_UNLOAD | GCO_LIST_DESTROY)
                                           // ^ entry is used for all list types (RECOMMENDED)
#define GCO_ALL                  (GCO_LIST_ALL)


typedef struct
{
    NvU32 childIndex;
} GPU_CHILD_ITER;

//
// Object 'get' macros for GPU relative object retrievals.
//

OBJGPU *gpuEngineGetGpu(struct Object *pObject);
// OBJGPU should be an ancestor of the param object, otherwise this will return NULL
#define ENG_GET_GPU(p)                  gpuEngineGetGpu(staticCast(p,Object))

// GPU_GET_FIFO_UC  is autogenerated, returns per Gpu pFifo.
#define GPU_GET_FIFO(p)                 GPU_GET_FIFO_UC(p)

// GPU_GET_KERNEL_FIFO_UC  is autogenerated, returns per Gpu pKernelFifo.
#define GPU_GET_KERNEL_FIFO(p)          gpuGetKernelFifoShared(p)

// gpuGetOneDeviceEntry is defined differently depending on whether PHYSICAL_RM is enabled
#define GET_DEVICE_ENTRY(gpu, type, instance, global, local, entry) \
    gpuGetOneDeviceEntry(gpu, type, instance, global, local, entry)

#define GPU_GET_HEAP(p)                 (RMCFG_MODULE_HEAP ? MEMORY_MANAGER_GET_HEAP(GPU_GET_MEMORY_MANAGER(p)) : NULL)

#define GPU_GET_HAL(p)                  (RMCFG_MODULE_HAL ? (p)->pHal : NULL)

#define GPU_GET_OS(p)                   (RMCFG_MODULE_OS ? (p)->pOS : NULL)     // TBD: replace with SYS_GET_OS
#define GPU_QUICK_PATH_GET_OS(p)        GPU_GET_OS(p)  // TBD: remove

#define GPU_GET_REGISTER_ACCESS(g)      (&(g)->registerAccess)

// Returns the pRmApi that routes to the physical driver, either via RPC or local calls
#define GPU_GET_PHYSICAL_RMAPI(g)       (&(g)->physicalRmApi)

//
// Defines and helpers for encoding and decoding PCI domain, bus and device.
//
// Ideally these would live in objbus.h (or somewhere else more appropriate) and
// not gpu/gpu.h, but keep them here for now while support for 32-bit domains is
// being added as part of bug 1904645.
//

// DRF macros for GPUBUSINFO::nvDomainBusDeviceFunc
#define NVGPU_BUSDEVICE_DOMAIN     63:32
#define NVGPU_BUSDEVICE_BUS        15:8
#define NVGPU_BUSDEVICE_DEVICE      7:0

static NV_INLINE NvU32 gpuDecodeDomain(NvU64 gpuDomainBusDevice)
{
    return (NvU32)DRF_VAL64(GPU, _BUSDEVICE, _DOMAIN, gpuDomainBusDevice);
}

static NV_INLINE NvU8 gpuDecodeBus(NvU64 gpuDomainBusDevice)
{
    return (NvU8)DRF_VAL64(GPU, _BUSDEVICE, _BUS, gpuDomainBusDevice);
}

static NV_INLINE NvU8 gpuDecodeDevice(NvU64 gpuDomainBusDevice)
{
    return (NvU8)DRF_VAL64(GPU, _BUSDEVICE, _DEVICE, gpuDomainBusDevice);
}

static NV_INLINE NvU64 gpuEncodeDomainBusDevice(NvU32 domain, NvU8 bus, NvU8 device)
{
    return DRF_NUM64(GPU, _BUSDEVICE, _DOMAIN, domain) |
           DRF_NUM64(GPU, _BUSDEVICE, _BUS, bus) |
           DRF_NUM64(GPU, _BUSDEVICE, _DEVICE, device);
}

static NV_INLINE NvU32 gpuEncodeBusDevice(NvU8 bus, NvU8 device)
{
    NvU64 busDevice = gpuEncodeDomainBusDevice(0, bus, device);

    // Bus and device are guaranteed to fit in the lower 32bits
    return (NvU32)busDevice;
}

#define GPU_LOG_AND_NOTIFY_INFOROM_XID_WITHOUT_INFO(pGpu, xidSuffix, ...)      \
do {                                                                           \
    nvErrorLog_va((void *)pGpu, INFOROM_##xidSuffix, __VA_ARGS__);             \
    gpuNotifySubDeviceEvent(pGpu, NV2080_NOTIFIERS_##xidSuffix, NULL, 0, 0, 0);\
} while (0)

//
// Generate a 32-bit id from domain, bus and device tuple.
//
NvU32 gpuGenerate32BitId(NvU32 domain, NvU8 bus, NvU8 device);

//
// Generate a 32-bit id from a physical address
//
NvU32 gpuGenerate32BitIdFromPhysAddr(RmPhysAddr addr);

//
// Helpers for getting domain, bus and device of a GPU
//
// Ideally these would be inline functions, but NVOC doesn't support that today,
// tracked in bug 1905882
//
#define gpuGetDBDF(pGpu) ((pGpu)->busInfo.nvDomainBusDeviceFunc)
#define gpuGetDomain(pGpu) gpuDecodeDomain((pGpu)->busInfo.nvDomainBusDeviceFunc)
#define gpuGetBus(pGpu)    gpuDecodeBus((pGpu)->busInfo.nvDomainBusDeviceFunc)
#define gpuGetDevice(pGpu) gpuDecodeDevice((pGpu)->busInfo.nvDomainBusDeviceFunc)
#define gpuIsDBDFValid(pGpu) ((pGpu)->busInfo.bNvDomainBusDeviceFuncValid)


#undef NVGPU_BUSDEVICE_DOMAIN
#undef NVGPU_BUSDEVICE_BUS
#undef NVGPU_BUSDEVICE_DEVICE

//
// One extra nibble should be added to the architecture version read from the
// PMC boot register to represent the architecture number in RM.
//
#define GPU_ARCH_SHIFT                  0x4

// Registry key for inst mem modification defines
#define INSTMEM_TAG_MASK    (0xf0000000)
#define INSTMEM_TAG(a)      ((INSTMEM_TAG_MASK & (a)) >> 28)


typedef struct
{

    NvU32                 PCIDeviceID;
    NvU32                 Manufacturer;
    NvU32                 PCISubDeviceID;
    NvU32                 PCIRevisionID;
    NvU32                 Subrevision;

} GPUIDINFO;


typedef struct
{
    NvU32                 impl;
    NvU32                 arch;
    NvU32                 majorRev;
    NvU32                 minorRev;
    NvU32                 minorExtRev;
} PMCBOOT0;

typedef struct
{
    NvU32                 impl;
    NvU32                 arch;
    NvU32                 majorRev;
    NvU32                 minorRev;
    NvU32                 minorExtRev;
} PMCBOOT42;

//
// Random collection of bus-related configuration state.
//
typedef struct
{
    RmPhysAddr            gpuPhysAddr;
    RmPhysAddr            gpuPhysFbAddr;
    RmPhysAddr            gpuPhysInstAddr;
    RmPhysAddr            gpuPhysIoAddr;
    NvU32                 iovaspaceId;
    NvU32                 IntLine;
    NvU32                 IsrHooked;
    NvU64                 nvDomainBusDeviceFunc;
    NvBool                bNvDomainBusDeviceFuncValid;
    OOR_ARCH              oorArch;
} GPUBUSINFO;

typedef struct
{
    CLASSDESCRIPTOR    *pClasses;
    NvU32              *pSuppressClasses;
    NvU32               numClasses;
    NvBool              bSuppressRead;
} GpuClassDb;

typedef struct
{
    CLASSDESCRIPTOR *pClassDescriptors;
    NvU32                  numClassDescriptors;

    ENGDESCRIPTOR         *pEngineInitDescriptors;
    ENGDESCRIPTOR         *pEngineDestroyDescriptors;
    ENGDESCRIPTOR         *pEngineLoadDescriptors;
    ENGDESCRIPTOR         *pEngineUnloadDescriptors;
    NvU32                  numEngineDescriptors;
} GpuEngineOrder;

//
// PCI Express Support
//
typedef struct NBADDR
{
    NvU32  domain;
    NvU8   bus;
    NvU8   device;
    NvU8   func;
    NvU8   valid;
    void  *handle;
} NBADDR;

typedef struct
{
    NBADDR  addr;
    void   *vAddr;              // virtual address of the port, if it has been mapped . Not used starting with Win10 BuildXXXXX
    NvU32   PCIECapPtr;         // offset of the PCIE capptr in the NB
    // Capability register set in enhanced configuration space
    //
    NvU32   PCIEErrorCapPtr;    // offset of the Advanced Error Reporting Capability register set
    NvU32   PCIEVCCapPtr;       // offset of the Virtual Channel (VC) Capability register set
    NvU32   PCIEL1SsCapPtr;     // Offset of the L1 Substates Capabilities
    NvU32   PCIEAcsCapPtr;      // Offset of the ACS redirect Capabilities
    NvU16   DeviceID, VendorID; // device and vendor ID for port
} PORTDATA;

typedef struct // GPU specific data for core logic object, stored in GPU object
{
    PORTDATA  upstreamPort;     // the upstream port info for the GPU
                                // If there is a switch this is equal to boardDownstreamPort
                                // If there is no switch this is equal to rootPort
    PORTDATA  rootPort;         // The root port of the PCI-E root complex
    PORTDATA  boardUpstreamPort;    // If there is no BR03 this is equal to rootPort.
    PORTDATA  boardDownstreamPort;  // If there is no BR03 these data are not set.
} GPUCLDATA;


//
// Flags for gpuStateLoad() and gpuStateUnload() routines. Flags *must* be used
// symmetrically across an Unload/Load pair.
//
#define GPU_STATE_FLAGS_PRESERVING         NVBIT(0)  // GPU state is preserved
#define GPU_STATE_FLAGS_VGA_TRANSITION     NVBIT(1)   // To be used with GPU_STATE_FLAGS_PRESERVING.
#define GPU_STATE_FLAGS_PM_TRANSITION      NVBIT(2)   // To be used with GPU_STATE_FLAGS_PRESERVING.
#define GPU_STATE_FLAGS_PM_SUSPEND         NVBIT(3)
#define GPU_STATE_FLAGS_PM_HIBERNATE       NVBIT(4)
#define GPU_STATE_FLAGS_GC6_TRANSITION     NVBIT(5)  // To be used with GPU_STATE_FLAGS_PRESERVING.
#define GPU_STATE_FLAGS_FAST_UNLOAD        NVBIT(6)  // Used during windows restart, skips stateDestroy steps
#define GPU_STATE_FLAGS_FORCE_GSP_UNLOAD   NVBIT(7)  // Force GSP to unload
#define GPU_STATE_DEFAULT                  0       // Default flags for destructive state loads
                                                   // and unloads

struct OBJHWBC;
typedef struct hwbc_list
{
    struct OBJHWBC *pHWBC;
    struct hwbc_list *pNext;
} HWBC_LIST;

/*!
 * GFID allocation state
 */
typedef enum
{
    GFID_FREE = 0,
    GFID_ALLOCATED = 1,
    GFID_INVALIDATED = 2,
} GFID_ALLOC_STATUS;

typedef struct SRIOV_P2P_INFO
{
    NvU32    gfid;
    NvBool   bAllowP2pAccess;
    NvU32    accessRefCount;
    NvU32    destRefCount;
} SRIOV_P2P_INFO, *PSRIOV_P2P_INFO;

//
// typedef of private struct used in OBJGPU's data field
//

typedef struct
{
    NvBool              isInitialized;
    NvU8                uuid[RM_SHA1_GID_SIZE];
} _GPU_UUID;

typedef struct
{
    NvBool              bValid;
    NvU8                id;
} _GPU_PCIE_PEER_CLIQUE;

typedef struct
{
    NvU32     platformId;         // used to identify soc
    NvU32     implementationId;   // soc-specific
    NvU32     revisionId;         // soc-revision
    NvU32     chipId;             // platform (architecture) + implementation
    PMCBOOT0  pmcBoot0;
    PMCBOOT42 pmcBoot42;
    NvU8      subRevision;        // sub-revision (NV_FUSE_OPT_SUBREVISION on GPU)
} _GPU_CHIP_INFO;


// Engine Database
typedef struct
{
    NvU32 size;
    RM_ENGINE_TYPE *pType;
    NvBool bValid;
} _GPU_ENGINE_DB;

#define MAX_NUM_BARS      (8)
// SRIOV state
typedef struct
{
    /*!
     * Total number of VFs available in this GPU
     */
    NvU32           totalVFs;

    /*!
     * First VF Offset
     */
    NvU32           firstVFOffset;

    /*!
     * Max GFID possible
     */
    NvU32           maxGfid;

    /*!
     *  Physical offset of Virtual BAR0 register. Stores the offset if the GPU is
     *  a physical function, else 0
     */
    NvU32           virtualRegPhysOffset;

    /*!
     * Allocated GFIDs. Will be used to ensure plugins doesn't use same GFID for multiple VFs
     */
    NvU8            *pAllocatedGfids;

    /*!
     * The sizes of the BAR regions on the VF
     */
    NvU64 vfBarSize[MAX_NUM_BARS];

    /*!
     * First PF's BAR addresses
     */
    NvU64 firstVFBarAddress[MAX_NUM_BARS];

    /*!
     * If the VF BARs are 64-bit addressable
     */
    NvBool b64bitVFBar0;
    NvBool b64bitVFBar1;
    NvBool b64bitVFBar2;

    /*!
     * GFID used for P2P access
     */
    PSRIOV_P2P_INFO pP2PInfo;
    NvBool          bP2PAllocated;
    NvU32           maxP2pGfid;
    NvU32           p2pFabricPartitionId;
} _GPU_SRIOV_STATE;

// Max # of instances for GPU children
#define GPU_MAX_CES                     20
#define GPU_MAX_GRS                     8
#define GPU_MAX_FIFOS                   1
#define GPU_MAX_MSENCS                  NV2080_CTRL_CMD_INTERNAL_MAX_MSENCS
#define GPU_MAX_NVDECS                  NV2080_CTRL_CMD_INTERNAL_MAX_BSPS
#define GPU_MAX_NVJPGS                  8
#define GPU_MAX_HSHUBS                  12
#define GPU_MAX_OFAS                    2
#define GPU_MAX_GSPLITES                4
#define GPU_MAX_LPSECS                  8
//
// Macro defines for OBJGPU fields -- Macro defines inside NVOC class block is
// gone after NVOC preprocessing stage.  For macros used outside gpu/gpu.h should
// not be defined inside the class block.
//

//
// Maximum number of Falcon objects that can be allocated on one GPU.
// This is purely a software limit and can be raised freely as more are added.
//
#define GPU_MAX_FALCON_ENGINES \
    ENG_IOCTRL__SIZE_1       + \
    ENG_GPCCS__SIZE_1        + \
    ENG_FECS__SIZE_1         + \
    ENG_NVJPEG__SIZE_1       + \
    ENG_NVDEC__SIZE_1        + \
    ENG_NVENC__SIZE_1        + \
    32

#define GPU_MAX_VIDEO_ENGINES  \
    (ENG_NVJPEG__SIZE_1      + \
     ENG_NVDEC__SIZE_1       + \
     ENG_NVENC__SIZE_1       + \
     ENG_OFA__SIZE_1)

// for OBJGPU::pRmCtrlDeferredCmd
#define MAX_DEFERRED_CMDS 2

// for OBJGPU::computeModeRefCount
#define NV_GPU_MODE_GRAPHICS_MODE                   0x00000001
#define NV_GPU_MODE_COMPUTE_MODE                    0x00000002
#define NV_GPU_COMPUTE_REFCOUNT_COMMAND_INCREMENT   0x0000000a
#define NV_GPU_COMPUTE_REFCOUNT_COMMAND_DECREMENT   0x0000000b

//
// Structure to hold information obtained from
// parsing the DEVICE_INFO2 table during init.
//
typedef struct
{
    NvU32 faultId;
    NvU32 instanceId;
    NvU32 typeEnum;
    NvU32 resetId;
    NvU32 devicePriBase;
    NvU32 isEngine;
    NvU32 rlEngId;
    NvU32 runlistPriBase;
    NvU32 groupId;
    NvU32 ginTargetId;
    NvU32 deviceBroadcastPriBase;
    NvU32 groupLocalInstanceId;
} DEVICE_INFO_ENTRY;

MAKE_VECTOR(DeviceInfoEntryVec, DEVICE_INFO_ENTRY);

#define DEVICE_INFO_INSTANCE_ID_ANY (-1)
#define DEVICE_INFO_GLOBAL_INSTANCE_ID_ANY    DEVICE_INFO_INSTANCE_ID_ANY
#define DEVICE_INFO_DIE_LOCAL_INSTANCE_ID_ANY DEVICE_INFO_INSTANCE_ID_ANY

#define DEVICE_INFO_DIELET_INSTANCE_ANY (-1)
typedef struct
{
    DEVICE_INFO_ENTRY *pEntry;
    NvU32              devTypeEnum;
    NvU32              dieletIdMask;
} DeviceInfoIter;

#define MAX_GROUP_COUNT 2

#define NV_GPU_INTERNAL_DEVICE_HANDLE    0xABCD0080
#define NV_GPU_INTERNAL_SUBDEVICE_HANDLE 0xABCD2080

//
// NV GPU simulation mode defines
// Keep in sync with os.h SIM MODE defines until osGetSimulationMode is deprecated.
//
#ifndef NV_SIM_MODE_DEFS
#define NV_SIM_MODE_DEFS
#define NV_SIM_MODE_HARDWARE            0U
#define NV_SIM_MODE_RTL                 1U
#define NV_SIM_MODE_CMODEL              2U
#define NV_SIM_MODE_MODS_AMODEL         3U
#define NV_SIM_MODE_TEGRA_FPGA          4U
#define NV_SIM_MODE_INVALID         (~0x0U)
#endif

#define GPU_IS_NVSWITCH_DETECTED(pGpu) \
    (pGpu->nvswitchSupport == NV2080_CTRL_PMGR_MODULE_INFO_NVSWITCH_SUPPORTED)


typedef struct GspTraceBuffer GspTraceBuffer;

typedef enum
{
    SCHED_POLICY_DEFAULT = 0,
    SCHED_POLICY_BEST_EFFORT = 1,
    SCHED_POLICY_VGPU_EQUAL_SHARE = 2,
    SCHED_POLICY_VGPU_FIXED_SHARE = 3,
} SCHED_POLICY;

// Scratch bits
#define NV_PBUS_SW_RESET_BITS_SCRATCH_REG   30

typedef enum
{
    NV_PM_DEPTH_NONE = 0,
    NV_PM_DEPTH_OS_LAYER = 1,
    NV_PM_DEPTH_SR_META = 2,
    NV_PM_DEPTH_STATE_LOAD = 3,
} NV_PM_DEPTH;

// Recovery action init value
#define GPU_RECOVERY_ACTION_UNKNOWN         0xff

typedef enum
{
    // NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_PROVIDER_TYPE_OPCODE
    THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_TYPE = 0,
    // NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_TARGET_TYPE_OPCODE
    THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_TYPE = 1,
    // NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_READING_RANGE_OPCODE
    THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MIN_TEMP = 2,
    // NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_READING_RANGE_OPCODE
    THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MAX_TEMP = 3,
    // NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_PROVIDER_OPCODE
    THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_INDEX = 4,
    // NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_TARGET_OPCODE
    THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_INDEX = 5,

    // Used to determine number of enum entries. Should always be last.
    THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_LAST = 6
} THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY;

//
// The actual GPU object definition
//

// Private field names are wrapped in PRIVATE_FIELD, which does nothing for
// the matching C source file, but causes diagnostics to be issued if another
// source file references the field.
#ifdef NVOC_GPU_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif


// Metadata with per-class RTTI with ancestor(s)
struct NVOC_METADATA__OBJGPU;
struct NVOC_METADATA__Object;
struct NVOC_METADATA__GpuHalspecOwner;
struct NVOC_METADATA__RmHalspecOwner;
struct NVOC_METADATA__OBJTRACEABLE;

union __nvoc_inner_struc_OBJGPU_1__ {
    struct {
        struct MemoryManager *pMemoryManager;
        struct KernelDisplay *pKernelDisplay;
        struct OBJTMR *pTmr;
        struct OBJDCECLIENTRM *pDceclientrm;
    } named;
    Dynamic *pChild[4];
};

struct __nvoc_inner_struc_OBJGPU_2__ {
    NvU32 numSensors;
    NvBool bNumSensorsCached;
    struct {
        NvU32 cache[6];
        NvBool bIsCached[6];
    } sensors[4];
};



struct OBJGPU {

    // Metadata starts with RTTI structure.
    union {
         const struct NVOC_METADATA__OBJGPU *__nvoc_metadata_ptr;
         const struct NVOC_RTTI *__nvoc_rtti;
    };

    // Parent (i.e. superclass or base class) objects
    struct Object __nvoc_base_Object;
    struct GpuHalspecOwner __nvoc_base_GpuHalspecOwner;
    struct RmHalspecOwner __nvoc_base_RmHalspecOwner;
    struct OBJTRACEABLE __nvoc_base_OBJTRACEABLE;

    // Ancestor object pointers for `staticCast` feature
    struct Object *__nvoc_pbase_Object;    // obj super
    struct GpuHalspecOwner *__nvoc_pbase_GpuHalspecOwner;    // gpuhalspecowner super
    struct RmHalspecOwner *__nvoc_pbase_RmHalspecOwner;    // rmhalspecowner super
    struct OBJTRACEABLE *__nvoc_pbase_OBJTRACEABLE;    // traceable super
    struct OBJGPU *__nvoc_pbase_OBJGPU;    // gpu

    // Vtable with 5 per-object function pointers
    void (*__gpuGetIdInfo__)(struct OBJGPU * /*this*/);  // halified (3 hals) body
    const GPUCHILDORDER * (*__gpuGetChildrenOrder__)(struct OBJGPU * /*this*/, NvU32 *);  // halified (2 hals)
    const GPUCHILDPRESENT * (*__gpuGetChildrenPresent__)(struct OBJGPU * /*this*/, NvU32 *);  // halified (2 hals)
    const CLASSDESCRIPTOR * (*__gpuGetEngClassDescriptorList__)(struct OBJGPU * /*this*/, NvU32 *);  // halified (3 hals)
    const NvU32 * (*__gpuGetNoEngClassList__)(struct OBJGPU * /*this*/, NvU32 *);  // halified (3 hals)

    // 121 PDB properties
    NvBool PDB_PROP_GPU_IN_STANDBY;
    NvBool PDB_PROP_GPU_IN_HIBERNATE;
    NvBool PDB_PROP_GPU_IN_PM_CODEPATH;
    NvBool PDB_PROP_GPU_IN_PM_RESUME_CODEPATH;
    NvBool PDB_PROP_GPU_STATE_INITIALIZED;
    NvBool PDB_PROP_GPU_EMULATION;
    NvBool PDB_PROP_GPU_PRIMARY_DEVICE;
    NvBool PDB_PROP_GPU_HYBRID_MGPU;
    NvBool PDB_PROP_GPU_ALTERNATE_TREE_HANDLE_LOCKLESS;
    NvBool PDB_PROP_GPU_3D_CONTROLLER;
    NvBool PDB_PROP_GPU_IS_ALL_INST_IN_SYSMEM;
    NvBool PDB_PROP_GPU_IS_CONNECTED;
    NvBool PDB_PROP_GPU_BROKEN_FB;
    NvBool PDB_PROP_GPU_SOC_FRM_RESTORE_HIBERNATE_RESUME;
    NvBool PDB_PROP_GPU_IN_FULLCHIP_RESET;
    NvBool PDB_PROP_GPU_IN_SECONDARY_BUS_RESET;
    NvBool PDB_PROP_GPU_IN_GC6_RESET;
    NvBool PDB_PROP_GPU_IS_GEMINI;
    NvBool PDB_PROP_GPU_PERSISTENT_SW_STATE;
    NvBool PDB_PROP_GPU_COHERENT_CPU_MAPPING;
    NvBool PDB_PROP_GPU_IS_LOST;
    NvBool PDB_PROP_GPU_IN_TIMEOUT_RECOVERY;
    NvBool PDB_PROP_GPU_ALLOW_PAGE_RETIREMENT;
    NvBool PDB_PROP_GPU_IN_MCDM_MODE;
    NvBool PDB_PROP_GPU_KEEP_WPR_ACROSS_GC6_SUPPORTED;
    NvBool PDB_PROP_GPU_TEGRA_SOC_NVDISPLAY;
    NvBool PDB_PROP_GPU_ATS_SUPPORTED;
    NvBool PDB_PROP_GPU_SC7_SUPPORTED;
    NvBool PDB_PROP_GPU_RTD3_RG_SUPPORTED;
    NvBool PDB_PROP_GPU_TRIGGER_PCIE_FLR;
    NvBool PDB_PROP_GPU_CLKS_IN_TEGRA_SOC;
    NvBool PDB_PROP_GPU_PREINITIALIZED_WPR_REGION;
    NvBool PDB_PROP_GPU_SECONDARY_BUS_RESET_PENDING;
    NvBool PDB_PROP_GPU_IN_BUGCHECK_CALLBACK_ROUTINE;
    NvBool PDB_PROP_GPU_BUG_3007008_EMULATE_VF_MMU_TLB_INVALIDATE;
    NvBool PDB_PROP_GPU_IS_UEFI;
    NvBool PDB_PROP_GPU_IS_EFI_INIT;
    NvBool PDB_PROP_GPU_CAN_OPTIMIZE_COMPUTE_USE_CASE;
    NvBool PDB_PROP_GPU_MIG_SUPPORTED;
    NvBool PDB_PROP_GPU_MIG_MIRROR_HOST_CI_ON_GUEST;
    NvBool PDB_PROP_GPU_MIG_SUPPORTS_SPLIT_CE_RANGES;
    NvBool PDB_PROP_GPU_MIG_GFX_SUPPORTED;
    NvBool PDB_PROP_GPU_MIG_TIMESLICING_SUPPORTED;
    NvBool PDB_PROP_GPU_VC_CAPABILITY_SUPPORTED;
    NvBool PDB_PROP_GPU_RESETLESS_MIG_SUPPORTED;
    NvBool PDB_PROP_GPU_IS_COT_ENABLED;
    NvBool PDB_PROP_GPU_FW_WPR_OFFSET_SET_BY_ACR;
    NvBool PDB_PROP_GPU_TOGGLE_DYNAMIC_THROTTLE_WINDOW_SIZE_SUPPORTED;
    NvBool PDB_PROP_GPU_UNIX_HDMI_FRL_COMPLIANCE_ENABLED;
    NvBool PDB_PROP_GPU_SRIOV_SYSMEM_DIRTY_PAGE_TRACKING_ENABLED;
    NvBool PDB_PROP_GPU_VGPU_OFFLOAD_CAPABLE;
    NvBool PDB_PROP_GPU_POWER_MANAGEMENT_UNSUPPORTED;
    NvBool PDB_PROP_GPU_PVMRL_GRANULAR_LOCKING;
    NvBool PDB_PROP_GPU_IS_PLX_PRESENT;
    NvBool PDB_PROP_GPU_IS_BR03_PRESENT;
    NvBool PDB_PROP_GPU_IS_BR04_PRESENT;
    NvBool PDB_PROP_GPU_BEHIND_BRIDGE;
    NvBool PDB_PROP_GPU_BEHIND_BR03;
    NvBool PDB_PROP_GPU_BEHIND_BR04;
    NvBool PDB_PROP_GPU_UPSTREAM_PORT_L0S_UNSUPPORTED;
    NvBool PDB_PROP_GPU_UPSTREAM_PORT_L1_UNSUPPORTED;
    NvBool PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_SUPPORTED;
    NvBool PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_MOBILE_ONLY;
    NvBool PDB_PROP_GPU_RM_UNLINKED_SLI;
    NvBool PDB_PROP_GPU_SLI_LINK_ACTIVE;
    NvBool PDB_PROP_GPU_ENABLE_REG_ACCESS_IN_LOW_POWER_FOR_SIM_SRTEST;
    NvBool PDB_PROP_GPU_DO_NOT_CHECK_REG_ACCESS_IN_PM_CODEPATH;
    NvBool PDB_PROP_GPU_EXTERNAL_HEAP_CONTROL;
    NvBool PDB_PROP_GPU_IS_MOBILE;
    NvBool PDB_PROP_GPU_IS_EXTERNAL_GPU;
    NvBool PDB_PROP_GPU_RTD3_GC6_SUPPORTED;
    NvBool PDB_PROP_GPU_RTD3_GC6_ACTIVE;
    NvBool PDB_PROP_GPU_FAST_GC6_ACTIVE;
    NvBool PDB_PROP_GPU_UNIX_DYNAMIC_POWER_SUPPORTED;
    NvBool PDB_PROP_GPU_MOVE_CTX_BUFFERS_TO_PMA;
    NvBool PDB_PROP_GPU_LEGACY_GCOFF_SUPPORTED;
    NvBool PDB_PROP_GPU_RTD3_GCOFF_SUPPORTED;
    NvBool PDB_PROP_GPU_GCOFF_STATE_ENTERING;
    NvBool PDB_PROP_GPU_GCOFF_STATE_ENTERED;
    NvBool PDB_PROP_GPU_RG_STATE_ENTERING;
    NvBool PDB_PROP_GPU_RG_STATE_ENTERED;
    NvBool PDB_PROP_GPU_ACCOUNTING_ON;
    NvBool PDB_PROP_GPU_INACCESSIBLE;
    NvBool PDB_PROP_GPU_NVLINK_SYSMEM;
    NvBool PDB_PROP_GPU_SKIP_CE_MAPPINGS_NO_NVLINK;
    NvBool PDB_PROP_GPU_C2C_SYSMEM;
    NvBool PDB_PROP_GPU_IN_TCC_MODE;
    NvBool PDB_PROP_GPU_SUPPORTS_TDR_EVENT;
    NvBool PDB_PROP_GPU_MSHYBRID_GC6_ACTIVE;
    NvBool PDB_PROP_GPU_VGPU_BIG_PAGE_SIZE_64K;
    NvBool PDB_PROP_GPU_OPTIMIZE_SPARSE_TEXTURE_BY_DEFAULT;
    NvBool PDB_PROP_GPU_ENABLE_IOMMU_SUPPORT;
    NvBool PDB_PROP_GPU_IGNORE_REPLAYABLE_FAULTS;
    NvBool PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VGPU;
    NvBool PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VSGA;
    NvBool PDB_PROP_GPU_IS_VGPU_HETEROGENEOUS_MODE;
    NvBool PDB_PROP_GPU_NVLINK_P2P_LOOPBACK_DISABLED;
    NvBool PDB_PROP_GPU_NV_USERMODE_ENABLED;
    NvBool PDB_PROP_GPU_IN_FATAL_ERROR;
    NvBool PDB_PROP_GPU_OPTIMUS_GOLD_CFG_SPACE_RESTORE;
    NvBool PDB_PROP_GPU_VGA_ENABLED;
    NvBool PDB_PROP_GPU_IS_MXM_3X;
    NvBool PDB_PROP_GPU_GSYNC_III_ATTACHED;
    NvBool PDB_PROP_GPU_QSYNC_II_ATTACHED;
    NvBool PDB_PROP_GPU_CC_FEATURE_CAPABLE;
    NvBool PDB_PROP_GPU_APM_FEATURE_CAPABLE;
    NvBool PDB_PROP_GPU_SKIP_TABLE_CE_MAP;
    NvBool PDB_PROP_GPU_CHIP_SUPPORTS_RTD3_DEF;
    NvBool PDB_PROP_GPU_IS_SOC_SDM;
    NvBool PDB_PROP_GPU_DISP_PB_REQUIRES_SMMU_BYPASS;
    NvBool PDB_PROP_GPU_FORCE_PERF_BIOS_LEVEL;
    NvBool PDB_PROP_GPU_FASTPATH_SEQ_ENABLED;
    NvBool PDB_PROP_GPU_PREPARING_FULLCHIP_RESET;
    NvBool PDB_PROP_GPU_RECOVERY_DRAIN_P2P_REQUIRED;
    NvBool PDB_PROP_GPU_RECOVERY_REBOOT_REQUIRED;
    NvBool PDB_PROP_GPU_RECOVERY_SQUASH_XID154;
    NvBool PDB_PROP_GPU_REUSE_INIT_CONTING_MEM;
    NvBool PDB_PROP_GPU_RUSD_POLLING_SUPPORT_MONOLITHIC;
    NvBool PDB_PROP_GPU_RUSD_DISABLE_CLK_PUBLIC_DOMAIN_INFO;
    NvBool PDB_PROP_GPU_ALLOC_ISO_SYS_MEM_FROM_CARVEOUT;
    NvBool PDB_PROP_GPU_HFRP_IS_KERNEL_OBJECT_ACTIVE_WAR;

    // Data members
    NvU32 moduleId;
    NvU8 nvswitchSupport;
    NV2080_CTRL_GPU_RECOVERY_ACTION currentRecoveryAction;
    struct GpuArch *pGpuArch;
    OS_GPU_INFO *pOsGpuInfo;
    OS_RM_CAPS *pOsRmCaps;
    NvU32 halImpl;
    void *hPci;
    GpuEngineEventNotificationList *engineNonstallIntrEventNotifications[84];
    NvBool bIsSOC;
    NvU32 gpuInstance;
    NvU32 gpuDisabled;
    NvU32 gpuId;
    NvU32 boardId;
    NvU32 deviceInstance;
    NvU32 subdeviceInstance;
    NvS32 numaNodeId;
    NvS32 cpuNumaNodeId;
    _GPU_UUID gpuUuid;
    NvU32 gpuPhysicalId;
    NvU32 gpuTerminatedLinkMask;
    NvBool gpuLinkTerminationEnabled;
    NvBool gspRmInitialized;
    NV_PM_DEPTH powerManagementDepth;
    _GPU_PCIE_PEER_CLIQUE pciePeerClique;
    NvU16 virtualConfigBits;
    NvBool bGspNocatEnabled;
    NvU32 i2cPortForExtdev;
    GPUIDINFO idInfo;
    _GPU_CHIP_INFO chipInfo;
    GPUBUSINFO busInfo;
    const GPUCHILDPRESENT *pChildrenPresent;
    NvU32 numChildrenPresent;
    GpuEngineOrder engineOrder;
    GpuClassDb classDB;
    NvU32 chipId0;
    NvU32 chipId1;
    NvU32 pmcEnable;
    NvU32 pmcRmOwnsIntrMask;
    NvBool testIntr;
    NvU32 numCEs;
    NvU32 ceFaultMethodBufferSize;
    NvS32 lockStressCounter;
    NvBool isVirtual;
    NvBool isGspClient;
    NvBool isDceClient;
    NvU64 fbLength;
    NvU32 instLength;
    NvBool instSetViaAttachArg;
    NvU64 activeFBIOs;
    NvU64 gpuVbiosPostTime;
    NvU32 uefiScanoutSurfaceSizeInMB;
    NvU32 gpuDeviceMapCount;
    DEVICE_MAPPING deviceMappings[60];
    struct IoAperture *pIOApertures[13];
    DEVICE_MAPPING *pDeviceMappingsByDeviceInstance[13];
    void *gpuCfgAddr;
    TIMEOUT_DATA timeoutData;
    NvU32 bug5203024OverrideTimeouts;
    NvU32 computeModeRules;
    NvS32 computeModeRefCount;
    NvHandle hComputeModeReservation;
    NvBool bIsDebugModeEnabled;
    NvU64 lastCallbackTime;
    _Atomic(NvU32) bCallbackQueued;
    NvU32 sliStatus;
    NvBool bIsRTD3Gc6D3HotTransition;
    NvBool bIsRTD3GcoffD3HotTransition;
    NvU32 simMode;
    struct OBJOS *pOS;
    struct OBJHAL *pHal;
    union __nvoc_inner_struc_OBJGPU_1__ children;
    HWBC_LIST *pHWBCList;
    GPUCLDATA gpuClData;
    _GPU_ENGINE_DB engineDB;
    NvU32 engineDBSize;
    NvU32 instCacheOverride;
    NvS32 numOfMclkLockRequests;
    NvU32 netlistNum;
    RmCtrlDeferredCmd pRmCtrlDeferredCmd[2];
    ACPI_DATA acpi;
    ACPI_METHOD_DATA acpiMethodData;
    NvBool bSystemHasMux;
    NV2080_CTRL_INTERNAL_INIT_BRIGHTC_STATE_LOAD_PARAMS backLightMethodData;
    struct Falcon *constructedFalcons[71];
    NvU32 numConstructedFalcons;
    struct GenericKernelFalcon *genericKernelFalcons[71];
    NvU32 numGenericKernelFalcons;
    struct KernelVideoEngine *kernelVideoEngines[22];
    NvU32 numKernelVideoEngines;
    NvU8 *pUserRegisterAccessMap;
    NvU8 *pUnrestrictedRegisterAccessMap;
    NvU32 userRegisterAccessMapSize;
    RegisterAccess registerAccess;
    NvBool bUseRegisterAccessMap;
    NvU32 *pRegopOffsetScratchBuffer;
    NvU32 *pRegopOffsetAddrScratchBuffer;
    NvU32 regopScratchBufferMaxOffsets;
    _GPU_SRIOV_STATE sriovState;
    NvU64 vmmuSegmentSize;
    NvHandle hDefaultClientShare;
    NvHandle hDefaultClientShareDevice;
    NvHandle hDefaultClientShareSubDevice;
    NvU32 externalKernelClientCount;
    DEVICE_INFO_ENTRY *pDeviceInfoTable;
    NvU32 numDeviceInfoEntries;
    NvHandle hInternalClient;
    NvHandle hInternalDevice;
    NvHandle hInternalSubdevice;
    NvHandle hInternalLockStressClient;
    struct Subdevice *pCachedSubdevice;
    struct RsClient *pCachedRsClient;
    RM_API physicalRmApi;
    struct Subdevice **pSubdeviceBackReferences;
    NvU32 numSubdeviceBackReferences;
    NvU32 maxSubdeviceBackReferences;
    PORT_SPINLOCK *pSubdeviceBackReferencesLock;
    NV2080_CTRL_INTERNAL_GPU_GET_CHIP_INFO_PARAMS *pChipInfo;
    NV2080_CTRL_GPU_GET_OEM_BOARD_INFO_PARAMS *boardInfo;
    GpuSharedDataMap userSharedData;
    NvBool bBar2MovedByVtd;
    NvBool bBar1Is64Bit;
    NvBool bSurpriseRemovalSupported;
    NvBool bTwoStageRcRecoveryEnabled;
    NvBool bReplayableTraceEnabled;
    NvBool bInD3Cold;
    NvBool bIsSimulation;
    NvBool bIsModsAmodel;
    NvBool bIsFmodel;
    NvBool bIsRtlsim;
    NvBool bIsPassthru;
    NvBool bIsVirtualWithSriov;
    NvBool bIsMigRm;
    NvBool bIsSelfHosted;
    NvBool bStateLoading;
    NvBool bStateUnloading;
    NvBool bStateLoaded;
    NvBool bFullyConstructed;
    NvBool bRecoveryMarginPresent;
    NvBool bBf3WarBug4040336Enabled;
    NvBool bUnifiedMemorySpaceEnabled;
    NvBool bSriovEnabled;
    NvBool bWarBug200577889SriovHeavyEnabled;
    NvBool bNonPowerOf2ChannelCountSupported;
    NvBool bWarBug4347206PowerCycleOnUnload;
    NvBool bCacheOnlyMode;
    NvBool bSplitVasManagementServerClientRm;
    NvU32 instLocOverrides;
    NvU32 instLocOverrides2;
    NvU32 instLocOverrides3;
    NvU32 instLocOverrides4;
    NvBool bInstLoc47bitPaWar;
    NvU32 instVprOverrides;
    NvU32 optimizeUseCaseOverride;
    NvS16 videoCtxswLogConsumerCount;
    THREAD_STATE_NODE *pDpcThreadState;
    struct OBJVASPACE *pFabricVAS;
    NvBool bPipelinedPteMemEnabled;
    NvBool bIsBarPteInSysmemSupported;
    NvBool bRegUsesGlobalSurfaceOverrides;
    NvBool bClientRmAllocatedCtxBuffer;
    NvBool bEccPageRetirementWithSliAllowed;
    NvBool bInstanceMemoryAlwaysCached;
    NvBool bUseRpcSimEscapes;
    NvBool bRmProfilingPrivileged;
    NvBool bGeforceSmb;
    NvBool bIsGeforce;
    NvBool bIsQuadro;
    NvBool bIsQuadroAD;
    NvBool bIsVgx;
    NvBool bIsNvidiaNvs;
    NvBool bIsTitan;
    NvBool bIsTesla;
    NvBool bComputePolicyTimesliceSupported;
    RmPhysAddr simAccessBufPhysAddr;
    RmPhysAddr notifyOpSharedSurfacePhysAddr;
    NvBool bVgpuGspPluginOffloadEnabled;
    NvBool bIgnoreGpuUIDmismatch;
    NvBool bSriovCapable;
    NvBool bRecheckSliSupportAtResume;
    NvBool bGpuNvEncAv1Supported;
    NvBool bIsGspOwnedFaultBuffersEnabled;
    NvBool bVfResizableBAR1Supported;
    NvBool bVoltaHubIntrSupported;
    NvBool bUsePmcDeviceEnableForHostEngine;
    NvBool bBlockNewWorkload;
    struct __nvoc_inner_struc_OBJGPU_2__ thermalSystemExecuteV2Cache;
    NVPOWERSTATE_FAILURE powerStateFailure;
    NvBool bMemsubsysErrDetectionEnabled;
    _GPU_GC6_STATE gc6State;
    NvU32 numUserKernelChannel;
};


// Metadata with per-class RTTI with ancestor(s)
struct NVOC_METADATA__OBJGPU {
    const struct NVOC_RTTI rtti;
    const struct NVOC_METADATA__Object metadata__Object;
    const struct NVOC_METADATA__GpuHalspecOwner metadata__GpuHalspecOwner;
    const struct NVOC_METADATA__RmHalspecOwner metadata__RmHalspecOwner;
    const struct NVOC_METADATA__OBJTRACEABLE metadata__OBJTRACEABLE;
};

#ifndef __nvoc_class_id_OBJGPU
#define __nvoc_class_id_OBJGPU 0x7ef3cbu
typedef struct OBJGPU OBJGPU;
#endif /* __nvoc_class_id_OBJGPU */

// Casting support
extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJGPU;

#define __staticCast_OBJGPU(pThis) \
    ((pThis)->__nvoc_pbase_OBJGPU)

#ifdef __nvoc_gpu_h_disabled
#define __dynamicCast_OBJGPU(pThis) ((OBJGPU*) NULL)
#else //__nvoc_gpu_h_disabled
#define __dynamicCast_OBJGPU(pThis) \
    ((OBJGPU*) __nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(OBJGPU)))
#endif //__nvoc_gpu_h_disabled

// Property macros
#define PDB_PROP_GPU_IN_STANDBY_BASE_CAST
#define PDB_PROP_GPU_IN_STANDBY_BASE_NAME PDB_PROP_GPU_IN_STANDBY
#define PDB_PROP_GPU_IN_HIBERNATE_BASE_CAST
#define PDB_PROP_GPU_IN_HIBERNATE_BASE_NAME PDB_PROP_GPU_IN_HIBERNATE
#define PDB_PROP_GPU_IN_PM_CODEPATH_BASE_CAST
#define PDB_PROP_GPU_IN_PM_CODEPATH_BASE_NAME PDB_PROP_GPU_IN_PM_CODEPATH
#define PDB_PROP_GPU_IN_PM_RESUME_CODEPATH_BASE_CAST
#define PDB_PROP_GPU_IN_PM_RESUME_CODEPATH_BASE_NAME PDB_PROP_GPU_IN_PM_RESUME_CODEPATH
#define PDB_PROP_GPU_STATE_INITIALIZED_BASE_CAST
#define PDB_PROP_GPU_STATE_INITIALIZED_BASE_NAME PDB_PROP_GPU_STATE_INITIALIZED
#define PDB_PROP_GPU_EMULATION_BASE_CAST
#define PDB_PROP_GPU_EMULATION_BASE_NAME PDB_PROP_GPU_EMULATION
#define PDB_PROP_GPU_PRIMARY_DEVICE_BASE_CAST
#define PDB_PROP_GPU_PRIMARY_DEVICE_BASE_NAME PDB_PROP_GPU_PRIMARY_DEVICE
#define PDB_PROP_GPU_HYBRID_MGPU_BASE_CAST
#define PDB_PROP_GPU_HYBRID_MGPU_BASE_NAME PDB_PROP_GPU_HYBRID_MGPU
#define PDB_PROP_GPU_ALTERNATE_TREE_HANDLE_LOCKLESS_BASE_CAST
#define PDB_PROP_GPU_ALTERNATE_TREE_HANDLE_LOCKLESS_BASE_NAME PDB_PROP_GPU_ALTERNATE_TREE_HANDLE_LOCKLESS
#define PDB_PROP_GPU_3D_CONTROLLER_BASE_CAST
#define PDB_PROP_GPU_3D_CONTROLLER_BASE_NAME PDB_PROP_GPU_3D_CONTROLLER
#define PDB_PROP_GPU_IS_ALL_INST_IN_SYSMEM_BASE_CAST
#define PDB_PROP_GPU_IS_ALL_INST_IN_SYSMEM_BASE_NAME PDB_PROP_GPU_IS_ALL_INST_IN_SYSMEM
#define PDB_PROP_GPU_IS_CONNECTED_BASE_CAST
#define PDB_PROP_GPU_IS_CONNECTED_BASE_NAME PDB_PROP_GPU_IS_CONNECTED
#define PDB_PROP_GPU_BROKEN_FB_BASE_CAST
#define PDB_PROP_GPU_BROKEN_FB_BASE_NAME PDB_PROP_GPU_BROKEN_FB
#define PDB_PROP_GPU_SOC_FRM_RESTORE_HIBERNATE_RESUME_BASE_CAST
#define PDB_PROP_GPU_SOC_FRM_RESTORE_HIBERNATE_RESUME_BASE_NAME PDB_PROP_GPU_SOC_FRM_RESTORE_HIBERNATE_RESUME
#define PDB_PROP_GPU_IN_FULLCHIP_RESET_BASE_CAST
#define PDB_PROP_GPU_IN_FULLCHIP_RESET_BASE_NAME PDB_PROP_GPU_IN_FULLCHIP_RESET
#define PDB_PROP_GPU_IN_SECONDARY_BUS_RESET_BASE_CAST
#define PDB_PROP_GPU_IN_SECONDARY_BUS_RESET_BASE_NAME PDB_PROP_GPU_IN_SECONDARY_BUS_RESET
#define PDB_PROP_GPU_IN_GC6_RESET_BASE_CAST
#define PDB_PROP_GPU_IN_GC6_RESET_BASE_NAME PDB_PROP_GPU_IN_GC6_RESET
#define PDB_PROP_GPU_IS_GEMINI_BASE_CAST
#define PDB_PROP_GPU_IS_GEMINI_BASE_NAME PDB_PROP_GPU_IS_GEMINI
#define PDB_PROP_GPU_PERSISTENT_SW_STATE_BASE_CAST
#define PDB_PROP_GPU_PERSISTENT_SW_STATE_BASE_NAME PDB_PROP_GPU_PERSISTENT_SW_STATE
#define PDB_PROP_GPU_COHERENT_CPU_MAPPING_BASE_CAST
#define PDB_PROP_GPU_COHERENT_CPU_MAPPING_BASE_NAME PDB_PROP_GPU_COHERENT_CPU_MAPPING
#define PDB_PROP_GPU_IS_LOST_BASE_CAST
#define PDB_PROP_GPU_IS_LOST_BASE_NAME PDB_PROP_GPU_IS_LOST
#define PDB_PROP_GPU_IN_TIMEOUT_RECOVERY_BASE_CAST
#define PDB_PROP_GPU_IN_TIMEOUT_RECOVERY_BASE_NAME PDB_PROP_GPU_IN_TIMEOUT_RECOVERY
#define PDB_PROP_GPU_ALLOW_PAGE_RETIREMENT_BASE_CAST
#define PDB_PROP_GPU_ALLOW_PAGE_RETIREMENT_BASE_NAME PDB_PROP_GPU_ALLOW_PAGE_RETIREMENT
#define PDB_PROP_GPU_IN_MCDM_MODE_BASE_CAST
#define PDB_PROP_GPU_IN_MCDM_MODE_BASE_NAME PDB_PROP_GPU_IN_MCDM_MODE
#define PDB_PROP_GPU_KEEP_WPR_ACROSS_GC6_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_KEEP_WPR_ACROSS_GC6_SUPPORTED_BASE_NAME PDB_PROP_GPU_KEEP_WPR_ACROSS_GC6_SUPPORTED
#define PDB_PROP_GPU_TEGRA_SOC_NVDISPLAY_BASE_CAST
#define PDB_PROP_GPU_TEGRA_SOC_NVDISPLAY_BASE_NAME PDB_PROP_GPU_TEGRA_SOC_NVDISPLAY
#define PDB_PROP_GPU_ATS_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_ATS_SUPPORTED_BASE_NAME PDB_PROP_GPU_ATS_SUPPORTED
#define PDB_PROP_GPU_SC7_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_SC7_SUPPORTED_BASE_NAME PDB_PROP_GPU_SC7_SUPPORTED
#define PDB_PROP_GPU_RTD3_RG_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_RTD3_RG_SUPPORTED_BASE_NAME PDB_PROP_GPU_RTD3_RG_SUPPORTED
#define PDB_PROP_GPU_TRIGGER_PCIE_FLR_BASE_CAST
#define PDB_PROP_GPU_TRIGGER_PCIE_FLR_BASE_NAME PDB_PROP_GPU_TRIGGER_PCIE_FLR
#define PDB_PROP_GPU_CLKS_IN_TEGRA_SOC_BASE_CAST
#define PDB_PROP_GPU_CLKS_IN_TEGRA_SOC_BASE_NAME PDB_PROP_GPU_CLKS_IN_TEGRA_SOC
#define PDB_PROP_GPU_PREINITIALIZED_WPR_REGION_BASE_CAST
#define PDB_PROP_GPU_PREINITIALIZED_WPR_REGION_BASE_NAME PDB_PROP_GPU_PREINITIALIZED_WPR_REGION
#define PDB_PROP_GPU_SECONDARY_BUS_RESET_PENDING_BASE_CAST
#define PDB_PROP_GPU_SECONDARY_BUS_RESET_PENDING_BASE_NAME PDB_PROP_GPU_SECONDARY_BUS_RESET_PENDING
#define PDB_PROP_GPU_IN_BUGCHECK_CALLBACK_ROUTINE_BASE_CAST
#define PDB_PROP_GPU_IN_BUGCHECK_CALLBACK_ROUTINE_BASE_NAME PDB_PROP_GPU_IN_BUGCHECK_CALLBACK_ROUTINE
#define PDB_PROP_GPU_BUG_3007008_EMULATE_VF_MMU_TLB_INVALIDATE_BASE_CAST
#define PDB_PROP_GPU_BUG_3007008_EMULATE_VF_MMU_TLB_INVALIDATE_BASE_NAME PDB_PROP_GPU_BUG_3007008_EMULATE_VF_MMU_TLB_INVALIDATE
#define PDB_PROP_GPU_IS_UEFI_BASE_CAST
#define PDB_PROP_GPU_IS_UEFI_BASE_NAME PDB_PROP_GPU_IS_UEFI
#define PDB_PROP_GPU_IS_EFI_INIT_BASE_CAST
#define PDB_PROP_GPU_IS_EFI_INIT_BASE_NAME PDB_PROP_GPU_IS_EFI_INIT
#define PDB_PROP_GPU_CAN_OPTIMIZE_COMPUTE_USE_CASE_BASE_CAST
#define PDB_PROP_GPU_CAN_OPTIMIZE_COMPUTE_USE_CASE_BASE_NAME PDB_PROP_GPU_CAN_OPTIMIZE_COMPUTE_USE_CASE
#define PDB_PROP_GPU_MIG_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_MIG_SUPPORTED_BASE_NAME PDB_PROP_GPU_MIG_SUPPORTED
#define PDB_PROP_GPU_MIG_MIRROR_HOST_CI_ON_GUEST_BASE_CAST
#define PDB_PROP_GPU_MIG_MIRROR_HOST_CI_ON_GUEST_BASE_NAME PDB_PROP_GPU_MIG_MIRROR_HOST_CI_ON_GUEST
#define PDB_PROP_GPU_MIG_SUPPORTS_SPLIT_CE_RANGES_BASE_CAST
#define PDB_PROP_GPU_MIG_SUPPORTS_SPLIT_CE_RANGES_BASE_NAME PDB_PROP_GPU_MIG_SUPPORTS_SPLIT_CE_RANGES
#define PDB_PROP_GPU_MIG_GFX_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_MIG_GFX_SUPPORTED_BASE_NAME PDB_PROP_GPU_MIG_GFX_SUPPORTED
#define PDB_PROP_GPU_MIG_TIMESLICING_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_MIG_TIMESLICING_SUPPORTED_BASE_NAME PDB_PROP_GPU_MIG_TIMESLICING_SUPPORTED
#define PDB_PROP_GPU_VC_CAPABILITY_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_VC_CAPABILITY_SUPPORTED_BASE_NAME PDB_PROP_GPU_VC_CAPABILITY_SUPPORTED
#define PDB_PROP_GPU_RESETLESS_MIG_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_RESETLESS_MIG_SUPPORTED_BASE_NAME PDB_PROP_GPU_RESETLESS_MIG_SUPPORTED
#define PDB_PROP_GPU_IS_COT_ENABLED_BASE_CAST
#define PDB_PROP_GPU_IS_COT_ENABLED_BASE_NAME PDB_PROP_GPU_IS_COT_ENABLED
#define PDB_PROP_GPU_FW_WPR_OFFSET_SET_BY_ACR_BASE_CAST
#define PDB_PROP_GPU_FW_WPR_OFFSET_SET_BY_ACR_BASE_NAME PDB_PROP_GPU_FW_WPR_OFFSET_SET_BY_ACR
#define PDB_PROP_GPU_TOGGLE_DYNAMIC_THROTTLE_WINDOW_SIZE_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_TOGGLE_DYNAMIC_THROTTLE_WINDOW_SIZE_SUPPORTED_BASE_NAME PDB_PROP_GPU_TOGGLE_DYNAMIC_THROTTLE_WINDOW_SIZE_SUPPORTED
#define PDB_PROP_GPU_UNIX_HDMI_FRL_COMPLIANCE_ENABLED_BASE_CAST
#define PDB_PROP_GPU_UNIX_HDMI_FRL_COMPLIANCE_ENABLED_BASE_NAME PDB_PROP_GPU_UNIX_HDMI_FRL_COMPLIANCE_ENABLED
#define PDB_PROP_GPU_SRIOV_SYSMEM_DIRTY_PAGE_TRACKING_ENABLED_BASE_CAST
#define PDB_PROP_GPU_SRIOV_SYSMEM_DIRTY_PAGE_TRACKING_ENABLED_BASE_NAME PDB_PROP_GPU_SRIOV_SYSMEM_DIRTY_PAGE_TRACKING_ENABLED
#define PDB_PROP_GPU_VGPU_OFFLOAD_CAPABLE_BASE_CAST
#define PDB_PROP_GPU_VGPU_OFFLOAD_CAPABLE_BASE_NAME PDB_PROP_GPU_VGPU_OFFLOAD_CAPABLE
#define PDB_PROP_GPU_POWER_MANAGEMENT_UNSUPPORTED_BASE_CAST
#define PDB_PROP_GPU_POWER_MANAGEMENT_UNSUPPORTED_BASE_NAME PDB_PROP_GPU_POWER_MANAGEMENT_UNSUPPORTED
#define PDB_PROP_GPU_PVMRL_GRANULAR_LOCKING_BASE_CAST
#define PDB_PROP_GPU_PVMRL_GRANULAR_LOCKING_BASE_NAME PDB_PROP_GPU_PVMRL_GRANULAR_LOCKING
#define PDB_PROP_GPU_IS_PLX_PRESENT_BASE_CAST
#define PDB_PROP_GPU_IS_PLX_PRESENT_BASE_NAME PDB_PROP_GPU_IS_PLX_PRESENT
#define PDB_PROP_GPU_IS_BR03_PRESENT_BASE_CAST
#define PDB_PROP_GPU_IS_BR03_PRESENT_BASE_NAME PDB_PROP_GPU_IS_BR03_PRESENT
#define PDB_PROP_GPU_IS_BR04_PRESENT_BASE_CAST
#define PDB_PROP_GPU_IS_BR04_PRESENT_BASE_NAME PDB_PROP_GPU_IS_BR04_PRESENT
#define PDB_PROP_GPU_BEHIND_BRIDGE_BASE_CAST
#define PDB_PROP_GPU_BEHIND_BRIDGE_BASE_NAME PDB_PROP_GPU_BEHIND_BRIDGE
#define PDB_PROP_GPU_BEHIND_BR03_BASE_CAST
#define PDB_PROP_GPU_BEHIND_BR03_BASE_NAME PDB_PROP_GPU_BEHIND_BR03
#define PDB_PROP_GPU_BEHIND_BR04_BASE_CAST
#define PDB_PROP_GPU_BEHIND_BR04_BASE_NAME PDB_PROP_GPU_BEHIND_BR04
#define PDB_PROP_GPU_UPSTREAM_PORT_L0S_UNSUPPORTED_BASE_CAST
#define PDB_PROP_GPU_UPSTREAM_PORT_L0S_UNSUPPORTED_BASE_NAME PDB_PROP_GPU_UPSTREAM_PORT_L0S_UNSUPPORTED
#define PDB_PROP_GPU_UPSTREAM_PORT_L1_UNSUPPORTED_BASE_CAST
#define PDB_PROP_GPU_UPSTREAM_PORT_L1_UNSUPPORTED_BASE_NAME PDB_PROP_GPU_UPSTREAM_PORT_L1_UNSUPPORTED
#define PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_SUPPORTED_BASE_NAME PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_SUPPORTED
#define PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_MOBILE_ONLY_BASE_CAST
#define PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_MOBILE_ONLY_BASE_NAME PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_MOBILE_ONLY
#define PDB_PROP_GPU_RM_UNLINKED_SLI_BASE_CAST
#define PDB_PROP_GPU_RM_UNLINKED_SLI_BASE_NAME PDB_PROP_GPU_RM_UNLINKED_SLI
#define PDB_PROP_GPU_SLI_LINK_ACTIVE_BASE_CAST
#define PDB_PROP_GPU_SLI_LINK_ACTIVE_BASE_NAME PDB_PROP_GPU_SLI_LINK_ACTIVE
#define PDB_PROP_GPU_ENABLE_REG_ACCESS_IN_LOW_POWER_FOR_SIM_SRTEST_BASE_CAST
#define PDB_PROP_GPU_ENABLE_REG_ACCESS_IN_LOW_POWER_FOR_SIM_SRTEST_BASE_NAME PDB_PROP_GPU_ENABLE_REG_ACCESS_IN_LOW_POWER_FOR_SIM_SRTEST
#define PDB_PROP_GPU_DO_NOT_CHECK_REG_ACCESS_IN_PM_CODEPATH_BASE_CAST
#define PDB_PROP_GPU_DO_NOT_CHECK_REG_ACCESS_IN_PM_CODEPATH_BASE_NAME PDB_PROP_GPU_DO_NOT_CHECK_REG_ACCESS_IN_PM_CODEPATH
#define PDB_PROP_GPU_EXTERNAL_HEAP_CONTROL_BASE_CAST
#define PDB_PROP_GPU_EXTERNAL_HEAP_CONTROL_BASE_NAME PDB_PROP_GPU_EXTERNAL_HEAP_CONTROL
#define PDB_PROP_GPU_IS_MOBILE_BASE_CAST
#define PDB_PROP_GPU_IS_MOBILE_BASE_NAME PDB_PROP_GPU_IS_MOBILE
#define PDB_PROP_GPU_IS_EXTERNAL_GPU_BASE_CAST
#define PDB_PROP_GPU_IS_EXTERNAL_GPU_BASE_NAME PDB_PROP_GPU_IS_EXTERNAL_GPU
#define PDB_PROP_GPU_RTD3_GC6_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_RTD3_GC6_SUPPORTED_BASE_NAME PDB_PROP_GPU_RTD3_GC6_SUPPORTED
#define PDB_PROP_GPU_RTD3_GC6_ACTIVE_BASE_CAST
#define PDB_PROP_GPU_RTD3_GC6_ACTIVE_BASE_NAME PDB_PROP_GPU_RTD3_GC6_ACTIVE
#define PDB_PROP_GPU_FAST_GC6_ACTIVE_BASE_CAST
#define PDB_PROP_GPU_FAST_GC6_ACTIVE_BASE_NAME PDB_PROP_GPU_FAST_GC6_ACTIVE
#define PDB_PROP_GPU_UNIX_DYNAMIC_POWER_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_UNIX_DYNAMIC_POWER_SUPPORTED_BASE_NAME PDB_PROP_GPU_UNIX_DYNAMIC_POWER_SUPPORTED
#define PDB_PROP_GPU_MOVE_CTX_BUFFERS_TO_PMA_BASE_CAST
#define PDB_PROP_GPU_MOVE_CTX_BUFFERS_TO_PMA_BASE_NAME PDB_PROP_GPU_MOVE_CTX_BUFFERS_TO_PMA
#define PDB_PROP_GPU_LEGACY_GCOFF_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_LEGACY_GCOFF_SUPPORTED_BASE_NAME PDB_PROP_GPU_LEGACY_GCOFF_SUPPORTED
#define PDB_PROP_GPU_RTD3_GCOFF_SUPPORTED_BASE_CAST
#define PDB_PROP_GPU_RTD3_GCOFF_SUPPORTED_BASE_NAME PDB_PROP_GPU_RTD3_GCOFF_SUPPORTED
#define PDB_PROP_GPU_GCOFF_STATE_ENTERING_BASE_CAST
#define PDB_PROP_GPU_GCOFF_STATE_ENTERING_BASE_NAME PDB_PROP_GPU_GCOFF_STATE_ENTERING
#define PDB_PROP_GPU_GCOFF_STATE_ENTERED_BASE_CAST
#define PDB_PROP_GPU_GCOFF_STATE_ENTERED_BASE_NAME PDB_PROP_GPU_GCOFF_STATE_ENTERED
#define PDB_PROP_GPU_RG_STATE_ENTERING_BASE_CAST
#define PDB_PROP_GPU_RG_STATE_ENTERING_BASE_NAME PDB_PROP_GPU_RG_STATE_ENTERING
#define PDB_PROP_GPU_RG_STATE_ENTERED_BASE_CAST
#define PDB_PROP_GPU_RG_STATE_ENTERED_BASE_NAME PDB_PROP_GPU_RG_STATE_ENTERED
#define PDB_PROP_GPU_ACCOUNTING_ON_BASE_CAST
#define PDB_PROP_GPU_ACCOUNTING_ON_BASE_NAME PDB_PROP_GPU_ACCOUNTING_ON
#define PDB_PROP_GPU_INACCESSIBLE_BASE_CAST
#define PDB_PROP_GPU_INACCESSIBLE_BASE_NAME PDB_PROP_GPU_INACCESSIBLE
#define PDB_PROP_GPU_NVLINK_SYSMEM_BASE_CAST
#define PDB_PROP_GPU_NVLINK_SYSMEM_BASE_NAME PDB_PROP_GPU_NVLINK_SYSMEM
#define PDB_PROP_GPU_SKIP_CE_MAPPINGS_NO_NVLINK_BASE_CAST
#define PDB_PROP_GPU_SKIP_CE_MAPPINGS_NO_NVLINK_BASE_NAME PDB_PROP_GPU_SKIP_CE_MAPPINGS_NO_NVLINK
#define PDB_PROP_GPU_C2C_SYSMEM_BASE_CAST
#define PDB_PROP_GPU_C2C_SYSMEM_BASE_NAME PDB_PROP_GPU_C2C_SYSMEM
#define PDB_PROP_GPU_IN_TCC_MODE_BASE_CAST
#define PDB_PROP_GPU_IN_TCC_MODE_BASE_NAME PDB_PROP_GPU_IN_TCC_MODE
#define PDB_PROP_GPU_SUPPORTS_TDR_EVENT_BASE_CAST
#define PDB_PROP_GPU_SUPPORTS_TDR_EVENT_BASE_NAME PDB_PROP_GPU_SUPPORTS_TDR_EVENT
#define PDB_PROP_GPU_MSHYBRID_GC6_ACTIVE_BASE_CAST
#define PDB_PROP_GPU_MSHYBRID_GC6_ACTIVE_BASE_NAME PDB_PROP_GPU_MSHYBRID_GC6_ACTIVE
#define PDB_PROP_GPU_VGPU_BIG_PAGE_SIZE_64K_BASE_CAST
#define PDB_PROP_GPU_VGPU_BIG_PAGE_SIZE_64K_BASE_NAME PDB_PROP_GPU_VGPU_BIG_PAGE_SIZE_64K
#define PDB_PROP_GPU_OPTIMIZE_SPARSE_TEXTURE_BY_DEFAULT_BASE_CAST
#define PDB_PROP_GPU_OPTIMIZE_SPARSE_TEXTURE_BY_DEFAULT_BASE_NAME PDB_PROP_GPU_OPTIMIZE_SPARSE_TEXTURE_BY_DEFAULT
#define PDB_PROP_GPU_ENABLE_IOMMU_SUPPORT_BASE_CAST
#define PDB_PROP_GPU_ENABLE_IOMMU_SUPPORT_BASE_NAME PDB_PROP_GPU_ENABLE_IOMMU_SUPPORT
#define PDB_PROP_GPU_IGNORE_REPLAYABLE_FAULTS_BASE_CAST
#define PDB_PROP_GPU_IGNORE_REPLAYABLE_FAULTS_BASE_NAME PDB_PROP_GPU_IGNORE_REPLAYABLE_FAULTS
#define PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VGPU_BASE_CAST
#define PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VGPU_BASE_NAME PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VGPU
#define PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VSGA_BASE_CAST
#define PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VSGA_BASE_NAME PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VSGA
#define PDB_PROP_GPU_IS_VGPU_HETEROGENEOUS_MODE_BASE_CAST
#define PDB_PROP_GPU_IS_VGPU_HETEROGENEOUS_MODE_BASE_NAME PDB_PROP_GPU_IS_VGPU_HETEROGENEOUS_MODE
#define PDB_PROP_GPU_NVLINK_P2P_LOOPBACK_DISABLED_BASE_CAST
#define PDB_PROP_GPU_NVLINK_P2P_LOOPBACK_DISABLED_BASE_NAME PDB_PROP_GPU_NVLINK_P2P_LOOPBACK_DISABLED
#define PDB_PROP_GPU_NV_USERMODE_ENABLED_BASE_CAST
#define PDB_PROP_GPU_NV_USERMODE_ENABLED_BASE_NAME PDB_PROP_GPU_NV_USERMODE_ENABLED
#define PDB_PROP_GPU_IN_FATAL_ERROR_BASE_CAST
#define PDB_PROP_GPU_IN_FATAL_ERROR_BASE_NAME PDB_PROP_GPU_IN_FATAL_ERROR
#define PDB_PROP_GPU_OPTIMUS_GOLD_CFG_SPACE_RESTORE_BASE_CAST
#define PDB_PROP_GPU_OPTIMUS_GOLD_CFG_SPACE_RESTORE_BASE_NAME PDB_PROP_GPU_OPTIMUS_GOLD_CFG_SPACE_RESTORE
#define PDB_PROP_GPU_VGA_ENABLED_BASE_CAST
#define PDB_PROP_GPU_VGA_ENABLED_BASE_NAME PDB_PROP_GPU_VGA_ENABLED
#define PDB_PROP_GPU_IS_MXM_3X_BASE_CAST
#define PDB_PROP_GPU_IS_MXM_3X_BASE_NAME PDB_PROP_GPU_IS_MXM_3X
#define PDB_PROP_GPU_GSYNC_III_ATTACHED_BASE_CAST
#define PDB_PROP_GPU_GSYNC_III_ATTACHED_BASE_NAME PDB_PROP_GPU_GSYNC_III_ATTACHED
#define PDB_PROP_GPU_QSYNC_II_ATTACHED_BASE_CAST
#define PDB_PROP_GPU_QSYNC_II_ATTACHED_BASE_NAME PDB_PROP_GPU_QSYNC_II_ATTACHED
#define PDB_PROP_GPU_CC_FEATURE_CAPABLE_BASE_CAST
#define PDB_PROP_GPU_CC_FEATURE_CAPABLE_BASE_NAME PDB_PROP_GPU_CC_FEATURE_CAPABLE
#define PDB_PROP_GPU_APM_FEATURE_CAPABLE_BASE_CAST
#define PDB_PROP_GPU_APM_FEATURE_CAPABLE_BASE_NAME PDB_PROP_GPU_APM_FEATURE_CAPABLE
#define PDB_PROP_GPU_SKIP_TABLE_CE_MAP_BASE_CAST
#define PDB_PROP_GPU_SKIP_TABLE_CE_MAP_BASE_NAME PDB_PROP_GPU_SKIP_TABLE_CE_MAP
#define PDB_PROP_GPU_CHIP_SUPPORTS_RTD3_DEF_BASE_CAST
#define PDB_PROP_GPU_CHIP_SUPPORTS_RTD3_DEF_BASE_NAME PDB_PROP_GPU_CHIP_SUPPORTS_RTD3_DEF
#define PDB_PROP_GPU_IS_SOC_SDM_BASE_CAST
#define PDB_PROP_GPU_IS_SOC_SDM_BASE_NAME PDB_PROP_GPU_IS_SOC_SDM
#define PDB_PROP_GPU_DISP_PB_REQUIRES_SMMU_BYPASS_BASE_CAST
#define PDB_PROP_GPU_DISP_PB_REQUIRES_SMMU_BYPASS_BASE_NAME PDB_PROP_GPU_DISP_PB_REQUIRES_SMMU_BYPASS
#define PDB_PROP_GPU_FORCE_PERF_BIOS_LEVEL_BASE_CAST
#define PDB_PROP_GPU_FORCE_PERF_BIOS_LEVEL_BASE_NAME PDB_PROP_GPU_FORCE_PERF_BIOS_LEVEL
#define PDB_PROP_GPU_FASTPATH_SEQ_ENABLED_BASE_CAST
#define PDB_PROP_GPU_FASTPATH_SEQ_ENABLED_BASE_NAME PDB_PROP_GPU_FASTPATH_SEQ_ENABLED
#define PDB_PROP_GPU_PREPARING_FULLCHIP_RESET_BASE_CAST
#define PDB_PROP_GPU_PREPARING_FULLCHIP_RESET_BASE_NAME PDB_PROP_GPU_PREPARING_FULLCHIP_RESET
#define PDB_PROP_GPU_RECOVERY_DRAIN_P2P_REQUIRED_BASE_CAST
#define PDB_PROP_GPU_RECOVERY_DRAIN_P2P_REQUIRED_BASE_NAME PDB_PROP_GPU_RECOVERY_DRAIN_P2P_REQUIRED
#define PDB_PROP_GPU_RECOVERY_REBOOT_REQUIRED_BASE_CAST
#define PDB_PROP_GPU_RECOVERY_REBOOT_REQUIRED_BASE_NAME PDB_PROP_GPU_RECOVERY_REBOOT_REQUIRED
#define PDB_PROP_GPU_RECOVERY_SQUASH_XID154_BASE_CAST
#define PDB_PROP_GPU_RECOVERY_SQUASH_XID154_BASE_NAME PDB_PROP_GPU_RECOVERY_SQUASH_XID154
#define PDB_PROP_GPU_REUSE_INIT_CONTING_MEM_BASE_CAST
#define PDB_PROP_GPU_REUSE_INIT_CONTING_MEM_BASE_NAME PDB_PROP_GPU_REUSE_INIT_CONTING_MEM
#define PDB_PROP_GPU_RUSD_POLLING_SUPPORT_MONOLITHIC_BASE_CAST
#define PDB_PROP_GPU_RUSD_POLLING_SUPPORT_MONOLITHIC_BASE_NAME PDB_PROP_GPU_RUSD_POLLING_SUPPORT_MONOLITHIC
#define PDB_PROP_GPU_RUSD_DISABLE_CLK_PUBLIC_DOMAIN_INFO_BASE_CAST
#define PDB_PROP_GPU_RUSD_DISABLE_CLK_PUBLIC_DOMAIN_INFO_BASE_NAME PDB_PROP_GPU_RUSD_DISABLE_CLK_PUBLIC_DOMAIN_INFO
#define PDB_PROP_GPU_ALLOC_ISO_SYS_MEM_FROM_CARVEOUT_BASE_CAST
#define PDB_PROP_GPU_ALLOC_ISO_SYS_MEM_FROM_CARVEOUT_BASE_NAME PDB_PROP_GPU_ALLOC_ISO_SYS_MEM_FROM_CARVEOUT
#define PDB_PROP_GPU_HFRP_IS_KERNEL_OBJECT_ACTIVE_WAR_BASE_CAST
#define PDB_PROP_GPU_HFRP_IS_KERNEL_OBJECT_ACTIVE_WAR_BASE_NAME PDB_PROP_GPU_HFRP_IS_KERNEL_OBJECT_ACTIVE_WAR


NV_STATUS __nvoc_objCreateDynamic_OBJGPU(OBJGPU**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_OBJGPU(OBJGPU**, Dynamic*, NvU32,
        NvU32 ChipHal_arch, NvU32 ChipHal_impl, NvU32 ChipHal_hidrev,
        TEGRA_CHIP_TYPE TegraChipHal_tegraType,
        RM_RUNTIME_VARIANT RmVariantHal_rmVariant,
        NvU32 DispIpHal_ipver, NvU32 gpuInstance, NvU32 gpuId, NvUuid *pUuid, struct GpuArch *pGpuArch);
#define __objCreate_OBJGPU(__nvoc_ppNewObj, __nvoc_pParent, __nvoc_createFlags, ChipHal_arch, ChipHal_impl, ChipHal_hidrev, TegraChipHal_tegraType, RmVariantHal_rmVariant, DispIpHal_ipver, gpuInstance, gpuId, pUuid, pGpuArch) \
    __nvoc_objCreate_OBJGPU((__nvoc_ppNewObj), staticCast((__nvoc_pParent), Dynamic), (__nvoc_createFlags), ChipHal_arch, ChipHal_impl, ChipHal_hidrev, TegraChipHal_tegraType, RmVariantHal_rmVariant, DispIpHal_ipver, gpuInstance, gpuId, pUuid, pGpuArch)


// Wrapper macros for implementation functions
NV_STATUS gpuConstruct_IMPL(struct OBJGPU *pGpu, NvU32 gpuInstance, NvU32 gpuId, NvUuid *pUuid, struct GpuArch *pGpuArch);
#define __nvoc_gpuConstruct(pGpu, gpuInstance, gpuId, pUuid, pGpuArch) gpuConstruct_IMPL(pGpu, gpuInstance, gpuId, pUuid, pGpuArch)

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuConstructPhysical(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuConstructPhysical(pGpu) gpuConstructPhysical_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuBindHalLegacy_IMPL(struct OBJGPU *pGpu, NvU32 chipId0, NvU32 chipId1, NvU32 socChipId0);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuBindHalLegacy(struct OBJGPU *pGpu, NvU32 chipId0, NvU32 chipId1, NvU32 socChipId0) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuBindHalLegacy(pGpu, chipId0, chipId1, socChipId0) gpuBindHalLegacy_IMPL(pGpu, chipId0, chipId1, socChipId0)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuPostConstruct_IMPL(struct OBJGPU *pGpu, GPUATTACHARG *arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPostConstruct(struct OBJGPU *pGpu, GPUATTACHARG *arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPostConstruct(pGpu, arg2) gpuPostConstruct_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

void gpuDestruct_IMPL(struct OBJGPU *pGpu);
#define __nvoc_gpuDestruct(pGpu) gpuDestruct_IMPL(pGpu)

#ifdef __nvoc_gpu_h_disabled
static inline void gpuDestructPhysical(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuDestructPhysical(pGpu) gpuDestructPhysical_d44104(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuStatePreInit(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuStatePreInit(pGpu) gpuStatePreInit_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuStateInit_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuStateInit(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuStateInit(pGpu) gpuStateInit_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuStateLoad(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuStateLoad(pGpu, arg2) gpuStateLoad_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuStateUnload_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuStateUnload(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuStateUnload(pGpu, arg2) gpuStateUnload_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuStateDestroy(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuStateDestroy(pGpu) gpuStateDestroy_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuStateInitStartedSatisfy(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuStateInitStartedSatisfy(pGpu) gpuStateInitStartedSatisfy_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuStateInitStartedRetract(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuStateInitStartedRetract(pGpu) gpuStateInitStartedRetract_d44104(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuInitDispIpHal_IMPL(struct OBJGPU *pGpu, NvU32 ipver);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuInitDispIpHal(struct OBJGPU *pGpu, NvU32 ipver) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuInitDispIpHal(pGpu, ipver) gpuInitDispIpHal_IMPL(pGpu, ipver)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsImplementation_IMPL(struct OBJGPU *pGpu, HAL_IMPLEMENTATION arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsImplementation(struct OBJGPU *pGpu, HAL_IMPLEMENTATION arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsImplementation(pGpu, arg2) gpuIsImplementation_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsImplementationOrBetter_IMPL(struct OBJGPU *pGpu, HAL_IMPLEMENTATION arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsImplementationOrBetter(struct OBJGPU *pGpu, HAL_IMPLEMENTATION arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsImplementationOrBetter(pGpu, arg2) gpuIsImplementationOrBetter_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsGpuFullPower_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsGpuFullPower(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsGpuFullPower(pGpu) gpuIsGpuFullPower_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsGpuFullPowerForPmResume_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsGpuFullPowerForPmResume(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsGpuFullPowerForPmResume(pGpu) gpuIsGpuFullPowerForPmResume_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPowerManagementEnterPreUnloadPhysical(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPowerManagementEnterPreUnloadPhysical(pGpu) gpuPowerManagementEnterPreUnloadPhysical_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPowerManagementEnterPostUnloadPhysical(struct OBJGPU *pGpu, NvU32 newLevel) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPowerManagementEnterPostUnloadPhysical(pGpu, newLevel) gpuPowerManagementEnterPostUnloadPhysical_ac1694(pGpu, newLevel)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPowerManagementResumePreLoadPhysical(struct OBJGPU *pGpu, NvU32 oldLevel, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPowerManagementResumePreLoadPhysical(pGpu, oldLevel, flags) gpuPowerManagementResumePreLoadPhysical_ac1694(pGpu, oldLevel, flags)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPowerManagementResumePostLoadPhysical(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPowerManagementResumePostLoadPhysical(pGpu) gpuPowerManagementResumePostLoadPhysical_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetOneDeviceEntry_IMPL(struct OBJGPU *pGpu, NvU32 deviceTypeEnum, NvS32 dieletInstance, NvS32 globalInstanceId, NvS32 dieLocalInstanceId, const DEVICE_INFO_ENTRY **ppDeviceEntry);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetOneDeviceEntry(struct OBJGPU *pGpu, NvU32 deviceTypeEnum, NvS32 dieletInstance, NvS32 globalInstanceId, NvS32 dieLocalInstanceId, const DEVICE_INFO_ENTRY **ppDeviceEntry) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetOneDeviceEntry(pGpu, deviceTypeEnum, dieletInstance, globalInstanceId, dieLocalInstanceId, ppDeviceEntry) gpuGetOneDeviceEntry_IMPL(pGpu, deviceTypeEnum, dieletInstance, globalInstanceId, dieLocalInstanceId, ppDeviceEntry)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuIterDeviceInfo_IMPL(struct OBJGPU *pGpu, DeviceInfoIter *pIter, NvU32 deviceTypeEnum, NvS32 dieletInstance);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuIterDeviceInfo(struct OBJGPU *pGpu, DeviceInfoIter *pIter, NvU32 deviceTypeEnum, NvS32 dieletInstance) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuIterDeviceInfo(pGpu, pIter, deviceTypeEnum, dieletInstance) gpuIterDeviceInfo_IMPL(pGpu, pIter, deviceTypeEnum, dieletInstance)
#endif // __nvoc_gpu_h_disabled

NvBool gpuDeviceInfoIterNext_IMPL(struct OBJGPU *pGpu, DeviceInfoIter *pIter);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuDeviceInfoIterNext(struct OBJGPU *pGpu, DeviceInfoIter *pIter) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuDeviceInfoIterNext(pGpu, pIter) gpuDeviceInfoIterNext_IMPL(pGpu, pIter)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuConstructDeviceInfoTable(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuConstructDeviceInfoTable(pGpu) gpuConstructDeviceInfoTable_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuInitializeMemDescFromPromotedCtx(struct OBJGPU *pGpu, MEMORY_DESCRIPTOR **ppMemDesc, NvU64 gpuPhysAddr, NvU64 size, NvU32 physAttr, NvBool bIsCallingContextVgpuPlugin) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuInitializeMemDescFromPromotedCtx(pGpu, ppMemDesc, gpuPhysAddr, size, physAttr, bIsCallingContextVgpuPlugin) gpuInitializeMemDescFromPromotedCtx_395e98(pGpu, ppMemDesc, gpuPhysAddr, size, physAttr, bIsCallingContextVgpuPlugin)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuBuildClassDB_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuBuildClassDB(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuBuildClassDB(pGpu) gpuBuildClassDB_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuDestroyClassDB_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuDestroyClassDB(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuDestroyClassDB(pGpu) gpuDestroyClassDB_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuDeleteEngineFromClassDB_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuDeleteEngineFromClassDB(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuDeleteEngineFromClassDB(pGpu, arg2) gpuDeleteEngineFromClassDB_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuDeleteEngineOnPreInit_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuDeleteEngineOnPreInit(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuDeleteEngineOnPreInit(pGpu, arg2) gpuDeleteEngineOnPreInit_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuAddClassToClassDBByEngTag_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuAddClassToClassDBByEngTag(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuAddClassToClassDBByEngTag(pGpu, arg2) gpuAddClassToClassDBByEngTag_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuAddClassToClassDBByClassId_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuAddClassToClassDBByClassId(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuAddClassToClassDBByClassId(pGpu, arg2) gpuAddClassToClassDBByClassId_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuAddClassToClassDBByEngTagClassId_IMPL(struct OBJGPU *pGpu, NvU32 arg2, NvU32 arg3);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuAddClassToClassDBByEngTagClassId(struct OBJGPU *pGpu, NvU32 arg2, NvU32 arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuAddClassToClassDBByEngTagClassId(pGpu, arg2, arg3) gpuAddClassToClassDBByEngTagClassId_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuDeleteClassFromClassDBByClassId_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuDeleteClassFromClassDBByClassId(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuDeleteClassFromClassDBByClassId(pGpu, arg2) gpuDeleteClassFromClassDBByClassId_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuDeleteClassFromClassDBByEngTag_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuDeleteClassFromClassDBByEngTag(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuDeleteClassFromClassDBByEngTag(pGpu, arg2) gpuDeleteClassFromClassDBByEngTag_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuDeleteClassFromClassDBByEngTagClassId_IMPL(struct OBJGPU *pGpu, NvU32 arg2, NvU32 arg3);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuDeleteClassFromClassDBByEngTagClassId(struct OBJGPU *pGpu, NvU32 arg2, NvU32 arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuDeleteClassFromClassDBByEngTagClassId(pGpu, arg2, arg3) gpuDeleteClassFromClassDBByEngTagClassId_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsClassSupported_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsClassSupported(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsClassSupported(pGpu, arg2) gpuIsClassSupported_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetClassByClassId_IMPL(struct OBJGPU *pGpu, NvU32 arg2, CLASSDESCRIPTOR **arg3);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetClassByClassId(struct OBJGPU *pGpu, NvU32 arg2, CLASSDESCRIPTOR **arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetClassByClassId(pGpu, arg2, arg3) gpuGetClassByClassId_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetClassByEngineAndClassId_IMPL(struct OBJGPU *pGpu, NvU32 arg2, NvU32 arg3, CLASSDESCRIPTOR **arg4);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetClassByEngineAndClassId(struct OBJGPU *pGpu, NvU32 arg2, NvU32 arg3, CLASSDESCRIPTOR **arg4) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetClassByEngineAndClassId(pGpu, arg2, arg3, arg4) gpuGetClassByEngineAndClassId_IMPL(pGpu, arg2, arg3, arg4)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetClassList_IMPL(struct OBJGPU *pGpu, NvU32 *arg2, NvU32 *arg3, NvU32 arg4);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetClassList(struct OBJGPU *pGpu, NvU32 *arg2, NvU32 *arg3, NvU32 arg4) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetClassList(pGpu, arg2, arg3, arg4) gpuGetClassList_IMPL(pGpu, arg2, arg3, arg4)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuConstructEngineTable_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuConstructEngineTable(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuConstructEngineTable(pGpu) gpuConstructEngineTable_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

void gpuDestroyEngineTable_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuDestroyEngineTable(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuDestroyEngineTable(pGpu) gpuDestroyEngineTable_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuUpdateEngineTable_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuUpdateEngineTable(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuUpdateEngineTable(pGpu) gpuUpdateEngineTable_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NvBool gpuCheckEngineTable_IMPL(struct OBJGPU *pGpu, RM_ENGINE_TYPE arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuCheckEngineTable(struct OBJGPU *pGpu, RM_ENGINE_TYPE arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuCheckEngineTable(pGpu, arg2) gpuCheckEngineTable_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuXlateEngDescToClientEngineId_IMPL(struct OBJGPU *pGpu, ENGDESCRIPTOR arg2, RM_ENGINE_TYPE *arg3);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuXlateEngDescToClientEngineId(struct OBJGPU *pGpu, ENGDESCRIPTOR arg2, RM_ENGINE_TYPE *arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuXlateEngDescToClientEngineId(pGpu, arg2, arg3) gpuXlateEngDescToClientEngineId_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuXlateClientEngineIdToEngDesc_IMPL(struct OBJGPU *pGpu, RM_ENGINE_TYPE arg2, ENGDESCRIPTOR *arg3);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuXlateClientEngineIdToEngDesc(struct OBJGPU *pGpu, RM_ENGINE_TYPE arg2, ENGDESCRIPTOR *arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuXlateClientEngineIdToEngDesc(pGpu, arg2, arg3) gpuXlateClientEngineIdToEngDesc_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetFlcnFromClientEngineId_IMPL(struct OBJGPU *pGpu, RM_ENGINE_TYPE arg2, struct Falcon **arg3);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetFlcnFromClientEngineId(struct OBJGPU *pGpu, RM_ENGINE_TYPE arg2, struct Falcon **arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetFlcnFromClientEngineId(pGpu, arg2, arg3) gpuGetFlcnFromClientEngineId_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsEngDescSupported_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsEngDescSupported(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsEngDescSupported(pGpu, arg2) gpuIsEngDescSupported_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

RM_ENGINE_TYPE gpuGetRmEngineType_IMPL(NvU32 index);
#define gpuGetRmEngineType(index) gpuGetRmEngineType_IMPL(index)

void gpuGetRmEngineTypeList_IMPL(NvU32 *pNv2080EngineList, NvU32 engineCount, RM_ENGINE_TYPE *pRmEngineList);
#define gpuGetRmEngineTypeList(pNv2080EngineList, engineCount, pRmEngineList) gpuGetRmEngineTypeList_IMPL(pNv2080EngineList, engineCount, pRmEngineList)

NvU32 gpuGetNv2080EngineType_IMPL(RM_ENGINE_TYPE index);
#define gpuGetNv2080EngineType(index) gpuGetNv2080EngineType_IMPL(index)

void gpuGetNv2080EngineTypeList_IMPL(RM_ENGINE_TYPE *pRmEngineList, NvU32 engineCount, NvU32 *pNv2080EngineList);
#define gpuGetNv2080EngineTypeList(pRmEngineList, engineCount, pNv2080EngineList) gpuGetNv2080EngineTypeList_IMPL(pRmEngineList, engineCount, pNv2080EngineList)

NV_STATUS gpuGetRmEngineTypeCapMask_IMPL(NvU32 *NV2080EngineTypeCap, NvU32 capSize, NvU32 *RmEngineTypeCap);
#define gpuGetRmEngineTypeCapMask(NV2080EngineTypeCap, capSize, RmEngineTypeCap) gpuGetRmEngineTypeCapMask_IMPL(NV2080EngineTypeCap, capSize, RmEngineTypeCap)

const char * gpuRmEngineTypeToString_IMPL(RM_ENGINE_TYPE engineType, NvBool bNvPrintfStr);
#define gpuRmEngineTypeToString(engineType, bNvPrintfStr) gpuRmEngineTypeToString_IMPL(engineType, bNvPrintfStr)

NvBool gpuIsGridDisplaylessClassSupported_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsGridDisplaylessClassSupported(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsGridDisplaylessClassSupported(pGpu) gpuIsGridDisplaylessClassSupported_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetNameString(struct OBJGPU *pGpu, void *arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetNameString(pGpu, arg2) gpuGetNameString_T234D(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetShortNameString(struct OBJGPU *pGpu, NvU8 *arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetShortNameString(pGpu, arg2) gpuGetShortNameString_T234D(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NvU32 gpuGetGpuMask_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetGpuMask(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetGpuMask(pGpu) gpuGetGpuMask_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

void gpuChangeComputeModeRefCount_IMPL(struct OBJGPU *pGpu, NvU32 arg2);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuChangeComputeModeRefCount(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuChangeComputeModeRefCount(pGpu, arg2) gpuChangeComputeModeRefCount_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuEnterShutdown_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuEnterShutdown(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuEnterShutdown(pGpu) gpuEnterShutdown_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuSanityCheck_IMPL(struct OBJGPU *pGpu, NvU32 arg2, NvU32 *arg3);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSanityCheck(struct OBJGPU *pGpu, NvU32 arg2, NvU32 *arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSanityCheck(pGpu, arg2, arg3) gpuSanityCheck_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

DEVICE_MAPPING * gpuGetDeviceMapping_IMPL(struct OBJGPU *pGpu, DEVICE_INDEX arg2, NvU32 arg3);
#ifdef __nvoc_gpu_h_disabled
static inline DEVICE_MAPPING * gpuGetDeviceMapping(struct OBJGPU *pGpu, DEVICE_INDEX arg2, NvU32 arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NULL;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetDeviceMapping(pGpu, arg2, arg3) gpuGetDeviceMapping_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

DEVICE_MAPPING * gpuGetDeviceMappingFromDeviceID_IMPL(struct OBJGPU *pGpu, NvU32 arg2, NvU32 arg3);
#ifdef __nvoc_gpu_h_disabled
static inline DEVICE_MAPPING * gpuGetDeviceMappingFromDeviceID(struct OBJGPU *pGpu, NvU32 arg2, NvU32 arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NULL;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetDeviceMappingFromDeviceID(pGpu, arg2, arg3) gpuGetDeviceMappingFromDeviceID_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetGidInfo_IMPL(struct OBJGPU *pGpu, NvU8 **ppGidString, NvU32 *pGidStrlen, NvU32 gidFlags);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetGidInfo(struct OBJGPU *pGpu, NvU8 **ppGidString, NvU32 *pGidStrlen, NvU32 gidFlags) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetGidInfo(pGpu, ppGidString, pGidStrlen, gidFlags) gpuGetGidInfo_IMPL(pGpu, ppGidString, pGidStrlen, gidFlags)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetUgidInfo_IMPL(struct OBJGPU *pGpu, NvU8 **ppGidString, NvU32 *pGidStrlen, NvU32 gidFlags, NvU32 ugpuId);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetUgidInfo(struct OBJGPU *pGpu, NvU8 **ppGidString, NvU32 *pGidStrlen, NvU32 gidFlags, NvU32 ugpuId) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetUgidInfo(pGpu, ppGidString, pGidStrlen, gidFlags, ugpuId) gpuGetUgidInfo_IMPL(pGpu, ppGidString, pGidStrlen, gidFlags, ugpuId)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuCheckEngine(struct OBJGPU *pGpu, ENGDESCRIPTOR desc) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuCheckEngine(pGpu, desc) gpuCheckEngine_e661f0(pGpu, desc)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsSocSdmEnabled(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsSocSdmEnabled(pGpu) gpuIsSocSdmEnabled_d69453(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuCheckEngineWithOrderList(struct OBJGPU *pGpu, ENGDESCRIPTOR desc, NvBool bCheckEngineOrder) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuCheckEngineWithOrderList(pGpu, desc, bCheckEngineOrder) gpuCheckEngineWithOrderList_KERNEL(pGpu, desc, bCheckEngineOrder)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsSliLinkSupported(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsSliLinkSupported(pGpu) gpuIsSliLinkSupported_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuSetThreadBcState(struct OBJGPU *pGpu, NvBool arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuSetThreadBcState(pGpu, arg2) gpuSetThreadBcState_d44104(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuDeterminePersistantIllumSettings(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuDeterminePersistantIllumSettings(pGpu) gpuDeterminePersistantIllumSettings_d44104(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuInitSliIllumination(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuInitSliIllumination(pGpu) gpuInitSliIllumination_395e98(pGpu)
#endif // __nvoc_gpu_h_disabled

void gpuSetDisconnectedProperties_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuSetDisconnectedProperties(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuSetDisconnectedProperties(pGpu) gpuSetDisconnectedProperties_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuAddConstructedFalcon_IMPL(struct OBJGPU *pGpu, struct Falcon *arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuAddConstructedFalcon(struct OBJGPU *pGpu, struct Falcon *arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuAddConstructedFalcon(pGpu, arg2) gpuAddConstructedFalcon_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuRemoveConstructedFalcon_IMPL(struct OBJGPU *pGpu, struct Falcon *arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuRemoveConstructedFalcon(struct OBJGPU *pGpu, struct Falcon *arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuRemoveConstructedFalcon(pGpu, arg2) gpuRemoveConstructedFalcon_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetConstructedFalcon_IMPL(struct OBJGPU *pGpu, NvU32 arg2, struct Falcon **arg3);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetConstructedFalcon(struct OBJGPU *pGpu, NvU32 arg2, struct Falcon **arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetConstructedFalcon(pGpu, arg2, arg3) gpuGetConstructedFalcon_IMPL(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuBuildGenericKernelFalconList(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuBuildGenericKernelFalconList(pGpu) gpuBuildGenericKernelFalconList_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuDestroyGenericKernelFalconList(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuDestroyGenericKernelFalconList(pGpu) gpuDestroyGenericKernelFalconList_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuBuildKernelVideoEngineList(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuBuildKernelVideoEngineList(pGpu) gpuBuildKernelVideoEngineList_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuInitVideoLogging(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuInitVideoLogging(pGpu) gpuInitVideoLogging_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuFreeVideoLogging(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuFreeVideoLogging(pGpu) gpuFreeVideoLogging_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuDestroyKernelVideoEngineList(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuDestroyKernelVideoEngineList(pGpu) gpuDestroyKernelVideoEngineList_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline struct GenericKernelFalcon * gpuGetGenericKernelFalconForEngine(struct OBJGPU *pGpu, ENGDESCRIPTOR arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NULL;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetGenericKernelFalconForEngine(pGpu, arg2) gpuGetGenericKernelFalconForEngine_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuRegisterGenericKernelFalconIntrService(struct OBJGPU *pGpu, void *pRecords) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuRegisterGenericKernelFalconIntrService(pGpu, pRecords) gpuRegisterGenericKernelFalconIntrService_IMPL(pGpu, pRecords)
#endif // __nvoc_gpu_h_disabled

struct OBJENGSTATE * gpuGetEngstate_IMPL(struct OBJGPU *pGpu, ENGDESCRIPTOR arg2);
#ifdef __nvoc_gpu_h_disabled
static inline struct OBJENGSTATE * gpuGetEngstate(struct OBJGPU *pGpu, ENGDESCRIPTOR arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NULL;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetEngstate(pGpu, arg2) gpuGetEngstate_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

struct OBJENGSTATE * gpuGetEngstateNoShare_IMPL(struct OBJGPU *pGpu, ENGDESCRIPTOR arg2);
#ifdef __nvoc_gpu_h_disabled
static inline struct OBJENGSTATE * gpuGetEngstateNoShare(struct OBJGPU *pGpu, ENGDESCRIPTOR arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NULL;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetEngstateNoShare(pGpu, arg2) gpuGetEngstateNoShare_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

void * gpuGetNextChildOfTypeUnsafe_IMPL(struct OBJGPU *pGpu, GPU_CHILD_ITER *pIt, NvU32 classId);
#ifdef __nvoc_gpu_h_disabled
static inline void * gpuGetNextChildOfTypeUnsafe(struct OBJGPU *pGpu, GPU_CHILD_ITER *pIt, NvU32 classId) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NULL;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetNextChildOfTypeUnsafe(pGpu, pIt, classId) gpuGetNextChildOfTypeUnsafe_IMPL(pGpu, pIt, classId)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuGetHwDefaults(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuGetHwDefaults(pGpu) gpuGetHwDefaults_d44104(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetGfidState_IMPL(struct OBJGPU *pGpu, NvU32 gfid, GFID_ALLOC_STATUS *pState);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetGfidState(struct OBJGPU *pGpu, NvU32 gfid, GFID_ALLOC_STATUS *pState) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetGfidState(pGpu, gfid, pState) gpuGetGfidState_IMPL(pGpu, gfid, pState)
#endif // __nvoc_gpu_h_disabled

void gpuSetGfidUsage_IMPL(struct OBJGPU *pGpu, NvU32 gfid, NvBool bInUse);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuSetGfidUsage(struct OBJGPU *pGpu, NvU32 gfid, NvBool bInUse) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuSetGfidUsage(pGpu, gfid, bInUse) gpuSetGfidUsage_IMPL(pGpu, gfid, bInUse)
#endif // __nvoc_gpu_h_disabled

void gpuSetGfidInvalidated_IMPL(struct OBJGPU *pGpu, NvU32 gfid);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuSetGfidInvalidated(struct OBJGPU *pGpu, NvU32 gfid) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuSetGfidInvalidated(pGpu, gfid) gpuSetGfidInvalidated_IMPL(pGpu, gfid)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline RmPhysAddr gpuGetDmaEndAddress(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetDmaEndAddress(pGpu) gpuGetDmaEndAddress_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuReadPBusScratch(struct OBJGPU *pGpu, NvU8 index) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuReadPBusScratch(pGpu, index) gpuReadPBusScratch_395e98(pGpu, index)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuWritePBusScratch(struct OBJGPU *pGpu, NvU8 index, NvU32 data) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuWritePBusScratch(pGpu, index, data) gpuWritePBusScratch_d44104(pGpu, index, data)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSetResetScratchBit(struct OBJGPU *pGpu, NvBool bResetRequired) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSetResetScratchBit(pGpu, bResetRequired) gpuSetResetScratchBit_395e98(pGpu, bResetRequired)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetResetScratchBit(struct OBJGPU *pGpu, NvBool *pbResetRequired) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetResetScratchBit(pGpu, pbResetRequired) gpuGetResetScratchBit_395e98(pGpu, pbResetRequired)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSetStateResetRequired(struct OBJGPU *pGpu, NvU32 exceptType) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSetStateResetRequired(pGpu, exceptType) gpuSetStateResetRequired_395e98(pGpu, exceptType)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuResetRequiredStateChanged(struct OBJGPU *pGpu, NvBool newState) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuResetRequiredStateChanged(pGpu, newState) gpuResetRequiredStateChanged_FWCLIENT(pGpu, newState)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuMarkDeviceForReset(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuMarkDeviceForReset(pGpu) gpuMarkDeviceForReset_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuUnmarkDeviceForReset(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuUnmarkDeviceForReset(pGpu) gpuUnmarkDeviceForReset_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuIsDeviceMarkedForReset(struct OBJGPU *pGpu, NvBool *pbResetRequired) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsDeviceMarkedForReset(pGpu, pbResetRequired) gpuIsDeviceMarkedForReset_IMPL(pGpu, pbResetRequired)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSetDrainAndResetScratchBit(struct OBJGPU *pGpu, NvBool bDrainRecommended) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSetDrainAndResetScratchBit(pGpu, bDrainRecommended) gpuSetDrainAndResetScratchBit_395e98(pGpu, bDrainRecommended)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetDrainAndResetScratchBit(struct OBJGPU *pGpu, NvBool *pbDrainRecommended) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetDrainAndResetScratchBit(pGpu, pbDrainRecommended) gpuGetDrainAndResetScratchBit_395e98(pGpu, pbDrainRecommended)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuMarkDeviceForDrainAndReset(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuMarkDeviceForDrainAndReset(pGpu) gpuMarkDeviceForDrainAndReset_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuUnmarkDeviceForDrainAndReset(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuUnmarkDeviceForDrainAndReset(pGpu) gpuUnmarkDeviceForDrainAndReset_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuIsDeviceMarkedForDrainAndReset(struct OBJGPU *pGpu, NvBool *pbDrainRecommended) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsDeviceMarkedForDrainAndReset(pGpu, pbDrainRecommended) gpuIsDeviceMarkedForDrainAndReset_IMPL(pGpu, pbDrainRecommended)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuGetRecoveryAction(struct OBJGPU *pGpu, NV2080_CTRL_GPU_GET_RECOVERY_ACTION_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuGetRecoveryAction(pGpu, pParams) gpuGetRecoveryAction_IMPL(pGpu, pParams)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuRefreshRecoveryAction(struct OBJGPU *pGpu, NvBool inLock) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuRefreshRecoveryAction(pGpu, inLock) gpuRefreshRecoveryAction_d44104(pGpu, inLock)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuSetRecoveryDrainP2P(struct OBJGPU *pGpu, NvBool bDrainP2P) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuSetRecoveryDrainP2P(pGpu, bDrainP2P) gpuSetRecoveryDrainP2P_KERNEL(pGpu, bDrainP2P)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuUnmarkDeviceForDrainP2P(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuUnmarkDeviceForDrainP2P(pGpu) gpuUnmarkDeviceForDrainP2P_KERNEL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuLogOobXidMessage(struct OBJGPU *pGpu, NvU32 xid, const char *string, NvU32 len) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuLogOobXidMessage(pGpu, xid, string, len) gpuLogOobXidMessage_KERNEL(pGpu, xid, string, len)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuSetExternalKernelClientCount_IMPL(struct OBJGPU *pGpu, NvBool bIncr);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSetExternalKernelClientCount(struct OBJGPU *pGpu, NvBool bIncr) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSetExternalKernelClientCount(pGpu, bIncr) gpuSetExternalKernelClientCount_IMPL(pGpu, bIncr)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsInUse_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsInUse(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsInUse(pGpu) gpuIsInUse_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NvU32 gpuGetUserClientCount_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetUserClientCount(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetUserClientCount(pGpu) gpuGetUserClientCount_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NvU32 gpuGetExternalClientCount_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetExternalClientCount(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetExternalClientCount(pGpu) gpuGetExternalClientCount_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPrivSecInitRegistryOverrides(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPrivSecInitRegistryOverrides(pGpu) gpuPrivSecInitRegistryOverrides_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

void gpuNotifySubDeviceEvent_IMPL(struct OBJGPU *pGpu, NvU32 notifyIndex, void *pNotifyParams, NvU32 notifyParamsSize, NvV32 info32, NvV16 info16);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuNotifySubDeviceEvent(struct OBJGPU *pGpu, NvU32 notifyIndex, void *pNotifyParams, NvU32 notifyParamsSize, NvV32 info32, NvV16 info16) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuNotifySubDeviceEvent(pGpu, notifyIndex, pNotifyParams, notifyParamsSize, info32, info16) gpuNotifySubDeviceEvent_IMPL(pGpu, notifyIndex, pNotifyParams, notifyParamsSize, info32, info16)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuRegisterSubdevice_IMPL(struct OBJGPU *pGpu, struct Subdevice *pSubdevice);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuRegisterSubdevice(struct OBJGPU *pGpu, struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuRegisterSubdevice(pGpu, pSubdevice) gpuRegisterSubdevice_IMPL(pGpu, pSubdevice)
#endif // __nvoc_gpu_h_disabled

void gpuUnregisterSubdevice_IMPL(struct OBJGPU *pGpu, struct Subdevice *pSubdevice);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuUnregisterSubdevice(struct OBJGPU *pGpu, struct Subdevice *pSubdevice) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuUnregisterSubdevice(pGpu, pSubdevice) gpuUnregisterSubdevice_IMPL(pGpu, pSubdevice)
#endif // __nvoc_gpu_h_disabled

void gpuGspPluginTriggeredEvent_IMPL(struct OBJGPU *pGpu, NvU32 gfid, NvU32 notifyIndex);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuGspPluginTriggeredEvent(struct OBJGPU *pGpu, NvU32 gfid, NvU32 notifyIndex) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuGspPluginTriggeredEvent(pGpu, gfid, notifyIndex) gpuGspPluginTriggeredEvent_IMPL(pGpu, gfid, notifyIndex)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetProcWithObject_IMPL(struct OBJGPU *pGpu, NvU32 elementID, NvU32 internalClassId, NvU32 *pPidArray, NvU32 *pPidArrayCount, MIG_INSTANCE_REF *pRef);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetProcWithObject(struct OBJGPU *pGpu, NvU32 elementID, NvU32 internalClassId, NvU32 *pPidArray, NvU32 *pPidArrayCount, MIG_INSTANCE_REF *pRef) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetProcWithObject(pGpu, elementID, internalClassId, pPidArray, pPidArrayCount, pRef) gpuGetProcWithObject_IMPL(pGpu, elementID, internalClassId, pPidArray, pPidArrayCount, pRef)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuFindClientInfoWithPidIterator_IMPL(struct OBJGPU *pGpu, NvU32 pid, NvU32 subPid, NvU32 internalClassId, NV2080_CTRL_GPU_PID_INFO_DATA *pData, NV2080_CTRL_SMC_SUBSCRIPTION_INFO *pSmcInfo, MIG_INSTANCE_REF *pRef, NvBool bGlobalInfo);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuFindClientInfoWithPidIterator(struct OBJGPU *pGpu, NvU32 pid, NvU32 subPid, NvU32 internalClassId, NV2080_CTRL_GPU_PID_INFO_DATA *pData, NV2080_CTRL_SMC_SUBSCRIPTION_INFO *pSmcInfo, MIG_INSTANCE_REF *pRef, NvBool bGlobalInfo) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuFindClientInfoWithPidIterator(pGpu, pid, subPid, internalClassId, pData, pSmcInfo, pRef, bGlobalInfo) gpuFindClientInfoWithPidIterator_IMPL(pGpu, pid, subPid, internalClassId, pData, pSmcInfo, pRef, bGlobalInfo)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPowerOff(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPowerOff(pGpu) gpuPowerOff_KERNEL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPowerOn(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPowerOn(pGpu) gpuPowerOn_KERNEL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPowerOffHda(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPowerOffHda(pGpu) gpuPowerOffHda_395e98(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPowerOnHda(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPowerOnHda(pGpu) gpuPowerOnHda_395e98(pGpu)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsOnTheBus_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsOnTheBus(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsOnTheBus(pGpu) gpuIsOnTheBus_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuEnterStandby_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuEnterStandby(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuEnterStandby(pGpu) gpuEnterStandby_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuEnterHibernate_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuEnterHibernate(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuEnterHibernate(pGpu) gpuEnterHibernate_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuResumeFromStandby_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuResumeFromStandby(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuResumeFromStandby(pGpu) gpuResumeFromStandby_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuResumeFromHibernate_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuResumeFromHibernate(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuResumeFromHibernate(pGpu) gpuResumeFromHibernate_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NvBool gpuCheckSysmemAccess_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuCheckSysmemAccess(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuCheckSysmemAccess(pGpu) gpuCheckSysmemAccess_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuBootGspRmProxy_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuBootGspRmProxy(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuBootGspRmProxy(pGpu) gpuBootGspRmProxy_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsInstInSysBootByAcrEnabled_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsInstInSysBootByAcrEnabled(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsInstInSysBootByAcrEnabled(pGpu) gpuIsInstInSysBootByAcrEnabled_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsInstInSysBootByRmEnabled_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsInstInSysBootByRmEnabled(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsInstInSysBootByRmEnabled(pGpu) gpuIsInstInSysBootByRmEnabled_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetBusIntfType(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetBusIntfType(pGpu) gpuGetBusIntfType_f222ee(pGpu)
#endif // __nvoc_gpu_h_disabled

NvBool gpuIsPciBusFamily_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsPciBusFamily(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsPciBusFamily(pGpu) gpuIsPciBusFamily_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuWriteBusConfigReg(struct OBJGPU *pGpu, NvU32 index, NvU32 value) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuWriteBusConfigReg(pGpu, index, value) gpuWriteBusConfigReg_395e98(pGpu, index, value)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuReadBusConfigReg(struct OBJGPU *pGpu, NvU32 index, NvU32 *data) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuReadBusConfigReg(pGpu, index, data) gpuReadBusConfigReg_395e98(pGpu, index, data)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuReadBusConfigRegEx(struct OBJGPU *pGpu, NvU32 index, NvU32 *data, THREAD_STATE_NODE *pThreadState) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuReadBusConfigRegEx(pGpu, index, data, pThreadState) gpuReadBusConfigRegEx_5baef9(pGpu, index, data, pThreadState)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuReadFunctionConfigReg(struct OBJGPU *pGpu, NvU32 function, NvU32 reg, NvU32 *data) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuReadFunctionConfigReg(pGpu, function, reg, data) gpuReadFunctionConfigReg_5baef9(pGpu, function, reg, data)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuWriteFunctionConfigReg(struct OBJGPU *pGpu, NvU32 function, NvU32 reg, NvU32 data) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuWriteFunctionConfigReg(pGpu, function, reg, data) gpuWriteFunctionConfigReg_5baef9(pGpu, function, reg, data)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuWriteFunctionConfigRegEx(struct OBJGPU *pGpu, NvU32 function, NvU32 reg, NvU32 data, THREAD_STATE_NODE *pThreadState) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuWriteFunctionConfigRegEx(pGpu, function, reg, data, pThreadState) gpuWriteFunctionConfigRegEx_5baef9(pGpu, function, reg, data, pThreadState)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuReadPassThruConfigReg(struct OBJGPU *pGpu, NvU32 index, NvU32 *data) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuReadPassThruConfigReg(pGpu, index, data) gpuReadPassThruConfigReg_395e98(pGpu, index, data)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuConfigAccessSanityCheck(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuConfigAccessSanityCheck(pGpu) gpuConfigAccessSanityCheck_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuReadBusConfigCycle(struct OBJGPU *pGpu, NvU32 hwDefAddr, NvU32 *pData) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuReadBusConfigCycle(pGpu, hwDefAddr, pData) gpuReadBusConfigCycle_GM107(pGpu, hwDefAddr, pData)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuWriteBusConfigCycle(struct OBJGPU *pGpu, NvU32 hwDefAddr, NvU32 value) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuWriteBusConfigCycle(pGpu, hwDefAddr, value) gpuWriteBusConfigCycle_GM107(pGpu, hwDefAddr, value)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuReadPcieConfigCycle(struct OBJGPU *pGpu, NvU32 hwDefAddr, NvU32 *pData, NvU8 func) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuReadPcieConfigCycle(pGpu, hwDefAddr, pData, func) gpuReadPcieConfigCycle_395e98(pGpu, hwDefAddr, pData, func)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuWritePcieConfigCycle(struct OBJGPU *pGpu, NvU32 hwDefAddr, NvU32 value, NvU8 func) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuWritePcieConfigCycle(pGpu, hwDefAddr, value, func) gpuWritePcieConfigCycle_395e98(pGpu, hwDefAddr, value, func)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetPlatformPowerDomain(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetPlatformPowerDomain(pGpu) gpuGetPlatformPowerDomain_b2b553(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuUpdateIdInfo(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuUpdateIdInfo(pGpu) gpuUpdateIdInfo_d44104(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetDeviceIDList(struct OBJGPU *pGpu, DEVICE_ID_MAPPING **arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetDeviceIDList(pGpu, arg2) gpuGetDeviceIDList_b2b553(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGenGidData(struct OBJGPU *pGpu, NvU8 *pGidData, NvU32 gidSize, NvU32 gidFlags) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGenGidData(pGpu, pGidData, gidSize, gidFlags) gpuGenGidData_SOC(pGpu, pGidData, gidSize, gidFlags)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuValidateUuid(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuValidateUuid(pGpu) gpuValidateUuid_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGenUgidData(struct OBJGPU *pGpu, NvU8 *pGidData, NvU32 gidSize, NvU32 gidFlags, NvU32 ugpuId) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGenUgidData(pGpu, pGidData, gidSize, gidFlags, ugpuId) gpuGenUgidData_395e98(pGpu, pGidData, gidSize, gidFlags, ugpuId)
#endif // __nvoc_gpu_h_disabled

void gpuInitChipInfo_IMPL(struct OBJGPU *pGpu);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuInitChipInfo(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuInitChipInfo(pGpu) gpuInitChipInfo_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU8 gpuGetChipSubRev(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetChipSubRev(pGpu) gpuGetChipSubRev_FWCLIENT(pGpu)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuGetChipDetails_IMPL(struct OBJGPU *pGpu, NV2080_CTRL_GPU_GET_CHIP_DETAILS_PARAMS *arg2);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetChipDetails(struct OBJGPU *pGpu, NV2080_CTRL_GPU_GET_CHIP_DETAILS_PARAMS *arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetChipDetails(pGpu, arg2) gpuGetChipDetails_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetSkuInfo(struct OBJGPU *pGpu, NV2080_CTRL_BIOS_GET_SKU_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetSkuInfo(pGpu, pParams) gpuGetSkuInfo_92bfc3(pGpu, pParams)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetPdi(struct OBJGPU *pGpu, NvU64 *pPdi) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetPdi(pGpu, pPdi) gpuGetPdi_FWCLIENT(pGpu, pPdi)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuPerformUniversalValidation(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuPerformUniversalValidation(pGpu) gpuPerformUniversalValidation_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetVirtRegPhysOffset(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetVirtRegPhysOffset(pGpu) gpuGetVirtRegPhysOffset_b2b553(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetRegBaseOffset(struct OBJGPU *pGpu, NvU32 arg2, NvU32 *arg3) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetRegBaseOffset(pGpu, arg2, arg3) gpuGetRegBaseOffset_FWCLIENT(pGpu, arg2, arg3)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuSanityCheckRegRead_IMPL(struct OBJGPU *pGpu, NvU32 addr, NvU32 size, void *pValue);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSanityCheckRegRead(struct OBJGPU *pGpu, NvU32 addr, NvU32 size, void *pValue) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSanityCheckRegRead(pGpu, addr, size, pValue) gpuSanityCheckRegRead_IMPL(pGpu, addr, size, pValue)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuHandleSanityCheckRegReadError(struct OBJGPU *pGpu, NvU32 addr, NvU32 value) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuHandleSanityCheckRegReadError(pGpu, addr, value) gpuHandleSanityCheckRegReadError_d44104(pGpu, addr, value)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuHandleSecFault(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuHandleSecFault(pGpu) gpuHandleSecFault_d44104(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuGetSanityCheckRegReadError(struct OBJGPU *pGpu, NvU32 value, const char **pErrorString) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuGetSanityCheckRegReadError(pGpu, value, pErrorString) gpuGetSanityCheckRegReadError_d44104(pGpu, value, pErrorString)
#endif // __nvoc_gpu_h_disabled

NV_STATUS gpuSanityCheckRegisterAccess_IMPL(struct OBJGPU *pGpu, NvU32 addr, NvU32 *pRetVal);
#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSanityCheckRegisterAccess(struct OBJGPU *pGpu, NvU32 addr, NvU32 *pRetVal) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSanityCheckRegisterAccess(pGpu, addr, pRetVal) gpuSanityCheckRegisterAccess_IMPL(pGpu, addr, pRetVal)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSanityCheckVirtRegAccess(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSanityCheckVirtRegAccess(pGpu, arg2) gpuSanityCheckVirtRegAccess_ac1694(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuInitRegistryOverrides(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuInitRegistryOverrides(pGpu) gpuInitRegistryOverrides_KERNEL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuInitInstLocOverrides(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuInitInstLocOverrides(pGpu) gpuInitInstLocOverrides_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

const NvU32 * gpuGetGenericClassList_IMPL(struct OBJGPU *pGpu, NvU32 *arg2);
#ifdef __nvoc_gpu_h_disabled
static inline const NvU32 * gpuGetGenericClassList(struct OBJGPU *pGpu, NvU32 *arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NULL;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetGenericClassList(pGpu, arg2) gpuGetGenericClassList_IMPL(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuInitSriov(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuInitSriov(pGpu) gpuInitSriov_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuDeinitSriov(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuDeinitSriov(pGpu) gpuDeinitSriov_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuMnocMboxSyncRecv(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port, RMTIMEOUT *pTimeout, void *pMsgAddr, NvU32 *pMsgSize) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxSyncRecv(pGpu, pMboxAperture, port, pTimeout, pMsgAddr, pMsgSize) gpuMnocMboxSyncRecv_395e98(pGpu, pMboxAperture, port, pTimeout, pMsgAddr, pMsgSize)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuMnocMboxSend(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port, RMTIMEOUT *pTimeout, void *pMsgAddr, NvU32 msgSize) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxSend(pGpu, pMboxAperture, port, pTimeout, pMsgAddr, msgSize) gpuMnocMboxSend_395e98(pGpu, pMboxAperture, port, pTimeout, pMsgAddr, msgSize)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuMnocMboxRecv(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port, void *pMsgAddr, NvU32 *pMsgSize) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxRecv(pGpu, pMboxAperture, port, pMsgAddr, pMsgSize) gpuMnocMboxRecv_395e98(pGpu, pMboxAperture, port, pMsgAddr, pMsgSize)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuMnocMboxIsMsgAvailable(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxIsMsgAvailable(pGpu, pMboxAperture, port) gpuMnocMboxIsMsgAvailable_d69453(pGpu, pMboxAperture, port)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuMnocMboxInterruptEnable(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxInterruptEnable(pGpu, pMboxAperture, port) gpuMnocMboxInterruptEnable_d44104(pGpu, pMboxAperture, port)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuMnocMboxInterruptDisable(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxInterruptDisable(pGpu, pMboxAperture, port) gpuMnocMboxInterruptDisable_d44104(pGpu, pMboxAperture, port)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuMnocMboxInterruptRaised(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxInterruptRaised(pGpu, pMboxAperture, port) gpuMnocMboxInterruptRaised_d69453(pGpu, pMboxAperture, port)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuMnocMboxInterruptClear(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxInterruptClear(pGpu, pMboxAperture, port) gpuMnocMboxInterruptClear_d44104(pGpu, pMboxAperture, port)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuMnocMboxMinMessageSize(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxMinMessageSize(pGpu) gpuMnocMboxMinMessageSize_edd98b(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuMnocMboxMaxMessageSize(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuMnocMboxMaxMessageSize(pGpu) gpuMnocMboxMaxMessageSize_edd98b(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuCreateDefaultClientShare(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuCreateDefaultClientShare(pGpu) gpuCreateDefaultClientShare_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuDestroyDefaultClientShare(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuDestroyDefaultClientShare(pGpu) gpuDestroyDefaultClientShare_d44104(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuFuseSupportsDisplay(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuFuseSupportsDisplay(pGpu) gpuFuseSupportsDisplay_e661f0(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuGetTerminatedLinkMask(struct OBJGPU *pGpu, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuGetTerminatedLinkMask(pGpu, arg2) gpuGetTerminatedLinkMask_GA100(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuJtVersionSanityCheck(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuJtVersionSanityCheck(pGpu) gpuJtVersionSanityCheck_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuValidateRmctrlCmd(struct OBJGPU *pGpu, NvU32 cmd) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuValidateRmctrlCmd(pGpu, cmd) gpuValidateRmctrlCmd_T234D(pGpu, cmd)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuValidateBusInfoIndex(struct OBJGPU *pGpu, NvU32 index) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuValidateBusInfoIndex(pGpu, index) gpuValidateBusInfoIndex_T234D(pGpu, index)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsSystemRebootRequired(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsSystemRebootRequired(pGpu) gpuIsSystemRebootRequired_FWCLIENT(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuDetermineSelfHostedMode(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuDetermineSelfHostedMode(pGpu) gpuDetermineSelfHostedMode_d44104(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuDetermineSelfHostedSocType(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuDetermineSelfHostedSocType(pGpu) gpuDetermineSelfHostedSocType_6ac90c(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuValidateMIGSupport(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuValidateMIGSupport(pGpu) gpuValidateMIGSupport_KERNEL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuInitOptimusSettings(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuInitOptimusSettings(pGpu) gpuInitOptimusSettings_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuDeinitOptimusSettings(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuDeinitOptimusSettings(pGpu) gpuDeinitOptimusSettings_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSetCacheOnlyModeOverrides(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSetCacheOnlyModeOverrides(pGpu) gpuSetCacheOnlyModeOverrides_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuGetCeFaultMethodBufferSize(struct OBJGPU *arg_this, NvU32 *arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetCeFaultMethodBufferSize(arg_this, arg2) gpuGetCeFaultMethodBufferSize_KERNEL(arg_this, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSetVFBarSizes(struct OBJGPU *pGpu, NV0080_CTRL_GPU_SET_VGPU_VF_BAR1_SIZE_PARAMS *arg2) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSetVFBarSizes(pGpu, arg2) gpuSetVFBarSizes_395e98(pGpu, arg2)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsSliCapableWithoutDisplay(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsSliCapableWithoutDisplay(pGpu) gpuIsSliCapableWithoutDisplay_d69453(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsCtxBufAllocInPmaSupported(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsCtxBufAllocInPmaSupported(pGpu) gpuIsCtxBufAllocInPmaSupported_d69453(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuLoadFailurePathTest(struct OBJGPU *pGpu, NvU32 engStage, NvU32 engDescIdx, NvBool bStopTest) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuLoadFailurePathTest(pGpu, engStage, engDescIdx, bStopTest) gpuLoadFailurePathTest_ac1694(pGpu, engStage, engDescIdx, bStopTest)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline const NV_ERROR_CONT_STATE_TABLE * gpuGetErrorContStateTableAndSize(struct OBJGPU *pGpu, NvU32 *pTableSize) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NULL;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetErrorContStateTableAndSize(pGpu, pTableSize) gpuGetErrorContStateTableAndSize_80f438(pGpu, pTableSize)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuUpdateErrorContainmentState(struct OBJGPU *pGpu, NV_ERROR_CONT_ERR_ID arg2, NV_ERROR_CONT_LOCATION arg3, NvU32 *arg4) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuUpdateErrorContainmentState(pGpu, arg2, arg3, arg4) gpuUpdateErrorContainmentState_9de355(pGpu, arg2, arg3, arg4)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSetPartitionErrorAttribution(struct OBJGPU *pGpu, NV_ERROR_CONT_ERR_ID arg2, NV_ERROR_CONT_LOCATION arg3, NvU32 arg4) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSetPartitionErrorAttribution(pGpu, arg2, arg3, arg4) gpuSetPartitionErrorAttribution_5baef9(pGpu, arg2, arg3, arg4)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuCreateRusdMemory(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuCreateRusdMemory(pGpu) gpuCreateRusdMemory_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuHandleRusdPollingRegistry(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuHandleRusdPollingRegistry(pGpu) gpuHandleRusdPollingRegistry_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuResumeRusdPollingOnLoad(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuResumeRusdPollingOnLoad(pGpu) gpuResumeRusdPollingOnLoad_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuStopRusdPollingOnUnload(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuStopRusdPollingOnUnload(pGpu) gpuStopRusdPollingOnUnload_ac1694(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuRusdInitPhysical(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuRusdInitPhysical(pGpu) gpuRusdInitPhysical_KERNEL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline void gpuDestroyRusdMemory(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuDestroyRusdMemory(pGpu) gpuDestroyRusdMemory_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuCheckEccCounts(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuCheckEccCounts(pGpu) gpuCheckEccCounts_d69453(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuWaitForGfwBootComplete(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuWaitForGfwBootComplete(pGpu) gpuWaitForGfwBootComplete_5baef9(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetFirstAsyncLce(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetFirstAsyncLce(pGpu) gpuGetFirstAsyncLce_77d68d(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuIsInternalSkuFuseEnabled(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuIsInternalSkuFuseEnabled(pGpu) gpuIsInternalSkuFuseEnabled_d69453(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuRequireGrCePresence(struct OBJGPU *pGpu, ENGDESCRIPTOR engDesc, NvBool *pIsEngineRequired) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuRequireGrCePresence(pGpu, engDesc, pIsEngineRequired) gpuRequireGrCePresence_ac1694(pGpu, engDesc, pIsEngineRequired)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuSocGetSecureRegionInfo(struct OBJGPU *pGpu, NvU32 srIndex, NvU64 *pBase, NvU64 *pSize) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuSocGetSecureRegionInfo(pGpu, srIndex, pBase, pSize) gpuSocGetSecureRegionInfo_395e98(pGpu, srIndex, pBase, pSize)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuWaitForBarFirewallHal(struct OBJGPU *pGpu, NvU32 domain, NvU8 bus, NvU8 device, NvU8 function, NvU16 devId, NvU16 subsystemId) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuWaitForBarFirewallHal(pGpu, domain, bus, device, function, devId, subsystemId) gpuWaitForBarFirewallHal_e661f0(pGpu, domain, bus, device, function, devId, subsystemId)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetDefaultResetFSMStateTransitionUs(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetDefaultResetFSMStateTransitionUs(pGpu) gpuGetDefaultResetFSMStateTransitionUs_f6f3c4(pGpu)
#endif // __nvoc_gpu_h_disabled

void gpuSetRecoveryRebootRequired_IMPL(struct OBJGPU *pGpu, NvBool bRebootRequired, NvBool bBlockNewWorkload);
#ifdef __nvoc_gpu_h_disabled
static inline void gpuSetRecoveryRebootRequired(struct OBJGPU *pGpu, NvBool bRebootRequired, NvBool bBlockNewWorkload) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
}
#else // __nvoc_gpu_h_disabled
#define gpuSetRecoveryRebootRequired(pGpu, bRebootRequired, bBlockNewWorkload) gpuSetRecoveryRebootRequired_IMPL(pGpu, bRebootRequired, bBlockNewWorkload)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvBool gpuGetIsCmpSku(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_FALSE;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetIsCmpSku(pGpu) gpuGetIsCmpSku_72a2e1(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NV_STATUS gpuRusdRequestPermanentDataPoll(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_gpu_h_disabled
#define gpuRusdRequestPermanentDataPoll(pGpu) gpuRusdRequestPermanentDataPoll_IMPL(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetXtlBaseAddr(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetXtlBaseAddr(pGpu) gpuGetXtlBaseAddr_474d46(pGpu)
#endif // __nvoc_gpu_h_disabled

#ifdef __nvoc_gpu_h_disabled
static inline NvU32 gpuGetPrimaryFuseBaseAddr(struct OBJGPU *pGpu) {
    NV_ASSERT_FAILED_PRECOMP("OBJGPU was disabled!");
    return 0;
}
#else // __nvoc_gpu_h_disabled
#define gpuGetPrimaryFuseBaseAddr(pGpu) gpuGetPrimaryFuseBaseAddr_b2b553(pGpu)
#endif // __nvoc_gpu_h_disabled


// Wrapper macros for halified functions
#define gpuConstructPhysical_HAL(pGpu) gpuConstructPhysical(pGpu)
#define gpuDestructPhysical_HAL(pGpu) gpuDestructPhysical(pGpu)
#define gpuStatePreInit_HAL(pGpu) gpuStatePreInit(pGpu)
#define gpuStateLoad_HAL(pGpu, arg2) gpuStateLoad(pGpu, arg2)
#define gpuStateDestroy_HAL(pGpu) gpuStateDestroy(pGpu)
#define gpuStateInitStartedSatisfy_HAL(pGpu) gpuStateInitStartedSatisfy(pGpu)
#define gpuStateInitStartedRetract_HAL(pGpu) gpuStateInitStartedRetract(pGpu)
#define gpuPowerManagementEnterPreUnloadPhysical_HAL(pGpu) gpuPowerManagementEnterPreUnloadPhysical(pGpu)
#define gpuPowerManagementEnterPostUnloadPhysical_HAL(pGpu, newLevel) gpuPowerManagementEnterPostUnloadPhysical(pGpu, newLevel)
#define gpuPowerManagementResumePreLoadPhysical_HAL(pGpu, oldLevel, flags) gpuPowerManagementResumePreLoadPhysical(pGpu, oldLevel, flags)
#define gpuPowerManagementResumePostLoadPhysical_HAL(pGpu) gpuPowerManagementResumePostLoadPhysical(pGpu)
#define gpuConstructDeviceInfoTable_HAL(pGpu) gpuConstructDeviceInfoTable(pGpu)
#define gpuInitializeMemDescFromPromotedCtx_HAL(pGpu, ppMemDesc, gpuPhysAddr, size, physAttr, bIsCallingContextVgpuPlugin) gpuInitializeMemDescFromPromotedCtx(pGpu, ppMemDesc, gpuPhysAddr, size, physAttr, bIsCallingContextVgpuPlugin)
#define gpuGetNameString_HAL(pGpu, arg2) gpuGetNameString(pGpu, arg2)
#define gpuGetShortNameString_HAL(pGpu, arg2) gpuGetShortNameString(pGpu, arg2)
#define gpuCheckEngine_HAL(pGpu, desc) gpuCheckEngine(pGpu, desc)
#define gpuIsSocSdmEnabled_HAL(pGpu) gpuIsSocSdmEnabled(pGpu)
#define gpuCheckEngineWithOrderList_HAL(pGpu, desc, bCheckEngineOrder) gpuCheckEngineWithOrderList(pGpu, desc, bCheckEngineOrder)
#define gpuIsSliLinkSupported_HAL(pGpu) gpuIsSliLinkSupported(pGpu)
#define gpuSetThreadBcState_HAL(pGpu, arg2) gpuSetThreadBcState(pGpu, arg2)
#define gpuDeterminePersistantIllumSettings_HAL(pGpu) gpuDeterminePersistantIllumSettings(pGpu)
#define gpuInitSliIllumination_HAL(pGpu) gpuInitSliIllumination(pGpu)
#define gpuBuildGenericKernelFalconList_HAL(pGpu) gpuBuildGenericKernelFalconList(pGpu)
#define gpuDestroyGenericKernelFalconList_HAL(pGpu) gpuDestroyGenericKernelFalconList(pGpu)
#define gpuBuildKernelVideoEngineList_HAL(pGpu) gpuBuildKernelVideoEngineList(pGpu)
#define gpuInitVideoLogging_HAL(pGpu) gpuInitVideoLogging(pGpu)
#define gpuFreeVideoLogging_HAL(pGpu) gpuFreeVideoLogging(pGpu)
#define gpuDestroyKernelVideoEngineList_HAL(pGpu) gpuDestroyKernelVideoEngineList(pGpu)
#define gpuGetGenericKernelFalconForEngine_HAL(pGpu, arg2) gpuGetGenericKernelFalconForEngine(pGpu, arg2)
#define gpuRegisterGenericKernelFalconIntrService_HAL(pGpu, pRecords) gpuRegisterGenericKernelFalconIntrService(pGpu, pRecords)
#define gpuGetHwDefaults_HAL(pGpu) gpuGetHwDefaults(pGpu)
#define gpuGetDmaEndAddress_HAL(pGpu) gpuGetDmaEndAddress(pGpu)
#define gpuReadPBusScratch_HAL(pGpu, index) gpuReadPBusScratch(pGpu, index)
#define gpuWritePBusScratch_HAL(pGpu, index, data) gpuWritePBusScratch(pGpu, index, data)
#define gpuSetResetScratchBit_HAL(pGpu, bResetRequired) gpuSetResetScratchBit(pGpu, bResetRequired)
#define gpuGetResetScratchBit_HAL(pGpu, pbResetRequired) gpuGetResetScratchBit(pGpu, pbResetRequired)
#define gpuSetStateResetRequired_HAL(pGpu, exceptType) gpuSetStateResetRequired(pGpu, exceptType)
#define gpuResetRequiredStateChanged_HAL(pGpu, newState) gpuResetRequiredStateChanged(pGpu, newState)
#define gpuMarkDeviceForReset_HAL(pGpu) gpuMarkDeviceForReset(pGpu)
#define gpuUnmarkDeviceForReset_HAL(pGpu) gpuUnmarkDeviceForReset(pGpu)
#define gpuIsDeviceMarkedForReset_HAL(pGpu, pbResetRequired) gpuIsDeviceMarkedForReset(pGpu, pbResetRequired)
#define gpuSetDrainAndResetScratchBit_HAL(pGpu, bDrainRecommended) gpuSetDrainAndResetScratchBit(pGpu, bDrainRecommended)
#define gpuGetDrainAndResetScratchBit_HAL(pGpu, pbDrainRecommended) gpuGetDrainAndResetScratchBit(pGpu, pbDrainRecommended)
#define gpuMarkDeviceForDrainAndReset_HAL(pGpu) gpuMarkDeviceForDrainAndReset(pGpu)
#define gpuUnmarkDeviceForDrainAndReset_HAL(pGpu) gpuUnmarkDeviceForDrainAndReset(pGpu)
#define gpuIsDeviceMarkedForDrainAndReset_HAL(pGpu, pbDrainRecommended) gpuIsDeviceMarkedForDrainAndReset(pGpu, pbDrainRecommended)
#define gpuGetRecoveryAction_HAL(pGpu, pParams) gpuGetRecoveryAction(pGpu, pParams)
#define gpuRefreshRecoveryAction_HAL(pGpu, inLock) gpuRefreshRecoveryAction(pGpu, inLock)
#define gpuSetRecoveryDrainP2P_HAL(pGpu, bDrainP2P) gpuSetRecoveryDrainP2P(pGpu, bDrainP2P)
#define gpuUnmarkDeviceForDrainP2P_HAL(pGpu) gpuUnmarkDeviceForDrainP2P(pGpu)
#define gpuLogOobXidMessage_HAL(pGpu, xid, string, len) gpuLogOobXidMessage(pGpu, xid, string, len)
#define gpuPrivSecInitRegistryOverrides_HAL(pGpu) gpuPrivSecInitRegistryOverrides(pGpu)
#define gpuPowerOff_HAL(pGpu) gpuPowerOff(pGpu)
#define gpuPowerOn_HAL(pGpu) gpuPowerOn(pGpu)
#define gpuPowerOffHda_HAL(pGpu) gpuPowerOffHda(pGpu)
#define gpuPowerOnHda_HAL(pGpu) gpuPowerOnHda(pGpu)
#define gpuGetBusIntfType_HAL(pGpu) gpuGetBusIntfType(pGpu)
#define gpuWriteBusConfigReg_HAL(pGpu, index, value) gpuWriteBusConfigReg(pGpu, index, value)
#define gpuReadBusConfigReg_HAL(pGpu, index, data) gpuReadBusConfigReg(pGpu, index, data)
#define gpuReadBusConfigRegEx_HAL(pGpu, index, data, pThreadState) gpuReadBusConfigRegEx(pGpu, index, data, pThreadState)
#define gpuReadFunctionConfigReg_HAL(pGpu, function, reg, data) gpuReadFunctionConfigReg(pGpu, function, reg, data)
#define gpuWriteFunctionConfigReg_HAL(pGpu, function, reg, data) gpuWriteFunctionConfigReg(pGpu, function, reg, data)
#define gpuWriteFunctionConfigRegEx_HAL(pGpu, function, reg, data, pThreadState) gpuWriteFunctionConfigRegEx(pGpu, function, reg, data, pThreadState)
#define gpuReadPassThruConfigReg_HAL(pGpu, index, data) gpuReadPassThruConfigReg(pGpu, index, data)
#define gpuConfigAccessSanityCheck_HAL(pGpu) gpuConfigAccessSanityCheck(pGpu)
#define gpuReadBusConfigCycle_HAL(pGpu, hwDefAddr, pData) gpuReadBusConfigCycle(pGpu, hwDefAddr, pData)
#define gpuWriteBusConfigCycle_HAL(pGpu, hwDefAddr, value) gpuWriteBusConfigCycle(pGpu, hwDefAddr, value)
#define gpuReadPcieConfigCycle_HAL(pGpu, hwDefAddr, pData, func) gpuReadPcieConfigCycle(pGpu, hwDefAddr, pData, func)
#define gpuWritePcieConfigCycle_HAL(pGpu, hwDefAddr, value, func) gpuWritePcieConfigCycle(pGpu, hwDefAddr, value, func)
#define gpuGetPlatformPowerDomain_HAL(pGpu) gpuGetPlatformPowerDomain(pGpu)
#define gpuGetIdInfo_FNPTR(pGpu) pGpu->__gpuGetIdInfo__
#define gpuGetIdInfo(pGpu) gpuGetIdInfo_DISPATCH(pGpu)
#define gpuGetIdInfo_HAL(pGpu) gpuGetIdInfo_DISPATCH(pGpu)
#define gpuUpdateIdInfo_HAL(pGpu) gpuUpdateIdInfo(pGpu)
#define gpuGetDeviceIDList_HAL(pGpu, arg2) gpuGetDeviceIDList(pGpu, arg2)
#define gpuGenGidData_HAL(pGpu, pGidData, gidSize, gidFlags) gpuGenGidData(pGpu, pGidData, gidSize, gidFlags)
#define gpuValidateUuid_HAL(pGpu) gpuValidateUuid(pGpu)
#define gpuGenUgidData_HAL(pGpu, pGidData, gidSize, gidFlags, ugpuId) gpuGenUgidData(pGpu, pGidData, gidSize, gidFlags, ugpuId)
#define gpuGetChipSubRev_HAL(pGpu) gpuGetChipSubRev(pGpu)
#define gpuGetSkuInfo_HAL(pGpu, pParams) gpuGetSkuInfo(pGpu, pParams)
#define gpuGetPdi_HAL(pGpu, pPdi) gpuGetPdi(pGpu, pPdi)
#define gpuPerformUniversalValidation_HAL(pGpu) gpuPerformUniversalValidation(pGpu)
#define gpuGetVirtRegPhysOffset_HAL(pGpu) gpuGetVirtRegPhysOffset(pGpu)
#define gpuGetRegBaseOffset_HAL(pGpu, arg2, arg3) gpuGetRegBaseOffset(pGpu, arg2, arg3)
#define gpuHandleSanityCheckRegReadError_HAL(pGpu, addr, value) gpuHandleSanityCheckRegReadError(pGpu, addr, value)
#define gpuHandleSecFault_HAL(pGpu) gpuHandleSecFault(pGpu)
#define gpuGetSanityCheckRegReadError_HAL(pGpu, value, pErrorString) gpuGetSanityCheckRegReadError(pGpu, value, pErrorString)
#define gpuSanityCheckVirtRegAccess_HAL(pGpu, arg2) gpuSanityCheckVirtRegAccess(pGpu, arg2)
#define gpuInitRegistryOverrides_HAL(pGpu) gpuInitRegistryOverrides(pGpu)
#define gpuInitInstLocOverrides_HAL(pGpu) gpuInitInstLocOverrides(pGpu)
#define gpuGetChildrenOrder_FNPTR(pGpu) pGpu->__gpuGetChildrenOrder__
#define gpuGetChildrenOrder(pGpu, pNumEntries) gpuGetChildrenOrder_DISPATCH(pGpu, pNumEntries)
#define gpuGetChildrenOrder_HAL(pGpu, pNumEntries) gpuGetChildrenOrder_DISPATCH(pGpu, pNumEntries)
#define gpuGetChildrenPresent_FNPTR(pGpu) pGpu->__gpuGetChildrenPresent__
#define gpuGetChildrenPresent(pGpu, pNumEntries) gpuGetChildrenPresent_DISPATCH(pGpu, pNumEntries)
#define gpuGetChildrenPresent_HAL(pGpu, pNumEntries) gpuGetChildrenPresent_DISPATCH(pGpu, pNumEntries)
#define gpuGetEngClassDescriptorList_FNPTR(pGpu) pGpu->__gpuGetEngClassDescriptorList__
#define gpuGetEngClassDescriptorList(pGpu, arg2) gpuGetEngClassDescriptorList_DISPATCH(pGpu, arg2)
#define gpuGetEngClassDescriptorList_HAL(pGpu, arg2) gpuGetEngClassDescriptorList_DISPATCH(pGpu, arg2)
#define gpuGetNoEngClassList_FNPTR(pGpu) pGpu->__gpuGetNoEngClassList__
#define gpuGetNoEngClassList(pGpu, arg2) gpuGetNoEngClassList_DISPATCH(pGpu, arg2)
#define gpuGetNoEngClassList_HAL(pGpu, arg2) gpuGetNoEngClassList_DISPATCH(pGpu, arg2)
#define gpuInitSriov_HAL(pGpu) gpuInitSriov(pGpu)
#define gpuDeinitSriov_HAL(pGpu) gpuDeinitSriov(pGpu)
#define gpuMnocMboxSyncRecv_HAL(pGpu, pMboxAperture, port, pTimeout, pMsgAddr, pMsgSize) gpuMnocMboxSyncRecv(pGpu, pMboxAperture, port, pTimeout, pMsgAddr, pMsgSize)
#define gpuMnocMboxSend_HAL(pGpu, pMboxAperture, port, pTimeout, pMsgAddr, msgSize) gpuMnocMboxSend(pGpu, pMboxAperture, port, pTimeout, pMsgAddr, msgSize)
#define gpuMnocMboxRecv_HAL(pGpu, pMboxAperture, port, pMsgAddr, pMsgSize) gpuMnocMboxRecv(pGpu, pMboxAperture, port, pMsgAddr, pMsgSize)
#define gpuMnocMboxIsMsgAvailable_HAL(pGpu, pMboxAperture, port) gpuMnocMboxIsMsgAvailable(pGpu, pMboxAperture, port)
#define gpuMnocMboxInterruptEnable_HAL(pGpu, pMboxAperture, port) gpuMnocMboxInterruptEnable(pGpu, pMboxAperture, port)
#define gpuMnocMboxInterruptDisable_HAL(pGpu, pMboxAperture, port) gpuMnocMboxInterruptDisable(pGpu, pMboxAperture, port)
#define gpuMnocMboxInterruptRaised_HAL(pGpu, pMboxAperture, port) gpuMnocMboxInterruptRaised(pGpu, pMboxAperture, port)
#define gpuMnocMboxInterruptClear_HAL(pGpu, pMboxAperture, port) gpuMnocMboxInterruptClear(pGpu, pMboxAperture, port)
#define gpuMnocMboxMinMessageSize_HAL(pGpu) gpuMnocMboxMinMessageSize(pGpu)
#define gpuMnocMboxMaxMessageSize_HAL(pGpu) gpuMnocMboxMaxMessageSize(pGpu)
#define gpuCreateDefaultClientShare_HAL(pGpu) gpuCreateDefaultClientShare(pGpu)
#define gpuDestroyDefaultClientShare_HAL(pGpu) gpuDestroyDefaultClientShare(pGpu)
#define gpuFuseSupportsDisplay_HAL(pGpu) gpuFuseSupportsDisplay(pGpu)
#define gpuGetTerminatedLinkMask_HAL(pGpu, arg2) gpuGetTerminatedLinkMask(pGpu, arg2)
#define gpuJtVersionSanityCheck_HAL(pGpu) gpuJtVersionSanityCheck(pGpu)
#define gpuValidateRmctrlCmd_HAL(pGpu, cmd) gpuValidateRmctrlCmd(pGpu, cmd)
#define gpuValidateBusInfoIndex_HAL(pGpu, index) gpuValidateBusInfoIndex(pGpu, index)
#define gpuIsSystemRebootRequired_HAL(pGpu) gpuIsSystemRebootRequired(pGpu)
#define gpuDetermineSelfHostedMode_HAL(pGpu) gpuDetermineSelfHostedMode(pGpu)
#define gpuDetermineSelfHostedSocType_HAL(pGpu) gpuDetermineSelfHostedSocType(pGpu)
#define gpuValidateMIGSupport_HAL(pGpu) gpuValidateMIGSupport(pGpu)
#define gpuInitOptimusSettings_HAL(pGpu) gpuInitOptimusSettings(pGpu)
#define gpuDeinitOptimusSettings_HAL(pGpu) gpuDeinitOptimusSettings(pGpu)
#define gpuSetCacheOnlyModeOverrides_HAL(pGpu) gpuSetCacheOnlyModeOverrides(pGpu)
#define gpuGetCeFaultMethodBufferSize_HAL(arg_this, arg2) gpuGetCeFaultMethodBufferSize(arg_this, arg2)
#define gpuSetVFBarSizes_HAL(pGpu, arg2) gpuSetVFBarSizes(pGpu, arg2)
#define gpuIsSliCapableWithoutDisplay_HAL(pGpu) gpuIsSliCapableWithoutDisplay(pGpu)
#define gpuIsCtxBufAllocInPmaSupported_HAL(pGpu) gpuIsCtxBufAllocInPmaSupported(pGpu)
#define gpuLoadFailurePathTest_HAL(pGpu, engStage, engDescIdx, bStopTest) gpuLoadFailurePathTest(pGpu, engStage, engDescIdx, bStopTest)
#define gpuGetErrorContStateTableAndSize_HAL(pGpu, pTableSize) gpuGetErrorContStateTableAndSize(pGpu, pTableSize)
#define gpuUpdateErrorContainmentState_HAL(pGpu, arg2, arg3, arg4) gpuUpdateErrorContainmentState(pGpu, arg2, arg3, arg4)
#define gpuSetPartitionErrorAttribution_HAL(pGpu, arg2, arg3, arg4) gpuSetPartitionErrorAttribution(pGpu, arg2, arg3, arg4)
#define gpuCreateRusdMemory_HAL(pGpu) gpuCreateRusdMemory(pGpu)
#define gpuHandleRusdPollingRegistry_HAL(pGpu) gpuHandleRusdPollingRegistry(pGpu)
#define gpuResumeRusdPollingOnLoad_HAL(pGpu) gpuResumeRusdPollingOnLoad(pGpu)
#define gpuStopRusdPollingOnUnload_HAL(pGpu) gpuStopRusdPollingOnUnload(pGpu)
#define gpuRusdInitPhysical_HAL(pGpu) gpuRusdInitPhysical(pGpu)
#define gpuDestroyRusdMemory_HAL(pGpu) gpuDestroyRusdMemory(pGpu)
#define gpuCheckEccCounts_HAL(pGpu) gpuCheckEccCounts(pGpu)
#define gpuWaitForGfwBootComplete_HAL(pGpu) gpuWaitForGfwBootComplete(pGpu)
#define gpuGetFirstAsyncLce_HAL(pGpu) gpuGetFirstAsyncLce(pGpu)
#define gpuIsInternalSkuFuseEnabled_HAL(pGpu) gpuIsInternalSkuFuseEnabled(pGpu)
#define gpuRequireGrCePresence_HAL(pGpu, engDesc, pIsEngineRequired) gpuRequireGrCePresence(pGpu, engDesc, pIsEngineRequired)
#define gpuSocGetSecureRegionInfo_HAL(pGpu, srIndex, pBase, pSize) gpuSocGetSecureRegionInfo(pGpu, srIndex, pBase, pSize)
#define gpuWaitForBarFirewallHal_HAL(pGpu, domain, bus, device, function, devId, subsystemId) gpuWaitForBarFirewallHal(pGpu, domain, bus, device, function, devId, subsystemId)
#define gpuGetDefaultResetFSMStateTransitionUs_HAL(pGpu) gpuGetDefaultResetFSMStateTransitionUs(pGpu)
#define gpuGetIsCmpSku_HAL(pGpu) gpuGetIsCmpSku(pGpu)
#define gpuRusdRequestPermanentDataPoll_HAL(pGpu) gpuRusdRequestPermanentDataPoll(pGpu)
#define gpuGetXtlBaseAddr_HAL(pGpu) gpuGetXtlBaseAddr(pGpu)
#define gpuGetPrimaryFuseBaseAddr_HAL(pGpu) gpuGetPrimaryFuseBaseAddr(pGpu)

// Dispatch functions
static inline void gpuGetIdInfo_DISPATCH(struct OBJGPU *pGpu) {
    pGpu->__gpuGetIdInfo__(pGpu);
}

static inline const GPUCHILDORDER * gpuGetChildrenOrder_DISPATCH(struct OBJGPU *pGpu, NvU32 *pNumEntries) {
    return pGpu->__gpuGetChildrenOrder__(pGpu, pNumEntries);
}

static inline const GPUCHILDPRESENT * gpuGetChildrenPresent_DISPATCH(struct OBJGPU *pGpu, NvU32 *pNumEntries) {
    return pGpu->__gpuGetChildrenPresent__(pGpu, pNumEntries);
}

static inline const CLASSDESCRIPTOR * gpuGetEngClassDescriptorList_DISPATCH(struct OBJGPU *pGpu, NvU32 *arg2) {
    return pGpu->__gpuGetEngClassDescriptorList__(pGpu, arg2);
}

static inline const NvU32 * gpuGetNoEngClassList_DISPATCH(struct OBJGPU *pGpu, NvU32 *arg2) {
    return pGpu->__gpuGetNoEngClassList__(pGpu, arg2);
}

// Virtual method declarations and/or inline definitions
// Exported method declarations and/or inline definitions
// HAL method declarations without bodies
NV_STATUS gpuStatePreInit_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuStateLoad_IMPL(struct OBJGPU *pGpu, NvU32 arg2);

NV_STATUS gpuStateDestroy_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuConstructDeviceInfoTable_FWCLIENT(struct OBJGPU *pGpu);

NV_STATUS gpuGetNameString_T234D(struct OBJGPU *pGpu, void *arg2);

NV_STATUS gpuGetShortNameString_T234D(struct OBJGPU *pGpu, NvU8 *arg2);

NvBool gpuCheckEngineWithOrderList_KERNEL(struct OBJGPU *pGpu, ENGDESCRIPTOR desc, NvBool bCheckEngineOrder);

NvBool gpuIsSliLinkSupported_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuBuildGenericKernelFalconList_IMPL(struct OBJGPU *pGpu);

void gpuDestroyGenericKernelFalconList_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuBuildKernelVideoEngineList_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuInitVideoLogging_IMPL(struct OBJGPU *pGpu);

void gpuFreeVideoLogging_IMPL(struct OBJGPU *pGpu);

void gpuDestroyKernelVideoEngineList_IMPL(struct OBJGPU *pGpu);

struct GenericKernelFalcon * gpuGetGenericKernelFalconForEngine_IMPL(struct OBJGPU *pGpu, ENGDESCRIPTOR arg2);

void gpuRegisterGenericKernelFalconIntrService_IMPL(struct OBJGPU *pGpu, void *pRecords);

RmPhysAddr gpuGetDmaEndAddress_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuResetRequiredStateChanged_FWCLIENT(struct OBJGPU *pGpu, NvBool newState);

NV_STATUS gpuMarkDeviceForReset_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuUnmarkDeviceForReset_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuIsDeviceMarkedForReset_IMPL(struct OBJGPU *pGpu, NvBool *pbResetRequired);

NV_STATUS gpuMarkDeviceForDrainAndReset_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuUnmarkDeviceForDrainAndReset_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuIsDeviceMarkedForDrainAndReset_IMPL(struct OBJGPU *pGpu, NvBool *pbDrainRecommended);

void gpuGetRecoveryAction_IMPL(struct OBJGPU *pGpu, NV2080_CTRL_GPU_GET_RECOVERY_ACTION_PARAMS *pParams);

void gpuSetRecoveryDrainP2P_KERNEL(struct OBJGPU *pGpu, NvBool bDrainP2P);

void gpuUnmarkDeviceForDrainP2P_KERNEL(struct OBJGPU *pGpu);

void gpuLogOobXidMessage_KERNEL(struct OBJGPU *pGpu, NvU32 xid, const char *string, NvU32 len);

NV_STATUS gpuPowerOff_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuPowerOff_KERNEL(struct OBJGPU *pGpu);

NV_STATUS gpuPowerOn_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuPowerOn_KERNEL(struct OBJGPU *pGpu);

NV_STATUS gpuReadBusConfigCycle_GM107(struct OBJGPU *pGpu, NvU32 hwDefAddr, NvU32 *pData);

NV_STATUS gpuWriteBusConfigCycle_GM107(struct OBJGPU *pGpu, NvU32 hwDefAddr, NvU32 value);

void gpuGetIdInfo_T234D(struct OBJGPU *pGpu);

void gpuGetIdInfo_T264D(struct OBJGPU *pGpu);

void gpuGetIdInfo_T256D(struct OBJGPU *pGpu);

NV_STATUS gpuGenGidData_SOC(struct OBJGPU *pGpu, NvU8 *pGidData, NvU32 gidSize, NvU32 gidFlags);

NvU8 gpuGetChipSubRev_FWCLIENT(struct OBJGPU *pGpu);

NV_STATUS gpuGetPdi_FWCLIENT(struct OBJGPU *pGpu, NvU64 *pPdi);

NV_STATUS gpuGetRegBaseOffset_FWCLIENT(struct OBJGPU *pGpu, NvU32 arg2, NvU32 *arg3);

NV_STATUS gpuInitRegistryOverrides_KERNEL(struct OBJGPU *pGpu);

NV_STATUS gpuInitInstLocOverrides_IMPL(struct OBJGPU *pGpu);

const GPUCHILDORDER * gpuGetChildrenOrder_T256D(struct OBJGPU *pGpu, NvU32 *pNumEntries);

const GPUCHILDORDER * gpuGetChildrenOrder_T234D(struct OBJGPU *pGpu, NvU32 *pNumEntries);

const GPUCHILDPRESENT * gpuGetChildrenPresent_T256D(struct OBJGPU *pGpu, NvU32 *pNumEntries);

const GPUCHILDPRESENT * gpuGetChildrenPresent_T234D(struct OBJGPU *pGpu, NvU32 *pNumEntries);

const CLASSDESCRIPTOR * gpuGetEngClassDescriptorList_T234D(struct OBJGPU *pGpu, NvU32 *arg2);

const CLASSDESCRIPTOR * gpuGetEngClassDescriptorList_T264D(struct OBJGPU *pGpu, NvU32 *arg2);

const CLASSDESCRIPTOR * gpuGetEngClassDescriptorList_T256D(struct OBJGPU *pGpu, NvU32 *arg2);

const NvU32 * gpuGetNoEngClassList_T234D(struct OBJGPU *pGpu, NvU32 *arg2);

const NvU32 * gpuGetNoEngClassList_T264D(struct OBJGPU *pGpu, NvU32 *arg2);

const NvU32 * gpuGetNoEngClassList_T256D(struct OBJGPU *pGpu, NvU32 *arg2);

void gpuGetTerminatedLinkMask_GA100(struct OBJGPU *pGpu, NvU32 arg2);

NV_STATUS gpuValidateRmctrlCmd_T234D(struct OBJGPU *pGpu, NvU32 cmd);

NV_STATUS gpuValidateBusInfoIndex_T234D(struct OBJGPU *pGpu, NvU32 index);

NvBool gpuIsSystemRebootRequired_FWCLIENT(struct OBJGPU *pGpu);

NvBool gpuValidateMIGSupport_KERNEL(struct OBJGPU *pGpu);

NV_STATUS gpuInitOptimusSettings_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuDeinitOptimusSettings_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuGetCeFaultMethodBufferSize_KERNEL(struct OBJGPU *arg_this, NvU32 *arg2);

NV_STATUS gpuRusdInitPhysical_KERNEL(struct OBJGPU *pGpu);

void gpuDestroyRusdMemory_IMPL(struct OBJGPU *pGpu);

NV_STATUS gpuRusdRequestPermanentDataPoll_IMPL(struct OBJGPU *pGpu);

// Inline HAL method definitions
static inline NV_STATUS gpuConstructPhysical_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline void gpuDestructPhysical_d44104(struct OBJGPU *pGpu){
    return;
}

static inline NV_STATUS gpuStateInitStartedSatisfy_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline void gpuStateInitStartedRetract_d44104(struct OBJGPU *pGpu){
    return;
}

static inline NV_STATUS gpuPowerManagementEnterPreUnloadPhysical_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuPowerManagementEnterPostUnloadPhysical_ac1694(struct OBJGPU *pGpu, NvU32 newLevel){
    return NV_OK;
}

static inline NV_STATUS gpuPowerManagementResumePreLoadPhysical_ac1694(struct OBJGPU *pGpu, NvU32 oldLevel, NvU32 flags){
    return NV_OK;
}

static inline NV_STATUS gpuPowerManagementResumePostLoadPhysical_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuConstructDeviceInfoTable_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuInitializeMemDescFromPromotedCtx_395e98(struct OBJGPU *pGpu, MEMORY_DESCRIPTOR **ppMemDesc, NvU64 gpuPhysAddr, NvU64 size, NvU32 physAttr, NvBool bIsCallingContextVgpuPlugin){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvBool gpuCheckEngine_e661f0(struct OBJGPU *pGpu, ENGDESCRIPTOR desc){
    return NV_TRUE;
}

static inline NvBool gpuIsSocSdmEnabled_d69453(struct OBJGPU *pGpu){
    return NV_FALSE;
}

static inline void gpuSetThreadBcState_d44104(struct OBJGPU *pGpu, NvBool arg2){
    return;
}

static inline void gpuDeterminePersistantIllumSettings_d44104(struct OBJGPU *pGpu){
    return;
}

static inline NV_STATUS gpuInitSliIllumination_395e98(struct OBJGPU *pGpu){
    return NV_ERR_NOT_SUPPORTED;
}

static inline void gpuGetHwDefaults_d44104(struct OBJGPU *pGpu){
    return;
}

static inline NV_STATUS gpuReadPBusScratch_395e98(struct OBJGPU *pGpu, NvU8 index){
    return NV_ERR_NOT_SUPPORTED;
}

static inline void gpuWritePBusScratch_d44104(struct OBJGPU *pGpu, NvU8 index, NvU32 data){
    return;
}

static inline NV_STATUS gpuSetResetScratchBit_395e98(struct OBJGPU *pGpu, NvBool bResetRequired){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuGetResetScratchBit_395e98(struct OBJGPU *pGpu, NvBool *pbResetRequired){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuSetStateResetRequired_395e98(struct OBJGPU *pGpu, NvU32 exceptType){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuSetDrainAndResetScratchBit_395e98(struct OBJGPU *pGpu, NvBool bDrainRecommended){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuGetDrainAndResetScratchBit_395e98(struct OBJGPU *pGpu, NvBool *pbDrainRecommended){
    return NV_ERR_NOT_SUPPORTED;
}

static inline void gpuRefreshRecoveryAction_d44104(struct OBJGPU *pGpu, NvBool inLock){
    return;
}

static inline void gpuSetRecoveryDrainP2P_d44104(struct OBJGPU *pGpu, NvBool bDrainP2P){
    return;
}

static inline void gpuUnmarkDeviceForDrainP2P_d44104(struct OBJGPU *pGpu){
    return;
}

static inline NV_STATUS gpuPrivSecInitRegistryOverrides_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuPowerOffHda_395e98(struct OBJGPU *pGpu){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuPowerOnHda_395e98(struct OBJGPU *pGpu){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvU32 gpuGetBusIntfType_f222ee(struct OBJGPU *pGpu){
    return (8);
}

static inline NV_STATUS gpuWriteBusConfigReg_395e98(struct OBJGPU *pGpu, NvU32 index, NvU32 value){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuReadBusConfigReg_395e98(struct OBJGPU *pGpu, NvU32 index, NvU32 *data){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuReadBusConfigRegEx_5baef9(struct OBJGPU *pGpu, NvU32 index, NvU32 *data, THREAD_STATE_NODE *pThreadState){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NV_STATUS gpuReadFunctionConfigReg_5baef9(struct OBJGPU *pGpu, NvU32 function, NvU32 reg, NvU32 *data){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NV_STATUS gpuWriteFunctionConfigReg_5baef9(struct OBJGPU *pGpu, NvU32 function, NvU32 reg, NvU32 data){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NV_STATUS gpuWriteFunctionConfigRegEx_5baef9(struct OBJGPU *pGpu, NvU32 function, NvU32 reg, NvU32 data, THREAD_STATE_NODE *pThreadState){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NV_STATUS gpuReadPassThruConfigReg_395e98(struct OBJGPU *pGpu, NvU32 index, NvU32 *data){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuConfigAccessSanityCheck_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuReadPcieConfigCycle_395e98(struct OBJGPU *pGpu, NvU32 hwDefAddr, NvU32 *pData, NvU8 func){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuWritePcieConfigCycle_395e98(struct OBJGPU *pGpu, NvU32 hwDefAddr, NvU32 value, NvU8 func){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvU32 gpuGetPlatformPowerDomain_b2b553(struct OBJGPU *pGpu){
    return 0;
}

static inline void gpuUpdateIdInfo_d44104(struct OBJGPU *pGpu){
    return;
}

static inline NvU32 gpuGetDeviceIDList_b2b553(struct OBJGPU *pGpu, DEVICE_ID_MAPPING **arg2){
    return 0;
}

static inline NV_STATUS gpuValidateUuid_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuGenUgidData_395e98(struct OBJGPU *pGpu, NvU8 *pGidData, NvU32 gidSize, NvU32 gidFlags, NvU32 ugpuId){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvU8 gpuGetChipSubRev_b2b553(struct OBJGPU *pGpu){
    return 0;
}

static inline NV_STATUS gpuGetSkuInfo_92bfc3(struct OBJGPU *pGpu, NV2080_CTRL_BIOS_GET_SKU_INFO_PARAMS *pParams){
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuGetPdi_395e98(struct OBJGPU *pGpu, NvU64 *pPdi){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuPerformUniversalValidation_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NvU32 gpuGetVirtRegPhysOffset_b2b553(struct OBJGPU *pGpu){
    return 0;
}

static inline NV_STATUS gpuGetRegBaseOffset_395e98(struct OBJGPU *pGpu, NvU32 arg2, NvU32 *arg3){
    return NV_ERR_NOT_SUPPORTED;
}

static inline void gpuHandleSanityCheckRegReadError_d44104(struct OBJGPU *pGpu, NvU32 addr, NvU32 value){
    return;
}

static inline void gpuHandleSecFault_d44104(struct OBJGPU *pGpu){
    return;
}

static inline void gpuGetSanityCheckRegReadError_d44104(struct OBJGPU *pGpu, NvU32 value, const char **pErrorString){
    return;
}

static inline NV_STATUS gpuSanityCheckVirtRegAccess_ac1694(struct OBJGPU *pGpu, NvU32 arg2){
    return NV_OK;
}

static inline NV_STATUS gpuInitSriov_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuDeinitSriov_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuMnocMboxSyncRecv_395e98(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port, RMTIMEOUT *pTimeout, void *pMsgAddr, NvU32 *pMsgSize){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuMnocMboxSend_395e98(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port, RMTIMEOUT *pTimeout, void *pMsgAddr, NvU32 msgSize){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuMnocMboxRecv_395e98(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port, void *pMsgAddr, NvU32 *pMsgSize){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvBool gpuMnocMboxIsMsgAvailable_d69453(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port){
    return NV_FALSE;
}

static inline void gpuMnocMboxInterruptEnable_d44104(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port){
    return;
}

static inline void gpuMnocMboxInterruptDisable_d44104(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port){
    return;
}

static inline NvBool gpuMnocMboxInterruptRaised_d69453(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port){
    return NV_FALSE;
}

static inline void gpuMnocMboxInterruptClear_d44104(struct OBJGPU *pGpu, struct IoAperture *pMboxAperture, NvU32 port){
    return;
}

static inline NvU32 gpuMnocMboxMinMessageSize_edd98b(struct OBJGPU *pGpu){
    return 0U;
}

static inline NvU32 gpuMnocMboxMaxMessageSize_edd98b(struct OBJGPU *pGpu){
    return 0U;
}

static inline NV_STATUS gpuCreateDefaultClientShare_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline void gpuDestroyDefaultClientShare_d44104(struct OBJGPU *pGpu){
    return;
}

static inline NvBool gpuFuseSupportsDisplay_e661f0(struct OBJGPU *pGpu){
    return NV_TRUE;
}

static inline NV_STATUS gpuJtVersionSanityCheck_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline void gpuDetermineSelfHostedMode_d44104(struct OBJGPU *pGpu){
    return;
}

static inline NvU32 gpuDetermineSelfHostedSocType_6ac90c(struct OBJGPU *pGpu){
    return NV0000_CTRL_SYSTEM_SH_SOC_TYPE_NA;
}

static inline NvBool gpuValidateMIGSupport_72a2e1(struct OBJGPU *pGpu){
    NV_ASSERT_PRECOMP(0);
    return NV_FALSE;
}

static inline NV_STATUS gpuSetCacheOnlyModeOverrides_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuSetVFBarSizes_395e98(struct OBJGPU *pGpu, NV0080_CTRL_GPU_SET_VGPU_VF_BAR1_SIZE_PARAMS *arg2){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvBool gpuIsSliCapableWithoutDisplay_d69453(struct OBJGPU *pGpu){
    return NV_FALSE;
}

static inline NvBool gpuIsCtxBufAllocInPmaSupported_d69453(struct OBJGPU *pGpu){
    return NV_FALSE;
}

static inline NV_STATUS gpuLoadFailurePathTest_ac1694(struct OBJGPU *pGpu, NvU32 engStage, NvU32 engDescIdx, NvBool bStopTest){
    return NV_OK;
}

static inline const NV_ERROR_CONT_STATE_TABLE * gpuGetErrorContStateTableAndSize_80f438(struct OBJGPU *pGpu, NvU32 *pTableSize){
    NV_ASSERT_OR_RETURN_PRECOMP(0, ((void *)0));
}

static inline NV_STATUS gpuUpdateErrorContainmentState_9de355(struct OBJGPU *pGpu, NV_ERROR_CONT_ERR_ID arg2, NV_ERROR_CONT_LOCATION arg3, NvU32 *arg4){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_OK);
}

static inline NV_STATUS gpuSetPartitionErrorAttribution_5baef9(struct OBJGPU *pGpu, NV_ERROR_CONT_ERR_ID arg2, NV_ERROR_CONT_LOCATION arg3, NvU32 arg4){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NV_STATUS gpuCreateRusdMemory_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuHandleRusdPollingRegistry_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuResumeRusdPollingOnLoad_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuStopRusdPollingOnUnload_ac1694(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NvBool gpuCheckEccCounts_d69453(struct OBJGPU *pGpu){
    return NV_FALSE;
}

static inline NV_STATUS gpuWaitForGfwBootComplete_5baef9(struct OBJGPU *pGpu){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NvU32 gpuGetFirstAsyncLce_77d68d(struct OBJGPU *pGpu){
    return (11);
}

static inline NvBool gpuIsInternalSkuFuseEnabled_d69453(struct OBJGPU *pGpu){
    return NV_FALSE;
}

static inline NV_STATUS gpuRequireGrCePresence_ac1694(struct OBJGPU *pGpu, ENGDESCRIPTOR engDesc, NvBool *pIsEngineRequired){
    return NV_OK;
}

static inline NV_STATUS gpuSocGetSecureRegionInfo_395e98(struct OBJGPU *pGpu, NvU32 srIndex, NvU64 *pBase, NvU64 *pSize){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvBool gpuWaitForBarFirewallHal_e661f0(struct OBJGPU *pGpu, NvU32 domain, NvU8 bus, NvU8 device, NvU8 function, NvU16 devId, NvU16 subsystemId){
    return NV_TRUE;
}

static inline NvU32 gpuGetDefaultResetFSMStateTransitionUs_f6f3c4(struct OBJGPU *pGpu){
    return 10;
}

static inline NvBool gpuGetIsCmpSku_d69453(struct OBJGPU *pGpu){
    return NV_FALSE;
}

static inline NvBool gpuGetIsCmpSku_72a2e1(struct OBJGPU *pGpu){
    NV_ASSERT_PRECOMP(0);
    return NV_FALSE;
}

static inline NvU32 gpuGetXtlBaseAddr_474d46(struct OBJGPU *pGpu){
    NV_ASSERT_OR_RETURN_PRECOMP(0, 0);
}

static inline NvU32 gpuGetPrimaryFuseBaseAddr_b2b553(struct OBJGPU *pGpu){
    return 0;
}

// Static dispatch method declarations
// Static inline method definitions
static inline void gpuServiceInterruptsAllGpus(struct OBJGPU *pGpu){
    return;
}

static inline ENGDESCRIPTOR * gpuGetInitEngineDescriptors(struct OBJGPU *pGpu){
    return pGpu->engineOrder.pEngineInitDescriptors;
}

static inline ENGDESCRIPTOR * gpuGetLoadEngineDescriptors(struct OBJGPU *pGpu){
    return pGpu->engineOrder.pEngineLoadDescriptors;
}

static inline ENGDESCRIPTOR * gpuGetUnloadEngineDescriptors(struct OBJGPU *pGpu){
    return pGpu->engineOrder.pEngineUnloadDescriptors;
}

static inline ENGDESCRIPTOR * gpuGetDestroyEngineDescriptors(struct OBJGPU *pGpu){
    return pGpu->engineOrder.pEngineDestroyDescriptors;
}

static inline NvU32 gpuGetNumEngDescriptors(struct OBJGPU *pGpu){
    return pGpu->engineOrder.numEngineDescriptors;
}

static inline NvU32 gpuGetMode(struct OBJGPU *pGpu){
    return pGpu->computeModeRefCount > 0 ? 2 : 1;
}

static inline ACPI_DSM_FUNCTION gpuGetDispStatusHotplugFunc(struct OBJGPU *pGpu){
    return pGpu->acpi.dispStatusHotplugFunc;
}

static inline ACPI_DSM_FUNCTION gpuGetDispStatusConfigFunc(struct OBJGPU *pGpu){
    return pGpu->acpi.dispStatusConfigFunc;
}

static inline ACPI_DSM_FUNCTION gpuGetPerfPostPowerStateFunc(struct OBJGPU *pGpu){
    return pGpu->acpi.perfPostPowerStateFunc;
}

static inline ACPI_DSM_FUNCTION gpuGetStereo3dStateActiveFunc(struct OBJGPU *pGpu){
    return pGpu->acpi.stereo3dStateActiveFunc;
}

static inline NvU32 gpuGetPmcBoot0(struct OBJGPU *pGpu){
    return pGpu->chipId0;
}

static inline NV_STATUS gpuGetSparseTextureComputeMode(struct OBJGPU *pGpu, NvU32 *arg2, NvU32 *arg3, NvU32 *arg4){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuSetSparseTextureComputeMode(struct OBJGPU *pGpu, NvU32 arg2){
    return NV_ERR_NOT_SUPPORTED;
}

static inline struct OBJFIFO * gpuGetFifoShared(struct OBJGPU *pGpu){
    return ((void *)0);
}

static inline struct KernelFifo * gpuGetKernelFifoShared(struct OBJGPU *pGpu){
    return ((void *)0);
}

static inline struct OBJHOSTENG * gpuGetHosteng(struct OBJGPU *pGpu, ENGDESCRIPTOR arg2){
    return ((void *)0);
}

static inline NV_STATUS gpuConstructUserRegisterAccessMap(struct OBJGPU *pGpu){
    return NV_OK;
}

static inline NV_STATUS gpuInitRegisterAccessMap(struct OBJGPU *pGpu, NvU8 *arg2, NvU32 arg3, const NvU8 *arg4, const NvU32 arg5){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuSetUserRegisterAccessPermissions(struct OBJGPU *pGpu, NvU32 offset, NvU32 size, NvBool bAllow){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS gpuSetUserRegisterAccessPermissionsInBulk(struct OBJGPU *pGpu, const NvU32 *regOffsetsAndSizesArr, NvU32 arrSizeBytes, NvBool bAllow){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvBool gpuGetUserRegisterAccessPermissions(struct OBJGPU *pGpu, NvU32 offset){
    return NV_FALSE;
}

static inline void gpuDumpCallbackRegister(struct OBJGPU *pGpu){
    return;
}

static inline NvU32 gpuGetChipMajRev(struct OBJGPU *pGpu){
    return pGpu->chipInfo.pmcBoot42.majorRev;
}

static inline NvU32 gpuGetChipMinRev(struct OBJGPU *pGpu){
    return pGpu->chipInfo.pmcBoot42.minorRev;
}

static inline NvBool gpuIsMaskRevisionA01(struct OBJGPU *pGpu){
    return (gpuGetChipMajRev(pGpu) == 10) && (gpuGetChipMinRev(pGpu) == 1);
}

static inline NvU32 gpuGetChipImpl(struct OBJGPU *pGpu){
    return pGpu->chipInfo.implementationId;
}

static inline NvU32 gpuGetChipArch(struct OBJGPU *pGpu){
    return pGpu->chipInfo.platformId;
}

static inline NvU32 gpuGetChipId(struct OBJGPU *pGpu){
    return pGpu->chipInfo.chipId;
}

static inline NvU32 gpuGetChipMinExtRev(struct OBJGPU *pGpu){
    return pGpu->chipInfo.pmcBoot42.minorExtRev;
}

static inline NvU64 gpuGetVmmuSegmentSize(struct OBJGPU *pGpu){
    return pGpu->vmmuSegmentSize;
}

static inline NvU32 gpuGetNumChildren(struct OBJGPU *pGpu){
    return ((sizeof (pGpu->children.pChild) / sizeof ((pGpu->children.pChild)[0])));
}

static inline Dynamic * gpuGetChild(struct OBJGPU *pGpu, NvU32 idx){
    return pGpu->children.pChild[idx];
}

static inline const NV2080_CTRL_INTERNAL_GPU_GET_CHIP_INFO_PARAMS * gpuGetChipInfo(struct OBJGPU *pGpu){
    return pGpu->pChipInfo;
}

static inline NvBool gpuIsBar2MovedByVtd(struct OBJGPU *pGpu){
    return pGpu->bBar2MovedByVtd;
}

static inline NvBool gpuIsBar1Size64Bit(struct OBJGPU *pGpu){
    return pGpu->bBar1Is64Bit;
}

static inline NvBool gpuIsSurpriseRemovalSupported(struct OBJGPU *pGpu){
    return pGpu->bSurpriseRemovalSupported;
}

static inline NvBool gpuIsReplayableTraceEnabled(struct OBJGPU *pGpu){
    return pGpu->bReplayableTraceEnabled;
}

static inline NvBool gpuIsStateLoading(struct OBJGPU *pGpu){
    return pGpu->bStateLoading;
}

static inline NvBool gpuIsStateUnloading(struct OBJGPU *pGpu){
    return pGpu->bStateUnloading;
}

static inline NvBool gpuIsStateLoaded(struct OBJGPU *pGpu){
    return pGpu->bStateLoaded;
}

static inline NvBool gpuIsFullyConstructed(struct OBJGPU *pGpu){
    return pGpu->bFullyConstructed;
}

static inline NvBool gpuIsUnifiedMemorySpaceEnabled(struct OBJGPU *pGpu){
    return pGpu->bUnifiedMemorySpaceEnabled;
}

static inline NvBool gpuIsWarBug4040336Enabled(struct OBJGPU *pGpu){
    return pGpu->bBf3WarBug4040336Enabled;
}

static inline NvBool gpuIsSriovEnabled(struct OBJGPU *pGpu){
    return pGpu->bSriovEnabled;
}

static inline NvBool gpuIsCacheOnlyModeEnabled(struct OBJGPU *pGpu){
    return pGpu->bCacheOnlyMode;
}

static inline NvBool gpuIsSplitVasManagementServerClientRmEnabled(struct OBJGPU *pGpu){
    return pGpu->bSplitVasManagementServerClientRm;
}

static inline NvBool gpuIsWarBug200577889SriovHeavyEnabled(struct OBJGPU *pGpu){
    return pGpu->bWarBug200577889SriovHeavyEnabled;
}

static inline NvBool gpuIsPipelinedPteMemEnabled(struct OBJGPU *pGpu){
    return pGpu->bPipelinedPteMemEnabled;
}

static inline NvBool gpuIsBarPteInSysmemSupported(struct OBJGPU *pGpu){
    return pGpu->bIsBarPteInSysmemSupported;
}

static inline NvBool gpuIsRegUsesGlobalSurfaceOverridesEnabled(struct OBJGPU *pGpu){
    return pGpu->bRegUsesGlobalSurfaceOverrides;
}

static inline NvBool gpuIsTwoStageRcRecoveryEnabled(struct OBJGPU *pGpu){
    return pGpu->bTwoStageRcRecoveryEnabled;
}

static inline NvBool gpuIsInD3Cold(struct OBJGPU *pGpu){
    return pGpu->bInD3Cold;
}

static inline NvBool gpuIsClientRmAllocatedCtxBufferEnabled(struct OBJGPU *pGpu){
    return pGpu->bClientRmAllocatedCtxBuffer;
}

static inline NvBool gpuIsEccPageRetirementWithSliAllowed(struct OBJGPU *pGpu){
    return pGpu->bEccPageRetirementWithSliAllowed;
}

static inline NvBool gpuIsInstanceMemoryAlwaysCached(struct OBJGPU *pGpu){
    return pGpu->bInstanceMemoryAlwaysCached;
}

static inline NvBool gpuIsRmProfilingPrivileged(struct OBJGPU *pGpu){
    return pGpu->bRmProfilingPrivileged;
}

static inline NvBool gpuIsGeforceSmb(struct OBJGPU *pGpu){
    return pGpu->bGeforceSmb;
}

static inline NvBool gpuIsGeforceBranded(struct OBJGPU *pGpu){
    return pGpu->bIsGeforce;
}

static inline NvBool gpuIsQuadroBranded(struct OBJGPU *pGpu){
    return pGpu->bIsQuadro;
}

static inline NvBool gpuIsVgxBranded(struct OBJGPU *pGpu){
    return pGpu->bIsVgx;
}

static inline NvBool gpuIsNvidiaNvsBranded(struct OBJGPU *pGpu){
    return pGpu->bIsNvidiaNvs;
}

static inline NvBool gpuIsTitanBranded(struct OBJGPU *pGpu){
    return pGpu->bIsTitan;
}

static inline NvBool gpuIsTeslaBranded(struct OBJGPU *pGpu){
    return pGpu->bIsTesla;
}

static inline NvBool gpuIsComputePolicyTimesliceSupported(struct OBJGPU *pGpu){
    return pGpu->bComputePolicyTimesliceSupported;
}

static inline NvBool gpuIsSriovCapable(struct OBJGPU *pGpu){
    return pGpu->bSriovCapable;
}

static inline NvBool gpuIsNonPowerOf2ChannelCountSupported(struct OBJGPU *pGpu){
    return pGpu->bNonPowerOf2ChannelCountSupported;
}

static inline NvBool gpuIsVfResizableBAR1Supported(struct OBJGPU *pGpu){
    return pGpu->bVfResizableBAR1Supported;
}

static inline NvBool gpuIsVoltaHubIntrSupported(struct OBJGPU *pGpu){
    return pGpu->bVoltaHubIntrSupported;
}

static inline NvBool gpuIsUsePmcDeviceEnableForHostEngineEnabled(struct OBJGPU *pGpu){
    return pGpu->bUsePmcDeviceEnableForHostEngine;
}

static inline NvBool gpuIsSelfHosted(struct OBJGPU *pGpu){
    return pGpu->bIsSelfHosted;
}

static inline NvBool gpuIsGspOwnedFaultBuffersEnabled(struct OBJGPU *pGpu){
    return pGpu->bIsGspOwnedFaultBuffersEnabled;
}

#undef PRIVATE_FIELD


// Look up pGpu associated with a pResourceRef
NV_STATUS gpuGetByRef (RsResourceRef *pContextRef, NvBool *pbBroadcast, struct OBJGPU **ppGpu);

// Look up pGpu associated with a hResource
NV_STATUS gpuGetByHandle(struct RsClient *pClient, NvHandle hResource, NvBool *pbBroadcast, struct OBJGPU **ppGpu);

#define GPU_GFID_PF         (0)
#define IS_GFID_PF(gfid)    (((NvU32)(gfid)) == GPU_GFID_PF)
#define IS_GFID_VF(gfid)    (((NvU32)(gfid)) != GPU_GFID_PF)
// Invalid P2P GFID
#define INVALID_P2P_GFID    (0xFFFFFFFF)
#define INVALID_FABRIC_PARTITION_ID (0xFFFFFFFF)

//
// Generates GPU child accessor macros (i.e.: GPU_GET_{ENG})
//
#define GPU_CHILD_SINGLE_INST(className, accessorName, numInstances, bConstructEarly, gpuField) \
    static NV_FORCEINLINE className *accessorName(OBJGPU *pGpu) { return pGpu->children.named.gpuField; }      \
    ct_assert(numInstances == 1);

#define GPU_CHILD_MULTI_INST(className, accessorName, numInstances, bConstructEarly, gpuField) \
    static NV_FORCEINLINE className *accessorName(OBJGPU *pGpu, NvU32 index) { return index < numInstances ? pGpu->children.named.gpuField[index] : NULL; }

#include "gpu/gpu_child_list.h"

static NV_FORCEINLINE struct Graphics *GPU_GET_GR(struct OBJGPU *pGpu) { return NULL; }

// Temporary stubs
#if RM_STRICT_CONFIG_EMIT_DISABLED_GPU_ENGINE_ACCESSORS
#define GPU_CHILD_LIST_DISABLED_ONLY
#define GPU_CHILD_SINGLE_INST(className, accessorName, numInstances, bConstructEarly, gpuField) \
    static NV_FORCEINLINE className *accessorName(OBJGPU *pGpu) { return NULL; }

#define GPU_CHILD_MULTI_INST(className, accessorName, numInstances, bConstructEarly, gpuField) \
    static NV_FORCEINLINE className *accessorName(OBJGPU *pGpu, NvU32 index) { return NULL; }

#include "gpu/gpu_child_list.h"
#endif // RM_STRICT_CONFIG_EMIT_DISABLED_GPU_ENGINE_ACCESSORS


// Type-safe wrapper for child iteration
#define GPU_GET_NEXT_CHILD_OF_TYPE(pGpu, pIt, className) \
    ((className*)gpuGetNextChildOfTypeUnsafe(pGpu, pIt, classId(className)))


//
// Inline functions
//

//
// This function returns subdevice mask for a GPU.
// For non SLI, subdeviceInstance is 0, so this
// function will always return 1.
//

static NV_INLINE NvU32
gpuGetSubdeviceMask
(
    struct OBJGPU *pGpu
)
{

    return 1 << pGpu->subdeviceInstance;
}

static NV_INLINE NvU32
gpuGetInstance
(
    struct OBJGPU *pGpu
)
{
    return pGpu->gpuInstance;
}

static NV_INLINE NvU32
gpuGetDeviceInstance
(
    struct OBJGPU *pGpu
)
{
    return pGpu->deviceInstance;
}

NV_INLINE
static NvU32 gpuGetNumCEs(struct OBJGPU *pGpu)
{
    return pGpu->numCEs;
}

// TODO: make `const` after bug 4292180 is fixed
static NV_INLINE /* const */ struct GpuArch *
gpuGetArch(struct OBJGPU *pGpu)
{
    return pGpu->pGpuArch;
}

//
// Per GPU mode flags macros. In general these macros should not be
// used and all code paths should be the same on all environments.
// However occasionally a tweak is needed to work around a limitation
// or improve speed on non-hardware.  Is_RTLSIM normally is handled
// in the IS_SIMULATION case and should almost never be used.
//
// IS_EMULATION     actual emulation hardware
// IS_SIMULATION    fmodel or RTL simulation
// IS_MODS_AMODEL   amodel under mods for trace player
// IS_LIVE_AMODEL   amodel under windows for 3D drivers (removed)
// IS_RTLSIM        RTL simulation
// IS_SILICON       Real hardware
// IS_VIRTUAL       RM is running within a guest VM
// IS_GSP_CLIENT    RM is a GSP client with GPU support offloaded to GSP
// IS_DCE_CLIENT    RM is a DCE client with GPU support offloaded to DCE
//

#define IS_EMULATION(pGpu)                 ((pGpu)->getProperty((pGpu), PDB_PROP_GPU_EMULATION))
#define IS_SIMULATION(pGpu)                (pGpu->bIsSimulation)
#define IS_MODS_AMODEL(pGpu)               (pGpu->bIsModsAmodel)
#define IS_FMODEL(pGpu)                    (pGpu->bIsFmodel)
#define IS_RTLSIM(pGpu)                    (pGpu->bIsRtlsim)
#define IS_SILICON(pGpu)                   (!(IS_EMULATION(pGpu) || IS_SIMULATION(pGpu)))
#define IS_PASSTHRU(pGpu)                  ((pGpu)->bIsPassthru)
#define IS_GSP_CLIENT(pGpu)                (RMCFG_FEATURE_GSP_CLIENT_RM && (pGpu)->isGspClient)
#define IS_DCE_CLIENT(pGpu)                (RMCFG_FEATURE_DCE_CLIENT_RM && (pGpu)->isDceClient)
#define IS_FW_CLIENT(pGpu)                 (IS_GSP_CLIENT(pGpu) || IS_DCE_CLIENT(pGpu))
#define IS_VIRTUAL(pGpu)                   ((pGpu)->isVirtual)
#define IS_VIRTUAL_WITH_SRIOV(pGpu)        ((pGpu)->bIsVirtualWithSriov)
#define IS_VIRTUAL_WITH_HEAVY_SRIOV(pGpu)  (IS_VIRTUAL_WITH_SRIOV(pGpu) && gpuIsWarBug200577889SriovHeavyEnabled(pGpu))
#define IS_VIRTUAL_WITH_FULL_SRIOV(pGpu)   (IS_VIRTUAL_WITH_SRIOV(pGpu) && !IS_VIRTUAL_WITH_HEAVY_SRIOV(pGpu))
#define IS_VIRTUAL_WITHOUT_SRIOV(pGpu)     (IS_VIRTUAL(pGpu) && !IS_VIRTUAL_WITH_SRIOV(pGpu))
#define IS_SRIOV_HEAVY(pGpu)        (gpuIsWarBug200577889SriovHeavyEnabled(pGpu))
#define IS_SRIOV_HEAVY_GUEST(pGpu)  ((IS_VIRTUAL_WITH_SRIOV(pGpu))  &&  IS_SRIOV_HEAVY(pGpu))
#define IS_SRIOV_FULL_GUEST(pGpu)   ((IS_VIRTUAL_WITH_SRIOV(pGpu))  &&  !IS_SRIOV_HEAVY(pGpu))
#define IS_SRIOV_HEAVY_HOST(pGpu)   ((hypervisorIsVgxHyper())       &&  IS_SRIOV_HEAVY(pGpu))
#define IS_SRIOV_FULL_HOST(pGpu)    ((hypervisorIsVgxHyper()) && gpuIsSriovEnabled(pGpu) && !IS_SRIOV_HEAVY(pGpu))
#define IS_VGPU_GSP_PLUGIN_OFFLOAD_ENABLED(pGpu)     ((pGpu)->bVgpuGspPluginOffloadEnabled)
#define IS_SRIOV_WITH_VGPU_GSP_ENABLED(pGpu)         (gpuIsSriovEnabled(pGpu) && IS_VGPU_GSP_PLUGIN_OFFLOAD_ENABLED(pGpu) && !IS_SRIOV_HEAVY(pGpu))
#define IS_SRIOV_WITH_VGPU_GSP_DISABLED(pGpu)        (gpuIsSriovEnabled(pGpu) && !IS_VGPU_GSP_PLUGIN_OFFLOAD_ENABLED(pGpu) && !IS_SRIOV_HEAVY(pGpu))
#define IS_MIG_RM(pGpu)                              ((pGpu)->bIsMigRm)

NV_STATUS gpuCtrlExecRegOps(struct OBJGPU *, struct Graphics *, NvHandle, NvHandle, NV2080_CTRL_GPU_REG_OP *, NvU32, NvBool);
NV_STATUS gpuValidateRegOps(struct OBJGPU *, NV2080_CTRL_GPU_REG_OP *, NvU32, NvBool, NvBool, NvBool);

// GPU Sanity Check Flags
#define GPU_SANITY_CHECK_FLAGS_BOOT_0                   NVBIT(0)
#define GPU_SANITY_CHECK_FLAGS_OFF_BY_N                 NVBIT(1)
#define GPU_SANITY_CHECK_FLAGS_PCI_SPACE_MATCH          NVBIT(2)
#define GPU_SANITY_CHECK_FLAGS_PCI_MEM_SPACE_ENABLED    NVBIT(3)
#define GPU_SANITY_CHECK_FLAGS_FB                       NVBIT(4)

#define GPU_SANITY_CHECK_FLAGS_NONE         0x0
#define GPU_SANITY_CHECK_FLAGS_ALL          0xffffffff

//
// Macro for checking if GPU is in reset.
//
#define API_GPU_IN_RESET_SANITY_CHECK(pGpu)                                 \
    ((NULL == pGpu) ||                                                      \
     pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_FULLCHIP_RESET) ||             \
     pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_SECONDARY_BUS_RESET) ||        \
     pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_GC6_RESET) ||                  \
     pGpu->getProperty(pGpu, PDB_PROP_GPU_SECONDARY_BUS_RESET_PENDING))

//
// Marco for checking if GPU is still connected.
//
#define API_GPU_ATTACHED_SANITY_CHECK(pGpu)                           \
    ((NULL != pGpu) &&                                                \
     pGpu->getProperty(pGpu, PDB_PROP_GPU_IS_CONNECTED) &&            \
     !pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_SECONDARY_BUS_RESET))

//
// Macro for checking if GPU has Full Sanity
//
#define FULL_GPU_SANITY_CHECK(pGpu)                                     \
    ((NULL != pGpu) &&                                                  \
     gpuIsGpuFullPower(pGpu) &&                                         \
     pGpu->getProperty(pGpu, PDB_PROP_GPU_IS_CONNECTED) &&              \
     !pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_FULLCHIP_RESET) &&        \
     !pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_SECONDARY_BUS_RESET) &&   \
     !pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_GC6_RESET) &&             \
     !pGpu->getProperty(pGpu, PDB_PROP_GPU_IS_LOST) &&                  \
     gpuCheckSysmemAccess(pGpu))

//
// Macro for checking if GPU has Full Sanity
//
#define FULL_GPU_SANITY_FOR_PM_RESUME(pGpu)                             \
    ((NULL != pGpu) &&                                                  \
     gpuIsGpuFullPowerForPmResume(pGpu) &&                              \
     pGpu->getProperty(pGpu, PDB_PROP_GPU_IS_CONNECTED) &&              \
     !pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_FULLCHIP_RESET) &&        \
     !pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_SECONDARY_BUS_RESET) &&   \
     !pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_GC6_RESET) &&             \
     !pGpu->getProperty(pGpu, PDB_PROP_GPU_IS_LOST))

//
// Macro for checking if GPU is in the recovery path
//
#define API_GPU_IN_RECOVERY_SANITY_CHECK(pGpu)                          \
    ((NULL == pGpu) ||                                                  \
    pGpu->getProperty(pGpu, PDB_PROP_GPU_IN_TIMEOUT_RECOVERY))

//******************************************************************************
//                            POWER SANITY CHECKS
//******************************************************************************
//
// Make sure the GPU is in full power or resuming from D3 state. Else,
// bailout from the calling function. An exception for systems, which support
// surprise removal feature. See Bugs 440565, 479003, and 499228.DO NOT IGNORE
// OR REMOVE THIS ASSERT.  If you have problems with it, please talk to cplummer.
//
// bAllowWithoutSysmemAccess: Allow this RM Control when sysmem access is not available
// from the GPU. SHould be NV_TRUE only for NV2080_CTRL_CMD_BUS_SYSMEM_ACCESS
//
// On systems supporting surprise removal, if the GPU is in D3 cold
// and still attached we would consider it a true D3 cold state
// and return NOT_FULL_POWER. See bug 1679965.
//
//
#define API_GPU_FULL_POWER_SANITY_CHECK(pGpu, bGpuAccess, bAllowWithoutSysmemAccess) \
    if ((!gpuIsGpuFullPower(pGpu)) &&                                                \
              (!(pGpu)->getProperty((pGpu),                                          \
                             PDB_PROP_GPU_IN_PM_RESUME_CODEPATH)))                   \
    {                                                                                \
        DBG_BREAKPOINT();                                                            \
        if (bGpuAccess || (!gpuIsSurpriseRemovalSupported(pGpu)))                    \
        {                                                                            \
            return NV_ERR_GPU_NOT_FULL_POWER;                                        \
        }                                                                            \
        else if (gpuIsSurpriseRemovalSupported(pGpu) &&                              \
                 (pGpu)->getProperty((pGpu), PDB_PROP_GPU_IS_CONNECTED))             \
        {                                                                            \
            return NV_ERR_GPU_NOT_FULL_POWER;                                        \
        }                                                                            \
    }                                                                                \
    if (!(bAllowWithoutSysmemAccess) && !gpuCheckSysmemAccess(pGpu))                 \
    {                                                                                \
        return NV_ERR_GPU_NOT_FULL_POWER;                                            \
    }

#define API_GPU_FULL_POWER_SANITY_CHECK_OR_GOTO(pGpu, bGpuAccess, bAllowWithoutSysmemAccess, status, tag) \
    if ((!gpuIsGpuFullPower(pGpu)) &&                                                                     \
              (!(pGpu)->getProperty((pGpu),                                                               \
                             PDB_PROP_GPU_IN_PM_RESUME_CODEPATH)))                                        \
    {                                                                                                     \
        DBG_BREAKPOINT();                                                                                 \
        if (bGpuAccess || (!gpuIsSurpriseRemovalSupported(pGpu)))                                         \
        {                                                                                                 \
            status = NV_ERR_GPU_NOT_FULL_POWER;                                                           \
            goto tag;                                                                                     \
        }                                                                                                 \
        else if (gpuIsSurpriseRemovalSupported(pGpu) &&                                                   \
                (pGpu)->getProperty((pGpu), PDB_PROP_GPU_IS_CONNECTED))                                   \
        {                                                                                                 \
            status = NV_ERR_GPU_NOT_FULL_POWER;                                                           \
            goto tag;                                                                                     \
        }                                                                                                 \
    }                                                                                                     \
    if (!(bAllowWithoutSysmemAccess) && !gpuCheckSysmemAccess(pGpu))                                      \
    {                                                                                                     \
        return NV_ERR_GPU_NOT_FULL_POWER;                                                                 \
    }

//
// Identifiers for gpuGetRegBaseOffset HAL interface.
//
#define NV_REG_BASE_GR                (0x00000001)
#define NV_REG_BASE_TIMER             (0x00000002)
#define NV_REG_BASE_MASTER            (0x00000003)
#define NV_REG_BASE_USERMODE          (0x00000004)
#define NV_REG_BASE_LAST              NV_REG_BASE_USERMODE
ct_assert(NV_REG_BASE_LAST < NV2080_CTRL_INTERNAL_GET_CHIP_INFO_REG_BASE_MAX);

#define GPU_READ_PRI_ERROR_MASK  0xFFF00000
#define GPU_READ_PRI_ERROR_CODE  0xBAD00000

//
// Define for invalid register value. GPU could have fallen off the bus or
// the GPU could be in reset.
//
#define GPU_REG_VALUE_INVALID    0xFFFFFFFF

typedef struct _vgpu_static_info VGPU_STATIC_INFO;
typedef struct GspStaticConfigInfo_t GspStaticConfigInfo;

// Static info getters
VGPU_STATIC_INFO *gpuGetStaticInfo(struct OBJGPU *pGpu);
#define GPU_GET_STATIC_INFO(pGpu) gpuGetStaticInfo(pGpu)
GspStaticConfigInfo *gpuGetGspStaticInfo(struct OBJGPU *pGpu);
#define GPU_GET_GSP_STATIC_INFO(pGpu) gpuGetGspStaticInfo(pGpu)

NV_STATUS gpuSimEscapeWrite(struct OBJGPU *, const char *path, NvU32 Index, NvU32 Size, NvU32 Value);
NV_STATUS gpuSimEscapeWriteBuffer(struct OBJGPU *, const char *path, NvU32 Index, NvU32 Size, void* pBuffer);
NV_STATUS gpuSimEscapeRead(struct OBJGPU *, const char *path, NvU32 Index, NvU32 Size, NvU32 *Value);
NV_STATUS gpuSimEscapeReadBuffer(struct OBJGPU *, const char *path, NvU32 Index, NvU32 Size, void* pBuffer);

//
// This function needs to be called when OBJGPU is not created. HAL
// infrastructure can't be used for this case, so it has been added manually.
// It will be invoked directly by gpumgrIsDeviceMsixAllowed().
//
NvBool gpuIsMsixAllowed_TU102(RmPhysAddr bar0BaseAddr);

//
// This function needs to be called when OBJGPU is not created. HAL
// infrastructure can't be used for this case, so it has been added manually.
// It will be invoked directly by gpumgrWaitForBarFirewall().
//

// Define for PMC reset delay
#define NV_PMC_RESET_DELAY_US  2

#endif // _OBJGPU_H_

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _G_GPU_NVOC_H_
