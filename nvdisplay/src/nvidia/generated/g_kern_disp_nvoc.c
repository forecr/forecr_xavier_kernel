#define NVOC_KERN_DISP_H_PRIVATE_ACCESS_ALLOWED
#include "nvoc/runtime.h"
#include "nvoc/rtti.h"
#include "nvtypes.h"
#include "nvport/nvport.h"
#include "nvport/inline/util_valist.h"
#include "utils/nvassert.h"
#include "g_kern_disp_nvoc.h"

#ifdef DEBUG
char __nvoc_class_id_uniqueness_check_0x55952e = 1;
#endif

extern const struct NVOC_CLASS_DEF __nvoc_class_def_KernelDisplay;

extern const struct NVOC_CLASS_DEF __nvoc_class_def_Object;

extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJENGSTATE;

void __nvoc_init_KernelDisplay(KernelDisplay*, RmHalspecOwner* );
void __nvoc_init_funcTable_KernelDisplay(KernelDisplay*, RmHalspecOwner* );
NV_STATUS __nvoc_ctor_KernelDisplay(KernelDisplay*, RmHalspecOwner* );
void __nvoc_init_dataField_KernelDisplay(KernelDisplay*, RmHalspecOwner* );
void __nvoc_dtor_KernelDisplay(KernelDisplay*);
extern const struct NVOC_EXPORT_INFO __nvoc_export_info_KernelDisplay;

static const struct NVOC_RTTI __nvoc_rtti_KernelDisplay_KernelDisplay = {
    /*pClassDef=*/          &__nvoc_class_def_KernelDisplay,
    /*dtor=*/               (NVOC_DYNAMIC_DTOR) &__nvoc_dtor_KernelDisplay,
    /*offset=*/             0,
};

static const struct NVOC_RTTI __nvoc_rtti_KernelDisplay_Object = {
    /*pClassDef=*/          &__nvoc_class_def_Object,
    /*dtor=*/               &__nvoc_destructFromBase,
    /*offset=*/             NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE.__nvoc_base_Object),
};

static const struct NVOC_RTTI __nvoc_rtti_KernelDisplay_OBJENGSTATE = {
    /*pClassDef=*/          &__nvoc_class_def_OBJENGSTATE,
    /*dtor=*/               &__nvoc_destructFromBase,
    /*offset=*/             NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE),
};

static const struct NVOC_CASTINFO __nvoc_castinfo_KernelDisplay = {
    /*numRelatives=*/       3,
    /*relatives=*/ {
        &__nvoc_rtti_KernelDisplay_KernelDisplay,
        &__nvoc_rtti_KernelDisplay_OBJENGSTATE,
        &__nvoc_rtti_KernelDisplay_Object,
    },
};

const struct NVOC_CLASS_DEF __nvoc_class_def_KernelDisplay = 
{
    /*classInfo=*/ {
        /*size=*/               sizeof(KernelDisplay),
        /*classId=*/            classId(KernelDisplay),
        /*providerId=*/         &__nvoc_rtti_provider,
#if NV_PRINTF_STRINGS_ALLOWED
        /*name=*/               "KernelDisplay",
#endif
    },
    /*objCreatefn=*/        (NVOC_DYNAMIC_OBJ_CREATE) &__nvoc_objCreateDynamic_KernelDisplay,
    /*pCastInfo=*/          &__nvoc_castinfo_KernelDisplay,
    /*pExportInfo=*/        &__nvoc_export_info_KernelDisplay
};

static NV_STATUS __nvoc_thunk_KernelDisplay_engstateConstructEngine(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay, ENGDESCRIPTOR engDesc) {
    return kdispConstructEngine(pGpu, (struct KernelDisplay *)(((unsigned char *)pKernelDisplay) - __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset), engDesc);
}

static NV_STATUS __nvoc_thunk_KernelDisplay_engstateStatePreInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay) {
    return kdispStatePreInitLocked(pGpu, (struct KernelDisplay *)(((unsigned char *)pKernelDisplay) - __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset));
}

static NV_STATUS __nvoc_thunk_KernelDisplay_engstateStateInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay) {
    return kdispStateInitLocked(pGpu, (struct KernelDisplay *)(((unsigned char *)pKernelDisplay) - __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset));
}

static void __nvoc_thunk_KernelDisplay_engstateStateDestroy(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay) {
    kdispStateDestroy(pGpu, (struct KernelDisplay *)(((unsigned char *)pKernelDisplay) - __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset));
}

static NV_STATUS __nvoc_thunk_KernelDisplay_engstateStateLoad(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay, NvU32 flags) {
    return kdispStateLoad(pGpu, (struct KernelDisplay *)(((unsigned char *)pKernelDisplay) - __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset), flags);
}

static NV_STATUS __nvoc_thunk_KernelDisplay_engstateStateUnload(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay, NvU32 flags) {
    return kdispStateUnload(pGpu, (struct KernelDisplay *)(((unsigned char *)pKernelDisplay) - __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset), flags);
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_kdispStatePreLoad(POBJGPU pGpu, struct KernelDisplay *pEngstate, NvU32 arg0) {
    return engstateStatePreLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset), arg0);
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_kdispStatePostUnload(POBJGPU pGpu, struct KernelDisplay *pEngstate, NvU32 arg0) {
    return engstateStatePostUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset), arg0);
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_kdispStatePreUnload(POBJGPU pGpu, struct KernelDisplay *pEngstate, NvU32 arg0) {
    return engstateStatePreUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset), arg0);
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_kdispStateInitUnlocked(POBJGPU pGpu, struct KernelDisplay *pEngstate) {
    return engstateStateInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset));
}

static void __nvoc_thunk_OBJENGSTATE_kdispInitMissing(POBJGPU pGpu, struct KernelDisplay *pEngstate) {
    engstateInitMissing(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset));
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_kdispStatePreInitUnlocked(POBJGPU pGpu, struct KernelDisplay *pEngstate) {
    return engstateStatePreInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset));
}

static NV_STATUS __nvoc_thunk_OBJENGSTATE_kdispStatePostLoad(POBJGPU pGpu, struct KernelDisplay *pEngstate, NvU32 arg0) {
    return engstateStatePostLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset), arg0);
}

static NvBool __nvoc_thunk_OBJENGSTATE_kdispIsPresent(POBJGPU pGpu, struct KernelDisplay *pEngstate) {
    return engstateIsPresent(pGpu, (struct OBJENGSTATE *)(((unsigned char *)pEngstate) + __nvoc_rtti_KernelDisplay_OBJENGSTATE.offset));
}

const struct NVOC_EXPORT_INFO __nvoc_export_info_KernelDisplay = 
{
    /*numEntries=*/     0,
    /*pExportEntries=*/  0
};

void __nvoc_dtor_OBJENGSTATE(OBJENGSTATE*);
void __nvoc_dtor_KernelDisplay(KernelDisplay *pThis) {
    __nvoc_kdispDestruct(pThis);
    __nvoc_dtor_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    PORT_UNREFERENCED_VARIABLE(pThis);
}

void __nvoc_init_dataField_KernelDisplay(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner) {
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    ChipHal *chipHal = &pRmhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    DispIpHal *dispIpHal = &pRmhalspecowner->dispIpHal;
    const unsigned long dispIpHal_HalVarIdx = (unsigned long)dispIpHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(dispIpHal);
    PORT_UNREFERENCED_VARIABLE(dispIpHal_HalVarIdx);

    // NVOC Property Hal field -- PDB_PROP_KDISP_IS_MISSING
    if (( ((rmVariantHal_HalVarIdx >> 5) == 0UL) && ((1UL << (rmVariantHal_HalVarIdx & 0x1f)) & 0x00000002UL) )) /* RmVariantHal: PF_KERNEL_ONLY */ 
    {
        pThis->setProperty(pThis, PDB_PROP_KDISP_IS_MISSING, ((NvBool)(0 != 0)));
    }

    // NVOC Property Hal field -- PDB_PROP_KDISP_IMP_ENABLE
    if (((( ((chipHal_HalVarIdx >> 5) == 2UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x04000000UL) )) /* ChipHal: T234D */  && (( ((rmVariantHal_HalVarIdx >> 5) == 0UL) && ((1UL << (rmVariantHal_HalVarIdx & 0x1f)) & 0x00000002UL) )) /* RmVariantHal: PF_KERNEL_ONLY */ ))
    {
        pThis->setProperty(pThis, PDB_PROP_KDISP_IMP_ENABLE, ((NvBool)(0 == 0)));
    }

    // NVOC Property Hal field -- PDB_PROP_KDISP_BUG_2089053_SERIALIZE_AGGRESSIVE_VBLANK_ALWAYS
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_KDISP_BUG_2089053_SERIALIZE_AGGRESSIVE_VBLANK_ALWAYS, ((NvBool)(0 != 0)));
    }

    pThis->pStaticInfo = ((void *)0);

    pThis->bWarPurgeSatellitesOnCoreFree = ((NvBool)(0 != 0));

    pThis->bExtdevIntrSupported = ((NvBool)(0 != 0));
}

NV_STATUS __nvoc_ctor_OBJENGSTATE(OBJENGSTATE* );
NV_STATUS __nvoc_ctor_KernelDisplay(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner) {
    NV_STATUS status = NV_OK;
    status = __nvoc_ctor_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    if (status != NV_OK) goto __nvoc_ctor_KernelDisplay_fail_OBJENGSTATE;
    __nvoc_init_dataField_KernelDisplay(pThis, pRmhalspecowner);
    goto __nvoc_ctor_KernelDisplay_exit; // Success

__nvoc_ctor_KernelDisplay_fail_OBJENGSTATE:
__nvoc_ctor_KernelDisplay_exit:

    return status;
}

static void __nvoc_init_funcTable_KernelDisplay_1(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner) {
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    ChipHal *chipHal = &pRmhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    DispIpHal *dispIpHal = &pRmhalspecowner->dispIpHal;
    const unsigned long dispIpHal_HalVarIdx = (unsigned long)dispIpHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(dispIpHal);
    PORT_UNREFERENCED_VARIABLE(dispIpHal_HalVarIdx);

    pThis->__kdispConstructEngine__ = &kdispConstructEngine_IMPL;

    pThis->__kdispStatePreInitLocked__ = &kdispStatePreInitLocked_IMPL;

    pThis->__kdispStateInitLocked__ = &kdispStateInitLocked_IMPL;

    pThis->__kdispStateDestroy__ = &kdispStateDestroy_IMPL;

    pThis->__kdispStateLoad__ = &kdispStateLoad_IMPL;

    pThis->__kdispStateUnload__ = &kdispStateUnload_IMPL;

    pThis->__nvoc_base_OBJENGSTATE.__engstateConstructEngine__ = &__nvoc_thunk_KernelDisplay_engstateConstructEngine;

    pThis->__nvoc_base_OBJENGSTATE.__engstateStatePreInitLocked__ = &__nvoc_thunk_KernelDisplay_engstateStatePreInitLocked;

    pThis->__nvoc_base_OBJENGSTATE.__engstateStateInitLocked__ = &__nvoc_thunk_KernelDisplay_engstateStateInitLocked;

    pThis->__nvoc_base_OBJENGSTATE.__engstateStateDestroy__ = &__nvoc_thunk_KernelDisplay_engstateStateDestroy;

    pThis->__nvoc_base_OBJENGSTATE.__engstateStateLoad__ = &__nvoc_thunk_KernelDisplay_engstateStateLoad;

    pThis->__nvoc_base_OBJENGSTATE.__engstateStateUnload__ = &__nvoc_thunk_KernelDisplay_engstateStateUnload;

    pThis->__kdispStatePreLoad__ = &__nvoc_thunk_OBJENGSTATE_kdispStatePreLoad;

    pThis->__kdispStatePostUnload__ = &__nvoc_thunk_OBJENGSTATE_kdispStatePostUnload;

    pThis->__kdispStatePreUnload__ = &__nvoc_thunk_OBJENGSTATE_kdispStatePreUnload;

    pThis->__kdispStateInitUnlocked__ = &__nvoc_thunk_OBJENGSTATE_kdispStateInitUnlocked;

    pThis->__kdispInitMissing__ = &__nvoc_thunk_OBJENGSTATE_kdispInitMissing;

    pThis->__kdispStatePreInitUnlocked__ = &__nvoc_thunk_OBJENGSTATE_kdispStatePreInitUnlocked;

    pThis->__kdispStatePostLoad__ = &__nvoc_thunk_OBJENGSTATE_kdispStatePostLoad;

    pThis->__kdispIsPresent__ = &__nvoc_thunk_OBJENGSTATE_kdispIsPresent;
}

void __nvoc_init_funcTable_KernelDisplay(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner) {
    __nvoc_init_funcTable_KernelDisplay_1(pThis, pRmhalspecowner);
}

void __nvoc_init_OBJENGSTATE(OBJENGSTATE*);
void __nvoc_init_KernelDisplay(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner) {
    pThis->__nvoc_pbase_KernelDisplay = pThis;
    pThis->__nvoc_pbase_Object = &pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object;
    pThis->__nvoc_pbase_OBJENGSTATE = &pThis->__nvoc_base_OBJENGSTATE;
    __nvoc_init_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    __nvoc_init_funcTable_KernelDisplay(pThis, pRmhalspecowner);
}

NV_STATUS __nvoc_objCreate_KernelDisplay(KernelDisplay **ppThis, Dynamic *pParent, NvU32 createFlags) {
    NV_STATUS status;
    Object *pParentObj;
    KernelDisplay *pThis;
    RmHalspecOwner *pRmhalspecowner;

    status = __nvoc_handleObjCreateMemAlloc(createFlags, sizeof(KernelDisplay), (void**)&pThis, (void**)ppThis);
    if (status != NV_OK)
        return status;

    portMemSet(pThis, 0, sizeof(KernelDisplay));

    __nvoc_initRtti(staticCast(pThis, Dynamic), &__nvoc_class_def_KernelDisplay);

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

    __nvoc_init_KernelDisplay(pThis, pRmhalspecowner);
    status = __nvoc_ctor_KernelDisplay(pThis, pRmhalspecowner);
    if (status != NV_OK) goto __nvoc_objCreate_KernelDisplay_cleanup;

    *ppThis = pThis;

    return NV_OK;

__nvoc_objCreate_KernelDisplay_cleanup:
    // do not call destructors here since the constructor already called them
    if (createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(pThis, 0, sizeof(KernelDisplay));
    else
        portMemFree(pThis);

    // coverity[leaked_storage:FALSE]
    return status;
}

NV_STATUS __nvoc_objCreateDynamic_KernelDisplay(KernelDisplay **ppThis, Dynamic *pParent, NvU32 createFlags, va_list args) {
    NV_STATUS status;

    status = __nvoc_objCreate_KernelDisplay(ppThis, pParent, createFlags);

    return status;
}

