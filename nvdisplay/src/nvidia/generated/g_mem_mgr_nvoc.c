#define NVOC_MEM_MGR_H_PRIVATE_ACCESS_ALLOWED
#include "nvoc/runtime.h"
#include "nvoc/rtti.h"
#include "nvtypes.h"
#include "nvport/nvport.h"
#include "nvport/inline/util_valist.h"
#include "utils/nvassert.h"
#include "g_mem_mgr_nvoc.h"

#ifdef DEBUG
char __nvoc_class_id_uniqueness_check_0x22ad47 = 1;
#endif

extern const struct NVOC_CLASS_DEF __nvoc_class_def_MemoryManager;

extern const struct NVOC_CLASS_DEF __nvoc_class_def_Object;

extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJENGSTATE;

void __nvoc_init_MemoryManager(MemoryManager*, RmHalspecOwner* );
void __nvoc_init_funcTable_MemoryManager(MemoryManager*, RmHalspecOwner* );
NV_STATUS __nvoc_ctor_MemoryManager(MemoryManager*, RmHalspecOwner* );
void __nvoc_init_dataField_MemoryManager(MemoryManager*, RmHalspecOwner* );
void __nvoc_dtor_MemoryManager(MemoryManager*);
extern const struct NVOC_EXPORT_INFO __nvoc_export_info_MemoryManager;

static const struct NVOC_RTTI __nvoc_rtti_MemoryManager_MemoryManager = {
    /*pClassDef=*/          &__nvoc_class_def_MemoryManager,
    /*dtor=*/               (NVOC_DYNAMIC_DTOR) &__nvoc_dtor_MemoryManager,
    /*offset=*/             0,
};

static const struct NVOC_RTTI __nvoc_rtti_MemoryManager_Object = {
    /*pClassDef=*/          &__nvoc_class_def_Object,
    /*dtor=*/               &__nvoc_destructFromBase,
    /*offset=*/             NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE.__nvoc_base_Object),
};

static const struct NVOC_RTTI __nvoc_rtti_MemoryManager_OBJENGSTATE = {
    /*pClassDef=*/          &__nvoc_class_def_OBJENGSTATE,
    /*dtor=*/               &__nvoc_destructFromBase,
    /*offset=*/             NV_OFFSETOF(MemoryManager, __nvoc_base_OBJENGSTATE),
};

static const struct NVOC_CASTINFO __nvoc_castinfo_MemoryManager = {
    /*numRelatives=*/       3,
    /*relatives=*/ {
        &__nvoc_rtti_MemoryManager_MemoryManager,
        &__nvoc_rtti_MemoryManager_OBJENGSTATE,
        &__nvoc_rtti_MemoryManager_Object,
    },
};

const struct NVOC_CLASS_DEF __nvoc_class_def_MemoryManager = 
{
    /*classInfo=*/ {
        /*size=*/               sizeof(MemoryManager),
        /*classId=*/            classId(MemoryManager),
        /*providerId=*/         &__nvoc_rtti_provider,
#if NV_PRINTF_STRINGS_ALLOWED
        /*name=*/               "MemoryManager",
#endif
    },
    /*objCreatefn=*/        (NVOC_DYNAMIC_OBJ_CREATE) &__nvoc_objCreateDynamic_MemoryManager,
    /*pCastInfo=*/          &__nvoc_castinfo_MemoryManager,
    /*pExportInfo=*/        &__nvoc_export_info_MemoryManager
};

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStateLoad(POBJGPU pGpu, struct MemoryManager *pEngstate, NvU32 arg0) {
    return engstateStateLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset), arg0);
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStateUnload(POBJGPU pGpu, struct MemoryManager *pEngstate, NvU32 arg0) {
    return engstateStateUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset), arg0);
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStateInitLocked(POBJGPU pGpu, struct MemoryManager *pEngstate) {
    return engstateStateInitLocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset));
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStatePreLoad(POBJGPU pGpu, struct MemoryManager *pEngstate, NvU32 arg0) {
    return engstateStatePreLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset), arg0);
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStatePostUnload(POBJGPU pGpu, struct MemoryManager *pEngstate, NvU32 arg0) {
    return engstateStatePostUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset), arg0);
}

static void __nvoc_thunk_OBJENGSTATE_memmgrStateDestroy(POBJGPU pGpu, struct MemoryManager *pEngstate) {
    engstateStateDestroy(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset));
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStatePreUnload(POBJGPU pGpu, struct MemoryManager *pEngstate, NvU32 arg0) {
    return engstateStatePreUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset), arg0);
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStateInitUnlocked(POBJGPU pGpu, struct MemoryManager *pEngstate) {
    return engstateStateInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset));
}

static void __nvoc_thunk_OBJENGSTATE_memmgrInitMissing(POBJGPU pGpu, struct MemoryManager *pEngstate) {
    engstateInitMissing(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset));
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStatePreInitLocked(POBJGPU pGpu, struct MemoryManager *pEngstate) {
    return engstateStatePreInitLocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset));
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStatePreInitUnlocked(POBJGPU pGpu, struct MemoryManager *pEngstate) {
    return engstateStatePreInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset));
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrStatePostLoad(POBJGPU pGpu, struct MemoryManager *pEngstate, NvU32 arg0) {
    return engstateStatePostLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset), arg0);
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_memmgrConstructEngine(POBJGPU pGpu, struct MemoryManager *pEngstate, ENGDESCRIPTOR arg0) {
    return engstateConstructEngine(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset), arg0);
}

static NvBool __nvoc_thunk_OBJENGSTATE_memmgrIsPresent(POBJGPU pGpu, struct MemoryManager *pEngstate) {
    return engstateIsPresent(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_MemoryManager_OBJENGSTATE.offset));
}

const struct NVOC_EXPORT_INFO __nvoc_export_info_MemoryManager = 
{
    /*numEntries=*/     0,
    /*pExportEntries=*/  0
};

void __nvoc_dtor_OBJENGSTATE(OBJENGSTATE*);
void __nvoc_dtor_MemoryManager(MemoryManager *pThis) {
    __nvoc_dtor_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    PORT_UNREFERENCED_VARIABLE(pThis);
}

void __nvoc_init_dataField_MemoryManager(MemoryManager *pThis, RmHalspecOwner *pRmhalspecowner) {
    ChipHal *chipHal = &pRmhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);

    // Hal field -- bFbRegionsSupported
    // default
    {
        pThis->bFbRegionsSupported = ((NvBool)(0 != 0));
    }

    // Hal field -- bPmaEnabled
    // default
    {
        pThis->bPmaEnabled = ((NvBool)(0 != 0));
    }

    // Hal field -- bClientPageTablesPmaManaged
    if (( ((chipHal_HalVarIdx >> 5) == 2UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x04000000UL) )) /* ChipHal: T234D */ 
    {
        pThis->bClientPageTablesPmaManaged = ((NvBool)(0 == 0));
    }

    // Hal field -- bScanoutSysmem
    if (( ((chipHal_HalVarIdx >> 5) == 2UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x04000000UL) )) /* ChipHal: T234D */ 
    {
        pThis->bScanoutSysmem = ((NvBool)(0 == 0));
    }

    // Hal field -- bDisallowSplitLowerMemory
    if (( ((chipHal_HalVarIdx >> 5) == 2UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x04000000UL) )) /* ChipHal: T234D */ 
    {
        pThis->bDisallowSplitLowerMemory = ((NvBool)(0 == 0));
    }

    // Hal field -- bSmallPageCompression
    // default
    {
        pThis->bSmallPageCompression = ((NvBool)(0 != 0));
    }

    // Hal field -- bSysmemCompressionSupportDef
    if (( ((chipHal_HalVarIdx >> 5) == 2UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x04000000UL) )) /* ChipHal: T234D */ 
    {
        pThis->bSysmemCompressionSupportDef = ((NvBool)(0 == 0));
    }

    // Hal field -- bBug2301372IncreaseRmReserveMemoryWar
    // default
    {
        pThis->bBug2301372IncreaseRmReserveMemoryWar = ((NvBool)(0 != 0));
    }

    pThis->bEnableDynamicPageOfflining = ((NvBool)(0 != 0));

    // Hal field -- bVgpuPmaSupport
    // default
    {
        pThis->bVgpuPmaSupport = ((NvBool)(0 != 0));
    }

    pThis->bEnableDynamicGranularityPageArrays = ((NvBool)(0 != 0));

    // Hal field -- bAllowNoncontiguousAllocation
    if (( ((chipHal_HalVarIdx >> 5) == 2UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x04000000UL) )) /* ChipHal: T234D */ 
    {
        pThis->bAllowNoncontiguousAllocation = ((NvBool)(0 == 0));
    }

    // Hal field -- bScrubOnFreeEnabled
    // default
    {
        pThis->bScrubOnFreeEnabled = ((NvBool)(0 != 0));
    }

    // Hal field -- bFastScrubberEnabled
    // default
    {
        pThis->bFastScrubberEnabled = ((NvBool)(0 != 0));
    }

    // Hal field -- bMonitoredFenceSupported
    if (( ((chipHal_HalVarIdx >> 5) == 2UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x04000000UL) )) /* ChipHal: T234D */ 
    {
        pThis->bMonitoredFenceSupported = ((NvBool)(0 != 0));
    }

    // Hal field -- b64BitSemaphoresSupported
    if (( ((chipHal_HalVarIdx >> 5) == 2UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x04000000UL) )) /* ChipHal: T234D */ 
    {
        pThis->b64BitSemaphoresSupported = ((NvBool)(0 != 0));
    }

    // Hal field -- bGenericKindSupport
    // default
    {
        pThis->bGenericKindSupport = ((NvBool)(0 != 0));
    }
}

NV_STATUS __nvoc_ctor_OBJENGSTATE(OBJENGSTATE* );
NV_STATUS __nvoc_ctor_MemoryManager(MemoryManager *pThis, RmHalspecOwner *pRmhalspecowner) {
    NV_STATUS status = NV_OK;
    status = __nvoc_ctor_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    if (status != NV_OK) goto __nvoc_ctor_MemoryManager_fail_OBJENGSTATE;
    __nvoc_init_dataField_MemoryManager(pThis, pRmhalspecowner);
    goto __nvoc_ctor_MemoryManager_exit; // Success

__nvoc_ctor_MemoryManager_fail_OBJENGSTATE:
__nvoc_ctor_MemoryManager_exit:

    return status;
}

static void __nvoc_init_funcTable_MemoryManager_1(MemoryManager *pThis, RmHalspecOwner *pRmhalspecowner) {
    ChipHal *chipHal = &pRmhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);

    pThis->__memmgrStateLoad__ = &__nvoc_thunk_OBJENGSTATE_memmgrStateLoad;

    pThis->__memmgrStateUnload__ = &__nvoc_thunk_OBJENGSTATE_memmgrStateUnload;

    pThis->__memmgrStateInitLocked__ = &__nvoc_thunk_OBJENGSTATE_memmgrStateInitLocked;

    pThis->__memmgrStatePreLoad__ = &__nvoc_thunk_OBJENGSTATE_memmgrStatePreLoad;

    pThis->__memmgrStatePostUnload__ = &__nvoc_thunk_OBJENGSTATE_memmgrStatePostUnload;

    pThis->__memmgrStateDestroy__ = &__nvoc_thunk_OBJENGSTATE_memmgrStateDestroy;

    pThis->__memmgrStatePreUnload__ = &__nvoc_thunk_OBJENGSTATE_memmgrStatePreUnload;

    pThis->__memmgrStateInitUnlocked__ = &__nvoc_thunk_OBJENGSTATE_memmgrStateInitUnlocked;

    pThis->__memmgrInitMissing__ = &__nvoc_thunk_OBJENGSTATE_memmgrInitMissing;

    pThis->__memmgrStatePreInitLocked__ = &__nvoc_thunk_OBJENGSTATE_memmgrStatePreInitLocked;

    pThis->__memmgrStatePreInitUnlocked__ = &__nvoc_thunk_OBJENGSTATE_memmgrStatePreInitUnlocked;

    pThis->__memmgrStatePostLoad__ = &__nvoc_thunk_OBJENGSTATE_memmgrStatePostLoad;

    pThis->__memmgrConstructEngine__ = &__nvoc_thunk_OBJENGSTATE_memmgrConstructEngine;

    pThis->__memmgrIsPresent__ = &__nvoc_thunk_OBJENGSTATE_memmgrIsPresent;
}

void __nvoc_init_funcTable_MemoryManager(MemoryManager *pThis, RmHalspecOwner *pRmhalspecowner) {
    __nvoc_init_funcTable_MemoryManager_1(pThis, pRmhalspecowner);
}

void __nvoc_init_OBJENGSTATE(OBJENGSTATE*);
void __nvoc_init_MemoryManager(MemoryManager *pThis, RmHalspecOwner *pRmhalspecowner) {
    pThis->__nvoc_pbase_MemoryManager = pThis;
    pThis->__nvoc_pbase_Object = &pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object;
    pThis->__nvoc_pbase_OBJENGSTATE = &pThis->__nvoc_base_OBJENGSTATE;
    __nvoc_init_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    __nvoc_init_funcTable_MemoryManager(pThis, pRmhalspecowner);
}

NV_STATUS __nvoc_objCreate_MemoryManager(MemoryManager **ppThis, Dynamic *pParent, NvU32 createFlags) {
    NV_STATUS status;
    Object *pParentObj;
    MemoryManager *pThis;
    RmHalspecOwner *pRmhalspecowner;

    status = __nvoc_handleObjCreateMemAlloc(createFlags, sizeof(MemoryManager), (void**)&pThis, (void**)ppThis);
    if (status != NV_OK)
        return status;

    portMemSet(pThis, 0, sizeof(MemoryManager));

    __nvoc_initRtti(staticCast(pThis, Dynamic), &__nvoc_class_def_MemoryManager);

    pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object.createFlags = createFlags;

    if (pParent != NULL && !(createFlags & NVOC_OBJ_CREATE_FLAGS_PARENT_HALSPEC_ONLY))
    {
        pParentObj = dynamicCast(pParent, Object);
        objAddChild(pParentObj, &pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object);
    }
    else
    {
        pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object.pParent = NULL;
    }

    if ((pRmhalspecowner = dynamicCast(pParent, RmHalspecOwner)) == NULL)
        pRmhalspecowner = objFindAncestorOfType(RmHalspecOwner, pParent);
    NV_ASSERT_OR_RETURN(pRmhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT);

    __nvoc_init_MemoryManager(pThis, pRmhalspecowner);
    status = __nvoc_ctor_MemoryManager(pThis, pRmhalspecowner);
    if (status != NV_OK) goto __nvoc_objCreate_MemoryManager_cleanup;

    *ppThis = pThis;

    return NV_OK;

__nvoc_objCreate_MemoryManager_cleanup:
    // do not call destructors here since the constructor already called them
    if (createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(pThis, 0, sizeof(MemoryManager));
    else
        portMemFree(pThis);

    // coverity[leaked_storage:FALSE]
    return status;
}

NV_STATUS __nvoc_objCreateDynamic_MemoryManager(MemoryManager **ppThis, Dynamic *pParent, NvU32 createFlags, va_list args) {
    NV_STATUS status;

    status = __nvoc_objCreate_MemoryManager(ppThis, pParent, createFlags);

    return status;
}

