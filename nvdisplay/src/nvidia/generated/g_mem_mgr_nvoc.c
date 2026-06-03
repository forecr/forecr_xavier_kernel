#define NVOC_MEM_MGR_H_PRIVATE_ACCESS_ALLOWED

// Version of generated metadata structures
#ifdef NVOC_METADATA_VERSION
#undef NVOC_METADATA_VERSION
#endif
#define NVOC_METADATA_VERSION 2

#include "nvoc/runtime.h"
#include "nvoc/rtti.h"
#include "nvtypes.h"
#include "nvport/nvport.h"
#include "nvport/inline/util_valist.h"
#include "utils/nvassert.h"
#include "g_mem_mgr_nvoc.h"


#ifdef DEBUG
char __nvoc_class_id_uniqueness_check__22ad47 = 1;
#endif

extern const struct NVOC_CLASS_DEF __nvoc_class_def_MemoryManager;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_Object;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJENGSTATE;

// Forward declarations for MemoryManager
void __nvoc_init__OBJENGSTATE(OBJENGSTATE*);
void __nvoc_init__MemoryManager(MemoryManager*, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner);
void __nvoc_init_funcTable_MemoryManager(MemoryManager*, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner);
NV_STATUS __nvoc_ctor_MemoryManager(MemoryManager*, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner);
void __nvoc_init_dataField_MemoryManager(MemoryManager*, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner);
void __nvoc_dtor_MemoryManager(MemoryManager*);

// Structures used within RTTI (run-time type information)
extern const struct NVOC_CASTINFO __nvoc_castinfo__MemoryManager;
extern const struct NVOC_EXPORT_INFO __nvoc_export_info__MemoryManager;

// Down-thunk(s) to bridge MemoryManager methods from ancestors (if any)

// Up-thunk(s) to bridge MemoryManager methods to ancestors (if any)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrConstructEngine(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, ENGDESCRIPTOR arg3);    // this
void __nvoc_up_thunk_OBJENGSTATE_memmgrInitMissing(struct OBJGPU *pGpu, struct MemoryManager *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePreInitLocked(struct OBJGPU *pGpu, struct MemoryManager *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePreInitUnlocked(struct OBJGPU *pGpu, struct MemoryManager *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStateInitLocked(struct OBJGPU *pGpu, struct MemoryManager *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStateInitUnlocked(struct OBJGPU *pGpu, struct MemoryManager *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePreLoad(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStateLoad(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePostLoad(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePreUnload(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStateUnload(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePostUnload(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3);    // this
void __nvoc_up_thunk_OBJENGSTATE_memmgrStateDestroy(struct OBJGPU *pGpu, struct MemoryManager *pEngstate);    // this
NvBool __nvoc_up_thunk_OBJENGSTATE_memmgrIsPresent(struct OBJGPU *pGpu, struct MemoryManager *pEngstate);    // this

// Class-specific details for MemoryManager
const struct NVOC_CLASS_DEF __nvoc_class_def_MemoryManager = 
{
    .classInfo.size =               sizeof(MemoryManager),
    .classInfo.classId =            classId(MemoryManager),
    .classInfo.providerId =         &__nvoc_rtti_provider,
#if NV_PRINTF_STRINGS_ALLOWED
    .classInfo.name =               "MemoryManager",
#endif
    .objCreatefn =        (NVOC_DYNAMIC_OBJ_CREATE) &__nvoc_objCreateDynamic_MemoryManager,
    .pCastInfo =          &__nvoc_castinfo__MemoryManager,
    .pExportInfo =        &__nvoc_export_info__MemoryManager
};


// Metadata with per-class RTTI and vtable with ancestor(s)
static const struct NVOC_METADATA__MemoryManager __nvoc_metadata__MemoryManager = {
    .rtti.pClassDef = &__nvoc_class_def_MemoryManager,    // (memmgr) this
    .rtti.dtor      = (NVOC_DYNAMIC_DTOR) &__nvoc_dtor_MemoryManager,
    .rtti.offset    = 0,
    .metadata__OBJENGSTATE.rtti.pClassDef = &__nvoc_class_def_OBJENGSTATE,    // (engstate) super
    .metadata__OBJENGSTATE.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__OBJENGSTATE.rtti.offset    = NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE),
    .metadata__OBJENGSTATE.metadata__Object.rtti.pClassDef = &__nvoc_class_def_Object,    // (obj) super^2
    .metadata__OBJENGSTATE.metadata__Object.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__OBJENGSTATE.metadata__Object.rtti.offset    = NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE.__nvoc_base_Object),

    .vtable.__memmgrConstructEngine__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrConstructEngine,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateConstructEngine__ = &engstateConstructEngine_IMPL,    // virtual
    .vtable.__memmgrInitMissing__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrInitMissing,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateInitMissing__ = &engstateInitMissing_IMPL,    // virtual
    .vtable.__memmgrStatePreInitLocked__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStatePreInitLocked,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreInitLocked__ = &engstateStatePreInitLocked_IMPL,    // virtual
    .vtable.__memmgrStatePreInitUnlocked__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStatePreInitUnlocked,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreInitUnlocked__ = &engstateStatePreInitUnlocked_IMPL,    // virtual
    .vtable.__memmgrStateInitLocked__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStateInitLocked,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateInitLocked__ = &engstateStateInitLocked_IMPL,    // virtual
    .vtable.__memmgrStateInitUnlocked__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStateInitUnlocked,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateInitUnlocked__ = &engstateStateInitUnlocked_IMPL,    // virtual
    .vtable.__memmgrStatePreLoad__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStatePreLoad,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreLoad__ = &engstateStatePreLoad_IMPL,    // virtual
    .vtable.__memmgrStateLoad__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStateLoad,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateLoad__ = &engstateStateLoad_IMPL,    // virtual
    .vtable.__memmgrStatePostLoad__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStatePostLoad,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePostLoad__ = &engstateStatePostLoad_IMPL,    // virtual
    .vtable.__memmgrStatePreUnload__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStatePreUnload,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreUnload__ = &engstateStatePreUnload_IMPL,    // virtual
    .vtable.__memmgrStateUnload__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStateUnload,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateUnload__ = &engstateStateUnload_IMPL,    // virtual
    .vtable.__memmgrStatePostUnload__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStatePostUnload,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePostUnload__ = &engstateStatePostUnload_IMPL,    // virtual
    .vtable.__memmgrStateDestroy__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrStateDestroy,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateDestroy__ = &engstateStateDestroy_IMPL,    // virtual
    .vtable.__memmgrIsPresent__ = &__nvoc_up_thunk_OBJENGSTATE_memmgrIsPresent,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateIsPresent__ = &engstateIsPresent_IMPL,    // virtual
};


// Dynamic down-casting information
const struct NVOC_CASTINFO __nvoc_castinfo__MemoryManager = {
    .numRelatives = 3,
    .relatives = {
        &__nvoc_metadata__MemoryManager.rtti,    // [0]: (memmgr) this
        &__nvoc_metadata__MemoryManager.metadata__OBJENGSTATE.rtti,    // [1]: (engstate) super
        &__nvoc_metadata__MemoryManager.metadata__OBJENGSTATE.metadata__Object.rtti,    // [2]: (obj) super^2
    }
};

// 14 up-thunk(s) defined to bridge methods in MemoryManager to superclasses

// memmgrConstructEngine: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrConstructEngine(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, ENGDESCRIPTOR arg3) {
    return engstateConstructEngine(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)), arg3);
}

// memmgrInitMissing: virtual inherited (engstate) base (engstate)
void __nvoc_up_thunk_OBJENGSTATE_memmgrInitMissing(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    engstateInitMissing(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)));
}

// memmgrStatePreInitLocked: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePreInitLocked(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return engstateStatePreInitLocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)));
}

// memmgrStatePreInitUnlocked: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePreInitUnlocked(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return engstateStatePreInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)));
}

// memmgrStateInitLocked: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStateInitLocked(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return engstateStateInitLocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)));
}

// memmgrStateInitUnlocked: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStateInitUnlocked(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return engstateStateInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)));
}

// memmgrStatePreLoad: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePreLoad(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return engstateStatePreLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)), arg3);
}

// memmgrStateLoad: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStateLoad(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return engstateStateLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)), arg3);
}

// memmgrStatePostLoad: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePostLoad(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return engstateStatePostLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)), arg3);
}

// memmgrStatePreUnload: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePreUnload(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return engstateStatePreUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)), arg3);
}

// memmgrStateUnload: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStateUnload(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return engstateStateUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)), arg3);
}

// memmgrStatePostUnload: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_memmgrStatePostUnload(struct OBJGPU *pGpu, struct MemoryManager *pEngstate, NvU32 arg3) {
    return engstateStatePostUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)), arg3);
}

// memmgrStateDestroy: virtual inherited (engstate) base (engstate)
void __nvoc_up_thunk_OBJENGSTATE_memmgrStateDestroy(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    engstateStateDestroy(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)));
}

// memmgrIsPresent: virtual inherited (engstate) base (engstate)
NvBool __nvoc_up_thunk_OBJENGSTATE_memmgrIsPresent(struct OBJGPU *pGpu, struct MemoryManager *pEngstate) {
    return engstateIsPresent(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE)));
}


const struct NVOC_EXPORT_INFO __nvoc_export_info__MemoryManager = 
{
    .numEntries=     0,
    .pExportEntries= 0
};


// Destruct MemoryManager object.
void __nvoc_dtor_OBJENGSTATE(OBJENGSTATE*);
void __nvoc_dtor_MemoryManager(MemoryManager* pThis) {

// Recurse to superclass destructors.
    __nvoc_dtor_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);

    PORT_UNREFERENCED_VARIABLE(pThis);
}
void __nvoc_init_dataField_MemoryManager(MemoryManager *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {
    ChipHal *chipHal = &pGpuhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pGpuhalspecowner);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);

    // Hal field -- bFbRegionsSupported
    // default
    {
        pThis->bFbRegionsSupported = NV_FALSE;
    }

    // Hal field -- bPmaEnabled
    // default
    {
        pThis->bPmaEnabled = NV_FALSE;
    }

    // Hal field -- bClientPageTablesPmaManaged
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0001a000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bClientPageTablesPmaManaged = NV_TRUE;
    }

    // Hal field -- bScanoutSysmem
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0001a000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bScanoutSysmem = NV_TRUE;
    }

    // Hal field -- bDisallowSplitLowerMemory
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0001a000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bDisallowSplitLowerMemory = NV_TRUE;
    }

    // Hal field -- bSmallPageCompression
    // default
    {
        pThis->bSmallPageCompression = NV_FALSE;
    }

    // Hal field -- bSysmemCompressionSupportDef
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0001a000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bSysmemCompressionSupportDef = NV_TRUE;
    }

    // Hal field -- bBug2301372IncreaseRmReserveMemoryWar
    // default
    {
        pThis->bBug2301372IncreaseRmReserveMemoryWar = NV_FALSE;
    }

    pThis->bEnableDynamicPageOfflining = NV_FALSE;

    // Hal field -- bVgpuPmaSupport
    // default
    {
        pThis->bVgpuPmaSupport = NV_FALSE;
    }

    pThis->bScrubChannelSetupInProgress = NV_FALSE;

    // Hal field -- bBug3922001DisableCtxBufOnSim
    // default
    {
        pThis->bBug3922001DisableCtxBufOnSim = NV_FALSE;
    }

    // Hal field -- bPlatformFullyCoherent
    // default
    {
        pThis->bPlatformFullyCoherent = NV_FALSE;
    }

    // Hal field -- bAllowNoncontiguousAllocation
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0001a000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bAllowNoncontiguousAllocation = NV_TRUE;
    }

    // Hal field -- bLocalEgmSupported
    // default
    {
        pThis->bLocalEgmSupported = NV_FALSE;
    }

    // Hal field -- bScrubOnFreeEnabled
    // default
    {
        pThis->bScrubOnFreeEnabled = NV_FALSE;
    }

    // Hal field -- bFastScrubberEnabled
    // default
    {
        pThis->bFastScrubberEnabled = NV_FALSE;
    }

    // Hal field -- bFastScrubberSupportsSysmem
    // default
    {
        pThis->bFastScrubberSupportsSysmem = NV_FALSE;
    }

    // Hal field -- bSysmemPageSizeDefaultAllowLargePages
    // default
    {
        pThis->bSysmemPageSizeDefaultAllowLargePages = NV_FALSE;
    }

    // Hal field -- bMonitoredFenceSupported
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0001a000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->bMonitoredFenceSupported = NV_FALSE;
    }

    // Hal field -- b64BitSemaphoresSupported
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x0001a000UL) )) /* ChipHal: T234D | T264D | T256D */ 
    {
        pThis->b64BitSemaphoresSupported = NV_FALSE;
    }

    // Hal field -- bGenericKindSupport
    // default
    {
        pThis->bGenericKindSupport = NV_FALSE;
    }

    pThis->bSkipCompressionCheck = NV_FALSE;

    // Hal field -- bUseVirtualCopyOnSuspend
    if (( ((rmVariantHal_HalVarIdx >> 5) == 0UL) && ((1UL << (rmVariantHal_HalVarIdx & 0x1f)) & 0x00000002UL) )) /* RmVariantHal: PF_KERNEL_ONLY */ 
    {
        // default
        {
            pThis->bUseVirtualCopyOnSuspend = NV_TRUE;
        }
    }
}


// Construct MemoryManager object.
NV_STATUS __nvoc_ctor_OBJENGSTATE(OBJENGSTATE *);
NV_STATUS __nvoc_ctor_MemoryManager(MemoryManager *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {
    NV_STATUS status = NV_OK;

    // Recurse to ancestor constructor(s).
    status = __nvoc_ctor_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    if (status != NV_OK) goto __nvoc_ctor_MemoryManager_fail_OBJENGSTATE;

    // Initialize data fields.
    __nvoc_init_dataField_MemoryManager(pThis, pGpuhalspecowner, pRmhalspecowner);
    goto __nvoc_ctor_MemoryManager_exit; // Success

    // Unwind on error.
__nvoc_ctor_MemoryManager_fail_OBJENGSTATE:
__nvoc_ctor_MemoryManager_exit:
    return status;
}

// Vtable initialization
static void __nvoc_init_funcTable_MemoryManager_1(MemoryManager *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {
    ChipHal *chipHal = &pGpuhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pGpuhalspecowner);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);
} // End __nvoc_init_funcTable_MemoryManager_1


// Initialize vtable(s) for 14 virtual method(s).
void __nvoc_init_funcTable_MemoryManager(MemoryManager *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {
    __nvoc_init_funcTable_MemoryManager_1(pThis, pGpuhalspecowner, pRmhalspecowner);
}

// Initialize newly constructed object.
void __nvoc_init__MemoryManager(MemoryManager *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {

    // Initialize pointers to inherited data.
    pThis->__nvoc_pbase_Object = &pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object;    // (obj) super^2
    pThis->__nvoc_pbase_OBJENGSTATE = &pThis->__nvoc_base_OBJENGSTATE;    // (engstate) super
    pThis->__nvoc_pbase_MemoryManager = pThis;    // (memmgr) this

    // Recurse to superclass initialization function(s).
    __nvoc_init__OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);

    // Pointer(s) to metadata structures(s)
    pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object.__nvoc_metadata_ptr = &__nvoc_metadata__MemoryManager.metadata__OBJENGSTATE.metadata__Object;    // (obj) super^2
    pThis->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr = &__nvoc_metadata__MemoryManager.metadata__OBJENGSTATE;    // (engstate) super
    pThis->__nvoc_metadata_ptr = &__nvoc_metadata__MemoryManager;    // (memmgr) this

    // Initialize per-object vtables.
    __nvoc_init_funcTable_MemoryManager(pThis, pGpuhalspecowner, pRmhalspecowner);
}

NV_STATUS __nvoc_objCreate_MemoryManager(MemoryManager **__nvoc_ppThis, Dynamic *__nvoc_pParent, NvU32 __nvoc_createFlags)
{
    NV_STATUS __nvoc_status;
    Object *__nvoc_pParentObj = NULL;
    MemoryManager *__nvoc_pThis;
    GpuHalspecOwner *pGpuhalspecowner;
    RmHalspecOwner *pRmhalspecowner;

    // Don't allocate memory if the caller has already done so.
    if (__nvoc_createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
    {
        NV_CHECK_OR_RETURN(LEVEL_ERROR, __nvoc_ppThis != NULL && *__nvoc_ppThis != NULL, NV_ERR_INVALID_PARAMETER);
        __nvoc_pThis = *__nvoc_ppThis;
    }

    // Allocate memory
    else
    {
        __nvoc_pThis = portMemAllocNonPaged(sizeof(MemoryManager));
        NV_CHECK_OR_RETURN(LEVEL_ERROR, __nvoc_pThis != NULL, NV_ERR_NO_MEMORY);
    }

    // Zero is the initial value for everything.
    portMemSet(__nvoc_pThis, 0, sizeof(MemoryManager));

    __nvoc_pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object.createFlags = __nvoc_createFlags;

    // pParent must be a valid object that derives from a halspec owner class.
    NV_CHECK_TRUE_OR_GOTO(__nvoc_status, LEVEL_ERROR, __nvoc_pParent != NULL, NV_ERR_INVALID_ARGUMENT, __nvoc_objCreate_MemoryManager_cleanup);

    // Link the child into the parent unless flagged not to do so.
    if (!(__nvoc_createFlags & NVOC_OBJ_CREATE_FLAGS_PARENT_HALSPEC_ONLY))
    {
        __nvoc_pParentObj = dynamicCast(__nvoc_pParent, Object);
        objAddChild(__nvoc_pParentObj, &__nvoc_pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object);
    }
    else
    {
        __nvoc_pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object.pParent = NULL;
    }

    // HALs are defined by the parent or the first super class.
    if ((pGpuhalspecowner = dynamicCast(__nvoc_pParent, GpuHalspecOwner)) == NULL)
        pGpuhalspecowner = objFindAncestorOfType(GpuHalspecOwner, __nvoc_pParent);
    NV_CHECK_TRUE_OR_GOTO(__nvoc_status, LEVEL_ERROR, pGpuhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT, __nvoc_objCreate_MemoryManager_cleanup);
    if ((pRmhalspecowner = dynamicCast(__nvoc_pParent, RmHalspecOwner)) == NULL)
        pRmhalspecowner = objFindAncestorOfType(RmHalspecOwner, __nvoc_pParent);
    NV_CHECK_TRUE_OR_GOTO(__nvoc_status, LEVEL_ERROR, pRmhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT, __nvoc_objCreate_MemoryManager_cleanup);

    // Initialize vtable, RTTI, etc., then call constructor.
    __nvoc_init__MemoryManager(__nvoc_pThis, pGpuhalspecowner, pRmhalspecowner);
    __nvoc_status = __nvoc_ctor_MemoryManager(__nvoc_pThis, pGpuhalspecowner, pRmhalspecowner);
    if (__nvoc_status != NV_OK) goto __nvoc_objCreate_MemoryManager_cleanup;

    // Assignment has no effect if NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT is set.
    *__nvoc_ppThis = __nvoc_pThis;

    // Success
    return NV_OK;

    // Do not call destructors here since the constructor already called them.
__nvoc_objCreate_MemoryManager_cleanup:

    // Unlink the child from the parent if it was linked above.
    if (__nvoc_pParentObj != NULL)
        objRemoveChild(__nvoc_pParentObj, &__nvoc_pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object);

    // Zero out memory that was allocated by caller.
    if (__nvoc_createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(__nvoc_pThis, 0, sizeof(MemoryManager));

    // Free memory allocated by `__nvoc_handleObjCreateMemAlloc`.
    else
    {
        portMemFree(__nvoc_pThis);
        *__nvoc_ppThis = NULL;
    }

    // Failure
    return __nvoc_status;
}

NV_STATUS __nvoc_objCreateDynamic_MemoryManager(MemoryManager **__nvoc_ppThis, Dynamic *__nvoc_pParent, NvU32 __nvoc_createFlags, va_list __nvoc_args) {
    NV_STATUS __nvoc_status;

    __nvoc_status = __nvoc_objCreate_MemoryManager(__nvoc_ppThis, __nvoc_pParent, __nvoc_createFlags);

    return __nvoc_status;
}

