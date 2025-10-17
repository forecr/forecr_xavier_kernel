
#ifndef _G_KERN_DISP_NVOC_H_
#define _G_KERN_DISP_NVOC_H_

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
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "g_kern_disp_nvoc.h"

#ifndef KERN_DISP_H
#define KERN_DISP_H

/******************************************************************************
*
*       Kernel Display module header
*       This file contains functions managing display on CPU RM
*
******************************************************************************/

#include "gpu/eng_state.h"
#include "gpu/gpu_halspec.h"
#include "gpu/disp/kern_disp_type.h"
#include "gpu/disp/kern_disp_max.h"
#include "gpu/mem_mgr/context_dma.h"
#include "gpu/disp/vblank_callback/vblank.h"
#include "gpu/disp/head/kernel_head.h"


#include "utils/nvbitvector.h"
TYPEDEF_BITVECTOR(MC_ENGINE_BITVECTOR);

#include "ctrl/ctrl2080/ctrl2080internal.h"

typedef NV2080_CTRL_INTERNAL_DISPLAY_GET_STATIC_INFO_PARAMS KernelDisplayStaticInfo;

typedef struct
{
    NvU32 kHeadVblankCount[OBJ_MAX_HEADS];
} KernelDisplaySharedMem;

// From DISP_v0501 NvDisplay HW IP has 4 interrupt vectors: high latency, low latency, GSP, PMU.
typedef enum
{
    DISP_INTERRUPT_VECTOR_HIGH_LATENCY = 0,
    DISP_INTERRUPT_VECTOR_LOW_LATENCY,
    DISP_INTERRUPT_VECTOR_PMU,
    DISP_INTERRUPT_VECTOR_GSP,
} DISP_INTERRUPT_VECTOR;

typedef struct
{
// Link configuration
    NvU32   linkBw;                     // Link Rate (270M unit)
    NvU32   dp2LinkBw;                  // Link Rate using DP2.x convention (10M unit)
    NvU32   laneCount;                  // Lane Count
    NvBool  bDP2xChannelCoding;         // If is using 128b/132b channel coding.
    NvBool  bEnhancedFraming;           // Enhanced Framing is enabled or not
    NvBool  bMultiStream;               // Multistream is enabled or not
    NvBool  bFecEnable;                 // FEC is enabled or not
    NvBool  bDisableEffBppSST8b10b;     // Disable effective bpp 8b10b coding or not

// DSC Information
    NvBool  bDscEnable;                 // DSC is enabled or not
    NvU32   sliceCount;
    NvU32   sliceWidth;
    NvU32   sliceHeight;
    NvU32   dscVersionMajor;
    NvU32   dscVersionMinor;

// Mode information.
    NvU64   PClkFreqHz;                 // Pixel clock in Hz
    NvU32   bpp;                        // Output bits per pixel
    NvU32   SetRasterSizeWidth;         // Raster Size width
    NvU32   SetRasterBlankStartX;       // Raster Blank Start X
    NvU32   SetRasterBlankEndX;         // Raster Blank End X
    NvU32   twoChannelAudioHz;
    NvU32   eightChannelAudioHz;
    NvU32   colorFormat;                // Same as DP_COLORFORMAT
} DPMODESETDATA;

// bpp in PPS is multiplied by 16 when DSC is enabled
#define DSC_BPP_FACTOR 16

typedef struct
{
    struct RsClient *pClient;
    NvHandle hChannel;
    NvU32    channelNum;
    NvBool   bInUse;
} KernelDisplayClientChannelMap;

typedef void (*OSVBLANKCALLBACKPROC)(NvP64 pParm1, NvP64 pParm2);

typedef struct _osvblankcallback {
    OSVBLANKCALLBACKPROC pProc;
    void * pParm1;
    void * pParm2;
    void * pCallback;
    void * pParm3;
} OSVBLANKCALLBACK, * POSVBLANKCALLBACK;

#define DISP_INTR_REG(reg)          NV_PDISP_FE_RM_INTR_##reg
#define DISP_INTR_REG_IDX(reg,i)    NV_PDISP_FE_RM_INTR_##reg(i)


struct DispChannel;

#ifndef __NVOC_CLASS_DispChannel_TYPEDEF__
#define __NVOC_CLASS_DispChannel_TYPEDEF__
typedef struct DispChannel DispChannel;
#endif /* __NVOC_CLASS_DispChannel_TYPEDEF__ */

#ifndef __nvoc_class_id_DispChannel
#define __nvoc_class_id_DispChannel 0xbd2ff3
#endif /* __nvoc_class_id_DispChannel */



struct RgLineCallback;

#ifndef __NVOC_CLASS_RgLineCallback_TYPEDEF__
#define __NVOC_CLASS_RgLineCallback_TYPEDEF__
typedef struct RgLineCallback RgLineCallback;
#endif /* __NVOC_CLASS_RgLineCallback_TYPEDEF__ */

#ifndef __nvoc_class_id_RgLineCallback
#define __nvoc_class_id_RgLineCallback 0xa3ff1c
#endif /* __nvoc_class_id_RgLineCallback */



#define KDISP_GET_HEAD(pKernelDisplay, headID)    (RMCFG_MODULE_KERNEL_HEAD ? kdispGetHead(pKernelDisplay, headID) : NULL)

/*!
 * KernelDisp is a logical abstraction of the GPU Display Engine. The
 * Public API of the Display Engine is exposed through this object, and any
 * interfaces which do not manage the underlying Display hardware can be
 * managed by this object.
 */

// Private field names are wrapped in PRIVATE_FIELD, which does nothing for
// the matching C source file, but causes diagnostics to be issued if another
// source file references the field.
#ifdef NVOC_KERN_DISP_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif


// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__KernelDisplay;
struct NVOC_METADATA__OBJENGSTATE;
struct NVOC_VTABLE__KernelDisplay;


struct KernelDisplay {

    // Metadata starts with RTTI structure.
    union {
         const struct NVOC_METADATA__KernelDisplay *__nvoc_metadata_ptr;
         const struct NVOC_RTTI *__nvoc_rtti;
    };

    // Parent (i.e. superclass or base class) objects
    struct OBJENGSTATE __nvoc_base_OBJENGSTATE;

    // Ancestor object pointers for `staticCast` feature
    struct Object *__nvoc_pbase_Object;    // obj super^2
    struct OBJENGSTATE *__nvoc_pbase_OBJENGSTATE;    // engstate super
    struct KernelDisplay *__nvoc_pbase_KernelDisplay;    // kdisp

    // Vtable with 2 per-object function pointers
    NvU32 (*__kdispGetPBTargetAperture__)(struct OBJGPU *, struct KernelDisplay * /*this*/, NvU32, NvU32);  // halified (2 hals) body
    NV_STATUS (*__kdispComputeDpModeSettings__)(struct OBJGPU *, struct KernelDisplay * /*this*/, NvU32, DPMODESETDATA *, DPIMPINFO *);  // halified (2 hals) body

    // 8 PDB properties
    NvBool PDB_PROP_KDISP_IMP_ALLOC_BW_IN_KERNEL_RM_DEF;
    NvBool PDB_PROP_KDISP_FEATURE_STRETCH_VBLANK_CAPABLE;
    NvBool PDB_PROP_KDISP_IN_AWAKEN_INTR;
    NvBool PDB_PROP_KDISP_HAS_SEPARATE_LOW_LATENCY_LINE;
    NvBool PDB_PROP_KDISP_INTERNAL_PANEL_DISCONNECTED;
    NvBool PDB_PROP_KDISP_ENABLE_INLINE_INTR_SERVICE;
    NvBool PDB_PROP_KDISP_AGGRESSIVE_VBLANK_HANDLING;

    // Data members
    struct DisplayInstanceMemory *pInst;
    struct KernelHead *pKernelHead[8];
    const KernelDisplayStaticInfo *pStaticInfo;
    NvBool bWarPurgeSatellitesOnCoreFree;
    struct RgLineCallback *rgLineCallbackPerHead[8][2];
    NvU32 isrVblankHeads;
    NvBool bExtdevIntrSupported;
    NvU32 numHeads;
    NvU32 deferredVblankHeadMask;
    NvHandle hInternalClient;
    NvHandle hInternalDevice;
    NvHandle hInternalSubdevice;
    NvHandle hDispCommonHandle;
    MEMORY_DESCRIPTOR *pSharedMemDesc;
    KernelDisplaySharedMem *pSharedData;
    NvBool bFeatureStretchVblankCapable;
    volatile NvS32 lowLatencyLock;
    NvU32 vblankCallbackHeadMask;
    POSVBLANKCALLBACK pOsVblankCallback;
    NvU32 numDispChannels;
    KernelDisplayClientChannelMap *pClientChannelTable;
    NvBool bIsPanelReplayEnabled;
    void *pRgVblankCb;
};


// Vtable with 14 per-class function pointers
struct NVOC_VTABLE__KernelDisplay {
    NV_STATUS (*__kdispConstructEngine__)(struct OBJGPU *, struct KernelDisplay * /*this*/, ENGDESCRIPTOR);  // virtual override (engstate) base (engstate)
    NV_STATUS (*__kdispStatePreInitLocked__)(struct OBJGPU *, struct KernelDisplay * /*this*/);  // virtual override (engstate) base (engstate)
    NV_STATUS (*__kdispStateInitLocked__)(struct OBJGPU *, struct KernelDisplay * /*this*/);  // virtual override (engstate) base (engstate)
    void (*__kdispStateDestroy__)(struct OBJGPU *, struct KernelDisplay * /*this*/);  // virtual override (engstate) base (engstate)
    NV_STATUS (*__kdispStateLoad__)(struct OBJGPU *, struct KernelDisplay * /*this*/, NvU32);  // virtual override (engstate) base (engstate)
    NV_STATUS (*__kdispStateUnload__)(struct OBJGPU *, struct KernelDisplay * /*this*/, NvU32);  // virtual override (engstate) base (engstate)
    void (*__kdispInitMissing__)(struct OBJGPU *, struct KernelDisplay * /*this*/);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__kdispStatePreInitUnlocked__)(struct OBJGPU *, struct KernelDisplay * /*this*/);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__kdispStateInitUnlocked__)(struct OBJGPU *, struct KernelDisplay * /*this*/);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__kdispStatePreLoad__)(struct OBJGPU *, struct KernelDisplay * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__kdispStatePostLoad__)(struct OBJGPU *, struct KernelDisplay * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__kdispStatePreUnload__)(struct OBJGPU *, struct KernelDisplay * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    NV_STATUS (*__kdispStatePostUnload__)(struct OBJGPU *, struct KernelDisplay * /*this*/, NvU32);  // virtual inherited (engstate) base (engstate)
    NvBool (*__kdispIsPresent__)(struct OBJGPU *, struct KernelDisplay * /*this*/);  // virtual inherited (engstate) base (engstate)
};

// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__KernelDisplay {
    const struct NVOC_RTTI rtti;
    const struct NVOC_METADATA__OBJENGSTATE metadata__OBJENGSTATE;
    const struct NVOC_VTABLE__KernelDisplay vtable;
};

#ifndef __NVOC_CLASS_KernelDisplay_TYPEDEF__
#define __NVOC_CLASS_KernelDisplay_TYPEDEF__
typedef struct KernelDisplay KernelDisplay;
#endif /* __NVOC_CLASS_KernelDisplay_TYPEDEF__ */

#ifndef __nvoc_class_id_KernelDisplay
#define __nvoc_class_id_KernelDisplay 0x55952e
#endif /* __nvoc_class_id_KernelDisplay */

// Casting support
extern const struct NVOC_CLASS_DEF __nvoc_class_def_KernelDisplay;

#define __staticCast_KernelDisplay(pThis) \
    ((pThis)->__nvoc_pbase_KernelDisplay)

#ifdef __nvoc_kern_disp_h_disabled
#define __dynamicCast_KernelDisplay(pThis) ((KernelDisplay*) NULL)
#else //__nvoc_kern_disp_h_disabled
#define __dynamicCast_KernelDisplay(pThis) \
    ((KernelDisplay*) __nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(KernelDisplay)))
#endif //__nvoc_kern_disp_h_disabled

// Property macros
#define PDB_PROP_KDISP_ENABLE_INLINE_INTR_SERVICE_BASE_CAST
#define PDB_PROP_KDISP_ENABLE_INLINE_INTR_SERVICE_BASE_NAME PDB_PROP_KDISP_ENABLE_INLINE_INTR_SERVICE
#define PDB_PROP_KDISP_AGGRESSIVE_VBLANK_HANDLING_BASE_CAST
#define PDB_PROP_KDISP_AGGRESSIVE_VBLANK_HANDLING_BASE_NAME PDB_PROP_KDISP_AGGRESSIVE_VBLANK_HANDLING
#define PDB_PROP_KDISP_IS_MISSING_BASE_CAST __nvoc_base_OBJENGSTATE.
#define PDB_PROP_KDISP_IS_MISSING_BASE_NAME PDB_PROP_ENGSTATE_IS_MISSING
#define PDB_PROP_KDISP_HAS_SEPARATE_LOW_LATENCY_LINE_BASE_CAST
#define PDB_PROP_KDISP_HAS_SEPARATE_LOW_LATENCY_LINE_BASE_NAME PDB_PROP_KDISP_HAS_SEPARATE_LOW_LATENCY_LINE
#define PDB_PROP_KDISP_IN_AWAKEN_INTR_BASE_CAST
#define PDB_PROP_KDISP_IN_AWAKEN_INTR_BASE_NAME PDB_PROP_KDISP_IN_AWAKEN_INTR
#define PDB_PROP_KDISP_IMP_ALLOC_BW_IN_KERNEL_RM_DEF_BASE_CAST
#define PDB_PROP_KDISP_IMP_ALLOC_BW_IN_KERNEL_RM_DEF_BASE_NAME PDB_PROP_KDISP_IMP_ALLOC_BW_IN_KERNEL_RM_DEF
#define PDB_PROP_KDISP_FEATURE_STRETCH_VBLANK_CAPABLE_BASE_CAST
#define PDB_PROP_KDISP_FEATURE_STRETCH_VBLANK_CAPABLE_BASE_NAME PDB_PROP_KDISP_FEATURE_STRETCH_VBLANK_CAPABLE
#define PDB_PROP_KDISP_INTERNAL_PANEL_DISCONNECTED_BASE_CAST
#define PDB_PROP_KDISP_INTERNAL_PANEL_DISCONNECTED_BASE_NAME PDB_PROP_KDISP_INTERNAL_PANEL_DISCONNECTED

NV_STATUS __nvoc_objCreateDynamic_KernelDisplay(KernelDisplay**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_KernelDisplay(KernelDisplay**, Dynamic*, NvU32);
#define __objCreate_KernelDisplay(ppNewObj, pParent, createFlags) \
    __nvoc_objCreate_KernelDisplay((ppNewObj), staticCast((pParent), Dynamic), (createFlags))


// Wrapper macros
#define kdispConstructEngine_FNPTR(pKernelDisplay) pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispConstructEngine__
#define kdispConstructEngine(pGpu, pKernelDisplay, engDesc) kdispConstructEngine_DISPATCH(pGpu, pKernelDisplay, engDesc)
#define kdispStatePreInitLocked_FNPTR(pKernelDisplay) pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStatePreInitLocked__
#define kdispStatePreInitLocked(pGpu, pKernelDisplay) kdispStatePreInitLocked_DISPATCH(pGpu, pKernelDisplay)
#define kdispStateInitLocked_FNPTR(pKernelDisplay) pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStateInitLocked__
#define kdispStateInitLocked(pGpu, pKernelDisplay) kdispStateInitLocked_DISPATCH(pGpu, pKernelDisplay)
#define kdispStateDestroy_FNPTR(pKernelDisplay) pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStateDestroy__
#define kdispStateDestroy(pGpu, pKernelDisplay) kdispStateDestroy_DISPATCH(pGpu, pKernelDisplay)
#define kdispStateLoad_FNPTR(pKernelDisplay) pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStateLoad__
#define kdispStateLoad(pGpu, pKernelDisplay, flags) kdispStateLoad_DISPATCH(pGpu, pKernelDisplay, flags)
#define kdispStateUnload_FNPTR(pKernelDisplay) pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStateUnload__
#define kdispStateUnload(pGpu, pKernelDisplay, flags) kdispStateUnload_DISPATCH(pGpu, pKernelDisplay, flags)
#define kdispGetPBTargetAperture_FNPTR(pKernelDisplay) pKernelDisplay->__kdispGetPBTargetAperture__
#define kdispGetPBTargetAperture(pGpu, pKernelDisplay, memAddrSpace, cacheSnoop) kdispGetPBTargetAperture_DISPATCH(pGpu, pKernelDisplay, memAddrSpace, cacheSnoop)
#define kdispGetPBTargetAperture_HAL(pGpu, pKernelDisplay, memAddrSpace, cacheSnoop) kdispGetPBTargetAperture_DISPATCH(pGpu, pKernelDisplay, memAddrSpace, cacheSnoop)
#define kdispComputeDpModeSettings_FNPTR(pKernelDisplay) pKernelDisplay->__kdispComputeDpModeSettings__
#define kdispComputeDpModeSettings(pGpu, pKernelDisplay, headIndex, pDpModesetData, dpInfo) kdispComputeDpModeSettings_DISPATCH(pGpu, pKernelDisplay, headIndex, pDpModesetData, dpInfo)
#define kdispComputeDpModeSettings_HAL(pGpu, pKernelDisplay, headIndex, pDpModesetData, dpInfo) kdispComputeDpModeSettings_DISPATCH(pGpu, pKernelDisplay, headIndex, pDpModesetData, dpInfo)
#define kdispInitMissing_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateInitMissing__
#define kdispInitMissing(pGpu, pEngstate) kdispInitMissing_DISPATCH(pGpu, pEngstate)
#define kdispStatePreInitUnlocked_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePreInitUnlocked__
#define kdispStatePreInitUnlocked(pGpu, pEngstate) kdispStatePreInitUnlocked_DISPATCH(pGpu, pEngstate)
#define kdispStateInitUnlocked_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStateInitUnlocked__
#define kdispStateInitUnlocked(pGpu, pEngstate) kdispStateInitUnlocked_DISPATCH(pGpu, pEngstate)
#define kdispStatePreLoad_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePreLoad__
#define kdispStatePreLoad(pGpu, pEngstate, arg3) kdispStatePreLoad_DISPATCH(pGpu, pEngstate, arg3)
#define kdispStatePostLoad_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePostLoad__
#define kdispStatePostLoad(pGpu, pEngstate, arg3) kdispStatePostLoad_DISPATCH(pGpu, pEngstate, arg3)
#define kdispStatePreUnload_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePreUnload__
#define kdispStatePreUnload(pGpu, pEngstate, arg3) kdispStatePreUnload_DISPATCH(pGpu, pEngstate, arg3)
#define kdispStatePostUnload_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateStatePostUnload__
#define kdispStatePostUnload(pGpu, pEngstate, arg3) kdispStatePostUnload_DISPATCH(pGpu, pEngstate, arg3)
#define kdispIsPresent_FNPTR(pEngstate) pEngstate->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr->vtable.__engstateIsPresent__
#define kdispIsPresent(pGpu, pEngstate) kdispIsPresent_DISPATCH(pGpu, pEngstate)

// Dispatch functions
static inline NV_STATUS kdispConstructEngine_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, ENGDESCRIPTOR engDesc) {
    return pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispConstructEngine__(pGpu, pKernelDisplay, engDesc);
}

static inline NV_STATUS kdispStatePreInitLocked_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    return pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStatePreInitLocked__(pGpu, pKernelDisplay);
}

static inline NV_STATUS kdispStateInitLocked_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    return pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStateInitLocked__(pGpu, pKernelDisplay);
}

static inline void kdispStateDestroy_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStateDestroy__(pGpu, pKernelDisplay);
}

static inline NV_STATUS kdispStateLoad_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 flags) {
    return pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStateLoad__(pGpu, pKernelDisplay, flags);
}

static inline NV_STATUS kdispStateUnload_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 flags) {
    return pKernelDisplay->__nvoc_metadata_ptr->vtable.__kdispStateUnload__(pGpu, pKernelDisplay, flags);
}

static inline NvU32 kdispGetPBTargetAperture_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 memAddrSpace, NvU32 cacheSnoop) {
    return pKernelDisplay->__kdispGetPBTargetAperture__(pGpu, pKernelDisplay, memAddrSpace, cacheSnoop);
}

static inline NV_STATUS kdispComputeDpModeSettings_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 headIndex, DPMODESETDATA *pDpModesetData, DPIMPINFO *dpInfo) {
    return pKernelDisplay->__kdispComputeDpModeSettings__(pGpu, pKernelDisplay, headIndex, pDpModesetData, dpInfo);
}

static inline void kdispInitMissing_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate) {
    pEngstate->__nvoc_metadata_ptr->vtable.__kdispInitMissing__(pGpu, pEngstate);
}

static inline NV_STATUS kdispStatePreInitUnlocked_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__kdispStatePreInitUnlocked__(pGpu, pEngstate);
}

static inline NV_STATUS kdispStateInitUnlocked_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__kdispStateInitUnlocked__(pGpu, pEngstate);
}

static inline NV_STATUS kdispStatePreLoad_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__kdispStatePreLoad__(pGpu, pEngstate, arg3);
}

static inline NV_STATUS kdispStatePostLoad_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__kdispStatePostLoad__(pGpu, pEngstate, arg3);
}

static inline NV_STATUS kdispStatePreUnload_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__kdispStatePreUnload__(pGpu, pEngstate, arg3);
}

static inline NV_STATUS kdispStatePostUnload_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__kdispStatePostUnload__(pGpu, pEngstate, arg3);
}

static inline NvBool kdispIsPresent_DISPATCH(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate) {
    return pEngstate->__nvoc_metadata_ptr->vtable.__kdispIsPresent__(pGpu, pEngstate);
}

void kdispServiceLowLatencyIntrs_KERNEL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 arg3, NvU32 arg4, THREAD_STATE_NODE *arg5, NvU32 *pIntrServicedHeadMask, MC_ENGINE_BITVECTOR *pIntrPending);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispServiceLowLatencyIntrs(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 arg3, NvU32 arg4, THREAD_STATE_NODE *arg5, NvU32 *pIntrServicedHeadMask, MC_ENGINE_BITVECTOR *pIntrPending) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispServiceLowLatencyIntrs(pGpu, pKernelDisplay, arg3, arg4, arg5, pIntrServicedHeadMask, pIntrPending) kdispServiceLowLatencyIntrs_KERNEL(pGpu, pKernelDisplay, arg3, arg4, arg5, pIntrServicedHeadMask, pIntrPending)
#endif //__nvoc_kern_disp_h_disabled

#define kdispServiceLowLatencyIntrs_HAL(pGpu, pKernelDisplay, arg3, arg4, arg5, pIntrServicedHeadMask, pIntrPending) kdispServiceLowLatencyIntrs(pGpu, pKernelDisplay, arg3, arg4, arg5, pIntrServicedHeadMask, pIntrPending)

NV_STATUS kdispConstructInstMem_IMPL(struct KernelDisplay *pKernelDisplay);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispConstructInstMem(struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispConstructInstMem(pKernelDisplay) kdispConstructInstMem_IMPL(pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispConstructInstMem_HAL(pKernelDisplay) kdispConstructInstMem(pKernelDisplay)

void kdispDestructInstMem_IMPL(struct KernelDisplay *pKernelDisplay);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispDestructInstMem(struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispDestructInstMem(pKernelDisplay) kdispDestructInstMem_IMPL(pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispDestructInstMem_HAL(pKernelDisplay) kdispDestructInstMem(pKernelDisplay)

NV_STATUS kdispSelectClass_v03_00_KERNEL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 swClass);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispSelectClass(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 swClass) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispSelectClass(pGpu, pKernelDisplay, swClass) kdispSelectClass_v03_00_KERNEL(pGpu, pKernelDisplay, swClass)
#endif //__nvoc_kern_disp_h_disabled

#define kdispSelectClass_HAL(pGpu, pKernelDisplay, swClass) kdispSelectClass(pGpu, pKernelDisplay, swClass)

NvS32 kdispGetBaseOffset_v04_02(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay);


#ifdef __nvoc_kern_disp_h_disabled
static inline NvS32 kdispGetBaseOffset(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return 0;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispGetBaseOffset(pGpu, pKernelDisplay) kdispGetBaseOffset_v04_02(pGpu, pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispGetBaseOffset_HAL(pGpu, pKernelDisplay) kdispGetBaseOffset(pGpu, pKernelDisplay)

NV_STATUS kdispGetChannelNum_v03_00(struct KernelDisplay *pKernelDisplay, DISPCHNCLASS channelClass, NvU32 channelInstance, NvU32 *pChannelNum);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispGetChannelNum(struct KernelDisplay *pKernelDisplay, DISPCHNCLASS channelClass, NvU32 channelInstance, NvU32 *pChannelNum) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispGetChannelNum(pKernelDisplay, channelClass, channelInstance, pChannelNum) kdispGetChannelNum_v03_00(pKernelDisplay, channelClass, channelInstance, pChannelNum)
#endif //__nvoc_kern_disp_h_disabled

#define kdispGetChannelNum_HAL(pKernelDisplay, channelClass, channelInstance, pChannelNum) kdispGetChannelNum(pKernelDisplay, channelClass, channelInstance, pChannelNum)

void kdispGetDisplayCapsBaseAndSize_v03_00(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 *pOffset, NvU32 *pSize);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispGetDisplayCapsBaseAndSize(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 *pOffset, NvU32 *pSize) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispGetDisplayCapsBaseAndSize(pGpu, pKernelDisplay, pOffset, pSize) kdispGetDisplayCapsBaseAndSize_v03_00(pGpu, pKernelDisplay, pOffset, pSize)
#endif //__nvoc_kern_disp_h_disabled

#define kdispGetDisplayCapsBaseAndSize_HAL(pGpu, pKernelDisplay, pOffset, pSize) kdispGetDisplayCapsBaseAndSize(pGpu, pKernelDisplay, pOffset, pSize)

void kdispGetDisplaySfUserBaseAndSize_v03_00(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 *pOffset, NvU32 *pSize);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispGetDisplaySfUserBaseAndSize(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 *pOffset, NvU32 *pSize) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispGetDisplaySfUserBaseAndSize(pGpu, pKernelDisplay, pOffset, pSize) kdispGetDisplaySfUserBaseAndSize_v03_00(pGpu, pKernelDisplay, pOffset, pSize)
#endif //__nvoc_kern_disp_h_disabled

#define kdispGetDisplaySfUserBaseAndSize_HAL(pGpu, pKernelDisplay, pOffset, pSize) kdispGetDisplaySfUserBaseAndSize(pGpu, pKernelDisplay, pOffset, pSize)

NV_STATUS kdispGetDisplayChannelUserBaseAndSize_v03_00(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, DISPCHNCLASS channelClass, NvU32 channelInstance, NvU32 *pOffset, NvU32 *pSize);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispGetDisplayChannelUserBaseAndSize(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, DISPCHNCLASS channelClass, NvU32 channelInstance, NvU32 *pOffset, NvU32 *pSize) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispGetDisplayChannelUserBaseAndSize(pGpu, pKernelDisplay, channelClass, channelInstance, pOffset, pSize) kdispGetDisplayChannelUserBaseAndSize_v03_00(pGpu, pKernelDisplay, channelClass, channelInstance, pOffset, pSize)
#endif //__nvoc_kern_disp_h_disabled

#define kdispGetDisplayChannelUserBaseAndSize_HAL(pGpu, pKernelDisplay, channelClass, channelInstance, pOffset, pSize) kdispGetDisplayChannelUserBaseAndSize(pGpu, pKernelDisplay, channelClass, channelInstance, pOffset, pSize)

NV_STATUS kdispImportImpData_IMPL(struct KernelDisplay *pKernelDisplay);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispImportImpData(struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispImportImpData(pKernelDisplay) kdispImportImpData_IMPL(pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispImportImpData_HAL(pKernelDisplay) kdispImportImpData(pKernelDisplay)

NV_STATUS kdispArbAndAllocDisplayBandwidth_v04_02(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, enum DISPLAY_ICC_BW_CLIENT iccBwClient, NvU32 minRequiredIsoBandwidthKBPS, NvU32 minRequiredFloorBandwidthKBPS);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispArbAndAllocDisplayBandwidth(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, enum DISPLAY_ICC_BW_CLIENT iccBwClient, NvU32 minRequiredIsoBandwidthKBPS, NvU32 minRequiredFloorBandwidthKBPS) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispArbAndAllocDisplayBandwidth(pGpu, pKernelDisplay, iccBwClient, minRequiredIsoBandwidthKBPS, minRequiredFloorBandwidthKBPS) kdispArbAndAllocDisplayBandwidth_v04_02(pGpu, pKernelDisplay, iccBwClient, minRequiredIsoBandwidthKBPS, minRequiredFloorBandwidthKBPS)
#endif //__nvoc_kern_disp_h_disabled

#define kdispArbAndAllocDisplayBandwidth_HAL(pGpu, pKernelDisplay, iccBwClient, minRequiredIsoBandwidthKBPS, minRequiredFloorBandwidthKBPS) kdispArbAndAllocDisplayBandwidth(pGpu, pKernelDisplay, iccBwClient, minRequiredIsoBandwidthKBPS, minRequiredFloorBandwidthKBPS)

NV_STATUS kdispSetPushBufferParamsToPhysical_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel, NvHandle hObjectBuffer, struct ContextDma *pBufferContextDma, NvU32 hClass, NvU32 channelInstance, DISPCHNCLASS internalDispChnClass, ChannelPBSize channelPBSize, NvU32 subDeviceId);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispSetPushBufferParamsToPhysical(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel, NvHandle hObjectBuffer, struct ContextDma *pBufferContextDma, NvU32 hClass, NvU32 channelInstance, DISPCHNCLASS internalDispChnClass, ChannelPBSize channelPBSize, NvU32 subDeviceId) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispSetPushBufferParamsToPhysical(pGpu, pKernelDisplay, pDispChannel, hObjectBuffer, pBufferContextDma, hClass, channelInstance, internalDispChnClass, channelPBSize, subDeviceId) kdispSetPushBufferParamsToPhysical_IMPL(pGpu, pKernelDisplay, pDispChannel, hObjectBuffer, pBufferContextDma, hClass, channelInstance, internalDispChnClass, channelPBSize, subDeviceId)
#endif //__nvoc_kern_disp_h_disabled

#define kdispSetPushBufferParamsToPhysical_HAL(pGpu, pKernelDisplay, pDispChannel, hObjectBuffer, pBufferContextDma, hClass, channelInstance, internalDispChnClass, channelPBSize, subDeviceId) kdispSetPushBufferParamsToPhysical(pGpu, pKernelDisplay, pDispChannel, hObjectBuffer, pBufferContextDma, hClass, channelInstance, internalDispChnClass, channelPBSize, subDeviceId)

static inline NV_STATUS kdispAcquireDispChannelHw_56cd7a(struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel, NvU32 channelInstance, NvHandle hObjectBuffer, NvU32 initialGetPutOffset, NvBool allowGrabWithinSameClient, NvBool connectPbAtGrab) {
    return NV_OK;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispAcquireDispChannelHw(struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel, NvU32 channelInstance, NvHandle hObjectBuffer, NvU32 initialGetPutOffset, NvBool allowGrabWithinSameClient, NvBool connectPbAtGrab) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispAcquireDispChannelHw(pKernelDisplay, pDispChannel, channelInstance, hObjectBuffer, initialGetPutOffset, allowGrabWithinSameClient, connectPbAtGrab) kdispAcquireDispChannelHw_56cd7a(pKernelDisplay, pDispChannel, channelInstance, hObjectBuffer, initialGetPutOffset, allowGrabWithinSameClient, connectPbAtGrab)
#endif //__nvoc_kern_disp_h_disabled

#define kdispAcquireDispChannelHw_HAL(pKernelDisplay, pDispChannel, channelInstance, hObjectBuffer, initialGetPutOffset, allowGrabWithinSameClient, connectPbAtGrab) kdispAcquireDispChannelHw(pKernelDisplay, pDispChannel, channelInstance, hObjectBuffer, initialGetPutOffset, allowGrabWithinSameClient, connectPbAtGrab)

static inline NV_STATUS kdispReleaseDispChannelHw_56cd7a(struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel) {
    return NV_OK;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispReleaseDispChannelHw(struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispReleaseDispChannelHw(pKernelDisplay, pDispChannel) kdispReleaseDispChannelHw_56cd7a(pKernelDisplay, pDispChannel)
#endif //__nvoc_kern_disp_h_disabled

#define kdispReleaseDispChannelHw_HAL(pKernelDisplay, pDispChannel) kdispReleaseDispChannelHw(pKernelDisplay, pDispChannel)

NV_STATUS kdispMapDispChannel_IMPL(struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispMapDispChannel(struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispMapDispChannel(pKernelDisplay, pDispChannel) kdispMapDispChannel_IMPL(pKernelDisplay, pDispChannel)
#endif //__nvoc_kern_disp_h_disabled

#define kdispMapDispChannel_HAL(pKernelDisplay, pDispChannel) kdispMapDispChannel(pKernelDisplay, pDispChannel)

void kdispUnbindUnmapDispChannel_IMPL(struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispUnbindUnmapDispChannel(struct KernelDisplay *pKernelDisplay, struct DispChannel *pDispChannel) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispUnbindUnmapDispChannel(pKernelDisplay, pDispChannel) kdispUnbindUnmapDispChannel_IMPL(pKernelDisplay, pDispChannel)
#endif //__nvoc_kern_disp_h_disabled

#define kdispUnbindUnmapDispChannel_HAL(pKernelDisplay, pDispChannel) kdispUnbindUnmapDispChannel(pKernelDisplay, pDispChannel)

NV_STATUS kdispRegisterRgLineCallback_IMPL(struct KernelDisplay *pKernelDisplay, struct RgLineCallback *pRgLineCallback, NvU32 head, NvU32 rgIntrLine, NvBool bEnable);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispRegisterRgLineCallback(struct KernelDisplay *pKernelDisplay, struct RgLineCallback *pRgLineCallback, NvU32 head, NvU32 rgIntrLine, NvBool bEnable) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispRegisterRgLineCallback(pKernelDisplay, pRgLineCallback, head, rgIntrLine, bEnable) kdispRegisterRgLineCallback_IMPL(pKernelDisplay, pRgLineCallback, head, rgIntrLine, bEnable)
#endif //__nvoc_kern_disp_h_disabled

#define kdispRegisterRgLineCallback_HAL(pKernelDisplay, pRgLineCallback, head, rgIntrLine, bEnable) kdispRegisterRgLineCallback(pKernelDisplay, pRgLineCallback, head, rgIntrLine, bEnable)

void kdispInvokeRgLineCallback_KERNEL(struct KernelDisplay *pKernelDisplay, NvU32 head, NvU32 rgIntrLine, NvBool bIsIrqlIsr);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispInvokeRgLineCallback(struct KernelDisplay *pKernelDisplay, NvU32 head, NvU32 rgIntrLine, NvBool bIsIrqlIsr) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispInvokeRgLineCallback(pKernelDisplay, head, rgIntrLine, bIsIrqlIsr) kdispInvokeRgLineCallback_KERNEL(pKernelDisplay, head, rgIntrLine, bIsIrqlIsr)
#endif //__nvoc_kern_disp_h_disabled

#define kdispInvokeRgLineCallback_HAL(pKernelDisplay, head, rgIntrLine, bIsIrqlIsr) kdispInvokeRgLineCallback(pKernelDisplay, head, rgIntrLine, bIsIrqlIsr)

NvU32 kdispReadPendingVblank_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *arg3);


#ifdef __nvoc_kern_disp_h_disabled
static inline NvU32 kdispReadPendingVblank(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *arg3) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return 0;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispReadPendingVblank(pGpu, pKernelDisplay, arg3) kdispReadPendingVblank_IMPL(pGpu, pKernelDisplay, arg3)
#endif //__nvoc_kern_disp_h_disabled

#define kdispReadPendingVblank_HAL(pGpu, pKernelDisplay, arg3) kdispReadPendingVblank(pGpu, pKernelDisplay, arg3)

static inline NvBool kdispGetVgaWorkspaceBase_72a2e1(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU64 *pOffset) {
    NV_ASSERT_PRECOMP(0);
    return NV_FALSE;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline NvBool kdispGetVgaWorkspaceBase(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU64 *pOffset) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_FALSE;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispGetVgaWorkspaceBase(pGpu, pKernelDisplay, pOffset) kdispGetVgaWorkspaceBase_72a2e1(pGpu, pKernelDisplay, pOffset)
#endif //__nvoc_kern_disp_h_disabled

#define kdispGetVgaWorkspaceBase_HAL(pGpu, pKernelDisplay, pOffset) kdispGetVgaWorkspaceBase(pGpu, pKernelDisplay, pOffset)

void kdispInvokeDisplayModesetCallback_KERNEL(struct KernelDisplay *pKernelDisplay, NvBool bModesetStart, NvU32 minRequiredIsoBandwidthKBPS, NvU32 minRequiredFloorBandwidthKBPS);

void kdispInvokeDisplayModesetCallback_PHYSICAL(struct KernelDisplay *pKernelDisplay, NvBool bModesetStart, NvU32 minRequiredIsoBandwidthKBPS, NvU32 minRequiredFloorBandwidthKBPS);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispInvokeDisplayModesetCallback(struct KernelDisplay *pKernelDisplay, NvBool bModesetStart, NvU32 minRequiredIsoBandwidthKBPS, NvU32 minRequiredFloorBandwidthKBPS) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispInvokeDisplayModesetCallback(pKernelDisplay, bModesetStart, minRequiredIsoBandwidthKBPS, minRequiredFloorBandwidthKBPS) kdispInvokeDisplayModesetCallback_KERNEL(pKernelDisplay, bModesetStart, minRequiredIsoBandwidthKBPS, minRequiredFloorBandwidthKBPS)
#endif //__nvoc_kern_disp_h_disabled

#define kdispInvokeDisplayModesetCallback_HAL(pKernelDisplay, bModesetStart, minRequiredIsoBandwidthKBPS, minRequiredFloorBandwidthKBPS) kdispInvokeDisplayModesetCallback(pKernelDisplay, bModesetStart, minRequiredIsoBandwidthKBPS, minRequiredFloorBandwidthKBPS)

NV_STATUS kdispReadRgLineCountAndFrameCount_v03_00_PHYSICAL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 head, NvU32 *pLineCount, NvU32 *pFrameCount);

NV_STATUS kdispReadRgLineCountAndFrameCount_v03_00_KERNEL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 head, NvU32 *pLineCount, NvU32 *pFrameCount);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispReadRgLineCountAndFrameCount(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 head, NvU32 *pLineCount, NvU32 *pFrameCount) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispReadRgLineCountAndFrameCount(pGpu, pKernelDisplay, head, pLineCount, pFrameCount) kdispReadRgLineCountAndFrameCount_v03_00_KERNEL(pGpu, pKernelDisplay, head, pLineCount, pFrameCount)
#endif //__nvoc_kern_disp_h_disabled

#define kdispReadRgLineCountAndFrameCount_HAL(pGpu, pKernelDisplay, head, pLineCount, pFrameCount) kdispReadRgLineCountAndFrameCount(pGpu, pKernelDisplay, head, pLineCount, pFrameCount)

static inline NV_STATUS kdispDsmMxmMxcbExecuteAcpi_92bfc3(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, void *pInOutData, NvU16 *outDataSize) {
    NV_ASSERT_PRECOMP(0);
    return NV_ERR_NOT_SUPPORTED;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispDsmMxmMxcbExecuteAcpi(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, void *pInOutData, NvU16 *outDataSize) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispDsmMxmMxcbExecuteAcpi(pGpu, pKernelDisplay, pInOutData, outDataSize) kdispDsmMxmMxcbExecuteAcpi_92bfc3(pGpu, pKernelDisplay, pInOutData, outDataSize)
#endif //__nvoc_kern_disp_h_disabled

#define kdispDsmMxmMxcbExecuteAcpi_HAL(pGpu, pKernelDisplay, pInOutData, outDataSize) kdispDsmMxmMxcbExecuteAcpi(pGpu, pKernelDisplay, pInOutData, outDataSize)

static inline NV_STATUS kdispInitBrightcStateLoad_56cd7a(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    return NV_OK;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispInitBrightcStateLoad(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispInitBrightcStateLoad(pGpu, pKernelDisplay) kdispInitBrightcStateLoad_56cd7a(pGpu, pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispInitBrightcStateLoad_HAL(pGpu, pKernelDisplay) kdispInitBrightcStateLoad(pGpu, pKernelDisplay)

static inline NV_STATUS kdispSetupAcpiEdid_56cd7a(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    return NV_OK;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispSetupAcpiEdid(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispSetupAcpiEdid(pGpu, pKernelDisplay) kdispSetupAcpiEdid_56cd7a(pGpu, pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispSetupAcpiEdid_HAL(pGpu, pKernelDisplay) kdispSetupAcpiEdid(pGpu, pKernelDisplay)

NV_STATUS kdispGetRgScanLock_v02_01(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 head0, struct OBJGPU *pPeerGpu, NvU32 head1, NvBool *pMasterScanLock, NvU32 *pMasterScanLockPin, NvBool *pSlaveScanLock, NvU32 *pSlaveScanLockPin);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispGetRgScanLock(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 head0, struct OBJGPU *pPeerGpu, NvU32 head1, NvBool *pMasterScanLock, NvU32 *pMasterScanLockPin, NvBool *pSlaveScanLock, NvU32 *pSlaveScanLockPin) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispGetRgScanLock(pGpu, pKernelDisplay, head0, pPeerGpu, head1, pMasterScanLock, pMasterScanLockPin, pSlaveScanLock, pSlaveScanLockPin) kdispGetRgScanLock_v02_01(pGpu, pKernelDisplay, head0, pPeerGpu, head1, pMasterScanLock, pMasterScanLockPin, pSlaveScanLock, pSlaveScanLockPin)
#endif //__nvoc_kern_disp_h_disabled

#define kdispGetRgScanLock_HAL(pGpu, pKernelDisplay, head0, pPeerGpu, head1, pMasterScanLock, pMasterScanLockPin, pSlaveScanLock, pSlaveScanLockPin) kdispGetRgScanLock(pGpu, pKernelDisplay, head0, pPeerGpu, head1, pMasterScanLock, pMasterScanLockPin, pSlaveScanLock, pSlaveScanLockPin)

NV_STATUS kdispDetectSliLink_v04_00(struct KernelDisplay *pKernelDisplay, struct OBJGPU *pParentGpu, struct OBJGPU *pChildGpu, NvU32 ParentDrPort, NvU32 ChildDrPort);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispDetectSliLink(struct KernelDisplay *pKernelDisplay, struct OBJGPU *pParentGpu, struct OBJGPU *pChildGpu, NvU32 ParentDrPort, NvU32 ChildDrPort) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispDetectSliLink(pKernelDisplay, pParentGpu, pChildGpu, ParentDrPort, ChildDrPort) kdispDetectSliLink_v04_00(pKernelDisplay, pParentGpu, pChildGpu, ParentDrPort, ChildDrPort)
#endif //__nvoc_kern_disp_h_disabled

#define kdispDetectSliLink_HAL(pKernelDisplay, pParentGpu, pChildGpu, ParentDrPort, ChildDrPort) kdispDetectSliLink(pKernelDisplay, pParentGpu, pChildGpu, ParentDrPort, ChildDrPort)

NV_STATUS kdispReadAwakenChannelNumMask_v03_00(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 *arg3, DISPCHNCLASS arg4, THREAD_STATE_NODE *arg5);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispReadAwakenChannelNumMask(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 *arg3, DISPCHNCLASS arg4, THREAD_STATE_NODE *arg5) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispReadAwakenChannelNumMask(pGpu, pKernelDisplay, arg3, arg4, arg5) kdispReadAwakenChannelNumMask_v03_00(pGpu, pKernelDisplay, arg3, arg4, arg5)
#endif //__nvoc_kern_disp_h_disabled

#define kdispReadAwakenChannelNumMask_HAL(pGpu, pKernelDisplay, arg3, arg4, arg5) kdispReadAwakenChannelNumMask(pGpu, pKernelDisplay, arg3, arg4, arg5)

NV_STATUS kdispAllocateCommonHandle_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay);


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispAllocateCommonHandle(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispAllocateCommonHandle(pGpu, pKernelDisplay) kdispAllocateCommonHandle_IMPL(pGpu, pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispAllocateCommonHandle_HAL(pGpu, pKernelDisplay) kdispAllocateCommonHandle(pGpu, pKernelDisplay)

void kdispDestroyCommonHandle_IMPL(struct KernelDisplay *pKernelDisplay);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispDestroyCommonHandle(struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispDestroyCommonHandle(pKernelDisplay) kdispDestroyCommonHandle_IMPL(pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispDestroyCommonHandle_HAL(pKernelDisplay) kdispDestroyCommonHandle(pKernelDisplay)

static inline NV_STATUS kdispAllocateSharedMem_46f6a7(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    return NV_ERR_NOT_SUPPORTED;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispAllocateSharedMem(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispAllocateSharedMem(pGpu, pKernelDisplay) kdispAllocateSharedMem_46f6a7(pGpu, pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispAllocateSharedMem_HAL(pGpu, pKernelDisplay) kdispAllocateSharedMem(pGpu, pKernelDisplay)

void kdispFreeSharedMem_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispFreeSharedMem(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispFreeSharedMem(pGpu, pKernelDisplay) kdispFreeSharedMem_IMPL(pGpu, pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispFreeSharedMem_HAL(pGpu, pKernelDisplay) kdispFreeSharedMem(pGpu, pKernelDisplay)

NvBool kdispIsDisplayConnected_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay);


#ifdef __nvoc_kern_disp_h_disabled
static inline NvBool kdispIsDisplayConnected(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_FALSE;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispIsDisplayConnected(pGpu, pKernelDisplay) kdispIsDisplayConnected_IMPL(pGpu, pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispIsDisplayConnected_HAL(pGpu, pKernelDisplay) kdispIsDisplayConnected(pGpu, pKernelDisplay)

NvU32 kdispGetSupportedDisplayMask_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay);


#ifdef __nvoc_kern_disp_h_disabled
static inline NvU32 kdispGetSupportedDisplayMask(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return 0;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispGetSupportedDisplayMask(pGpu, pKernelDisplay) kdispGetSupportedDisplayMask_IMPL(pGpu, pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispGetSupportedDisplayMask_HAL(pGpu, pKernelDisplay) kdispGetSupportedDisplayMask(pGpu, pKernelDisplay)

static inline void kdispUpdatePdbAfterIpHalInit_b3696a(struct KernelDisplay *pKernelDisplay) {
    return;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispUpdatePdbAfterIpHalInit(struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispUpdatePdbAfterIpHalInit(pKernelDisplay) kdispUpdatePdbAfterIpHalInit_b3696a(pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

#define kdispUpdatePdbAfterIpHalInit_HAL(pKernelDisplay) kdispUpdatePdbAfterIpHalInit(pKernelDisplay)

NvBool kdispReadPendingWinSemIntr_v04_01(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *arg3, NvU32 *arg4);


#ifdef __nvoc_kern_disp_h_disabled
static inline NvBool kdispReadPendingWinSemIntr(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *arg3, NvU32 *arg4) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_FALSE;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispReadPendingWinSemIntr(pGpu, pKernelDisplay, arg3, arg4) kdispReadPendingWinSemIntr_v04_01(pGpu, pKernelDisplay, arg3, arg4)
#endif //__nvoc_kern_disp_h_disabled

#define kdispReadPendingWinSemIntr_HAL(pGpu, pKernelDisplay, arg3, arg4) kdispReadPendingWinSemIntr(pGpu, pKernelDisplay, arg3, arg4)

void kdispHandleWinSemEvt_v04_01(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *arg3);


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispHandleWinSemEvt(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *arg3) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispHandleWinSemEvt(pGpu, pKernelDisplay, arg3) kdispHandleWinSemEvt_v04_01(pGpu, pKernelDisplay, arg3)
#endif //__nvoc_kern_disp_h_disabled

#define kdispHandleWinSemEvt_HAL(pGpu, pKernelDisplay, arg3) kdispHandleWinSemEvt(pGpu, pKernelDisplay, arg3)

static inline void kdispIntrRetrigger_b3696a(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 index, THREAD_STATE_NODE *arg4) {
    return;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispIntrRetrigger(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 index, THREAD_STATE_NODE *arg4) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispIntrRetrigger(pGpu, pKernelDisplay, index, arg4) kdispIntrRetrigger_b3696a(pGpu, pKernelDisplay, index, arg4)
#endif //__nvoc_kern_disp_h_disabled

#define kdispIntrRetrigger_HAL(pGpu, pKernelDisplay, index, arg4) kdispIntrRetrigger(pGpu, pKernelDisplay, index, arg4)

NvU32 kdispServiceAwakenIntr_v03_00(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *pThreadState);


#ifdef __nvoc_kern_disp_h_disabled
static inline NvU32 kdispServiceAwakenIntr(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *pThreadState) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return 0;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispServiceAwakenIntr(pGpu, pKernelDisplay, pThreadState) kdispServiceAwakenIntr_v03_00(pGpu, pKernelDisplay, pThreadState)
#endif //__nvoc_kern_disp_h_disabled

#define kdispServiceAwakenIntr_HAL(pGpu, pKernelDisplay, pThreadState) kdispServiceAwakenIntr(pGpu, pKernelDisplay, pThreadState)

static inline void kdispSetChannelTrashAndAbortAccel_b3696a(struct OBJGPU *arg1, struct KernelDisplay *arg2, NvU32 arg3, NvU32 arg4, NvBool arg5) {
    return;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispSetChannelTrashAndAbortAccel(struct OBJGPU *arg1, struct KernelDisplay *arg2, NvU32 arg3, NvU32 arg4, NvBool arg5) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispSetChannelTrashAndAbortAccel(arg1, arg2, arg3, arg4, arg5) kdispSetChannelTrashAndAbortAccel_b3696a(arg1, arg2, arg3, arg4, arg5)
#endif //__nvoc_kern_disp_h_disabled

#define kdispSetChannelTrashAndAbortAccel_HAL(arg1, arg2, arg3, arg4, arg5) kdispSetChannelTrashAndAbortAccel(arg1, arg2, arg3, arg4, arg5)

static inline NvBool kdispIsChannelIdle_3dd2c9(struct OBJGPU *arg1, struct KernelDisplay *arg2, NvU32 arg3, NvU32 arg4) {
    return NV_FALSE;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline NvBool kdispIsChannelIdle(struct OBJGPU *arg1, struct KernelDisplay *arg2, NvU32 arg3, NvU32 arg4) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_FALSE;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispIsChannelIdle(arg1, arg2, arg3, arg4) kdispIsChannelIdle_3dd2c9(arg1, arg2, arg3, arg4)
#endif //__nvoc_kern_disp_h_disabled

#define kdispIsChannelIdle_HAL(arg1, arg2, arg3, arg4) kdispIsChannelIdle(arg1, arg2, arg3, arg4)

static inline void kdispApplyChannelConnectDisconnect_b3696a(struct OBJGPU *arg1, struct KernelDisplay *arg2, NvU32 arg3, NvU32 arg4, NvU32 arg5) {
    return;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispApplyChannelConnectDisconnect(struct OBJGPU *arg1, struct KernelDisplay *arg2, NvU32 arg3, NvU32 arg4, NvU32 arg5) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispApplyChannelConnectDisconnect(arg1, arg2, arg3, arg4, arg5) kdispApplyChannelConnectDisconnect_b3696a(arg1, arg2, arg3, arg4, arg5)
#endif //__nvoc_kern_disp_h_disabled

#define kdispApplyChannelConnectDisconnect_HAL(arg1, arg2, arg3, arg4, arg5) kdispApplyChannelConnectDisconnect(arg1, arg2, arg3, arg4, arg5)

static inline NvBool kdispIsChannelAllocatedHw_3dd2c9(struct OBJGPU *arg1, struct KernelDisplay *arg2, NvU32 arg3, NvU32 arg4) {
    return NV_FALSE;
}


#ifdef __nvoc_kern_disp_h_disabled
static inline NvBool kdispIsChannelAllocatedHw(struct OBJGPU *arg1, struct KernelDisplay *arg2, NvU32 arg3, NvU32 arg4) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_FALSE;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispIsChannelAllocatedHw(arg1, arg2, arg3, arg4) kdispIsChannelAllocatedHw_3dd2c9(arg1, arg2, arg3, arg4)
#endif //__nvoc_kern_disp_h_disabled

#define kdispIsChannelAllocatedHw_HAL(arg1, arg2, arg3, arg4) kdispIsChannelAllocatedHw(arg1, arg2, arg3, arg4)

NV_STATUS kdispConstructEngine_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, ENGDESCRIPTOR engDesc);

NV_STATUS kdispStatePreInitLocked_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay);

NV_STATUS kdispStateInitLocked_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay);

void kdispStateDestroy_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay);

NV_STATUS kdispStateLoad_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 flags);

NV_STATUS kdispStateUnload_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 flags);

NvU32 kdispGetPBTargetAperture_v03_00(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 memAddrSpace, NvU32 cacheSnoop);

NvU32 kdispGetPBTargetAperture_v05_01(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 memAddrSpace, NvU32 cacheSnoop);

NV_STATUS kdispComputeDpModeSettings_v02_04(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 headIndex, DPMODESETDATA *pDpModesetData, DPIMPINFO *dpInfo);

NV_STATUS kdispComputeDpModeSettings_v05_01(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 headIndex, DPMODESETDATA *pDpModesetData, DPIMPINFO *dpInfo);

void kdispDestruct_IMPL(struct KernelDisplay *pKernelDisplay);

#define __nvoc_kdispDestruct(pKernelDisplay) kdispDestruct_IMPL(pKernelDisplay)
NV_STATUS kdispConstructKhead_IMPL(struct KernelDisplay *pKernelDisplay);

#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispConstructKhead(struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispConstructKhead(pKernelDisplay) kdispConstructKhead_IMPL(pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

void kdispDestructKhead_IMPL(struct KernelDisplay *pKernelDisplay);

#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispDestructKhead(struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispDestructKhead(pKernelDisplay) kdispDestructKhead_IMPL(pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

NV_STATUS kdispGetIntChnClsForHwCls_IMPL(struct KernelDisplay *pKernelDisplay, NvU32 hwClass, DISPCHNCLASS *pDispChnClass);

#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispGetIntChnClsForHwCls(struct KernelDisplay *pKernelDisplay, NvU32 hwClass, DISPCHNCLASS *pDispChnClass) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispGetIntChnClsForHwCls(pKernelDisplay, hwClass, pDispChnClass) kdispGetIntChnClsForHwCls_IMPL(pKernelDisplay, hwClass, pDispChnClass)
#endif //__nvoc_kern_disp_h_disabled

void kdispNotifyCommonEvent_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 notifyIndex, void *pNotifyParams);

#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispNotifyCommonEvent(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 notifyIndex, void *pNotifyParams) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispNotifyCommonEvent(pGpu, pKernelDisplay, notifyIndex, pNotifyParams) kdispNotifyCommonEvent_IMPL(pGpu, pKernelDisplay, notifyIndex, pNotifyParams)
#endif //__nvoc_kern_disp_h_disabled

void kdispNotifyEvent_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 notifyIndex, void *pNotifyParams, NvU32 notifyParamsSize, NvV32 info32, NvV16 info16);

#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispNotifyEvent(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvU32 notifyIndex, void *pNotifyParams, NvU32 notifyParamsSize, NvV32 info32, NvV16 info16) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispNotifyEvent(pGpu, pKernelDisplay, notifyIndex, pNotifyParams, notifyParamsSize, info32, info16) kdispNotifyEvent_IMPL(pGpu, pKernelDisplay, notifyIndex, pNotifyParams, notifyParamsSize, info32, info16)
#endif //__nvoc_kern_disp_h_disabled

NV_STATUS kdispOptimizePerFrameOsCallbacks_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvBool clearIntr, THREAD_STATE_NODE *pThreadState, NvU32 *intrServicedHeadMask, MC_ENGINE_BITVECTOR *intrPending);

#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispOptimizePerFrameOsCallbacks(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, NvBool clearIntr, THREAD_STATE_NODE *pThreadState, NvU32 *intrServicedHeadMask, MC_ENGINE_BITVECTOR *intrPending) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispOptimizePerFrameOsCallbacks(pGpu, pKernelDisplay, clearIntr, pThreadState, intrServicedHeadMask, intrPending) kdispOptimizePerFrameOsCallbacks_IMPL(pGpu, pKernelDisplay, clearIntr, pThreadState, intrServicedHeadMask, intrPending)
#endif //__nvoc_kern_disp_h_disabled

NV_STATUS kdispSetupVBlank_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, void *pProc, void *pParm1, void *pParm2, NvU32 Head, void *pParm3);

#ifdef __nvoc_kern_disp_h_disabled
static inline NV_STATUS kdispSetupVBlank(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, void *pProc, void *pParm1, void *pParm2, NvU32 Head, void *pParm3) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_kern_disp_h_disabled
#define kdispSetupVBlank(pGpu, pKernelDisplay, pProc, pParm1, pParm2, Head, pParm3) kdispSetupVBlank_IMPL(pGpu, pKernelDisplay, pProc, pParm1, pParm2, Head, pParm3)
#endif //__nvoc_kern_disp_h_disabled

void kdispDestroyVBlank_IMPL(struct KernelDisplay *pKernelDisplay);

#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispDestroyVBlank(struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispDestroyVBlank(pKernelDisplay) kdispDestroyVBlank_IMPL(pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

void kdispHandleAggressiveVblank_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *arg3, MC_ENGINE_BITVECTOR *arg4);

#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispHandleAggressiveVblank(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay, THREAD_STATE_NODE *arg3, MC_ENGINE_BITVECTOR *arg4) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispHandleAggressiveVblank(pGpu, pKernelDisplay, arg3, arg4) kdispHandleAggressiveVblank_IMPL(pGpu, pKernelDisplay, arg3, arg4)
#endif //__nvoc_kern_disp_h_disabled

void kdispApplyAggressiveVblankHandlingWar_IMPL(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay);

#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispApplyAggressiveVblankHandlingWar(struct OBJGPU *pGpu, struct KernelDisplay *pKernelDisplay) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispApplyAggressiveVblankHandlingWar(pGpu, pKernelDisplay) kdispApplyAggressiveVblankHandlingWar_IMPL(pGpu, pKernelDisplay)
#endif //__nvoc_kern_disp_h_disabled

void kdispSetWarPurgeSatellitesOnCoreFree_IMPL(struct KernelDisplay *pKernelDisplay, NvBool value);

#ifdef __nvoc_kern_disp_h_disabled
static inline void kdispSetWarPurgeSatellitesOnCoreFree(struct KernelDisplay *pKernelDisplay, NvBool value) {
    NV_ASSERT_FAILED_PRECOMP("KernelDisplay was disabled!");
}
#else //__nvoc_kern_disp_h_disabled
#define kdispSetWarPurgeSatellitesOnCoreFree(pKernelDisplay, value) kdispSetWarPurgeSatellitesOnCoreFree_IMPL(pKernelDisplay, value)
#endif //__nvoc_kern_disp_h_disabled

#undef PRIVATE_FIELD


/*!
 * Custom roll a conditional acquire spinlock.
 * To be replaced when nvport supports drop-in replacement
 */
void kdispAcquireLowLatencyLock(volatile NvS32 *pLowLatencyLock);
NvBool kdispAcquireLowLatencyLockConditional(volatile NvS32 *pLowLatencyLock);
void kdispReleaseLowLatencyLock(volatile NvS32 *pLowLatencyLock);

void
dispdeviceFillVgaSavedDisplayState( struct OBJGPU *pGpu,
    NvU64   vgaAddr,
    NvU8    vgaMemType,
    NvBool  vgaValid,
    NvU64   workspaceAddr,
    NvU8    workspaceMemType,
    NvBool  workspaceValid,
    NvBool  baseValid,
    NvBool  workspaceBaseValid
);

/*! PushBuffer Target Aperture Types */
typedef enum
{
    IOVA,
    PHYS_NVM,
    PHYS_PCI,
    PHYS_PCI_COHERENT
} PBTARGETAPERTURE;

static NV_INLINE struct KernelHead*
kdispGetHead
(
    struct KernelDisplay *pKernelDisplay,
    NvU32 head
)
{
    if (head >= OBJ_MAX_HEADS)
    {
        return NULL;
    }

    return pKernelDisplay->pKernelHead[head];
}

static NV_INLINE NvU32
kdispGetNumHeads(struct KernelDisplay *pKernelDisplay)
{
    NV_ASSERT(pKernelDisplay != NULL);
    return pKernelDisplay->numHeads;
}

static NV_INLINE NvU32
kdispGetDeferredVblankHeadMask(struct KernelDisplay *pKernelDisplay)
{
    return pKernelDisplay->deferredVblankHeadMask;
}

static NV_INLINE void
kdispSetDeferredVblankHeadMask(struct KernelDisplay *pKernelDisplay, NvU32 vblankHeadMask)
{
    pKernelDisplay->deferredVblankHeadMask = vblankHeadMask;
}

static NV_INLINE NvHandle
kdispGetInternalClientHandle(struct KernelDisplay *pKernelDisplay)
{
    return pKernelDisplay->hInternalClient;
}

static NV_INLINE NvHandle
kdispGetDispCommonHandle(struct KernelDisplay *pKernelDisplay)
{
    return pKernelDisplay->hDispCommonHandle;
}

#endif // KERN_DISP_H

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _G_KERN_DISP_NVOC_H_
