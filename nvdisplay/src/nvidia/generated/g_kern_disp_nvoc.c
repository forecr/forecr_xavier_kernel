#define NVOC_KERN_DISP_H_PRIVATE_ACCESS_ALLOWED

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
#include "g_kern_disp_nvoc.h"


#ifdef DEBUG
char __nvoc_class_id_uniqueness_check__0x55952e = 1;
#endif

extern const struct NVOC_CLASS_DEF __nvoc_class_def_KernelDisplay;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_Object;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJENGSTATE;

// Forward declarations for KernelDisplay
void __nvoc_init__OBJENGSTATE(OBJENGSTATE*);
void __nvoc_init__KernelDisplay(KernelDisplay*, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner);
void __nvoc_init_funcTable_KernelDisplay(KernelDisplay*, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner);
NV_STATUS __nvoc_ctor_KernelDisplay(KernelDisplay*, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner);
void __nvoc_init_dataField_KernelDisplay(KernelDisplay*, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner);
void __nvoc_dtor_KernelDisplay(KernelDisplay*);

// Structures used within RTTI (run-time type information)
extern const struct NVOC_CASTINFO __nvoc_castinfo__KernelDisplay;
extern const struct NVOC_EXPORT_INFO __nvoc_export_info__KernelDisplay;

// Down-thunk(s) to bridge KernelDisplay methods from ancestors (if any)
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateConstructEngine(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay, ENGDESCRIPTOR engDesc);    // this
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateStatePreInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay);    // this
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateStateInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay);    // this
void __nvoc_down_thunk_KernelDisplay_engstateStateDestroy(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay);    // this
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateStateLoad(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay, NvU32 flags);    // this
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateStateUnload(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay, NvU32 flags);    // this

// Up-thunk(s) to bridge KernelDisplay methods to ancestors (if any)
void __nvoc_up_thunk_OBJENGSTATE_kdispInitMissing(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePreInitUnlocked(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStateInitUnlocked(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePreLoad(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePostLoad(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePreUnload(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePostUnload(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3);    // this
NvBool __nvoc_up_thunk_OBJENGSTATE_kdispIsPresent(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate);    // this

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
    /*pCastInfo=*/          &__nvoc_castinfo__KernelDisplay,
    /*pExportInfo=*/        &__nvoc_export_info__KernelDisplay
};


// Metadata with per-class RTTI and vtable with ancestor(s)
static const struct NVOC_METADATA__KernelDisplay __nvoc_metadata__KernelDisplay = {
    .rtti.pClassDef = &__nvoc_class_def_KernelDisplay,    // (kdisp) this
    .rtti.dtor      = (NVOC_DYNAMIC_DTOR) &__nvoc_dtor_KernelDisplay,
    .rtti.offset    = 0,
    .metadata__OBJENGSTATE.rtti.pClassDef = &__nvoc_class_def_OBJENGSTATE,    // (engstate) super
    .metadata__OBJENGSTATE.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__OBJENGSTATE.rtti.offset    = NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE),
    .metadata__OBJENGSTATE.metadata__Object.rtti.pClassDef = &__nvoc_class_def_Object,    // (obj) super^2
    .metadata__OBJENGSTATE.metadata__Object.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__OBJENGSTATE.metadata__Object.rtti.offset    = NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE.__nvoc_base_Object),

    .vtable.__kdispConstructEngine__ = &kdispConstructEngine_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateConstructEngine__ = &__nvoc_down_thunk_KernelDisplay_engstateConstructEngine,    // virtual
    .vtable.__kdispStatePreInitLocked__ = &kdispStatePreInitLocked_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreInitLocked__ = &__nvoc_down_thunk_KernelDisplay_engstateStatePreInitLocked,    // virtual
    .vtable.__kdispStateInitLocked__ = &kdispStateInitLocked_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateInitLocked__ = &__nvoc_down_thunk_KernelDisplay_engstateStateInitLocked,    // virtual
    .vtable.__kdispStateDestroy__ = &kdispStateDestroy_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateDestroy__ = &__nvoc_down_thunk_KernelDisplay_engstateStateDestroy,    // virtual
    .vtable.__kdispStateLoad__ = &kdispStateLoad_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateLoad__ = &__nvoc_down_thunk_KernelDisplay_engstateStateLoad,    // virtual
    .vtable.__kdispStateUnload__ = &kdispStateUnload_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateUnload__ = &__nvoc_down_thunk_KernelDisplay_engstateStateUnload,    // virtual
    .vtable.__kdispInitMissing__ = &__nvoc_up_thunk_OBJENGSTATE_kdispInitMissing,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateInitMissing__ = &engstateInitMissing_IMPL,    // virtual
    .vtable.__kdispStatePreInitUnlocked__ = &__nvoc_up_thunk_OBJENGSTATE_kdispStatePreInitUnlocked,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreInitUnlocked__ = &engstateStatePreInitUnlocked_IMPL,    // virtual
    .vtable.__kdispStateInitUnlocked__ = &__nvoc_up_thunk_OBJENGSTATE_kdispStateInitUnlocked,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateInitUnlocked__ = &engstateStateInitUnlocked_IMPL,    // virtual
    .vtable.__kdispStatePreLoad__ = &__nvoc_up_thunk_OBJENGSTATE_kdispStatePreLoad,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreLoad__ = &engstateStatePreLoad_IMPL,    // virtual
    .vtable.__kdispStatePostLoad__ = &__nvoc_up_thunk_OBJENGSTATE_kdispStatePostLoad,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePostLoad__ = &engstateStatePostLoad_IMPL,    // virtual
    .vtable.__kdispStatePreUnload__ = &__nvoc_up_thunk_OBJENGSTATE_kdispStatePreUnload,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreUnload__ = &engstateStatePreUnload_IMPL,    // virtual
    .vtable.__kdispStatePostUnload__ = &__nvoc_up_thunk_OBJENGSTATE_kdispStatePostUnload,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePostUnload__ = &engstateStatePostUnload_IMPL,    // virtual
    .vtable.__kdispIsPresent__ = &__nvoc_up_thunk_OBJENGSTATE_kdispIsPresent,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateIsPresent__ = &engstateIsPresent_IMPL,    // virtual
};


// Dynamic down-casting information
const struct NVOC_CASTINFO __nvoc_castinfo__KernelDisplay = {
    .numRelatives = 3,
    .relatives = {
        &__nvoc_metadata__KernelDisplay.rtti,    // [0]: (kdisp) this
        &__nvoc_metadata__KernelDisplay.metadata__OBJENGSTATE.rtti,    // [1]: (engstate) super
        &__nvoc_metadata__KernelDisplay.metadata__OBJENGSTATE.metadata__Object.rtti,    // [2]: (obj) super^2
    }
};

// 6 down-thunk(s) defined to bridge methods in KernelDisplay from superclasses

// kdispConstructEngine: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateConstructEngine(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay, ENGDESCRIPTOR engDesc) {
    return kdispConstructEngine(pGpu, (struct KernelDisplay *)(((unsigned char *) pKernelDisplay) - NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)), engDesc);
}

// kdispStatePreInitLocked: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateStatePreInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay) {
    return kdispStatePreInitLocked(pGpu, (struct KernelDisplay *)(((unsigned char *) pKernelDisplay) - NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)));
}

// kdispStateInitLocked: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateStateInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay) {
    return kdispStateInitLocked(pGpu, (struct KernelDisplay *)(((unsigned char *) pKernelDisplay) - NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)));
}

// kdispStateDestroy: virtual override (engstate) base (engstate)
void __nvoc_down_thunk_KernelDisplay_engstateStateDestroy(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay) {
    kdispStateDestroy(pGpu, (struct KernelDisplay *)(((unsigned char *) pKernelDisplay) - NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)));
}

// kdispStateLoad: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateStateLoad(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay, NvU32 flags) {
    return kdispStateLoad(pGpu, (struct KernelDisplay *)(((unsigned char *) pKernelDisplay) - NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)), flags);
}

// kdispStateUnload: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_KernelDisplay_engstateStateUnload(struct OBJGPU *pGpu, struct OBJENGSTATE *pKernelDisplay, NvU32 flags) {
    return kdispStateUnload(pGpu, (struct KernelDisplay *)(((unsigned char *) pKernelDisplay) - NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)), flags);
}


// 8 up-thunk(s) defined to bridge methods in KernelDisplay to superclasses

// kdispInitMissing: virtual inherited (engstate) base (engstate)
void __nvoc_up_thunk_OBJENGSTATE_kdispInitMissing(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate) {
    engstateInitMissing(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)));
}

// kdispStatePreInitUnlocked: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePreInitUnlocked(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate) {
    return engstateStatePreInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)));
}

// kdispStateInitUnlocked: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStateInitUnlocked(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate) {
    return engstateStateInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)));
}

// kdispStatePreLoad: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePreLoad(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3) {
    return engstateStatePreLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)), arg3);
}

// kdispStatePostLoad: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePostLoad(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3) {
    return engstateStatePostLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)), arg3);
}

// kdispStatePreUnload: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePreUnload(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3) {
    return engstateStatePreUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)), arg3);
}

// kdispStatePostUnload: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_kdispStatePostUnload(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate, NvU32 arg3) {
    return engstateStatePostUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)), arg3);
}

// kdispIsPresent: virtual inherited (engstate) base (engstate)
NvBool __nvoc_up_thunk_OBJENGSTATE_kdispIsPresent(struct OBJGPU *pGpu, struct KernelDisplay *pEngstate) {
    return engstateIsPresent(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(KernelDisplay, __nvoc_base_OBJENGSTATE)));
}


const struct NVOC_EXPORT_INFO __nvoc_export_info__KernelDisplay = 
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

void __nvoc_init_dataField_KernelDisplay(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    ChipHal *chipHal = &pGpuhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    DispIpHal *dispIpHal = &pRmhalspecowner->dispIpHal;
    const unsigned long dispIpHal_HalVarIdx = (unsigned long)dispIpHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(pGpuhalspecowner);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(dispIpHal);
    PORT_UNREFERENCED_VARIABLE(dispIpHal_HalVarIdx);

    // NVOC Property Hal field -- PDB_PROP_KDISP_IS_MISSING
    if (( ((rmVariantHal_HalVarIdx >> 5) == 0UL) && ((1UL << (rmVariantHal_HalVarIdx & 0x1f)) & 0x00000002UL) )) /* RmVariantHal: PF_KERNEL_ONLY */ 
    {
        pThis->setProperty(pThis, PDB_PROP_KDISP_IS_MISSING, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_KDISP_IMP_ALLOC_BW_IN_KERNEL_RM_DEF
    if (( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00005000UL) )) /* ChipHal: T234D | T264D */ 
    {
        pThis->setProperty(pThis, PDB_PROP_KDISP_IMP_ALLOC_BW_IN_KERNEL_RM_DEF, NV_TRUE);
    }
    // default
    else
    {
        pThis->setProperty(pThis, PDB_PROP_KDISP_IMP_ALLOC_BW_IN_KERNEL_RM_DEF, NV_FALSE);
    }
    pThis->setProperty(pThis, PDB_PROP_KDISP_FEATURE_STRETCH_VBLANK_CAPABLE, (0));

    // NVOC Property Hal field -- PDB_PROP_KDISP_HAS_SEPARATE_LOW_LATENCY_LINE
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_KDISP_HAS_SEPARATE_LOW_LATENCY_LINE, NV_FALSE);
    }
    pThis->setProperty(pThis, PDB_PROP_KDISP_ENABLE_INLINE_INTR_SERVICE, NV_TRUE);

    pThis->pStaticInfo = ((void *)0);

    pThis->bWarPurgeSatellitesOnCoreFree = NV_FALSE;

    pThis->bExtdevIntrSupported = NV_FALSE;

    pThis->bIsPanelReplayEnabled = NV_FALSE;
}

NV_STATUS __nvoc_ctor_OBJENGSTATE(OBJENGSTATE* );
NV_STATUS __nvoc_ctor_KernelDisplay(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {
    NV_STATUS status = NV_OK;
    status = __nvoc_ctor_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    if (status != NV_OK) goto __nvoc_ctor_KernelDisplay_fail_OBJENGSTATE;
    __nvoc_init_dataField_KernelDisplay(pThis, pRmhalspecowner, pGpuhalspecowner);
    goto __nvoc_ctor_KernelDisplay_exit; // Success

__nvoc_ctor_KernelDisplay_fail_OBJENGSTATE:
__nvoc_ctor_KernelDisplay_exit:

    return status;
}

// Vtable initialization
static void __nvoc_init_funcTable_KernelDisplay_1(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    ChipHal *chipHal = &pGpuhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    DispIpHal *dispIpHal = &pRmhalspecowner->dispIpHal;
    const unsigned long dispIpHal_HalVarIdx = (unsigned long)dispIpHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(pGpuhalspecowner);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(dispIpHal);
    PORT_UNREFERENCED_VARIABLE(dispIpHal_HalVarIdx);

    // kdispGetPBTargetAperture -- halified (2 hals) body
    if (((( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* ChipHal: T234D */  && (( ((dispIpHal_HalVarIdx >> 5) == 0UL) && ((1UL << (dispIpHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* DispIpHal: DISPv0402 */ ))
    {
        pThis->__kdispGetPBTargetAperture__ = &kdispGetPBTargetAperture_v03_00;
    }
    else
    {
        pThis->__kdispGetPBTargetAperture__ = &kdispGetPBTargetAperture_v05_01;
    }

    // kdispComputeDpModeSettings -- halified (2 hals) body
    if (((( ((chipHal_HalVarIdx >> 5) == 3UL) && ((1UL << (chipHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* ChipHal: T234D */  && (( ((dispIpHal_HalVarIdx >> 5) == 0UL) && ((1UL << (dispIpHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* DispIpHal: DISPv0402 */ ))
    {
        pThis->__kdispComputeDpModeSettings__ = &kdispComputeDpModeSettings_v02_04;
    }
    else
    {
        pThis->__kdispComputeDpModeSettings__ = &kdispComputeDpModeSettings_v05_01;
    }
} // End __nvoc_init_funcTable_KernelDisplay_1 with approximately 4 basic block(s).


// Initialize vtable(s) for 16 virtual method(s).
void __nvoc_init_funcTable_KernelDisplay(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {

    // Initialize vtable(s) with 2 per-object function pointer(s).
    __nvoc_init_funcTable_KernelDisplay_1(pThis, pRmhalspecowner, pGpuhalspecowner);
}

// Initialize newly constructed object.
void __nvoc_init__KernelDisplay(KernelDisplay *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {

    // Initialize pointers to inherited data.
    pThis->__nvoc_pbase_Object = &pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object;    // (obj) super^2
    pThis->__nvoc_pbase_OBJENGSTATE = &pThis->__nvoc_base_OBJENGSTATE;    // (engstate) super
    pThis->__nvoc_pbase_KernelDisplay = pThis;    // (kdisp) this

    // Recurse to superclass initialization function(s).
    __nvoc_init__OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);

    // Pointer(s) to metadata structures(s)
    pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object.__nvoc_metadata_ptr = &__nvoc_metadata__KernelDisplay.metadata__OBJENGSTATE.metadata__Object;    // (obj) super^2
    pThis->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr = &__nvoc_metadata__KernelDisplay.metadata__OBJENGSTATE;    // (engstate) super
    pThis->__nvoc_metadata_ptr = &__nvoc_metadata__KernelDisplay;    // (kdisp) this

    // Initialize per-object vtables.
    __nvoc_init_funcTable_KernelDisplay(pThis, pRmhalspecowner, pGpuhalspecowner);
}

NV_STATUS __nvoc_objCreate_KernelDisplay(KernelDisplay **ppThis, Dynamic *pParent, NvU32 createFlags)
{
    NV_STATUS status;
    Object *pParentObj = NULL;
    KernelDisplay *pThis;
    RmHalspecOwner *pRmhalspecowner;
    GpuHalspecOwner *pGpuhalspecowner;

    // Assign `pThis`, allocating memory unless suppressed by flag.
    status = __nvoc_handleObjCreateMemAlloc(createFlags, sizeof(KernelDisplay), (void**)&pThis, (void**)ppThis);
    if (status != NV_OK)
        return status;

    // Zero is the initial value for everything.
    portMemSet(pThis, 0, sizeof(KernelDisplay));

    pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object.createFlags = createFlags;

    // pParent must be a valid object that derives from a halspec owner class.
    NV_ASSERT_OR_RETURN(pParent != NULL, NV_ERR_INVALID_ARGUMENT);

    // Link the child into the parent unless flagged not to do so.
    if (!(createFlags & NVOC_OBJ_CREATE_FLAGS_PARENT_HALSPEC_ONLY))
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
    if ((pGpuhalspecowner = dynamicCast(pParent, GpuHalspecOwner)) == NULL)
        pGpuhalspecowner = objFindAncestorOfType(GpuHalspecOwner, pParent);
    NV_ASSERT_OR_RETURN(pGpuhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT);

    __nvoc_init__KernelDisplay(pThis, pRmhalspecowner, pGpuhalspecowner);
    status = __nvoc_ctor_KernelDisplay(pThis, pRmhalspecowner, pGpuhalspecowner);
    if (status != NV_OK) goto __nvoc_objCreate_KernelDisplay_cleanup;

    // Assignment has no effect if NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT is set.
    *ppThis = pThis;

    return NV_OK;

__nvoc_objCreate_KernelDisplay_cleanup:

    // Unlink the child from the parent if it was linked above.
    if (pParentObj != NULL)
        objRemoveChild(pParentObj, &pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object);

    // Do not call destructors here since the constructor already called them.
    if (createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(pThis, 0, sizeof(KernelDisplay));
    else
    {
        portMemFree(pThis);
        *ppThis = NULL;
    }

    // coverity[leaked_storage:FALSE]
    return status;
}

NV_STATUS __nvoc_objCreateDynamic_KernelDisplay(KernelDisplay **ppThis, Dynamic *pParent, NvU32 createFlags, va_list args) {
    NV_STATUS status;

    status = __nvoc_objCreate_KernelDisplay(ppThis, pParent, createFlags);

    return status;
}

