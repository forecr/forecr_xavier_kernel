
#ifndef _G_MEM_MGR_NVOC_H_
#define _G_MEM_MGR_NVOC_H_

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
#include "g_mem_mgr_nvoc.h"

#ifndef MEM_MGR_H
#define MEM_MGR_H

#include "core/core.h"
#include "gpu/eng_state.h"

#include "gpu/gpu.h"

#include "mem_mgr/mem.h"
#include "kernel/gpu_mgr/gpu_mgr.h"

#include "gpu/mem_mgr/virt_mem_allocator_common.h"
#include "containers/map.h"
#include "gpu/mem_mgr/heap_base.h"
#include "mem_mgr/vaspace.h"

struct _PMA;
typedef struct _PMA PMA;

#include "ctrl/ctrl2080/ctrl2080fb.h" // NV2080_CTRL_FB_GET_CARVEOUT_REGION_INFO_PARAMS

struct CeUtils;

#ifndef __nvoc_class_id_CeUtils
#define __nvoc_class_id_CeUtils 0x8b8baeu
typedef struct CeUtils CeUtils;
#endif /* __nvoc_class_id_CeUtils */


struct SysmemScrubber;

#ifndef __nvoc_class_id_SysmemScrubber
#define __nvoc_class_id_SysmemScrubber 0x266962u
typedef struct SysmemScrubber SysmemScrubber;
#endif /* __nvoc_class_id_SysmemScrubber */



typedef volatile struct _cl906f_tag1 Nv906fControl;
typedef struct KERNEL_MIG_GPU_INSTANCE KERNEL_MIG_GPU_INSTANCE;

typedef struct
{
    MEMORY_DESCRIPTOR *pMemDesc;
    NvU64              offset;

    //
    // Private, should only be used by MemUtils layer
    // Expected to be 0 when map is called
    // Become 0 when unmapped
    //
    void              *pMapping;
    void              *pMappingPriv;
} TRANSFER_SURFACE;

// Memory transfer engine types.
typedef enum
{
    TRANSFER_TYPE_PROCESSOR = 0,       // CPU/GSP/DPU depending on execution context
    TRANSFER_TYPE_GSP_DMA,             // Dma engine internal to GSP
    TRANSFER_TYPE_CE,                  // Copy Engine using CeUtils channel
    TRANSFER_TYPE_CE_PRI,              // Copy Engine using PRIs
    TRANSFER_TYPE_BAR0,                // Copy using BAR0 PRAMIN
} TRANSFER_TYPE;

#define TRANSFER_FLAGS_NONE                             0
#define TRANSFER_FLAGS_DEFER_FLUSH                      NVBIT32(0) // Applicable only for write operations
#define TRANSFER_FLAGS_SHADOW_ALLOC                     NVBIT32(1) // Applicable only for non-PROCESSOR transfers
#define TRANSFER_FLAGS_SHADOW_INIT_MEM                  NVBIT32(2) // Applicable only for non-PROCESSOR transfers
#define TRANSFER_FLAGS_PERSISTENT_CPU_MAPPING           NVBIT32(3) // Require long lived PROCESSOR mapping
#define TRANSFER_FLAGS_DESTROY_MAPPING                  NVBIT32(4) // Destroy any cached mappings when complete
#define TRANSFER_FLAGS_USE_BAR1                         NVBIT32(5) // Use only BAR1 for PROCESSOR transfers
#define TRANSFER_FLAGS_PREFER_CE                        NVBIT32(6) // Use CE if possible (BAR0 on simulation for perf)
#define TRANSFER_FLAGS_CE_PRI_DEFER_FLUSH               NVBIT32(7) // Defer CE flush; only affects PRI CE operations

// Protection flags: at most 1 may be set, none means READ_WRITE by default
#define TRANSFER_FLAGS_MAP_PROTECT_READABLE             NVBIT32(8) // Transfer is only reading data
#define TRANSFER_FLAGS_MAP_PROTECT_WRITEABLE            NVBIT32(9) // Transfer is only writing data

#define TRANSFER_FLAGS_PREFER_PROCESSOR                 NVBIT32(10) // Use BAR1/2 if possible
#define TRANSFER_FLAGS_ALLOW_MAPPING_REUSE              NVBIT32(11) // Prefer existing full-allocation mapping
                                                                    // (see memdescGetKernelMapping())
                                                                    // Only affects BeginTransfer/EndTransfer
                                                                    // Mapping lifetime controlled by original mapper
                                                                    // Intented for short uses,
                                                                    // where it can't be unmapped by the owner

#define TRANSFER_FLAGS_FLUSH_CPU_CACHE_WAR_BUG4686457   NVBIT32(12) // Flush the data from CPU cache
typedef struct
{
    NvU32   bar1Size;
    NvU32   bar1AvailSize;
    NvU32   bankSwizzleAlignment;
    NvU32   bar1MaxContigAvailSize;
} GETBAR1INFO, *PGETBAR1INFO;

//
// RM Default PTE kind
// Bug #2242255, introducing the RM Default kind to allow sharing memory between
// different architectures especially between Turing+ and Pre Turing chips
//
#define RM_DEFAULT_PTE_KIND                 0x100

typedef enum
{
    FB_IS_KIND_Z,                           // Kind is a Z buffer
    FB_IS_KIND_ZBC,                         // Zero bandwidth clears
    FB_IS_KIND_ZBC_ALLOWS_1,                // ZBC with 1 bit of tag
    FB_IS_KIND_ZBC_ALLOWS_2,                // ZBC with 2 bits of tag
    FB_IS_KIND_ZBC_ALLOWS_4,                // ZBC with 4 bits of tag
    FB_IS_KIND_COMPRESSIBLE,                // Any compressible kind
    FB_IS_KIND_COMPRESSIBLE_1,              // Compressible with 1 comp tag bit
    FB_IS_KIND_COMPRESSIBLE_2,              // Compressible with 2 comp tag bits
    FB_IS_KIND_COMPRESSIBLE_4,              // Compressible with 4 comp tag bits
    FB_IS_KIND_SUPPORTED,                   // Kind is supported
    FB_IS_KIND_DISALLOW_PLC,                // Kind Disallows PLC
    FB_IS_KIND_SWIZZLED,                    // Kind is swizzled (not pitch or GMK)
} FB_IS_KIND_OP;

// Surface compression parameters
typedef struct COMPR_INFO
{
    // Surface kind; if not compressed, following parameters are ignored
    NvU32  kind;

    // Compression page shift;  0 if kind is uncompressed
    NvU32  compPageShift;

    //
    // Are comptags are determined per-page by PA?
    // If set, following parameters are ignored
    //
    NvBool bPhysBasedComptags;

    // see GMMU_COMPR_INFO
    NvU32  compPageIndexLo;
    NvU32  compPageIndexHi;
    NvU32  compTagLineMin;
    NvU32  compTagLineMultiplier;
} COMPR_INFO;

//
// Individual entry for logging Fb reserved use-cases
//
typedef struct NV_FB_RSVD_BLOCK_LOG_ENTRY
{
    // Owner tag associated with reservation block
    NvU32 ownerId;

    // Size of the memory reserved
    NvU64 rsvdSize;
} NV_FB_RSVD_BLOCK_LOG_ENTRY;

// Total number of FB internal reservation enries
#define NV_FB_RSVD_BLOCK_LOG_ENTRY_MAX 10U

//
// Structure for logging Fb reserved use-cases
//
typedef struct NV_FB_RSVD_BLOCK_LOG_INFO
{
    // Counter for logging entries
    NvU32 counter;

    // List of all reserved entries
    NV_FB_RSVD_BLOCK_LOG_ENTRY rsvdBlockList[NV_FB_RSVD_BLOCK_LOG_ENTRY_MAX];
} NV_FB_RSVD_BLOCK_LOG_INFO;

//
// Macro for initializing reserved block log data
//
#define NV_FB_RSVD_BLOCK_LOG_INIT(pMem)                                                                    \
        {                                                                                                  \
            ((pMem)->rsvdBlockInfo).counter = 0;                                                           \
            for (NvU32 i = 0; i < NV_FB_RSVD_BLOCK_LOG_ENTRY_MAX; i++)                                     \
            {                                                                                              \
                ((pMem)->rsvdBlockInfo).rsvdBlockList[i].ownerId = 0;                                      \
                ((pMem)->rsvdBlockInfo).rsvdBlockList[i].rsvdSize = 0;                                     \
            }                                                                                              \
        }

//
// Macro for adding new reserved block entry to the list
// If unable to log, marks the status as NV_ERR_NO_MEMORY otherwise keeps it unchanged
//
#define NV_FB_RSVD_BLOCK_LOG_ENTRY_ADD(status, pMem, tag, size)                                            \
        {                                                                                                  \
            if(((pMem)->rsvdBlockInfo).counter < NV_FB_RSVD_BLOCK_LOG_ENTRY_MAX)                           \
            {                                                                                              \
                ((pMem)->rsvdBlockInfo).rsvdBlockList[((pMem)->rsvdBlockInfo).counter].ownerId = (tag);    \
                ((pMem)->rsvdBlockInfo).rsvdBlockList[((pMem)->rsvdBlockInfo).counter].rsvdSize = (size);  \
                (((pMem)->rsvdBlockInfo).counter)++;                                                       \
            }                                                                                              \
            else                                                                                           \
            {                                                                                              \
                status = NV_ERR_NO_MEMORY;                                                                 \
            }                                                                                              \
        }

//
// Fixed Channel Properties for Memutils Object
//

typedef NV_STATUS FbScrubCallback(OBJGPU *);

#define BLOCK_INDEX_FROM_ADDR(addr,size)             ((NvU32)((addr) >> size))
#define BLOCK_ADDR_FROM_INDEX(idx,size)              (((NvU64)(idx)) << size)

#define MEMUTILS_SIZE_PER_BLOCK_INBYTES               (0x68)
#define MEMUTILS_TOTAL_SIZE_PER_BLOCK_INBYTES         (0x60) //(COPY + PB SEMA)
#define MEMUTILS_TD_BLOCKS_PER_CHUNK                   0x40

#define BLOCK_INDEX_FROM_ADDR(addr,size)              ((NvU32)((addr) >> size))
#define BLOCK_ADDR_FROM_INDEX(idx,size)               (((NvU64)(idx)) << size)

#define MEMUTILS_NUM_PAYLOAD_SEMAPHORES               (2)
#define MEMUTILS_NUM_GPFIFIO_ENTRIES                  (32)
// PB size should be a multiple of chunk size
#define MEMUTILS_CHANNEL_PB_SIZE                      (0x10 * MEMUTILS_SIZE_PER_BLOCK_INBYTES  * \
                                                       MEMUTILS_TD_BLOCKS_PER_CHUNK)
#define MEMUTILS_CHANNEL_SEMAPHORE_SIZE               (4 * MEMUTILS_NUM_PAYLOAD_SEMAPHORES)
#define MEMUTILS_CHANNEL_NOTIFIER_SIZE                (sizeof(NvNotification) * 1)

// offset and line length should be a multiple of 4KB
#define MEMUTIL_SCRUB_OFFSET_ALIGNMENT        (4 * 1024)
#define MEMUTIL_SCRUB_LINE_LENGTH_ALIGNMENT   (4 * 1024)

typedef enum {
    CE_SCRUBBER_CHANNEL,
    FAST_SCRUBBER_CHANNEL,
    COPY_CHANNEL,
    SWL_SCRUBBER_CHANNEL,
    MAX_CHANNEL_TYPE
} CHANNEL_KIND;

// This will be moved to a channel object next
typedef struct OBJCHANNEL
{
    NvHandle                        deviceId;           // Device Handle
    NvHandle                        physMemId;          // Memory Handle
    NvHandle                        channelId;          // Channel Handle
    NvHandle                        subdeviceId;        // Subdevice Handle
    NvHandle                        errNotifierIdVirt;
    NvHandle                        errNotifierIdPhys;
    NvHandle                        engineObjectId;
    NvHandle                        eventId;
    NvHandle                        pushBufferId;
    NvHandle                        bitMapSemPhysId;
    NvHandle                        bitMapSemVirtId;
    NvHandle                        hVASpaceId;           // VASpace handle, when scrubber in virtual mode
    NvHandle                        hFbAlias;             // Used only for virtual channels
    NvHandle                        hFbAliasVA;
    // to be moved later

    NvU32                           channelSize;
    NvU32                           channelNumGpFifioEntries;
    NvU32                           channelPbSize;
    NvU32                           channelNotifierSize;
    NvU32                           methodSizePerBlock;
    NvU32                           semaOffset;
    NvU32                           finishPayloadOffset;
    NvU32                           authTagBufSemaOffset;
    NvU32                           finishPayload;
    NvBool                          isChannelSynchronized;
    NvBool                          isProgressChecked;
//
// RM internal channels are created as privileged channels (physical address access) by default
// For MMU Bug: 2739505, we need to switch to use channels in non-privileged mode.
//
    NvBool                          bUseVasForCeCopy;         // set to NV_TRUE, when scrubber operates in virtual address
    struct RsClient                        *pRsClient;
    struct OBJVASPACE                     *pVAS;
    NvU32                           engineType;
    NvU64                           startFbOffset;
    NvU64                           fbSize;
    NvU64                           fbAliasVA;
    NvU64                           vaStartOffset;
    // to be moved to a separate object later

    NvU32                           *pBlockPendingState;
    NvU32                           *pBlockDoneState;
    NvU32                           blockCount;
    NvHandle                        hClient;
    NvBool                          bClientAllocated;
    NvU64                           pbGpuVA;
    NvU64                           pbGpuBitMapVA;
    NvU64                           pbGpuNotifierVA;
    MEMORY_DESCRIPTOR               *pUserdMemdesc;
    MEMORY_DESCRIPTOR               *pChannelBufferMemdesc;
    MEMORY_DESCRIPTOR               *pErrNotifierMemdesc;
    NvU8                            *pbCpuVA;
    NvU8                            *pbBitMapVA;
    Nv906fControl                   *pControlGPFifo;
    NvU32                           classEngineID;
    NVOS10_EVENT_KERNEL_CALLBACK_EX callback;
    NvU32                           state;
    NvU32                           hTdCopyClass;
    NvU32                           sec2Class;
    NvU32                           minBlockSize;
    NvU32                           maxBlockSize;
    NvU32                           channelPutOffset;
    NvU8                            blockShift;
    NvU32                           lastPayloadPushed;
    NvBool                          isChannelActive;
    NvU32                           workSubmitToken;
    //
    // Work submit token read from notifier memory.
    //
    NvNotification                  *pTokenFromNotifier;
    NvU32                           lastSubmittedEntry;
    NvHandle                        lastAllocatedHandle;
    CHANNEL_KIND                    type;

    // Used for Volta+
    NvHandle                        doorbellRegionHandle;
    NvU8                            *pDoorbellRegion;
    NvU32                           *pDoorbellRegisterOffset;
    NvBool                          bUseDoorbellRegister;
    NvHandle                        hUserD;
    NvBool                          bClientUserd;

    OBJGPU                         *pGpu;
    NvU32                           ceId;

    // Used by Partition Scrubber
    KERNEL_MIG_GPU_INSTANCE         *pKernelMIGGpuInstance;
    NvHandle                        hPartitionRef;

    NvBool                          bUseBar1;

} OBJCHANNEL;

#define NV_METHOD(SubCh, Method, Num)                        \
    (DRF_DEF(906F, _DMA_INCR, _OPCODE,     _VALUE)         | \
     DRF_NUM(906F, _DMA_INCR, _COUNT,      Num)            | \
     DRF_NUM(906F, _DMA_INCR, _SUBCHANNEL, SubCh)          | \
     DRF_NUM(906F, _DMA_INCR, _ADDRESS,    (Method) >> 2))

#define PUSH_DATA(Data) MEM_WR32(ptr++, (Data))

#define PUSH_PAIR(SubCh, Method, Data)            \
    do                                            \
    {                                             \
        PUSH_DATA(NV_METHOD(SubCh, (Method), 1)); \
        PUSH_DATA((Data));                        \
    } while (0)

//-----------------------------------------------------------------------------

typedef struct
{
    NvU32  lastSubmittedBlock;
    NvBool isTopDownScrubber;
    NvBool isActive;
    NvU32  scrubberState;
    NvU32  currentFbRegion;
    NvU32  startBlock;
    NvU32  endBlock;
    NvU32 *pPendingBitMap;
    NvU32 *pDoneBitMap;
    NvU32  blockCount;
    struct OBJCE *pCe;
    NvBool bCeInUse;
    OBJCHANNEL tdHeapState;
    OBJCHANNEL allocationScrubberState;
} OBJSCRUB;

typedef struct
{
    NvU64   base;               // Base/start address of the region
    NvU64   limit;              // Last/end address of region
    NvU64   rsvdSize;           // Memory RM may be required to allocate in this region
    NvBool  bRsvdRegion;        // Reserved region -- not publicly usable
    NvU32   performance;        // Relative performance.  Higher is faster
    NvBool  bSupportCompressed; // Support compressed kinds
    NvBool  bSupportISO;        // Support ISO (display, cursor, video) surfaces
    NvBool  bProtected;         // Represents a protected region of memory.
    NvBool  bInternalHeap;      // PMA:Used for internal RM allocations
    NvBool  bLostOnSuspend;     // Not required to be Saved during S/R.
    NvBool  bPreserveOnSuspend; // Required to be Saved during S/R.
} FB_REGION_DESCRIPTOR, *PFB_REGION_DESCRIPTOR;

#define MAX_FB_REGIONS  16

// Maximum number of contexts created for WHQL test WDDM Max Contexts
#define WHQL_TEST_MAX_CONTEXTS          100

// Object 'get' macros for FB relative object retrievals.
#define MEMORY_MANAGER_GET_HEAP(p)          ((p)->pHeap)

typedef struct _def_fb_mem_node
{
    struct _def_fb_mem_node *pNext;

    NvBool bFreeDescriptor;
    PMEMORY_DESCRIPTOR pMemDesc;

} FB_MEM_NODE, *PFB_MEM_NODE;

// defines for MemoryManager::fbsrReservedRanges
#define MAX_FBSR_RESERVED_REGIONS                   4  // Max. Memory descriptors for RM Instance memory
#define FBSR_RESERVED_INST_MEMORY_BEFORE_BAR2PTE    0  // Described on Kernel-RM and Physical-RM (Monolithic / GSP offload)
#define FBSR_RESERVED_INST_MEMORY_AFTER_BAR2PTE     1  // Described on Kernel-RM and Physical-RM (Monolithic / GSP offload)
#define FBSR_RESERVED_INST_MEMORY_GSP_NON_WPR       2  // Described on Physical-RM (GSP offload)
#define FBSR_RESERVED_INST_MEMORY_VGA_WORKSPACE     3  // Described on Physical-RM (GSP offload)

/*!
 * MemoryManager provides the root memory management of GPU video memory.
 * External entities might provide suballocators on top of MemoryManager.
 *
 * MemoryManager can have static information on the memory system (e.g.: list of
 * kinds, etc), however MemoryManager does not have direct access to the GPU
 * memory system (e.g.: BAR0 registers). It relies on KernelMemorySystem for
 * operations on the memory system.
 *
 * MemoryManager is instantiated in VGPU guest/GSP Client as well as the VGPU
 * host/GSP-RM.
 */

#define MEM_MGR_STUB_ORIN(...) { return __VA_ARGS__; }


// Private field names are wrapped in PRIVATE_FIELD, which does nothing for
// the matching C source file, but causes diagnostics to be issued if another
// source file references the field.
#ifdef NVOC_MEM_MGR_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif


// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__MemoryManager;
struct NVOC_METADATA__OBJENGSTATE;
struct NVOC_VTABLE__MemoryManager;

struct RM_POOL_ALLOC_MEM_RESERVE_INFO;

struct __nvoc_inner_struc_MemoryManager_1__ {
    NvBool bEnabled;
    NvU32 peerId;
};

struct MIG_MEMORY_PARTITIONING_INFO {
    struct NV_RANGE partitionableMemoryRange;
    struct NV_RANGE partitionableBar1Range;
    NvHandle hClient;
    NvHandle hDevice;
    NvHandle hSubdevice;
};



struct MemoryManager {

    // Metadata starts with RTTI structure.
    union {
         const struct NVOC_METADATA__MemoryManager *__nvoc_metadata_ptr;
         const struct NVOC_RTTI *__nvoc_rtti;
    };

    // Parent (i.e. superclass or base class) objects
    struct OBJENGSTATE __nvoc_base_OBJENGSTATE;

    // Ancestor object pointers for `staticCast` feature
    struct Object *__nvoc_pbase_Object;    // obj super^2
    struct OBJENGSTATE *__nvoc_pbase_OBJENGSTATE;    // engstate super
    struct MemoryManager *__nvoc_pbase_MemoryManager;    // memmgr

    // 1 PDB property
//  NvBool PDB_PROP_MEMMGR_IS_MISSING inherited from OBJENGSTATE

    // Data members
    NvBool bFbsrWddmModeEnabled;
    NvBool bFbRegionsSupported;
    NvBool bPmaSupportedOnPlatform;
    NvBool bPmaEnabled;
    NvBool bPmaInitialized;
    NvBool bPmaForcePersistence;
    NvBool bClientPageTablesPmaManaged;
    NvBool bScanoutSysmem;
    NvBool bMixedDensityFbp;
    NvBool bPreferSlowRegion;
    NvBool bPersistentStandbyBuffer;
    NvBool bEnableFbsrPagedDma;
    NvBool bDisallowSplitLowerMemory;
    NvBool bIgnoreUpperMemory;
    NvBool bSmallPageCompression;
    NvBool bSysmemCompressionSupportDef;
    NvBool bBug1698088IncreaseRmReserveMemoryWar;
    NvBool bBug2301372IncreaseRmReserveMemoryWar;
    NvBool bEnableFbsrFileMode;
    NvBool bEnableDynamicPageOfflining;
    NvBool bVgpuPmaSupport;
    NvBool bScrubChannelSetupInProgress;
    NvBool bBug3922001DisableCtxBufOnSim;
    NvBool bPlatformFullyCoherent;
    NvBool bAllowNoncontiguousAllocation;
    NvBool bLocalEgmSupported;
    struct __nvoc_inner_struc_MemoryManager_1__ localEgmOverride;
    NvBool bLocalEgmEnabled;
    NvU32 localEgmPeerId;
    NvS32 localEgmNodeId;
    NvU64 localEgmBasePhysAddr;
    NvU64 localEgmSize;
    NvBool bForceEnableFlaSysmem;
    NvBool bEccInterleavedVidmemScrub;
    NvBool bScrubberInitialized;
    NvBool bAllowSysmemHugePages;
    NvBool bEccScrubOverride;
    struct Heap *pHeap;
    NvBool bScrubOnFreeEnabled;
    NvBool bFastScrubberEnabled;
    NvBool bFastScrubberSupportsSysmem;
    NvBool bSysmemPageSizeDefaultAllowLargePages;
    NvBool bDisableAsyncScrubforMods;
    NvBool bUseVasForCeMemoryOps;
    NvBool bCePhysicalVidmemAccessNotSupported;
    NvU64 heapStartOffset;
    NvU64 rsvdMemoryBase;
    NvU64 rsvdMemorySize;
    struct CeUtils *pCeUtils;
    NvBool bDisableGlobalCeUtils;
    struct SysmemScrubber *pSysmemScrubber;
    struct RM_POOL_ALLOC_MEM_RESERVE_INFO *pPageLevelReserve;
    struct MIG_MEMORY_PARTITIONING_INFO MIGMemoryPartitioningInfo;
    NV_FB_RSVD_BLOCK_LOG_INFO rsvdBlockInfo;
    NvHandle hClient;
    NvHandle hDevice;
    NvHandle hSubdevice;
    NvBool bReservedMemAtBottom;
    NvU64 bug4146226ReserveOffset;
    NvBool bBug4146226ReserveWar;
    NvBool bMonitoredFenceSupported;
    NvBool b64BitSemaphoresSupported;
    NvBool bGenericKindSupport;
    NvBool bSkipCompressionCheck;
    NvBool bUseVirtualCopyOnSuspend;
    NvBool bLocalizedMemorySupported;
    NvU64 localizedMask;
    NvHandle hGspHeapSysMemHandle;
};


// Vtable with 14 per-class function pointers
struct NVOC_VTABLE__MemoryManager {
    NV_STATUS (*__memmgrConstructEngine__)(struct OBJGPU *, struct MemoryManager * /*this*/, ENGDESCRIPTOR);  // virtual inherited (engstate) base (engstate)
    void (*__memmgrInitMissing__)(struct OBJGPU *, struct MemoryManager * /*this*/);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStatePreInitLocked__)(struct OBJGPU *, struct MemoryManager * /*this*/);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStatePreInitUnlocked__)(struct OBJGPU *, struct MemoryManager * /*this*/);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStateInitLocked__)(struct OBJGPU *, struct MemoryManager * /*this*/);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStateInitUnlocked__)(struct OBJGPU *, struct MemoryManager * /*this*/);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStatePreLoad__)(struct OBJGPU *, struct MemoryManager * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStateLoad__)(struct OBJGPU *, struct MemoryManager * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStatePostLoad__)(struct OBJGPU *, struct MemoryManager * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStatePreUnload__)(struct OBJGPU *, struct MemoryManager * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStateUnload__)(struct OBJGPU *, struct MemoryManager * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__memmgrStatePostUnload__)(struct OBJGPU *, struct MemoryManager * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    void (*__memmgrStateDestroy__)(struct OBJGPU *, struct MemoryManager * /*this*/);  // virtual inherited (engstate) base (engstate)
    NvBool (*__memmgrIsPresent__)(struct OBJGPU *, struct MemoryManager * /*this*/);  // virtual inherited (engstate) base (engstate)
};

// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__MemoryManager {
    const struct NVOC_RTTI rtti;
    const struct NVOC_METADATA__OBJENGSTATE metadata__OBJENGSTATE;
    const struct NVOC_VTABLE__MemoryManager vtable;
};

#ifndef __nvoc_class_id_MemoryManager
#define __nvoc_class_id_MemoryManager 0x22ad47u
typedef struct MemoryManager MemoryManager;
#endif /* __nvoc_class_id_MemoryManager */

// Casting support
extern const struct NVOC_CLASS_DEF __nvoc_class_def_MemoryManager;

#define __staticCast_MemoryManager(pThis) \
    ((pThis)->__nvoc_pbase_MemoryManager)

#ifdef __nvoc_mem_mgr_h_disabled
#define __dynamicCast_MemoryManager(pThis) ((MemoryManager*) NULL)
#else //__nvoc_mem_mgr_h_disabled
#define __dynamicCast_MemoryManager(pThis) \
    ((MemoryManager*) __nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(MemoryManager)))
#endif //__nvoc_mem_mgr_h_disabled

// Property macros
#define PDB_PROP_MEMMGR_IS_MISSING_BASE_CAST __nvoc_base_OBJENGSTATE.
#define PDB_PROP_MEMMGR_IS_MISSING_BASE_NAME PDB_PROP_ENGSTATE_IS_MISSING


NV_STATUS __nvoc_objCreateDynamic_MemoryManager(MemoryManager**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_MemoryManager(MemoryManager**, Dynamic*, NvU32);
#define __objCreate_MemoryManager(__nvoc_ppNewObj, __nvoc_pParent, __nvoc_createFlags) \
    __nvoc_objCreate_MemoryManager((__nvoc_ppNewObj), staticCast((__nvoc_pParent), Dynamic), (__nvoc_createFlags))


// Wrapper macros for implementation functions
NV_STATUS memmgrAllocResources_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, MEMORY_ALLOCATION_REQUEST *pAllocRequest, FB_ALLOC_INFO *pFbAllocInfo);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrAllocResources(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, MEMORY_ALLOCATION_REQUEST *pAllocRequest, FB_ALLOC_INFO *pFbAllocInfo) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAllocResources(pGpu, pMemoryManager, pAllocRequest, pFbAllocInfo) memmgrAllocResources_IMPL(pGpu, pMemoryManager, pAllocRequest, pFbAllocInfo)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrAddMemNode_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvBool bFreeDescriptor);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrAddMemNode(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvBool bFreeDescriptor) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAddMemNode(pGpu, pMemoryManager, pMemDesc, bFreeDescriptor) memmgrAddMemNode_IMPL(pGpu, pMemoryManager, pMemDesc, bFreeDescriptor)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrAddMemNodes_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvBool bSaveAllRmAllocations);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrAddMemNodes(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvBool bSaveAllRmAllocations) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAddMemNodes(pGpu, pMemoryManager, bSaveAllRmAllocations) memmgrAddMemNodes_IMPL(pGpu, pMemoryManager, bSaveAllRmAllocations)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrRemoveMemNodes_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrRemoveMemNodes(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrRemoveMemNodes(pGpu, pMemoryManager) memmgrRemoveMemNodes_IMPL(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU64 memmgrDeterminePageSize(struct MemoryManager *pMemoryManager, NvHandle hClient, NvU64 memSize, NvU32 memFormat, NvU32 pageFormatFlags, NvU32 *pRetAttr, NvU32 *pRetAttr2) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrDeterminePageSize(pMemoryManager, hClient, memSize, memFormat, pageFormatFlags, pRetAttr, pRetAttr2) memmgrDeterminePageSize_3eaa85(pMemoryManager, hClient, memSize, memFormat, pageFormatFlags, pRetAttr, pRetAttr2)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrMemCopy_IMPL(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pDst, TRANSFER_SURFACE *pSrc, NvU32 size, NvU32 flags);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemCopy(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pDst, TRANSFER_SURFACE *pSrc, NvU32 size, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemCopy(pMemoryManager, pDst, pSrc, size, flags) memmgrMemCopy_IMPL(pMemoryManager, pDst, pSrc, size, flags)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrMemsetInBlocks_IMPL(struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvU32 value, NvU64 baseOffset, NvU64 size, NvU32 flags, NvU32 blockSize);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemsetInBlocks(struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvU32 value, NvU64 baseOffset, NvU64 size, NvU32 flags, NvU32 blockSize) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemsetInBlocks(pMemoryManager, pMemDesc, value, baseOffset, size, flags, blockSize) memmgrMemsetInBlocks_IMPL(pMemoryManager, pMemDesc, value, baseOffset, size, flags, blockSize)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrMemSet_IMPL(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pDst, NvU32 value, NvU32 size, NvU32 flags);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemSet(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pDst, NvU32 value, NvU32 size, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemSet(pMemoryManager, pDst, value, size, flags) memmgrMemSet_IMPL(pMemoryManager, pDst, value, size, flags)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrMemWrite_IMPL(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pDst, void *pBuf, NvU64 size, NvU32 flags);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemWrite(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pDst, void *pBuf, NvU64 size, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemWrite(pMemoryManager, pDst, pBuf, size, flags) memmgrMemWrite_IMPL(pMemoryManager, pDst, pBuf, size, flags)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrMemRead_IMPL(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pSrc, void *pBuf, NvU64 size, NvU32 flags);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemRead(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pSrc, void *pBuf, NvU64 size, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemRead(pMemoryManager, pSrc, pBuf, size, flags) memmgrMemRead_IMPL(pMemoryManager, pSrc, pBuf, size, flags)
#endif // __nvoc_mem_mgr_h_disabled

NvU8 * memmgrMemBeginTransfer_IMPL(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pTransferInfo, NvU64 shadowBufSize, NvU32 flags);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU8 * memmgrMemBeginTransfer(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pTransferInfo, NvU64 shadowBufSize, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NULL;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemBeginTransfer(pMemoryManager, pTransferInfo, shadowBufSize, flags) memmgrMemBeginTransfer_IMPL(pMemoryManager, pTransferInfo, shadowBufSize, flags)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrMemEndTransfer_IMPL(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pTransferInfo, NvU64 shadowBufSize, NvU32 flags);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrMemEndTransfer(struct MemoryManager *pMemoryManager, TRANSFER_SURFACE *pTransferInfo, NvU64 shadowBufSize, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemEndTransfer(pMemoryManager, pTransferInfo, shadowBufSize, flags) memmgrMemEndTransfer_IMPL(pMemoryManager, pTransferInfo, shadowBufSize, flags)
#endif // __nvoc_mem_mgr_h_disabled

NvU8 * memmgrMemDescBeginTransfer_IMPL(struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvU32 flags);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU8 * memmgrMemDescBeginTransfer(struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NULL;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemDescBeginTransfer(pMemoryManager, pMemDesc, flags) memmgrMemDescBeginTransfer_IMPL(pMemoryManager, pMemDesc, flags)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrMemDescEndTransfer_IMPL(struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvU32 flags);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrMemDescEndTransfer(struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemDescEndTransfer(pMemoryManager, pMemDesc, flags) memmgrMemDescEndTransfer_IMPL(pMemoryManager, pMemDesc, flags)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrMemDescMemSet_IMPL(struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvU32 value, NvU32 flags);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemDescMemSet(struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR *pMemDesc, NvU32 value, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemDescMemSet(pMemoryManager, pMemDesc, value, flags) memmgrMemDescMemSet_IMPL(pMemoryManager, pMemDesc, value, flags)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrFreeHwResources(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrFreeHwResources(pGpu, pMemoryManager, arg3) memmgrFreeHwResources_395e98(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrScrubInit(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrScrubInit(pGpu, pMemoryManager) memmgrScrubInit_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrScrubHandlePostSchedulingEnable(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrScrubHandlePostSchedulingEnable(pGpu, pMemoryManager) memmgrScrubHandlePostSchedulingEnable_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrInitInternalChannels_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrInitInternalChannels(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrInitInternalChannels(pGpu, pMemoryManager) memmgrInitInternalChannels_IMPL(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrDestroyInternalChannels_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrDestroyInternalChannels(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrDestroyInternalChannels(pGpu, pMemoryManager) memmgrDestroyInternalChannels_IMPL(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrGetScrubState(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *arg3, NvU64 *arg4, NvBool *arg5) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetScrubState(pGpu, pMemoryManager, arg3, arg4, arg5) memmgrGetScrubState_f2d351(pGpu, pMemoryManager, arg3, arg4, arg5)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrScrubInternalRegions(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrScrubInternalRegions(pGpu, pMemoryManager) memmgrScrubInternalRegions_d44104(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrEccScrubInProgress(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrEccScrubInProgress(pGpu, pMemoryManager) memmgrEccScrubInProgress_d69453(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrAsyncScrubRegion(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 arg3, NvU64 arg4) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAsyncScrubRegion(pGpu, pMemoryManager, arg3, arg4) memmgrAsyncScrubRegion_f2d351(pGpu, pMemoryManager, arg3, arg4)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrScrubHandlePreSchedulingDisable(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrScrubHandlePreSchedulingDisable(pGpu, pMemoryManager) memmgrScrubHandlePreSchedulingDisable_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrScrubDestroy(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrScrubDestroy(pGpu, pMemoryManager) memmgrScrubDestroy_d44104(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrScrubMemory(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, RmPhysAddr arg3, NvU64 arg4) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrScrubMemory(pGpu, pMemoryManager, arg3, arg4) memmgrScrubMemory_d44104(pGpu, pMemoryManager, arg3, arg4)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemUtilsMemSetBlocking(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3, RmPhysAddr arg4, NvU64 arg5) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsMemSetBlocking(pGpu, pMemoryManager, arg3, arg4, arg5) memmgrMemUtilsMemSetBlocking_92bfc3(pGpu, pMemoryManager, arg3, arg4, arg5)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemUtilsMemSet(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3, RmPhysAddr arg4, NvU64 arg5, NvU32 arg6, NvU32 *arg7) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsMemSet(pGpu, pMemoryManager, arg3, arg4, arg5, arg6, arg7) memmgrMemUtilsMemSet_92bfc3(pGpu, pMemoryManager, arg3, arg4, arg5, arg6, arg7)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemUtilsAllocateEccScrubber(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsAllocateEccScrubber(pGpu, pMemoryManager, arg3) memmgrMemUtilsAllocateEccScrubber_92bfc3(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemUtilsAllocateEccAllocScrubber(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsAllocateEccAllocScrubber(pGpu, pMemoryManager, arg3) memmgrMemUtilsAllocateEccAllocScrubber_92bfc3(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemUtilsChannelInitialize(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsChannelInitialize(pGpu, pMemoryManager, arg3) memmgrMemUtilsChannelInitialize_ac1694(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemUtilsCopyEngineInitialize(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsCopyEngineInitialize(pGpu, pMemoryManager, arg3) memmgrMemUtilsCopyEngineInitialize_ac1694(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemUtilsSec2CtxInit(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsSec2CtxInit(pGpu, pMemoryManager, arg3) memmgrMemUtilsSec2CtxInit_395e98(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemUtilsGetCopyEngineClass(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 *pClass) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsGetCopyEngineClass(pGpu, pMemoryManager, pClass) memmgrMemUtilsGetCopyEngineClass_ac1694(pGpu, pMemoryManager, pClass)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrMemUtilsCreateMemoryAlias(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsCreateMemoryAlias(pGpu, pMemoryManager, arg3) memmgrMemUtilsCreateMemoryAlias_ac1694(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrMemUtilsCheckMemoryFastScrubEnable(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 arg3, NvBool arg4, RmPhysAddr arg5, NvU32 arg6, NV_ADDRESS_SPACE arg7) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsCheckMemoryFastScrubEnable(pGpu, pMemoryManager, arg3, arg4, arg5, arg6, arg7) memmgrMemUtilsCheckMemoryFastScrubEnable_d69453(pGpu, pMemoryManager, arg3, arg4, arg5, arg6, arg7)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrInitCeUtils_IMPL(struct MemoryManager *pMemoryManager, NvBool bFifoLite, NvBool bVirtualMode);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrInitCeUtils(struct MemoryManager *pMemoryManager, NvBool bFifoLite, NvBool bVirtualMode) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrInitCeUtils(pMemoryManager, bFifoLite, bVirtualMode) memmgrInitCeUtils_IMPL(pMemoryManager, bFifoLite, bVirtualMode)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrDestroyCeUtils_IMPL(struct MemoryManager *pMemoryManager);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrDestroyCeUtils(struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrDestroyCeUtils(pMemoryManager) memmgrDestroyCeUtils_IMPL(pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrGetInternalClientHandles_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct Device *arg3, NvHandle *arg4, NvHandle *arg5, NvHandle *arg6);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrGetInternalClientHandles(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct Device *arg3, NvHandle *arg4, NvHandle *arg5, NvHandle *arg6) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetInternalClientHandles(pGpu, pMemoryManager, arg3, arg4, arg5, arg6) memmgrGetInternalClientHandles_IMPL(pGpu, pMemoryManager, arg3, arg4, arg5, arg6)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrAllocHal(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *pFbAllocInfo) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAllocHal(pGpu, pMemoryManager, pFbAllocInfo) memmgrAllocHal_92bfc3(pGpu, pMemoryManager, pFbAllocInfo)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrFreeHal(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *pFbAllocInfo, PRMTIMEOUT pTimeout) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrFreeHal(pGpu, pMemoryManager, pFbAllocInfo, pTimeout) memmgrFreeHal_92bfc3(pGpu, pMemoryManager, pFbAllocInfo, pTimeout)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrUpdateSurfaceCompression(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, Memory *arg3, NvBool arg4) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrUpdateSurfaceCompression(pGpu, pMemoryManager, arg3, arg4) memmgrUpdateSurfaceCompression_5baef9(pGpu, pMemoryManager, arg3, arg4)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrGetBankPlacementData(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 *pBankPlacementLowData) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetBankPlacementData(pGpu, pMemoryManager, pBankPlacementLowData) memmgrGetBankPlacementData_395e98(pGpu, pMemoryManager, pBankPlacementLowData)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrDirtyForPmTest(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvBool partialDirty) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrDirtyForPmTest(pGpu, pMemoryManager, partialDirty) memmgrDirtyForPmTest_d44104(pGpu, pMemoryManager, partialDirty)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU64 memmgrGetReservedHeapSizeMb(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetReservedHeapSizeMb(pGpu, pMemoryManager) memmgrGetReservedHeapSizeMb_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrAllocDetermineAlignment(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *pMemSize, NvU64 *pAlign, NvU64 alignPad, NvU32 allocFlags, NvU32 retAttr, NvU32 retAttr2, NvU64 hwAlignment) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAllocDetermineAlignment(pGpu, pMemoryManager, pMemSize, pAlign, alignPad, allocFlags, retAttr, retAttr2, hwAlignment) memmgrAllocDetermineAlignment_5baef9(pGpu, pMemoryManager, pMemSize, pAlign, alignPad, allocFlags, retAttr, retAttr2, hwAlignment)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrInitFbRegionsHal(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrInitFbRegionsHal(pGpu, pMemoryManager) memmgrInitFbRegionsHal_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU64 memmgrGetMaxContextSize(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetMaxContextSize(pGpu, pMemoryManager) memmgrGetMaxContextSize_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrHandleSizeOverrides(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrHandleSizeOverrides(pGpu, pMemoryManager) memmgrHandleSizeOverrides_d44104(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrFinishHandleSizeOverrides(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrFinishHandleSizeOverrides(pGpu, pMemoryManager) memmgrFinishHandleSizeOverrides_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrGetBAR1InfoForDevice(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct Device *pDevice, PGETBAR1INFO bar1Info) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetBAR1InfoForDevice(pGpu, pMemoryManager, pDevice, bar1Info) memmgrGetBAR1InfoForDevice_395e98(pGpu, pMemoryManager, pDevice, bar1Info)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU64 memmgrGetFbTaxSize(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetFbTaxSize(pGpu, pMemoryManager) memmgrGetFbTaxSize_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU64 memmgrGetVgpuHostRmReservedFb(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 vgpuTypeId) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetVgpuHostRmReservedFb(pGpu, pMemoryManager, vgpuTypeId) memmgrGetVgpuHostRmReservedFb_KERNEL(pGpu, pMemoryManager, vgpuTypeId)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrScrubRegistryOverrides(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrScrubRegistryOverrides(pGpu, pMemoryManager) memmgrScrubRegistryOverrides_d44104(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU64 memmgrGetRsvdSizeForSr(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetRsvdSizeForSr(pGpu, pMemoryManager) memmgrGetRsvdSizeForSr_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrVerifyDepthSurfaceAttrs(struct MemoryManager *pMemoryManager, NvU32 arg2, NvU32 arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrVerifyDepthSurfaceAttrs(pMemoryManager, arg2, arg3) memmgrVerifyDepthSurfaceAttrs_e661f0(pMemoryManager, arg2, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrAllocMemToSaveVgaWorkspace(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR **arg3, MEMORY_DESCRIPTOR **arg4) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAllocMemToSaveVgaWorkspace(pGpu, pMemoryManager, arg3, arg4) memmgrAllocMemToSaveVgaWorkspace_5baef9(pGpu, pMemoryManager, arg3, arg4)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrComparePhysicalAddresses(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 arg3, NvU64 arg4, NvU32 arg5, NvU64 arg6) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrComparePhysicalAddresses(pGpu, pMemoryManager, arg3, arg4, arg5, arg6) memmgrComparePhysicalAddresses_86b752(pGpu, pMemoryManager, arg3, arg4, arg5, arg6)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline RmPhysAddr memmgrGetInvalidOffset(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetInvalidOffset(pGpu, pMemoryManager) memmgrGetInvalidOffset_0b83bf(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU64 memmgrGetAddrSpaceSizeMB(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetAddrSpaceSizeMB(pGpu, pMemoryManager) memmgrGetAddrSpaceSizeMB_474d46(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU64 memmgrGetUsableMemSizeMB(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetUsableMemSizeMB(pGpu, pMemoryManager) memmgrGetUsableMemSizeMB_13cd8d(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrVerifyComprAttrs(struct MemoryManager *pMemoryManager, NvU32 arg2, NvU32 arg3, NvU32 arg4) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrVerifyComprAttrs(pMemoryManager, arg2, arg3, arg4) memmgrVerifyComprAttrs_e661f0(pMemoryManager, arg2, arg3, arg4)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrIsKindCompressible(struct MemoryManager *pMemoryManager, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrIsKindCompressible(pMemoryManager, arg2) memmgrIsKindCompressible_d69453(pMemoryManager, arg2)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrIsKindBlocklinear(struct MemoryManager *pMemoryManager, NvU32 arg2) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrIsKindBlocklinear(pMemoryManager, arg2) memmgrIsKindBlocklinear_d69453(pMemoryManager, arg2)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetPteKindBl(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetPteKindBl(pGpu, pMemoryManager) memmgrGetPteKindBl_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetPteKindPitch(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetPteKindPitch(pGpu, pMemoryManager) memmgrGetPteKindPitch_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrChooseKindZ(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_PAGE_FORMAT *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrChooseKindZ(pGpu, pMemoryManager, arg3) memmgrChooseKindZ_474d46(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrChooseKindCompressZ(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_PAGE_FORMAT *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrChooseKindCompressZ(pGpu, pMemoryManager, arg3) memmgrChooseKindCompressZ_474d46(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrChooseKindCompressC(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_PAGE_FORMAT *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrChooseKindCompressC(pGpu, pMemoryManager, arg3) memmgrChooseKindCompressC_474d46(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrChooseKindCompressCForMS2(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrChooseKindCompressCForMS2(pGpu, pMemoryManager, arg3) memmgrChooseKindCompressCForMS2_b2b553(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetPteKindGenericMemoryCompressible(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetPteKindGenericMemoryCompressible(pGpu, pMemoryManager) memmgrGetPteKindGenericMemoryCompressible_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetUncompressedKind(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 kind, NvBool releaseReacquire) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetUncompressedKind(pGpu, pMemoryManager, kind, releaseReacquire) memmgrGetUncompressedKind_474d46(pGpu, pMemoryManager, kind, releaseReacquire)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrGetUncompressedKindForMS2(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 arg3, NvU32 *arg4) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetUncompressedKindForMS2(pGpu, pMemoryManager, arg3, arg4) memmgrGetUncompressedKindForMS2_5baef9(pGpu, pMemoryManager, arg3, arg4)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetCompressedKind(struct MemoryManager *pMemoryManager, NvU32 kind, NvBool bDisablePlc) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetCompressedKind(pMemoryManager, kind, bDisablePlc) memmgrGetCompressedKind_d1515c(pMemoryManager, kind, bDisablePlc)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrChooseKind(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_PAGE_FORMAT *arg3, NvU32 arg4, NvU32 *pKind) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrChooseKind(pGpu, pMemoryManager, arg3, arg4, pKind) memmgrChooseKind_f4fe90(pGpu, pMemoryManager, arg3, arg4, pKind)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrIsKind(struct MemoryManager *pMemoryManager, FB_IS_KIND_OP arg2, NvU32 arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrIsKind(pMemoryManager, arg2, arg3) memmgrIsKind_TU102(pMemoryManager, arg2, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetMessageKind(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetMessageKind(pGpu, pMemoryManager) memmgrGetMessageKind_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetDefaultPteKindForNoHandle(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetDefaultPteKindForNoHandle(pGpu, pMemoryManager) memmgrGetDefaultPteKindForNoHandle_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrIsSurfaceBlockLinear(struct MemoryManager *pMemoryManager, Memory *arg2, NvU32 arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrIsSurfaceBlockLinear(pMemoryManager, arg2, arg3) memmgrIsSurfaceBlockLinear_TU102(pMemoryManager, arg2, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrGetFlaKind(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetFlaKind(pGpu, pMemoryManager, arg3) memmgrGetFlaKind_395e98(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrIsMemDescSupportedByFla(OBJGPU *arg1, struct MemoryManager *arg_this, MEMORY_DESCRIPTOR *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrIsMemDescSupportedByFla(arg1, arg_this, arg3) memmgrIsMemDescSupportedByFla_395e98(arg1, arg_this, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrIsValidFlaPageSize(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 pageSize, NvBool bIsMulticast) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrIsValidFlaPageSize(pGpu, pMemoryManager, pageSize, bIsMulticast) memmgrIsValidFlaPageSize_d69453(pGpu, pMemoryManager, pageSize, bIsMulticast)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetHwPteKindFromSwPteKind(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 pteKind) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetHwPteKindFromSwPteKind(pGpu, pMemoryManager, pteKind) memmgrGetHwPteKindFromSwPteKind_26237f(pGpu, pMemoryManager, pteKind)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetSwPteKindFromHwPteKind(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 pteKind) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetSwPteKindFromHwPteKind(pGpu, pMemoryManager, pteKind) memmgrGetSwPteKindFromHwPteKind_26237f(pGpu, pMemoryManager, pteKind)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrGetPteKindForScrubber(struct MemoryManager *pMemoryManager, NvU32 *arg2) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetPteKindForScrubber(pMemoryManager, arg2) memmgrGetPteKindForScrubber_f2d351(pMemoryManager, arg2)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetCtagOffsetFromParams(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetCtagOffsetFromParams(pGpu, pMemoryManager, arg3) memmgrGetCtagOffsetFromParams_fc0f62(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrSetCtagOffsetInParams(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *arg3, NvU32 arg4) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrSetCtagOffsetInParams(pGpu, pMemoryManager, arg3, arg4) memmgrSetCtagOffsetInParams_d44104(pGpu, pMemoryManager, arg3, arg4)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrDetermineComptag(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, RmPhysAddr arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrDetermineComptag(pGpu, pMemoryManager, arg3) memmgrDetermineComptag_13cd8d(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrChannelPushSemaphoreMethodsBlock(struct MemoryManager *pMemoryManager, NvU32 arg2, NvU64 arg3, NvU32 arg4, NvU32 **arg5) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrChannelPushSemaphoreMethodsBlock(pMemoryManager, arg2, arg3, arg4, arg5) memmgrChannelPushSemaphoreMethodsBlock_f2d351(pMemoryManager, arg2, arg3, arg4, arg5)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrChannelPushAddressMethodsBlock(struct MemoryManager *pMemoryManager, NvBool arg2, NvU32 arg3, RmPhysAddr arg4, NvU32 **arg5) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrChannelPushAddressMethodsBlock(pMemoryManager, arg2, arg3, arg4, arg5) memmgrChannelPushAddressMethodsBlock_f2d351(pMemoryManager, arg2, arg3, arg4, arg5)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrScrubMapDoorbellRegion(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrScrubMapDoorbellRegion(pGpu, pMemoryManager, arg3) memmgrScrubMapDoorbellRegion_ac1694(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrSetAllocParameters(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *pFbAllocInfo) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrSetAllocParameters(pGpu, pMemoryManager, pFbAllocInfo) memmgrSetAllocParameters_dffb6f(pGpu, pMemoryManager, pFbAllocInfo)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrCalcReservedFbSpaceForUVM(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrCalcReservedFbSpaceForUVM(pGpu, pMemoryManager, arg3) memmgrCalcReservedFbSpaceForUVM_d44104(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrCalcReservedFbSpaceHal(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *arg3, NvU64 *arg4, NvU64 *arg5) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrCalcReservedFbSpaceHal(pGpu, pMemoryManager, arg3, arg4, arg5) memmgrCalcReservedFbSpaceHal_d44104(pGpu, pMemoryManager, arg3, arg4, arg5)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetGrHeapReservationSize(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetGrHeapReservationSize(pGpu, pMemoryManager) memmgrGetGrHeapReservationSize_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetRunlistEntriesReservedFbSpace(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetRunlistEntriesReservedFbSpace(pGpu, pMemoryManager) memmgrGetRunlistEntriesReservedFbSpace_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetUserdReservedFbSpace(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetUserdReservedFbSpace(pGpu, pMemoryManager) memmgrGetUserdReservedFbSpace_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrCheckReservedMemorySize(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrCheckReservedMemorySize(pGpu, pMemoryManager) memmgrCheckReservedMemorySize_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrInitReservedMemory(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrInitReservedMemory(pGpu, pMemoryManager, arg3) memmgrInitReservedMemory_ac1694(pGpu, pMemoryManager, arg3)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrPreInitReservedMemory(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrPreInitReservedMemory(pGpu, pMemoryManager) memmgrPreInitReservedMemory_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrReadMmuLock(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvBool *pbIsValid, NvU64 *pMmuLockLo, NvU64 *pMmuLockHi) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrReadMmuLock(pGpu, pMemoryManager, pbIsValid, pMmuLockLo, pMmuLockHi) memmgrReadMmuLock_ccda6f(pGpu, pMemoryManager, pbIsValid, pMmuLockLo, pMmuLockHi)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrBlockMemLockedMemory(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrBlockMemLockedMemory(pGpu, pMemoryManager) memmgrBlockMemLockedMemory_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrInsertUnprotectedRegionAtBottomOfFb(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *pSize) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrInsertUnprotectedRegionAtBottomOfFb(pGpu, pMemoryManager, pSize) memmgrInsertUnprotectedRegionAtBottomOfFb_ac1694(pGpu, pMemoryManager, pSize)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrInitBaseFbRegions(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrInitBaseFbRegions(pGpu, pMemoryManager) memmgrInitBaseFbRegions_FWCLIENT(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrGetDisablePlcKind(struct MemoryManager *pMemoryManager, NvU32 *pteKind) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetDisablePlcKind(pMemoryManager, pteKind) memmgrGetDisablePlcKind_d44104(pMemoryManager, pteKind)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrEnableDynamicPageOfflining(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrEnableDynamicPageOfflining(pGpu, pMemoryManager) memmgrEnableDynamicPageOfflining_d44104(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrSetMemDescPageSize(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, PMEMORY_DESCRIPTOR arg3, ADDRESS_TRANSLATION arg4, RM_ATTR_PAGE_SIZE arg5) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrSetMemDescPageSize(pGpu, pMemoryManager, arg3, arg4, arg5) memmgrSetMemDescPageSize_ac1694(pGpu, pMemoryManager, arg3, arg4, arg5)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrSetPartitionableMem(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrSetPartitionableMem(pGpu, pMemoryManager) memmgrSetPartitionableMem_IMPL(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrSetMIGPartitionableBAR1Range_IMPL(OBJGPU *arg1, struct MemoryManager *arg_this);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrSetMIGPartitionableBAR1Range(OBJGPU *arg1, struct MemoryManager *arg_this) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrSetMIGPartitionableBAR1Range(arg1, arg_this) memmgrSetMIGPartitionableBAR1Range_IMPL(arg1, arg_this)
#endif // __nvoc_mem_mgr_h_disabled

struct NV_RANGE memmgrGetMIGPartitionableBAR1Range_IMPL(OBJGPU *arg1, struct MemoryManager *arg_this);
#ifdef __nvoc_mem_mgr_h_disabled
static inline struct NV_RANGE memmgrGetMIGPartitionableBAR1Range(OBJGPU *arg1, struct MemoryManager *arg_this) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    struct NV_RANGE ret;
    portMemSet(&ret, 0, sizeof(struct NV_RANGE));
    return ret;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetMIGPartitionableBAR1Range(arg1, arg_this) memmgrGetMIGPartitionableBAR1Range_IMPL(arg1, arg_this)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrSetMIGPartitionableMemoryRange_IMPL(OBJGPU *arg1, struct MemoryManager *arg_this, struct NV_RANGE arg3);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrSetMIGPartitionableMemoryRange(OBJGPU *arg1, struct MemoryManager *arg_this, struct NV_RANGE arg3) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrSetMIGPartitionableMemoryRange(arg1, arg_this, arg3) memmgrSetMIGPartitionableMemoryRange_IMPL(arg1, arg_this, arg3)
#endif // __nvoc_mem_mgr_h_disabled

struct NV_RANGE memmgrGetMIGPartitionableMemoryRange_IMPL(OBJGPU *arg1, struct MemoryManager *arg_this);
#ifdef __nvoc_mem_mgr_h_disabled
static inline struct NV_RANGE memmgrGetMIGPartitionableMemoryRange(OBJGPU *arg1, struct MemoryManager *arg_this) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    struct NV_RANGE ret;
    portMemSet(&ret, 0, sizeof(struct NV_RANGE));
    return ret;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetMIGPartitionableMemoryRange(arg1, arg_this) memmgrGetMIGPartitionableMemoryRange_IMPL(arg1, arg_this)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrAllocMIGGPUInstanceMemory(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 swizzId, NvHandle *phMemory, struct NV_RANGE *pAddrRange, struct Heap **ppMemoryPartitionHeap) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAllocMIGGPUInstanceMemory(pGpu, pMemoryManager, swizzId, phMemory, pAddrRange, ppMemoryPartitionHeap) memmgrAllocMIGGPUInstanceMemory_PF(pGpu, pMemoryManager, swizzId, phMemory, pAddrRange, ppMemoryPartitionHeap)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrFreeMIGGPUInstanceMemory_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 swizzId, NvHandle hMemory, struct Heap **ppMemoryPartitionHeap);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrFreeMIGGPUInstanceMemory(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 swizzId, NvHandle hMemory, struct Heap **ppMemoryPartitionHeap) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrFreeMIGGPUInstanceMemory(pGpu, pMemoryManager, swizzId, hMemory, ppMemoryPartitionHeap) memmgrFreeMIGGPUInstanceMemory_IMPL(pGpu, pMemoryManager, swizzId, hMemory, ppMemoryPartitionHeap)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrGetBlackListPagesForHeap(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct Heap *pHeap) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetBlackListPagesForHeap(pGpu, pMemoryManager, pHeap) memmgrGetBlackListPagesForHeap_395e98(pGpu, pMemoryManager, pHeap)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrGetBlackListPages(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, BLACKLIST_ADDRESS *pBlAddrs, NvU32 *pCount) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetBlackListPages(pGpu, pMemoryManager, pBlAddrs, pCount) memmgrGetBlackListPages_395e98(pGpu, pMemoryManager, pBlAddrs, pCount)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrPageLevelPoolsCreate_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrPageLevelPoolsCreate(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrPageLevelPoolsCreate(pGpu, pMemoryManager) memmgrPageLevelPoolsCreate_IMPL(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrPageLevelPoolsDestroy_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrPageLevelPoolsDestroy(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrPageLevelPoolsDestroy(pGpu, pMemoryManager) memmgrPageLevelPoolsDestroy_IMPL(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrPageLevelPoolsGetInfo_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct Device *pDevice, struct RM_POOL_ALLOC_MEM_RESERVE_INFO **arg4);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrPageLevelPoolsGetInfo(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct Device *pDevice, struct RM_POOL_ALLOC_MEM_RESERVE_INFO **arg4) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrPageLevelPoolsGetInfo(pGpu, pMemoryManager, pDevice, arg4) memmgrPageLevelPoolsGetInfo_IMPL(pGpu, pMemoryManager, pDevice, arg4)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrAllocMIGMemoryAllocationInternalHandles_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrAllocMIGMemoryAllocationInternalHandles(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAllocMIGMemoryAllocationInternalHandles(pGpu, pMemoryManager) memmgrAllocMIGMemoryAllocationInternalHandles_IMPL(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrFreeMIGMemoryAllocationInternalHandles_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrFreeMIGMemoryAllocationInternalHandles(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrFreeMIGMemoryAllocationInternalHandles(pGpu, pMemoryManager) memmgrFreeMIGMemoryAllocationInternalHandles_IMPL(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrGetFreeMemoryForAllMIGGPUInstances_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *pBytes);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrGetFreeMemoryForAllMIGGPUInstances(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *pBytes) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetFreeMemoryForAllMIGGPUInstances(pGpu, pMemoryManager, pBytes) memmgrGetFreeMemoryForAllMIGGPUInstances_IMPL(pGpu, pMemoryManager, pBytes)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrGetTotalMemoryForAllMIGGPUInstances_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *pBytes);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrGetTotalMemoryForAllMIGGPUInstances(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *pBytes) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetTotalMemoryForAllMIGGPUInstances(pGpu, pMemoryManager, pBytes) memmgrGetTotalMemoryForAllMIGGPUInstances_IMPL(pGpu, pMemoryManager, pBytes)
#endif // __nvoc_mem_mgr_h_disabled

void memmgrGetTopLevelScrubberStatus_IMPL(OBJGPU *arg1, struct MemoryManager *arg_this, NvBool *pbTopLevelScrubberEnabled, NvBool *pbTopLevelScrubberConstructed);
#ifdef __nvoc_mem_mgr_h_disabled
static inline void memmgrGetTopLevelScrubberStatus(OBJGPU *arg1, struct MemoryManager *arg_this, NvBool *pbTopLevelScrubberEnabled, NvBool *pbTopLevelScrubberConstructed) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetTopLevelScrubberStatus(arg1, arg_this, pbTopLevelScrubberEnabled, pbTopLevelScrubberConstructed) memmgrGetTopLevelScrubberStatus_IMPL(arg1, arg_this, pbTopLevelScrubberEnabled, pbTopLevelScrubberConstructed)
#endif // __nvoc_mem_mgr_h_disabled

MEMORY_DESCRIPTOR * memmgrMemUtilsGetMemDescFromHandle_IMPL(struct MemoryManager *pMemoryManager, NvHandle hClient, NvHandle hMemory);
#ifdef __nvoc_mem_mgr_h_disabled
static inline MEMORY_DESCRIPTOR * memmgrMemUtilsGetMemDescFromHandle(struct MemoryManager *pMemoryManager, NvHandle hClient, NvHandle hMemory) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NULL;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrMemUtilsGetMemDescFromHandle(pMemoryManager, hClient, hMemory) memmgrMemUtilsGetMemDescFromHandle_IMPL(pMemoryManager, hClient, hMemory)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrVerifyGspDmaOps_IMPL(OBJGPU *arg1, struct MemoryManager *arg_this);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrVerifyGspDmaOps(OBJGPU *arg1, struct MemoryManager *arg_this) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrVerifyGspDmaOps(arg1, arg_this) memmgrVerifyGspDmaOps_IMPL(arg1, arg_this)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrDiscoverMIGPartitionableMemoryRange(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct NV_RANGE *pMemoryRange) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrDiscoverMIGPartitionableMemoryRange(pGpu, pMemoryManager, pMemoryRange) memmgrDiscoverMIGPartitionableMemoryRange_395e98(pGpu, pMemoryManager, pMemoryRange)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU32 memmgrGetFBEndReserveSizeEstimate(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetFBEndReserveSizeEstimate(pGpu, pMemoryManager) memmgrGetFBEndReserveSizeEstimate_GM107(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrValidateFBEndReservation(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrValidateFBEndReservation(pGpu, pMemoryManager) memmgrValidateFBEndReservation_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrAllocReservedFBRegionMemdesc_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR **ppMemdesc, NvU64 rangeStart, NvU64 allocSize, NvU64 memdescFlags, NV_FB_ALLOC_RM_INTERNAL_OWNER allocTag);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrAllocReservedFBRegionMemdesc(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR **ppMemdesc, NvU64 rangeStart, NvU64 allocSize, NvU64 memdescFlags, NV_FB_ALLOC_RM_INTERNAL_OWNER allocTag) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrAllocReservedFBRegionMemdesc(pGpu, pMemoryManager, ppMemdesc, rangeStart, allocSize, memdescFlags, allocTag) memmgrAllocReservedFBRegionMemdesc_IMPL(pGpu, pMemoryManager, ppMemdesc, rangeStart, allocSize, memdescFlags, allocTag)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrReserveMemoryForFakeWPR(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrReserveMemoryForFakeWPR(pGpu, pMemoryManager) memmgrReserveMemoryForFakeWPR_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrReserveMemoryForPmu(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrReserveMemoryForPmu(pGpu, pMemoryManager) memmgrReserveMemoryForPmu_ac1694(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

NV_STATUS memmgrReserveMemoryForFsp_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);
#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrReserveMemoryForFsp(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrReserveMemoryForFsp(pGpu, pMemoryManager) memmgrReserveMemoryForFsp_IMPL(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrReserveVgaWorkspaceMemDescForFbsr(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrReserveVgaWorkspaceMemDescForFbsr(pGpu, pMemoryManager) memmgrReserveVgaWorkspaceMemDescForFbsr_395e98(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrCalculateHeapOffsetWithGSP(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 *offset) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrCalculateHeapOffsetWithGSP(pGpu, pMemoryManager, offset) memmgrCalculateHeapOffsetWithGSP_395e98(pGpu, pMemoryManager, offset)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrGetCarveoutRegionInfo(POBJGPU pGpu, struct MemoryManager *pMemoryManager, NV2080_CTRL_FB_GET_CARVEOUT_REGION_INFO_PARAMS *pParams) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetCarveoutRegionInfo(pGpu, pMemoryManager, pParams) memmgrGetCarveoutRegionInfo_KERNEL(pGpu, pMemoryManager, pParams)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrIsMemoryIoCoherent(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NV_MEMORY_ALLOCATION_PARAMS *pAllocData) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrIsMemoryIoCoherent(pGpu, pMemoryManager, pAllocData) memmgrIsMemoryIoCoherent_e661f0(pGpu, pMemoryManager, pAllocData)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NV_STATUS memmgrSc7SrInitGsp(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrSc7SrInitGsp(pGpu, pMemoryManager) memmgrSc7SrInitGsp_395e98(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvU8 memmgrGetLocalizedOffset(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return 0;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetLocalizedOffset(pGpu, pMemoryManager) memmgrGetLocalizedOffset_b2b553(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrIsFlaSysmemSupported(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrIsFlaSysmemSupported(pGpu, pMemoryManager) memmgrIsFlaSysmemSupported_d69453(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled

#ifdef __nvoc_mem_mgr_h_disabled
static inline NvBool memmgrGetLocalizedMemorySupported(OBJGPU *pGpu, struct MemoryManager *pMemoryManager) {
    NV_ASSERT_FAILED_PRECOMP("MemoryManager was disabled!");
    return NV_FALSE;
}
#else // __nvoc_mem_mgr_h_disabled
#define memmgrGetLocalizedMemorySupported(pGpu, pMemoryManager) memmgrGetLocalizedMemorySupported_d69453(pGpu, pMemoryManager)
#endif // __nvoc_mem_mgr_h_disabled


// Wrapper macros for halified functions
#define memmgrDeterminePageSize_HAL(pMemoryManager, hClient, memSize, memFormat, pageFormatFlags, pRetAttr, pRetAttr2) memmgrDeterminePageSize(pMemoryManager, hClient, memSize, memFormat, pageFormatFlags, pRetAttr, pRetAttr2)
#define memmgrFreeHwResources_HAL(pGpu, pMemoryManager, arg3) memmgrFreeHwResources(pGpu, pMemoryManager, arg3)
#define memmgrScrubInit_HAL(pGpu, pMemoryManager) memmgrScrubInit(pGpu, pMemoryManager)
#define memmgrScrubHandlePostSchedulingEnable_HAL(pGpu, pMemoryManager) memmgrScrubHandlePostSchedulingEnable(pGpu, pMemoryManager)
#define memmgrGetScrubState_HAL(pGpu, pMemoryManager, arg3, arg4, arg5) memmgrGetScrubState(pGpu, pMemoryManager, arg3, arg4, arg5)
#define memmgrScrubInternalRegions_HAL(pGpu, pMemoryManager) memmgrScrubInternalRegions(pGpu, pMemoryManager)
#define memmgrEccScrubInProgress_HAL(pGpu, pMemoryManager) memmgrEccScrubInProgress(pGpu, pMemoryManager)
#define memmgrAsyncScrubRegion_HAL(pGpu, pMemoryManager, arg3, arg4) memmgrAsyncScrubRegion(pGpu, pMemoryManager, arg3, arg4)
#define memmgrScrubHandlePreSchedulingDisable_HAL(pGpu, pMemoryManager) memmgrScrubHandlePreSchedulingDisable(pGpu, pMemoryManager)
#define memmgrScrubDestroy_HAL(pGpu, pMemoryManager) memmgrScrubDestroy(pGpu, pMemoryManager)
#define memmgrScrubMemory_HAL(pGpu, pMemoryManager, arg3, arg4) memmgrScrubMemory(pGpu, pMemoryManager, arg3, arg4)
#define memmgrMemUtilsMemSetBlocking_HAL(pGpu, pMemoryManager, arg3, arg4, arg5) memmgrMemUtilsMemSetBlocking(pGpu, pMemoryManager, arg3, arg4, arg5)
#define memmgrMemUtilsMemSet_HAL(pGpu, pMemoryManager, arg3, arg4, arg5, arg6, arg7) memmgrMemUtilsMemSet(pGpu, pMemoryManager, arg3, arg4, arg5, arg6, arg7)
#define memmgrMemUtilsAllocateEccScrubber_HAL(pGpu, pMemoryManager, arg3) memmgrMemUtilsAllocateEccScrubber(pGpu, pMemoryManager, arg3)
#define memmgrMemUtilsAllocateEccAllocScrubber_HAL(pGpu, pMemoryManager, arg3) memmgrMemUtilsAllocateEccAllocScrubber(pGpu, pMemoryManager, arg3)
#define memmgrMemUtilsChannelInitialize_HAL(pGpu, pMemoryManager, arg3) memmgrMemUtilsChannelInitialize(pGpu, pMemoryManager, arg3)
#define memmgrMemUtilsCopyEngineInitialize_HAL(pGpu, pMemoryManager, arg3) memmgrMemUtilsCopyEngineInitialize(pGpu, pMemoryManager, arg3)
#define memmgrMemUtilsSec2CtxInit_HAL(pGpu, pMemoryManager, arg3) memmgrMemUtilsSec2CtxInit(pGpu, pMemoryManager, arg3)
#define memmgrMemUtilsGetCopyEngineClass_HAL(pGpu, pMemoryManager, pClass) memmgrMemUtilsGetCopyEngineClass(pGpu, pMemoryManager, pClass)
#define memmgrMemUtilsCreateMemoryAlias_HAL(pGpu, pMemoryManager, arg3) memmgrMemUtilsCreateMemoryAlias(pGpu, pMemoryManager, arg3)
#define memmgrMemUtilsCheckMemoryFastScrubEnable_HAL(pGpu, pMemoryManager, arg3, arg4, arg5, arg6, arg7) memmgrMemUtilsCheckMemoryFastScrubEnable(pGpu, pMemoryManager, arg3, arg4, arg5, arg6, arg7)
#define memmgrAllocHal_HAL(pGpu, pMemoryManager, pFbAllocInfo) memmgrAllocHal(pGpu, pMemoryManager, pFbAllocInfo)
#define memmgrFreeHal_HAL(pGpu, pMemoryManager, pFbAllocInfo, pTimeout) memmgrFreeHal(pGpu, pMemoryManager, pFbAllocInfo, pTimeout)
#define memmgrUpdateSurfaceCompression_HAL(pGpu, pMemoryManager, arg3, arg4) memmgrUpdateSurfaceCompression(pGpu, pMemoryManager, arg3, arg4)
#define memmgrGetBankPlacementData_HAL(pGpu, pMemoryManager, pBankPlacementLowData) memmgrGetBankPlacementData(pGpu, pMemoryManager, pBankPlacementLowData)
#define memmgrDirtyForPmTest_HAL(pGpu, pMemoryManager, partialDirty) memmgrDirtyForPmTest(pGpu, pMemoryManager, partialDirty)
#define memmgrGetReservedHeapSizeMb_HAL(pGpu, pMemoryManager) memmgrGetReservedHeapSizeMb(pGpu, pMemoryManager)
#define memmgrAllocDetermineAlignment_HAL(pGpu, pMemoryManager, pMemSize, pAlign, alignPad, allocFlags, retAttr, retAttr2, hwAlignment) memmgrAllocDetermineAlignment(pGpu, pMemoryManager, pMemSize, pAlign, alignPad, allocFlags, retAttr, retAttr2, hwAlignment)
#define memmgrInitFbRegionsHal_HAL(pGpu, pMemoryManager) memmgrInitFbRegionsHal(pGpu, pMemoryManager)
#define memmgrGetMaxContextSize_HAL(pGpu, pMemoryManager) memmgrGetMaxContextSize(pGpu, pMemoryManager)
#define memmgrHandleSizeOverrides_HAL(pGpu, pMemoryManager) memmgrHandleSizeOverrides(pGpu, pMemoryManager)
#define memmgrFinishHandleSizeOverrides_HAL(pGpu, pMemoryManager) memmgrFinishHandleSizeOverrides(pGpu, pMemoryManager)
#define memmgrGetBAR1InfoForDevice_HAL(pGpu, pMemoryManager, pDevice, bar1Info) memmgrGetBAR1InfoForDevice(pGpu, pMemoryManager, pDevice, bar1Info)
#define memmgrGetFbTaxSize_HAL(pGpu, pMemoryManager) memmgrGetFbTaxSize(pGpu, pMemoryManager)
#define memmgrGetVgpuHostRmReservedFb_HAL(pGpu, pMemoryManager, vgpuTypeId) memmgrGetVgpuHostRmReservedFb(pGpu, pMemoryManager, vgpuTypeId)
#define memmgrScrubRegistryOverrides_HAL(pGpu, pMemoryManager) memmgrScrubRegistryOverrides(pGpu, pMemoryManager)
#define memmgrGetRsvdSizeForSr_HAL(pGpu, pMemoryManager) memmgrGetRsvdSizeForSr(pGpu, pMemoryManager)
#define memmgrVerifyDepthSurfaceAttrs_HAL(pMemoryManager, arg2, arg3) memmgrVerifyDepthSurfaceAttrs(pMemoryManager, arg2, arg3)
#define memmgrAllocMemToSaveVgaWorkspace_HAL(pGpu, pMemoryManager, arg3, arg4) memmgrAllocMemToSaveVgaWorkspace(pGpu, pMemoryManager, arg3, arg4)
#define memmgrComparePhysicalAddresses_HAL(pGpu, pMemoryManager, arg3, arg4, arg5, arg6) memmgrComparePhysicalAddresses(pGpu, pMemoryManager, arg3, arg4, arg5, arg6)
#define memmgrGetInvalidOffset_HAL(pGpu, pMemoryManager) memmgrGetInvalidOffset(pGpu, pMemoryManager)
#define memmgrGetAddrSpaceSizeMB_HAL(pGpu, pMemoryManager) memmgrGetAddrSpaceSizeMB(pGpu, pMemoryManager)
#define memmgrGetUsableMemSizeMB_HAL(pGpu, pMemoryManager) memmgrGetUsableMemSizeMB(pGpu, pMemoryManager)
#define memmgrVerifyComprAttrs_HAL(pMemoryManager, arg2, arg3, arg4) memmgrVerifyComprAttrs(pMemoryManager, arg2, arg3, arg4)
#define memmgrIsKindCompressible_HAL(pMemoryManager, arg2) memmgrIsKindCompressible(pMemoryManager, arg2)
#define memmgrIsKindBlocklinear_HAL(pMemoryManager, arg2) memmgrIsKindBlocklinear(pMemoryManager, arg2)
#define memmgrGetPteKindBl_HAL(pGpu, pMemoryManager) memmgrGetPteKindBl(pGpu, pMemoryManager)
#define memmgrGetPteKindPitch_HAL(pGpu, pMemoryManager) memmgrGetPteKindPitch(pGpu, pMemoryManager)
#define memmgrChooseKindZ_HAL(pGpu, pMemoryManager, arg3) memmgrChooseKindZ(pGpu, pMemoryManager, arg3)
#define memmgrChooseKindCompressZ_HAL(pGpu, pMemoryManager, arg3) memmgrChooseKindCompressZ(pGpu, pMemoryManager, arg3)
#define memmgrChooseKindCompressC_HAL(pGpu, pMemoryManager, arg3) memmgrChooseKindCompressC(pGpu, pMemoryManager, arg3)
#define memmgrChooseKindCompressCForMS2_HAL(pGpu, pMemoryManager, arg3) memmgrChooseKindCompressCForMS2(pGpu, pMemoryManager, arg3)
#define memmgrGetPteKindGenericMemoryCompressible_HAL(pGpu, pMemoryManager) memmgrGetPteKindGenericMemoryCompressible(pGpu, pMemoryManager)
#define memmgrGetUncompressedKind_HAL(pGpu, pMemoryManager, kind, releaseReacquire) memmgrGetUncompressedKind(pGpu, pMemoryManager, kind, releaseReacquire)
#define memmgrGetUncompressedKindForMS2_HAL(pGpu, pMemoryManager, arg3, arg4) memmgrGetUncompressedKindForMS2(pGpu, pMemoryManager, arg3, arg4)
#define memmgrGetCompressedKind_HAL(pMemoryManager, kind, bDisablePlc) memmgrGetCompressedKind(pMemoryManager, kind, bDisablePlc)
#define memmgrChooseKind_HAL(pGpu, pMemoryManager, arg3, arg4, pKind) memmgrChooseKind(pGpu, pMemoryManager, arg3, arg4, pKind)
#define memmgrIsKind_HAL(pMemoryManager, arg2, arg3) memmgrIsKind(pMemoryManager, arg2, arg3)
#define memmgrGetMessageKind_HAL(pGpu, pMemoryManager) memmgrGetMessageKind(pGpu, pMemoryManager)
#define memmgrGetDefaultPteKindForNoHandle_HAL(pGpu, pMemoryManager) memmgrGetDefaultPteKindForNoHandle(pGpu, pMemoryManager)
#define memmgrIsSurfaceBlockLinear_HAL(pMemoryManager, arg2, arg3) memmgrIsSurfaceBlockLinear(pMemoryManager, arg2, arg3)
#define memmgrGetFlaKind_HAL(pGpu, pMemoryManager, arg3) memmgrGetFlaKind(pGpu, pMemoryManager, arg3)
#define memmgrIsMemDescSupportedByFla_HAL(arg1, arg_this, arg3) memmgrIsMemDescSupportedByFla(arg1, arg_this, arg3)
#define memmgrIsValidFlaPageSize_HAL(pGpu, pMemoryManager, pageSize, bIsMulticast) memmgrIsValidFlaPageSize(pGpu, pMemoryManager, pageSize, bIsMulticast)
#define memmgrGetHwPteKindFromSwPteKind_HAL(pGpu, pMemoryManager, pteKind) memmgrGetHwPteKindFromSwPteKind(pGpu, pMemoryManager, pteKind)
#define memmgrGetSwPteKindFromHwPteKind_HAL(pGpu, pMemoryManager, pteKind) memmgrGetSwPteKindFromHwPteKind(pGpu, pMemoryManager, pteKind)
#define memmgrGetPteKindForScrubber_HAL(pMemoryManager, arg2) memmgrGetPteKindForScrubber(pMemoryManager, arg2)
#define memmgrGetCtagOffsetFromParams_HAL(pGpu, pMemoryManager, arg3) memmgrGetCtagOffsetFromParams(pGpu, pMemoryManager, arg3)
#define memmgrSetCtagOffsetInParams_HAL(pGpu, pMemoryManager, arg3, arg4) memmgrSetCtagOffsetInParams(pGpu, pMemoryManager, arg3, arg4)
#define memmgrDetermineComptag_HAL(pGpu, pMemoryManager, arg3) memmgrDetermineComptag(pGpu, pMemoryManager, arg3)
#define memmgrChannelPushSemaphoreMethodsBlock_HAL(pMemoryManager, arg2, arg3, arg4, arg5) memmgrChannelPushSemaphoreMethodsBlock(pMemoryManager, arg2, arg3, arg4, arg5)
#define memmgrChannelPushAddressMethodsBlock_HAL(pMemoryManager, arg2, arg3, arg4, arg5) memmgrChannelPushAddressMethodsBlock(pMemoryManager, arg2, arg3, arg4, arg5)
#define memmgrScrubMapDoorbellRegion_HAL(pGpu, pMemoryManager, arg3) memmgrScrubMapDoorbellRegion(pGpu, pMemoryManager, arg3)
#define memmgrSetAllocParameters_HAL(pGpu, pMemoryManager, pFbAllocInfo) memmgrSetAllocParameters(pGpu, pMemoryManager, pFbAllocInfo)
#define memmgrCalcReservedFbSpaceForUVM_HAL(pGpu, pMemoryManager, arg3) memmgrCalcReservedFbSpaceForUVM(pGpu, pMemoryManager, arg3)
#define memmgrCalcReservedFbSpaceHal_HAL(pGpu, pMemoryManager, arg3, arg4, arg5) memmgrCalcReservedFbSpaceHal(pGpu, pMemoryManager, arg3, arg4, arg5)
#define memmgrGetGrHeapReservationSize_HAL(pGpu, pMemoryManager) memmgrGetGrHeapReservationSize(pGpu, pMemoryManager)
#define memmgrGetRunlistEntriesReservedFbSpace_HAL(pGpu, pMemoryManager) memmgrGetRunlistEntriesReservedFbSpace(pGpu, pMemoryManager)
#define memmgrGetUserdReservedFbSpace_HAL(pGpu, pMemoryManager) memmgrGetUserdReservedFbSpace(pGpu, pMemoryManager)
#define memmgrCheckReservedMemorySize_HAL(pGpu, pMemoryManager) memmgrCheckReservedMemorySize(pGpu, pMemoryManager)
#define memmgrInitReservedMemory_HAL(pGpu, pMemoryManager, arg3) memmgrInitReservedMemory(pGpu, pMemoryManager, arg3)
#define memmgrPreInitReservedMemory_HAL(pGpu, pMemoryManager) memmgrPreInitReservedMemory(pGpu, pMemoryManager)
#define memmgrReadMmuLock_HAL(pGpu, pMemoryManager, pbIsValid, pMmuLockLo, pMmuLockHi) memmgrReadMmuLock(pGpu, pMemoryManager, pbIsValid, pMmuLockLo, pMmuLockHi)
#define memmgrBlockMemLockedMemory_HAL(pGpu, pMemoryManager) memmgrBlockMemLockedMemory(pGpu, pMemoryManager)
#define memmgrInsertUnprotectedRegionAtBottomOfFb_HAL(pGpu, pMemoryManager, pSize) memmgrInsertUnprotectedRegionAtBottomOfFb(pGpu, pMemoryManager, pSize)
#define memmgrInitBaseFbRegions_HAL(pGpu, pMemoryManager) memmgrInitBaseFbRegions(pGpu, pMemoryManager)
#define memmgrGetDisablePlcKind_HAL(pMemoryManager, pteKind) memmgrGetDisablePlcKind(pMemoryManager, pteKind)
#define memmgrEnableDynamicPageOfflining_HAL(pGpu, pMemoryManager) memmgrEnableDynamicPageOfflining(pGpu, pMemoryManager)
#define memmgrSetMemDescPageSize_HAL(pGpu, pMemoryManager, arg3, arg4, arg5) memmgrSetMemDescPageSize(pGpu, pMemoryManager, arg3, arg4, arg5)
#define memmgrSetPartitionableMem_HAL(pGpu, pMemoryManager) memmgrSetPartitionableMem(pGpu, pMemoryManager)
#define memmgrAllocMIGGPUInstanceMemory_HAL(pGpu, pMemoryManager, swizzId, phMemory, pAddrRange, ppMemoryPartitionHeap) memmgrAllocMIGGPUInstanceMemory(pGpu, pMemoryManager, swizzId, phMemory, pAddrRange, ppMemoryPartitionHeap)
#define memmgrGetBlackListPagesForHeap_HAL(pGpu, pMemoryManager, pHeap) memmgrGetBlackListPagesForHeap(pGpu, pMemoryManager, pHeap)
#define memmgrGetBlackListPages_HAL(pGpu, pMemoryManager, pBlAddrs, pCount) memmgrGetBlackListPages(pGpu, pMemoryManager, pBlAddrs, pCount)
#define memmgrDiscoverMIGPartitionableMemoryRange_HAL(pGpu, pMemoryManager, pMemoryRange) memmgrDiscoverMIGPartitionableMemoryRange(pGpu, pMemoryManager, pMemoryRange)
#define memmgrGetFBEndReserveSizeEstimate_HAL(pGpu, pMemoryManager) memmgrGetFBEndReserveSizeEstimate(pGpu, pMemoryManager)
#define memmgrValidateFBEndReservation_HAL(pGpu, pMemoryManager) memmgrValidateFBEndReservation(pGpu, pMemoryManager)
#define memmgrReserveMemoryForFakeWPR_HAL(pGpu, pMemoryManager) memmgrReserveMemoryForFakeWPR(pGpu, pMemoryManager)
#define memmgrReserveMemoryForPmu_HAL(pGpu, pMemoryManager) memmgrReserveMemoryForPmu(pGpu, pMemoryManager)
#define memmgrReserveVgaWorkspaceMemDescForFbsr_HAL(pGpu, pMemoryManager) memmgrReserveVgaWorkspaceMemDescForFbsr(pGpu, pMemoryManager)
#define memmgrCalculateHeapOffsetWithGSP_HAL(pGpu, pMemoryManager, offset) memmgrCalculateHeapOffsetWithGSP(pGpu, pMemoryManager, offset)
#define memmgrGetCarveoutRegionInfo_HAL(pGpu, pMemoryManager, pParams) memmgrGetCarveoutRegionInfo(pGpu, pMemoryManager, pParams)
#define memmgrIsMemoryIoCoherent_HAL(pGpu, pMemoryManager, pAllocData) memmgrIsMemoryIoCoherent(pGpu, pMemoryManager, pAllocData)
#define memmgrSc7SrInitGsp_HAL(pGpu, pMemoryManager) memmgrSc7SrInitGsp(pGpu, pMemoryManager)
#define memmgrGetLocalizedOffset_HAL(pGpu, pMemoryManager) memmgrGetLocalizedOffset(pGpu, pMemoryManager)
#define memmgrIsFlaSysmemSupported_HAL(pGpu, pMemoryManager) memmgrIsFlaSysmemSupported(pGpu, pMemoryManager)
#define memmgrGetLocalizedMemorySupported_HAL(pGpu, pMemoryManager) memmgrGetLocalizedMemorySupported(pGpu, pMemoryManager)
#define memmgrConstructEngine_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateConstructEngine__
#define memmgrConstructEngine(pGpu, pEngstate, arg3) memmgrConstructEngine_DISPATCH(pGpu, pEngstate, arg3)
#define memmgrInitMissing_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateInitMissing__
#define memmgrInitMissing(pGpu, pEngstate) memmgrInitMissing_DISPATCH(pGpu, pEngstate)
#define memmgrStatePreInitLocked_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePreInitLocked__
#define memmgrStatePreInitLocked(pGpu, pEngstate) memmgrStatePreInitLocked_DISPATCH(pGpu, pEngstate)
#define memmgrStatePreInitUnlocked_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePreInitUnlocked__
#define memmgrStatePreInitUnlocked(pGpu, pEngstate) memmgrStatePreInitUnlocked_DISPATCH(pGpu, pEngstate)
#define memmgrStateInitLocked_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStateInitLocked__
#define memmgrStateInitLocked(pGpu, pEngstate) memmgrStateInitLocked_DISPATCH(pGpu, pEngstate)
#define memmgrStateInitUnlocked_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStateInitUnlocked__
#define memmgrStateInitUnlocked(pGpu, pEngstate) memmgrStateInitUnlocked_DISPATCH(pGpu, pEngstate)
#define memmgrStatePreLoad_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePreLoad__
#define memmgrStatePreLoad(pGpu, pEngstate, arg3) memmgrStatePreLoad_DISPATCH(pGpu, pEngstate, arg3)
#define memmgrStateLoad_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStateLoad__
#define memmgrStateLoad(pGpu, pEngstate, arg3) memmgrStateLoad_DISPATCH(pGpu, pEngstate, arg3)
#define memmgrStatePostLoad_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePostLoad__
#define memmgrStatePostLoad(pGpu, pEngstate, arg3) memmgrStatePostLoad_DISPATCH(pGpu, pEngstate, arg3)
#define memmgrStatePreUnload_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePreUnload__
#define memmgrStatePreUnload(pGpu, pEngstate, arg3) memmgrStatePreUnload_DISPATCH(pGpu, pEngstate, arg3)
#define memmgrStateUnload_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStateUnload__
#define memmgrStateUnload(pGpu, pEngstate, arg3) memmgrStateUnload_DISPATCH(pGpu, pEngstate, arg3)
#define memmgrStatePostUnload_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePostUnload__
#define memmgrStatePostUnload(pGpu, pEngstate, arg3) memmgrStatePostUnload_DISPATCH(pGpu, pEngstate, arg3)
#define memmgrStateDestroy_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStateDestroy__
#define memmgrStateDestroy(pGpu, pEngstate) memmgrStateDestroy_DISPATCH(pGpu, pEngstate)
#define memmgrIsPresent_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateIsPresent__
#define memmgrIsPresent(pGpu, pEngstate) memmgrIsPresent_DISPATCH(pGpu, pEngstate)

// Dispatch functions
static inline NV_STATUS memmgrConstructEngine_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, ENGDESCRIPTOR arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrConstructEngine__(pGpu, pEngstate, arg3);
}

static inline void memmgrInitMissing_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    pEngstate->__nvoc_metadata_ptr->vtable.__memmgrInitMissing__(pGpu, pEngstate);
}

static inline NV_STATUS memmgrStatePreInitLocked_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStatePreInitLocked__(pGpu, pEngstate);
}

static inline NV_STATUS memmgrStatePreInitUnlocked_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStatePreInitUnlocked__(pGpu, pEngstate);
}

static inline NV_STATUS memmgrStateInitLocked_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStateInitLocked__(pGpu, pEngstate);
}

static inline NV_STATUS memmgrStateInitUnlocked_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStateInitUnlocked__(pGpu, pEngstate);
}

static inline NV_STATUS memmgrStatePreLoad_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStatePreLoad__(pGpu, pEngstate, arg3);
}

static inline NV_STATUS memmgrStateLoad_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStateLoad__(pGpu, pEngstate, arg3);
}

static inline NV_STATUS memmgrStatePostLoad_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStatePostLoad__(pGpu, pEngstate, arg3);
}

static inline NV_STATUS memmgrStatePreUnload_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStatePreUnload__(pGpu, pEngstate, arg3);
}

static inline NV_STATUS memmgrStateUnload_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStateUnload__(pGpu, pEngstate, arg3);
}

static inline NV_STATUS memmgrStatePostUnload_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStatePostUnload__(pGpu, pEngstate, arg3);
}

static inline void memmgrStateDestroy_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    pEngstate->__nvoc_metadata_ptr->vtable.__memmgrStateDestroy__(pGpu, pEngstate);
}

static inline NvBool memmgrIsPresent_DISPATCH(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__memmgrIsPresent__(pGpu, pEngstate);
}

// Virtual method declarations and/or inline definitions
// Exported method declarations and/or inline definitions
// HAL method declarations without bodies
NvU64 memmgrGetVgpuHostRmReservedFb_KERNEL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 vgpuTypeId);

NvBool memmgrIsKind_TU102(struct MemoryManager *pMemoryManager, FB_IS_KIND_OP arg2, NvU32 arg3);

NvBool memmgrIsSurfaceBlockLinear_TU102(struct MemoryManager *pMemoryManager, Memory *arg2, NvU32 arg3);

NV_STATUS memmgrInitBaseFbRegions_FWCLIENT(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);

NV_STATUS memmgrSetPartitionableMem_IMPL(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);

NV_STATUS memmgrAllocMIGGPUInstanceMemory_PF(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 swizzId, NvHandle *phMemory, struct NV_RANGE *pAddrRange, struct Heap **ppMemoryPartitionHeap);

NvU32 memmgrGetFBEndReserveSizeEstimate_GM107(OBJGPU *pGpu, struct MemoryManager *pMemoryManager);

NV_STATUS memmgrGetCarveoutRegionInfo_KERNEL(POBJGPU pGpu, struct MemoryManager *pMemoryManager, NV2080_CTRL_FB_GET_CARVEOUT_REGION_INFO_PARAMS *pParams);

// Inline HAL method definitions
static inline NvU64 memmgrDeterminePageSize_3eaa85(struct MemoryManager *pMemoryManager, NvHandle hClient, NvU64 memSize, NvU32 memFormat, NvU32 pageFormatFlags, NvU32 *pRetAttr, NvU32 *pRetAttr2){
    return 4096;
}

static inline NV_STATUS memmgrFreeHwResources_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *arg3){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrScrubInit_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrScrubHandlePostSchedulingEnable_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline void memmgrGetScrubState_f2d351(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *arg3, NvU64 *arg4, NvBool *arg5){
    NV_ASSERT_PRECOMP(0);
}

static inline void memmgrScrubInternalRegions_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return;
}

static inline NvBool memmgrEccScrubInProgress_e661f0(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_TRUE;
}

static inline NvBool memmgrEccScrubInProgress_d69453(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_FALSE;
}

static inline void memmgrAsyncScrubRegion_f2d351(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 arg3, NvU64 arg4){
    NV_ASSERT_PRECOMP(0);
}

static inline NV_STATUS memmgrScrubHandlePreSchedulingDisable_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline void memmgrScrubDestroy_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return;
}

static inline void memmgrScrubMemory_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, RmPhysAddr arg3, NvU64 arg4){
    return;
}

static inline NV_STATUS memmgrMemUtilsMemSetBlocking_92bfc3(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3, RmPhysAddr arg4, NvU64 arg5){
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrMemUtilsMemSet_92bfc3(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3, RmPhysAddr arg4, NvU64 arg5, NvU32 arg6, NvU32 *arg7){
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrMemUtilsAllocateEccScrubber_92bfc3(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3){
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrMemUtilsAllocateEccAllocScrubber_92bfc3(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3){
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrMemUtilsChannelInitialize_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3){
    return NV_OK;
}

static inline NV_STATUS memmgrMemUtilsCopyEngineInitialize_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3){
    return NV_OK;
}

static inline NV_STATUS memmgrMemUtilsSec2CtxInit_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrMemUtilsGetCopyEngineClass_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 *pClass){
    return NV_OK;
}

static inline NV_STATUS memmgrMemUtilsCreateMemoryAlias_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3){
    return NV_OK;
}

static inline NvBool memmgrMemUtilsCheckMemoryFastScrubEnable_d69453(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 arg3, NvBool arg4, RmPhysAddr arg5, NvU32 arg6, NV_ADDRESS_SPACE arg7){
    return NV_FALSE;
}

static inline NV_STATUS memmgrAllocHal_92bfc3(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *pFbAllocInfo){
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrFreeHal_92bfc3(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *pFbAllocInfo, PRMTIMEOUT pTimeout){
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrUpdateSurfaceCompression_5baef9(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, Memory *arg3, NvBool arg4){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NV_STATUS memmgrGetBankPlacementData_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 *pBankPlacementLowData){
    return NV_ERR_NOT_SUPPORTED;
}

static inline void memmgrDirtyForPmTest_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvBool partialDirty){
    return;
}

static inline NvU64 memmgrGetReservedHeapSizeMb_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NV_STATUS memmgrAllocDetermineAlignment_5baef9(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *pMemSize, NvU64 *pAlign, NvU64 alignPad, NvU32 allocFlags, NvU32 retAttr, NvU32 retAttr2, NvU64 hwAlignment){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NV_STATUS memmgrInitFbRegionsHal_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NvU64 memmgrGetMaxContextSize_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline void memmgrHandleSizeOverrides_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return;
}

static inline NV_STATUS memmgrFinishHandleSizeOverrides_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrGetBAR1InfoForDevice_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct Device *pDevice, PGETBAR1INFO bar1Info){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvU64 memmgrGetFbTaxSize_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NvU64 memmgrGetVgpuHostRmReservedFb_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 vgpuTypeId){
    return 0;
}

static inline void memmgrScrubRegistryOverrides_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return;
}

static inline NvU64 memmgrGetRsvdSizeForSr_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NvBool memmgrVerifyDepthSurfaceAttrs_e661f0(struct MemoryManager *pMemoryManager, NvU32 arg2, NvU32 arg3){
    return NV_TRUE;
}

static inline NV_STATUS memmgrAllocMemToSaveVgaWorkspace_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR **arg3, MEMORY_DESCRIPTOR **arg4){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrAllocMemToSaveVgaWorkspace_5baef9(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, MEMORY_DESCRIPTOR **arg3, MEMORY_DESCRIPTOR **arg4){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NvBool memmgrComparePhysicalAddresses_86b752(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 arg3, NvU64 arg4, NvU32 arg5, NvU64 arg6){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_FALSE);
}

static inline RmPhysAddr memmgrGetInvalidOffset_0b83bf(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 4294967295U;
}

static inline NvU64 memmgrGetAddrSpaceSizeMB_474d46(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    NV_ASSERT_OR_RETURN_PRECOMP(0, 0);
}

static inline NvU64 memmgrGetUsableMemSizeMB_13cd8d(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    NV_ASSERT_PRECOMP(0);
    return 0;
}

static inline NvBool memmgrVerifyComprAttrs_e661f0(struct MemoryManager *pMemoryManager, NvU32 arg2, NvU32 arg3, NvU32 arg4){
    return NV_TRUE;
}

static inline NvBool memmgrIsKindCompressible_d69453(struct MemoryManager *pMemoryManager, NvU32 arg2){
    return NV_FALSE;
}

static inline NvBool memmgrIsKindBlocklinear_d69453(struct MemoryManager *pMemoryManager, NvU32 arg2){
    return NV_FALSE;
}

static inline NvU32 memmgrGetPteKindBl_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NvU32 memmgrGetPteKindPitch_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NvU32 memmgrChooseKindZ_474d46(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_PAGE_FORMAT *arg3){
    NV_ASSERT_OR_RETURN_PRECOMP(0, 0);
}

static inline NvU32 memmgrChooseKindCompressZ_474d46(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_PAGE_FORMAT *arg3){
    NV_ASSERT_OR_RETURN_PRECOMP(0, 0);
}

static inline NvU32 memmgrChooseKindCompressC_474d46(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_PAGE_FORMAT *arg3){
    NV_ASSERT_OR_RETURN_PRECOMP(0, 0);
}

static inline NvU32 memmgrChooseKindCompressCForMS2_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 arg3){
    return 0;
}

static inline NvU32 memmgrGetPteKindGenericMemoryCompressible_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NvU32 memmgrGetUncompressedKind_474d46(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 kind, NvBool releaseReacquire){
    NV_ASSERT_OR_RETURN_PRECOMP(0, 0);
}

static inline NV_STATUS memmgrGetUncompressedKindForMS2_5baef9(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 arg3, NvU32 *arg4){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline NvU32 memmgrGetCompressedKind_d1515c(struct MemoryManager *pMemoryManager, NvU32 kind, NvBool bDisablePlc){
    NV_ASSERT_OR_RETURN_PRECOMP(0, kind);
}

static inline NV_STATUS memmgrChooseKind_f4fe90(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_PAGE_FORMAT *arg3, NvU32 arg4, NvU32 *pKind){
    NV_ASSERT_OR_RETURN_PRECOMP(pKind != ((void *)0), NV_ERR_INVALID_ARGUMENT);
    *pKind = 0;
    return NV_OK;
}

static inline NvU32 memmgrGetMessageKind_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NvU32 memmgrGetDefaultPteKindForNoHandle_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NV_STATUS memmgrGetFlaKind_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 *arg3){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvBool memmgrIsMemDescSupportedByFla_395e98(OBJGPU *arg1, struct MemoryManager *arg_this, MEMORY_DESCRIPTOR *arg3){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvBool memmgrIsValidFlaPageSize_d69453(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 pageSize, NvBool bIsMulticast){
    return NV_FALSE;
}

static inline NvU32 memmgrGetHwPteKindFromSwPteKind_26237f(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 pteKind){
    return pteKind;
}

static inline NvU32 memmgrGetSwPteKindFromHwPteKind_26237f(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 pteKind){
    return pteKind;
}

static inline void memmgrGetPteKindForScrubber_f2d351(struct MemoryManager *pMemoryManager, NvU32 *arg2){
    NV_ASSERT_PRECOMP(0);
}

static inline NvU32 memmgrGetCtagOffsetFromParams_fc0f62(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *arg3){
    return -1;
}

static inline void memmgrSetCtagOffsetInParams_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *arg3, NvU32 arg4){
    return;
}

static inline NvU32 memmgrDetermineComptag_13cd8d(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, RmPhysAddr arg3){
    NV_ASSERT_PRECOMP(0);
    return 0;
}

static inline void memmgrChannelPushSemaphoreMethodsBlock_f2d351(struct MemoryManager *pMemoryManager, NvU32 arg2, NvU64 arg3, NvU32 arg4, NvU32 **arg5){
    NV_ASSERT_PRECOMP(0);
}

static inline void memmgrChannelPushAddressMethodsBlock_f2d351(struct MemoryManager *pMemoryManager, NvBool arg2, NvU32 arg3, RmPhysAddr arg4, NvU32 **arg5){
    NV_ASSERT_PRECOMP(0);
}

static inline NV_STATUS memmgrScrubMapDoorbellRegion_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, OBJCHANNEL *arg3){
    return NV_OK;
}

static inline NV_STATUS memmgrSetAllocParameters_dffb6f(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, FB_ALLOC_INFO *pFbAllocInfo){
    NV_ASSERT_PRECOMP(0);
    return NV_OK;
}

static inline void memmgrCalcReservedFbSpaceForUVM_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *arg3){
    return;
}

static inline void memmgrCalcReservedFbSpaceHal_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *arg3, NvU64 *arg4, NvU64 *arg5){
    return;
}

static inline NvU32 memmgrGetGrHeapReservationSize_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NvU32 memmgrGetRunlistEntriesReservedFbSpace_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NvU32 memmgrGetUserdReservedFbSpace_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NV_STATUS memmgrCheckReservedMemorySize_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrInitReservedMemory_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 arg3){
    return NV_OK;
}

static inline NV_STATUS memmgrPreInitReservedMemory_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrReadMmuLock_ccda6f(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvBool *pbIsValid, NvU64 *pMmuLockLo, NvU64 *pMmuLockHi){
    *pbIsValid = NV_FALSE;
    return NV_OK;
}

static inline NV_STATUS memmgrBlockMemLockedMemory_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrInsertUnprotectedRegionAtBottomOfFb_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU64 *pSize){
    return NV_OK;
}

static inline NV_STATUS memmgrInitBaseFbRegions_5baef9(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    NV_ASSERT_OR_RETURN_PRECOMP(0, NV_ERR_NOT_SUPPORTED);
}

static inline void memmgrGetDisablePlcKind_d44104(struct MemoryManager *pMemoryManager, NvU32 *pteKind){
    return;
}

static inline void memmgrEnableDynamicPageOfflining_d44104(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return;
}

static inline NV_STATUS memmgrSetMemDescPageSize_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, PMEMORY_DESCRIPTOR arg3, ADDRESS_TRANSLATION arg4, RM_ATTR_PAGE_SIZE arg5){
    return NV_OK;
}

static inline NV_STATUS memmgrGetBlackListPagesForHeap_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct Heap *pHeap){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrGetBlackListPages_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, BLACKLIST_ADDRESS *pBlAddrs, NvU32 *pCount){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrDiscoverMIGPartitionableMemoryRange_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct NV_RANGE *pMemoryRange){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrValidateFBEndReservation_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrReserveMemoryForFakeWPR_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrReserveMemoryForPmu_ac1694(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrReserveVgaWorkspaceMemDescForFbsr_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrCalculateHeapOffsetWithGSP_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NvU32 *offset){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NV_STATUS memmgrGetCarveoutRegionInfo_ac1694(POBJGPU pGpu, struct MemoryManager *pMemoryManager, NV2080_CTRL_FB_GET_CARVEOUT_REGION_INFO_PARAMS *pParams){
    return NV_OK;
}

static inline NvBool memmgrIsMemoryIoCoherent_e661f0(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, NV_MEMORY_ALLOCATION_PARAMS *pAllocData){
    return NV_TRUE;
}

static inline NV_STATUS memmgrSc7SrInitGsp_395e98(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_ERR_NOT_SUPPORTED;
}

static inline NvU8 memmgrGetLocalizedOffset_b2b553(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return 0;
}

static inline NvBool memmgrIsFlaSysmemSupported_d69453(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_FALSE;
}

static inline NvBool memmgrGetLocalizedMemorySupported_d69453(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_FALSE;
}

// Static dispatch method declarations
// Static inline method definitions
static inline NV_STATUS memmgrSavePowerMgmtState(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrRestorePowerMgmtState(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return NV_OK;
}

static inline NV_STATUS memmgrFree(OBJGPU *pGpu, struct MemoryManager *pMemoryManager, struct Heap *arg3, NvHandle arg4, NvHandle arg5, NvHandle arg6, NvU32 arg7, MEMORY_DESCRIPTOR *arg8){
    return NV_ERR_NOT_SUPPORTED;
}

static inline struct Heap * memmgrGetDeviceSuballocator(struct MemoryManager *pMemoryManager, NvBool bForceSubheap){
    return ((void *)0);
}

static inline NV_ADDRESS_SPACE memmgrAllocGetAddrSpace(struct MemoryManager *pMemoryManager, NvU32 flags, NvU32 attr){
    return 2;
}

static inline void memmgrFreeFbsrMemory(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return;
}

static inline NvBool memmgrIsLocalEgmSupported(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bLocalEgmSupported;
}

static inline NvBool memmgrIsLocalEgmEnabled(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bLocalEgmEnabled;
}

static inline NvU32 memmgrLocalEgmPeerId(struct MemoryManager *pMemoryManager){
    return pMemoryManager->localEgmPeerId;
}

static inline NvU64 memmgrLocalEgmBaseAddress(struct MemoryManager *pMemoryManager){
    return pMemoryManager->localEgmBasePhysAddr;
}

static inline NvBool memmgrIsScrubOnFreeEnabled(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bScrubOnFreeEnabled;
}

static inline NvBool memmgrIsFastScrubberEnabled(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bFastScrubberEnabled;
}

static inline NvBool memmgrUseVasForCeMemoryOps(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bUseVasForCeMemoryOps;
}

static inline NvBool memmgrIsPmaInitialized(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bPmaInitialized;
}

static inline void memmgrSetPmaInitialized(struct MemoryManager *pMemoryManager, NvBool val){
    pMemoryManager->bPmaInitialized = val;
}

static inline NvBool memmgrAreFbRegionsSupported(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bFbRegionsSupported;
}

static inline NvBool memmgrIsPmaSupportedOnPlatform(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bPmaSupportedOnPlatform;
}

static inline NvBool memmgrIsPmaEnabled(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bPmaEnabled;
}

static inline NvBool memmgrIsPmaForcePersistence(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bPmaForcePersistence;
}

static inline void memmgrSetPmaForcePersistence(struct MemoryManager *pMemoryManager, NvBool val){
    pMemoryManager->bPmaForcePersistence = val;
}

static inline NvBool memmgrAreClientPageTablesPmaManaged(struct MemoryManager *pMemoryManager){
    return pMemoryManager->bClientPageTablesPmaManaged;
}

static inline void memmgrSetClientPageTablesPmaManaged(struct MemoryManager *pMemoryManager, NvBool val){
    pMemoryManager->bClientPageTablesPmaManaged = val;
}

static inline NvU64 memmgrGetRsvdMemoryBase(struct MemoryManager *pMemoryManager){
    return pMemoryManager->rsvdMemoryBase;
}

static inline NvU64 memmgrGetRsvdMemorySize(struct MemoryManager *pMemoryManager){
    return pMemoryManager->rsvdMemorySize;
}

static inline NvBool memmgrBug3922001DisableCtxBufOnSim(OBJGPU *pGpu, struct MemoryManager *pMemoryManager){
    return pMemoryManager->bBug3922001DisableCtxBufOnSim;
}

#undef PRIVATE_FIELD


#endif // MEM_MGR_H

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _G_MEM_MGR_NVOC_H_
