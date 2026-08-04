#define NVOC_OBJTMR_H_PRIVATE_ACCESS_ALLOWED

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
#include "g_objtmr_nvoc.h"


#ifdef DEBUG
char __nvoc_class_id_uniqueness_check__0x9ddede = 1;
#endif

extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJTMR;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_Object;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJENGSTATE;

// Forward declarations for OBJTMR
void __nvoc_init__OBJENGSTATE(OBJENGSTATE*);
void __nvoc_init__OBJTMR(OBJTMR*, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner);
void __nvoc_init_funcTable_OBJTMR(OBJTMR*, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner);
NV_STATUS __nvoc_ctor_OBJTMR(OBJTMR*, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner);
void __nvoc_init_dataField_OBJTMR(OBJTMR*, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner);
void __nvoc_dtor_OBJTMR(OBJTMR*);

// Structures used within RTTI (run-time type information)
extern const struct NVOC_CASTINFO __nvoc_castinfo__OBJTMR;
extern const struct NVOC_EXPORT_INFO __nvoc_export_info__OBJTMR;

// Down-thunk(s) to bridge OBJTMR methods from ancestors (if any)
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateConstructEngine(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr, ENGDESCRIPTOR arg3);    // this
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateStatePreInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr);    // this
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateStateInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr);    // this
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateStateLoad(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr, NvU32 arg3);    // this
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateStateUnload(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr, NvU32 arg3);    // this
void __nvoc_down_thunk_OBJTMR_engstateStateDestroy(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr);    // this

// Up-thunk(s) to bridge OBJTMR methods to ancestors (if any)
void __nvoc_up_thunk_OBJENGSTATE_tmrInitMissing(struct OBJGPU *pGpu, struct OBJTMR *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePreInitUnlocked(struct OBJGPU *pGpu, struct OBJTMR *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStateInitUnlocked(struct OBJGPU *pGpu, struct OBJTMR *pEngstate);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePreLoad(struct OBJGPU *pGpu, struct OBJTMR *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePostLoad(struct OBJGPU *pGpu, struct OBJTMR *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePreUnload(struct OBJGPU *pGpu, struct OBJTMR *pEngstate, NvU32 arg3);    // this
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePostUnload(struct OBJGPU *pGpu, struct OBJTMR *pEngstate, NvU32 arg3);    // this
NvBool __nvoc_up_thunk_OBJENGSTATE_tmrIsPresent(struct OBJGPU *pGpu, struct OBJTMR *pEngstate);    // this

const struct NVOC_CLASS_DEF __nvoc_class_def_OBJTMR = 
{
    /*classInfo=*/ {
        /*size=*/               sizeof(OBJTMR),
        /*classId=*/            classId(OBJTMR),
        /*providerId=*/         &__nvoc_rtti_provider,
#if NV_PRINTF_STRINGS_ALLOWED
        /*name=*/               "OBJTMR",
#endif
    },
    /*objCreatefn=*/        (NVOC_DYNAMIC_OBJ_CREATE) &__nvoc_objCreateDynamic_OBJTMR,
    /*pCastInfo=*/          &__nvoc_castinfo__OBJTMR,
    /*pExportInfo=*/        &__nvoc_export_info__OBJTMR
};


// Metadata with per-class RTTI and vtable with ancestor(s)
static const struct NVOC_METADATA__OBJTMR __nvoc_metadata__OBJTMR = {
    .rtti.pClassDef = &__nvoc_class_def_OBJTMR,    // (tmr) this
    .rtti.dtor      = (NVOC_DYNAMIC_DTOR) &__nvoc_dtor_OBJTMR,
    .rtti.offset    = 0,
    .metadata__OBJENGSTATE.rtti.pClassDef = &__nvoc_class_def_OBJENGSTATE,    // (engstate) super
    .metadata__OBJENGSTATE.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__OBJENGSTATE.rtti.offset    = NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE),
    .metadata__OBJENGSTATE.metadata__Object.rtti.pClassDef = &__nvoc_class_def_Object,    // (obj) super^2
    .metadata__OBJENGSTATE.metadata__Object.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__OBJENGSTATE.metadata__Object.rtti.offset    = NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE.__nvoc_base_Object),

    .vtable.__tmrConstructEngine__ = &tmrConstructEngine_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateConstructEngine__ = &__nvoc_down_thunk_OBJTMR_engstateConstructEngine,    // virtual
    .vtable.__tmrStatePreInitLocked__ = &tmrStatePreInitLocked_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreInitLocked__ = &__nvoc_down_thunk_OBJTMR_engstateStatePreInitLocked,    // virtual
    .vtable.__tmrStateInitLocked__ = &tmrStateInitLocked_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateInitLocked__ = &__nvoc_down_thunk_OBJTMR_engstateStateInitLocked,    // virtual
    .vtable.__tmrStateLoad__ = &tmrStateLoad_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateLoad__ = &__nvoc_down_thunk_OBJTMR_engstateStateLoad,    // virtual
    .vtable.__tmrStateUnload__ = &tmrStateUnload_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateUnload__ = &__nvoc_down_thunk_OBJTMR_engstateStateUnload,    // virtual
    .vtable.__tmrStateDestroy__ = &tmrStateDestroy_IMPL,    // virtual override (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateDestroy__ = &__nvoc_down_thunk_OBJTMR_engstateStateDestroy,    // virtual
    .vtable.__tmrInitMissing__ = &__nvoc_up_thunk_OBJENGSTATE_tmrInitMissing,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateInitMissing__ = &engstateInitMissing_IMPL,    // virtual
    .vtable.__tmrStatePreInitUnlocked__ = &__nvoc_up_thunk_OBJENGSTATE_tmrStatePreInitUnlocked,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreInitUnlocked__ = &engstateStatePreInitUnlocked_IMPL,    // virtual
    .vtable.__tmrStateInitUnlocked__ = &__nvoc_up_thunk_OBJENGSTATE_tmrStateInitUnlocked,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStateInitUnlocked__ = &engstateStateInitUnlocked_IMPL,    // virtual
    .vtable.__tmrStatePreLoad__ = &__nvoc_up_thunk_OBJENGSTATE_tmrStatePreLoad,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreLoad__ = &engstateStatePreLoad_IMPL,    // virtual
    .vtable.__tmrStatePostLoad__ = &__nvoc_up_thunk_OBJENGSTATE_tmrStatePostLoad,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePostLoad__ = &engstateStatePostLoad_IMPL,    // virtual
    .vtable.__tmrStatePreUnload__ = &__nvoc_up_thunk_OBJENGSTATE_tmrStatePreUnload,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePreUnload__ = &engstateStatePreUnload_IMPL,    // virtual
    .vtable.__tmrStatePostUnload__ = &__nvoc_up_thunk_OBJENGSTATE_tmrStatePostUnload,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateStatePostUnload__ = &engstateStatePostUnload_IMPL,    // virtual
    .vtable.__tmrIsPresent__ = &__nvoc_up_thunk_OBJENGSTATE_tmrIsPresent,    // virtual inherited (engstate) base (engstate)
    .metadata__OBJENGSTATE.vtable.__engstateIsPresent__ = &engstateIsPresent_IMPL,    // virtual
};


// Dynamic down-casting information
const struct NVOC_CASTINFO __nvoc_castinfo__OBJTMR = {
    .numRelatives = 3,
    .relatives = {
        &__nvoc_metadata__OBJTMR.rtti,    // [0]: (tmr) this
        &__nvoc_metadata__OBJTMR.metadata__OBJENGSTATE.rtti,    // [1]: (engstate) super
        &__nvoc_metadata__OBJTMR.metadata__OBJENGSTATE.metadata__Object.rtti,    // [2]: (obj) super^2
    }
};

// 6 down-thunk(s) defined to bridge methods in OBJTMR from superclasses

// tmrConstructEngine: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateConstructEngine(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr, ENGDESCRIPTOR arg3) {
    return tmrConstructEngine(pGpu, (struct OBJTMR *)(((unsigned char *) pTmr) - NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)), arg3);
}

// tmrStatePreInitLocked: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateStatePreInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr) {
    return tmrStatePreInitLocked(pGpu, (struct OBJTMR *)(((unsigned char *) pTmr) - NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)));
}

// tmrStateInitLocked: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateStateInitLocked(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr) {
    return tmrStateInitLocked(pGpu, (struct OBJTMR *)(((unsigned char *) pTmr) - NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)));
}

// tmrStateLoad: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateStateLoad(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr, NvU32 arg3) {
    return tmrStateLoad(pGpu, (struct OBJTMR *)(((unsigned char *) pTmr) - NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)), arg3);
}

// tmrStateUnload: virtual override (engstate) base (engstate)
NV_STATUS __nvoc_down_thunk_OBJTMR_engstateStateUnload(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr, NvU32 arg3) {
    return tmrStateUnload(pGpu, (struct OBJTMR *)(((unsigned char *) pTmr) - NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)), arg3);
}

// tmrStateDestroy: virtual override (engstate) base (engstate)
void __nvoc_down_thunk_OBJTMR_engstateStateDestroy(struct OBJGPU *pGpu, struct OBJENGSTATE *pTmr) {
    tmrStateDestroy(pGpu, (struct OBJTMR *)(((unsigned char *) pTmr) - NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)));
}


// 8 up-thunk(s) defined to bridge methods in OBJTMR to superclasses

// tmrInitMissing: virtual inherited (engstate) base (engstate)
void __nvoc_up_thunk_OBJENGSTATE_tmrInitMissing(struct OBJGPU *pGpu, struct OBJTMR *pEngstate) {
    engstateInitMissing(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)));
}

// tmrStatePreInitUnlocked: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePreInitUnlocked(struct OBJGPU *pGpu, struct OBJTMR *pEngstate) {
    return engstateStatePreInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)));
}

// tmrStateInitUnlocked: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStateInitUnlocked(struct OBJGPU *pGpu, struct OBJTMR *pEngstate) {
    return engstateStateInitUnlocked(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)));
}

// tmrStatePreLoad: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePreLoad(struct OBJGPU *pGpu, struct OBJTMR *pEngstate, NvU32 arg3) {
    return engstateStatePreLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)), arg3);
}

// tmrStatePostLoad: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePostLoad(struct OBJGPU *pGpu, struct OBJTMR *pEngstate, NvU32 arg3) {
    return engstateStatePostLoad(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)), arg3);
}

// tmrStatePreUnload: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePreUnload(struct OBJGPU *pGpu, struct OBJTMR *pEngstate, NvU32 arg3) {
    return engstateStatePreUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)), arg3);
}

// tmrStatePostUnload: virtual inherited (engstate) base (engstate)
NV_STATUS __nvoc_up_thunk_OBJENGSTATE_tmrStatePostUnload(struct OBJGPU *pGpu, struct OBJTMR *pEngstate, NvU32 arg3) {
    return engstateStatePostUnload(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)), arg3);
}

// tmrIsPresent: virtual inherited (engstate) base (engstate)
NvBool __nvoc_up_thunk_OBJENGSTATE_tmrIsPresent(struct OBJGPU *pGpu, struct OBJTMR *pEngstate) {
    return engstateIsPresent(pGpu, (struct OBJENGSTATE *)(((unsigned char *) pEngstate) + NV_OFFSETOF(OBJTMR, __nvoc_base_OBJENGSTATE)));
}


const struct NVOC_EXPORT_INFO __nvoc_export_info__OBJTMR = 
{
    /*numEntries=*/     0,
    /*pExportEntries=*/  0
};

void __nvoc_dtor_OBJENGSTATE(OBJENGSTATE*);
void __nvoc_dtor_OBJTMR(OBJTMR *pThis) {
    __nvoc_tmrDestruct(pThis);
    __nvoc_dtor_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    PORT_UNREFERENCED_VARIABLE(pThis);
}

void __nvoc_init_dataField_OBJTMR(OBJTMR *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {
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

    // NVOC Property Hal field -- PDB_PROP_TMR_USE_COUNTDOWN_TIMER_FOR_RM_CALLBACKS
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_TMR_USE_COUNTDOWN_TIMER_FOR_RM_CALLBACKS, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_TMR_ALARM_INTR_REMOVED_FROM_PMC_TREE
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_TMR_ALARM_INTR_REMOVED_FROM_PMC_TREE, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_TMR_USE_OS_TIMER_FOR_CALLBACKS
    if (( ((rmVariantHal_HalVarIdx >> 5) == 0UL) && ((1UL << (rmVariantHal_HalVarIdx & 0x1f)) & 0x00000002UL) )) /* RmVariantHal: PF_KERNEL_ONLY */ 
    {
        pThis->setProperty(pThis, PDB_PROP_TMR_USE_OS_TIMER_FOR_CALLBACKS, NV_TRUE);
    }
    pThis->setProperty(pThis, PDB_PROP_TMR_USE_PTIMER_FOR_OSTIMER_CALLBACKS, (0));
    pThis->setProperty(pThis, PDB_PROP_TMR_USE_POLLING_FOR_CALLBACKS, (0));

    // NVOC Property Hal field -- PDB_PROP_TMR_USE_SECOND_COUNTDOWN_TIMER_FOR_SWRL
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_TMR_USE_SECOND_COUNTDOWN_TIMER_FOR_SWRL, NV_FALSE);
    }

    // NVOC Property Hal field -- PDB_PROP_TMR_WAR_FOR_BUG_4679970_DEF
    // default
    {
        pThis->setProperty(pThis, PDB_PROP_TMR_WAR_FOR_BUG_4679970_DEF, NV_FALSE);
    }
}

NV_STATUS __nvoc_ctor_OBJENGSTATE(OBJENGSTATE* );
NV_STATUS __nvoc_ctor_OBJTMR(OBJTMR *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {
    NV_STATUS status = NV_OK;
    status = __nvoc_ctor_OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);
    if (status != NV_OK) goto __nvoc_ctor_OBJTMR_fail_OBJENGSTATE;
    __nvoc_init_dataField_OBJTMR(pThis, pGpuhalspecowner, pRmhalspecowner);
    goto __nvoc_ctor_OBJTMR_exit; // Success

__nvoc_ctor_OBJTMR_fail_OBJENGSTATE:
__nvoc_ctor_OBJTMR_exit:

    return status;
}

// Vtable initialization
static void __nvoc_init_funcTable_OBJTMR_1(OBJTMR *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {
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
} // End __nvoc_init_funcTable_OBJTMR_1


// Initialize vtable(s) for 14 virtual method(s).
void __nvoc_init_funcTable_OBJTMR(OBJTMR *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {
    __nvoc_init_funcTable_OBJTMR_1(pThis, pGpuhalspecowner, pRmhalspecowner);
}

// Initialize newly constructed object.
void __nvoc_init__OBJTMR(OBJTMR *pThis, GpuHalspecOwner *pGpuhalspecowner, RmHalspecOwner *pRmhalspecowner) {

    // Initialize pointers to inherited data.
    pThis->__nvoc_pbase_Object = &pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object;    // (obj) super^2
    pThis->__nvoc_pbase_OBJENGSTATE = &pThis->__nvoc_base_OBJENGSTATE;    // (engstate) super
    pThis->__nvoc_pbase_OBJTMR = pThis;    // (tmr) this

    // Recurse to superclass initialization function(s).
    __nvoc_init__OBJENGSTATE(&pThis->__nvoc_base_OBJENGSTATE);

    // Pointer(s) to metadata structures(s)
    pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object.__nvoc_metadata_ptr = &__nvoc_metadata__OBJTMR.metadata__OBJENGSTATE.metadata__Object;    // (obj) super^2
    pThis->__nvoc_base_OBJENGSTATE.__nvoc_metadata_ptr = &__nvoc_metadata__OBJTMR.metadata__OBJENGSTATE;    // (engstate) super
    pThis->__nvoc_metadata_ptr = &__nvoc_metadata__OBJTMR;    // (tmr) this

    // Initialize per-object vtables.
    __nvoc_init_funcTable_OBJTMR(pThis, pGpuhalspecowner, pRmhalspecowner);
}

NV_STATUS __nvoc_objCreate_OBJTMR(OBJTMR **ppThis, Dynamic *pParent, NvU32 createFlags)
{
    NV_STATUS status;
    Object *pParentObj = NULL;
    OBJTMR *pThis;
    GpuHalspecOwner *pGpuhalspecowner;
    RmHalspecOwner *pRmhalspecowner;

    // Assign `pThis`, allocating memory unless suppressed by flag.
    status = __nvoc_handleObjCreateMemAlloc(createFlags, sizeof(OBJTMR), (void**)&pThis, (void**)ppThis);
    if (status != NV_OK)
        return status;

    // Zero is the initial value for everything.
    portMemSet(pThis, 0, sizeof(OBJTMR));

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

    if ((pGpuhalspecowner = dynamicCast(pParent, GpuHalspecOwner)) == NULL)
        pGpuhalspecowner = objFindAncestorOfType(GpuHalspecOwner, pParent);
    NV_ASSERT_OR_RETURN(pGpuhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT);
    if ((pRmhalspecowner = dynamicCast(pParent, RmHalspecOwner)) == NULL)
        pRmhalspecowner = objFindAncestorOfType(RmHalspecOwner, pParent);
    NV_ASSERT_OR_RETURN(pRmhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT);

    __nvoc_init__OBJTMR(pThis, pGpuhalspecowner, pRmhalspecowner);
    status = __nvoc_ctor_OBJTMR(pThis, pGpuhalspecowner, pRmhalspecowner);
    if (status != NV_OK) goto __nvoc_objCreate_OBJTMR_cleanup;

    // Assignment has no effect if NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT is set.
    *ppThis = pThis;

    return NV_OK;

__nvoc_objCreate_OBJTMR_cleanup:

    // Unlink the child from the parent if it was linked above.
    if (pParentObj != NULL)
        objRemoveChild(pParentObj, &pThis->__nvoc_base_OBJENGSTATE.__nvoc_base_Object);

    // Do not call destructors here since the constructor already called them.
    if (createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(pThis, 0, sizeof(OBJTMR));
    else
    {
        portMemFree(pThis);
        *ppThis = NULL;
    }

    // coverity[leaked_storage:FALSE]
    return status;
}

NV_STATUS __nvoc_objCreateDynamic_OBJTMR(OBJTMR **ppThis, Dynamic *pParent, NvU32 createFlags, va_list args) {
    NV_STATUS status;

    status = __nvoc_objCreate_OBJTMR(ppThis, pParent, createFlags);

    return status;
}

