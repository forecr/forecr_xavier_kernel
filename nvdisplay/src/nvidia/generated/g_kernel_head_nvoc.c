#define NVOC_KERNEL_HEAD_H_PRIVATE_ACCESS_ALLOWED

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
#include "g_kernel_head_nvoc.h"


#ifdef DEBUG
char __nvoc_class_id_uniqueness_check__0145e6 = 1;
#endif

extern const struct NVOC_CLASS_DEF __nvoc_class_def_KernelHead;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_Object;

// Forward declarations for KernelHead
void __nvoc_init__Object(Object*);
void __nvoc_init__KernelHead(KernelHead*, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner);
void __nvoc_init_funcTable_KernelHead(KernelHead*, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner);
NV_STATUS __nvoc_ctor_KernelHead(KernelHead*, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner);
void __nvoc_init_dataField_KernelHead(KernelHead*, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner);
void __nvoc_dtor_KernelHead(KernelHead*);

// Structures used within RTTI (run-time type information)
extern const struct NVOC_CASTINFO __nvoc_castinfo__KernelHead;
extern const struct NVOC_EXPORT_INFO __nvoc_export_info__KernelHead;

// Down-thunk(s) to bridge KernelHead methods from ancestors (if any)

// Up-thunk(s) to bridge KernelHead methods to ancestors (if any)

// Class-specific details for KernelHead
const struct NVOC_CLASS_DEF __nvoc_class_def_KernelHead = 
{
    .classInfo.size =               sizeof(KernelHead),
    .classInfo.classId =            classId(KernelHead),
    .classInfo.providerId =         &__nvoc_rtti_provider,
#if NV_PRINTF_STRINGS_ALLOWED
    .classInfo.name =               "KernelHead",
#endif
    .objCreatefn =        (NVOC_DYNAMIC_OBJ_CREATE) &__nvoc_objCreateDynamic_KernelHead,
    .pCastInfo =          &__nvoc_castinfo__KernelHead,
    .pExportInfo =        &__nvoc_export_info__KernelHead
};


// Metadata with per-class RTTI with ancestor(s)
static const struct NVOC_METADATA__KernelHead __nvoc_metadata__KernelHead = {
    .rtti.pClassDef = &__nvoc_class_def_KernelHead,    // (khead) this
    .rtti.dtor      = (NVOC_DYNAMIC_DTOR) &__nvoc_dtor_KernelHead,
    .rtti.offset    = 0,
    .metadata__Object.rtti.pClassDef = &__nvoc_class_def_Object,    // (obj) super
    .metadata__Object.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__Object.rtti.offset    = NV_OFFSETOF(KernelHead, __nvoc_base_Object),
};


// Dynamic down-casting information
const struct NVOC_CASTINFO __nvoc_castinfo__KernelHead = {
    .numRelatives = 2,
    .relatives = {
        &__nvoc_metadata__KernelHead.rtti,    // [0]: (khead) this
        &__nvoc_metadata__KernelHead.metadata__Object.rtti,    // [1]: (obj) super
    }
};

const struct NVOC_EXPORT_INFO __nvoc_export_info__KernelHead = 
{
    .numEntries=     0,
    .pExportEntries= 0
};


// Destruct KernelHead object.
void __nvoc_kheadDestruct(KernelHead*);
void __nvoc_dtor_Object(Object*);
void __nvoc_dtor_KernelHead(KernelHead* pThis) {

// Call destructor.
    __nvoc_kheadDestruct(pThis);

// Recurse to superclass destructors.
    __nvoc_dtor_Object(&pThis->__nvoc_base_Object);

    PORT_UNREFERENCED_VARIABLE(pThis);
}
void __nvoc_init_dataField_KernelHead(KernelHead *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    DispIpHal *dispIpHal = &pRmhalspecowner->dispIpHal;
    const unsigned long dispIpHal_HalVarIdx = (unsigned long)dispIpHal->__nvoc_HalVarIdx;
    ChipHal *chipHal = &pGpuhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(pGpuhalspecowner);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(dispIpHal);
    PORT_UNREFERENCED_VARIABLE(dispIpHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);

    pThis->bIsPanelReplayEnabled = NV_FALSE;
}


// Construct KernelHead object.
NV_STATUS __nvoc_ctor_Object(Object *);
NV_STATUS __nvoc_ctor_KernelHead(KernelHead *pKernelHead, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {
    NV_STATUS status = NV_OK;

    // Recurse to ancestor constructor(s).
    status = __nvoc_ctor_Object(&pKernelHead->__nvoc_base_Object);
    if (status != NV_OK) goto __nvoc_ctor_KernelHead_fail_Object;

    // Initialize data fields.
    __nvoc_init_dataField_KernelHead(pKernelHead, pRmhalspecowner, pGpuhalspecowner);

    // Call the constructor for this class.
    status = __nvoc_kheadConstruct(pKernelHead);
    if (status != NV_OK) goto __nvoc_ctor_KernelHead_fail__init;
    goto __nvoc_ctor_KernelHead_exit; // Success

    // Unwind on error.
__nvoc_ctor_KernelHead_fail__init:
    __nvoc_dtor_Object(&pKernelHead->__nvoc_base_Object);
__nvoc_ctor_KernelHead_fail_Object:
__nvoc_ctor_KernelHead_exit:
    return status;
}

// Vtable initialization
static void __nvoc_init_funcTable_KernelHead_1(KernelHead *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {
    RmVariantHal *rmVariantHal = &pRmhalspecowner->rmVariantHal;
    const unsigned long rmVariantHal_HalVarIdx = (unsigned long)rmVariantHal->__nvoc_HalVarIdx;
    DispIpHal *dispIpHal = &pRmhalspecowner->dispIpHal;
    const unsigned long dispIpHal_HalVarIdx = (unsigned long)dispIpHal->__nvoc_HalVarIdx;
    ChipHal *chipHal = &pGpuhalspecowner->chipHal;
    const unsigned long chipHal_HalVarIdx = (unsigned long)chipHal->__nvoc_HalVarIdx;
    PORT_UNREFERENCED_VARIABLE(pThis);
    PORT_UNREFERENCED_VARIABLE(pRmhalspecowner);
    PORT_UNREFERENCED_VARIABLE(pGpuhalspecowner);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal);
    PORT_UNREFERENCED_VARIABLE(rmVariantHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(dispIpHal);
    PORT_UNREFERENCED_VARIABLE(dispIpHal_HalVarIdx);
    PORT_UNREFERENCED_VARIABLE(chipHal);
    PORT_UNREFERENCED_VARIABLE(chipHal_HalVarIdx);

    // kheadGetLoadVCounter -- halified (2 hals) body
    if (( ((dispIpHal_HalVarIdx >> 5) == 0UL) && ((1UL << (dispIpHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* DispIpHal: DISPv0402 */ 
    {
        pThis->__kheadGetLoadVCounter__ = &kheadGetLoadVCounter_v03_00;
    }
    else
    {
        pThis->__kheadGetLoadVCounter__ = &kheadGetLoadVCounter_v05_01;
    }

    // kheadGetCrashLockCounterV -- halified (2 hals) body
    if (( ((dispIpHal_HalVarIdx >> 5) == 0UL) && ((1UL << (dispIpHal_HalVarIdx & 0x1f)) & 0x00048000UL) )) /* DispIpHal: DISPv0501 | DISPv0504 */ 
    {
        pThis->__kheadGetCrashLockCounterV__ = &kheadGetCrashLockCounterV_v05_01;
    }
    // default
    else
    {
        pThis->__kheadGetCrashLockCounterV__ = &kheadGetCrashLockCounterV_b2b553;
    }

    // kheadVsyncNotificationOverRgVblankIntr -- halified (2 hals) body
    if (( ((dispIpHal_HalVarIdx >> 5) == 0UL) && ((1UL << (dispIpHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* DispIpHal: DISPv0402 */ 
    {
        pThis->__kheadVsyncNotificationOverRgVblankIntr__ = &kheadVsyncNotificationOverRgVblankIntr_d44104;
    }
    else
    {
        pThis->__kheadVsyncNotificationOverRgVblankIntr__ = &kheadVsyncNotificationOverRgVblankIntr_v04_04;
    }
} // End __nvoc_init_funcTable_KernelHead_1 with approximately 6 basic block(s).


// Initialize vtable(s) for 3 virtual method(s).
void __nvoc_init_funcTable_KernelHead(KernelHead *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {

    // Initialize vtable(s) with 3 per-object function pointer(s).
    __nvoc_init_funcTable_KernelHead_1(pThis, pRmhalspecowner, pGpuhalspecowner);
}

// Initialize newly constructed object.
void __nvoc_init__KernelHead(KernelHead *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {

    // Initialize pointers to inherited data.
    pThis->__nvoc_pbase_Object = &pThis->__nvoc_base_Object;    // (obj) super
    pThis->__nvoc_pbase_KernelHead = pThis;    // (khead) this

    // Recurse to superclass initialization function(s).
    __nvoc_init__Object(&pThis->__nvoc_base_Object);

    // Pointer(s) to metadata structures(s)
    pThis->__nvoc_base_Object.__nvoc_metadata_ptr = &__nvoc_metadata__KernelHead.metadata__Object;    // (obj) super
    pThis->__nvoc_metadata_ptr = &__nvoc_metadata__KernelHead;    // (khead) this

    // Initialize per-object vtables.
    __nvoc_init_funcTable_KernelHead(pThis, pRmhalspecowner, pGpuhalspecowner);
}

NV_STATUS __nvoc_objCreate_KernelHead(KernelHead **__nvoc_ppThis, Dynamic *__nvoc_pParent, NvU32 __nvoc_createFlags)
{
    NV_STATUS __nvoc_status;
    Object *__nvoc_pParentObj = NULL;
    KernelHead *__nvoc_pThis;
    RmHalspecOwner *pRmhalspecowner;
    GpuHalspecOwner *pGpuhalspecowner;

    // Don't allocate memory if the caller has already done so.
    if (__nvoc_createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
    {
        NV_CHECK_OR_RETURN(LEVEL_ERROR, __nvoc_ppThis != NULL && *__nvoc_ppThis != NULL, NV_ERR_INVALID_PARAMETER);
        __nvoc_pThis = *__nvoc_ppThis;
    }

    // Allocate memory
    else
    {
        __nvoc_pThis = portMemAllocNonPaged(sizeof(KernelHead));
        NV_CHECK_OR_RETURN(LEVEL_ERROR, __nvoc_pThis != NULL, NV_ERR_NO_MEMORY);
    }

    // Zero is the initial value for everything.
    portMemSet(__nvoc_pThis, 0, sizeof(KernelHead));

    __nvoc_pThis->__nvoc_base_Object.createFlags = __nvoc_createFlags;

    // pParent must be a valid object that derives from a halspec owner class.
    NV_CHECK_TRUE_OR_GOTO(__nvoc_status, LEVEL_ERROR, __nvoc_pParent != NULL, NV_ERR_INVALID_ARGUMENT, __nvoc_objCreate_KernelHead_cleanup);

    // Link the child into the parent unless flagged not to do so.
    if (!(__nvoc_createFlags & NVOC_OBJ_CREATE_FLAGS_PARENT_HALSPEC_ONLY))
    {
        __nvoc_pParentObj = dynamicCast(__nvoc_pParent, Object);
        objAddChild(__nvoc_pParentObj, &__nvoc_pThis->__nvoc_base_Object);
    }
    else
    {
        __nvoc_pThis->__nvoc_base_Object.pParent = NULL;
    }

    // HALs are defined by the parent or the first super class.
    if ((pRmhalspecowner = dynamicCast(__nvoc_pParent, RmHalspecOwner)) == NULL)
        pRmhalspecowner = objFindAncestorOfType(RmHalspecOwner, __nvoc_pParent);
    NV_CHECK_TRUE_OR_GOTO(__nvoc_status, LEVEL_ERROR, pRmhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT, __nvoc_objCreate_KernelHead_cleanup);
    if ((pGpuhalspecowner = dynamicCast(__nvoc_pParent, GpuHalspecOwner)) == NULL)
        pGpuhalspecowner = objFindAncestorOfType(GpuHalspecOwner, __nvoc_pParent);
    NV_CHECK_TRUE_OR_GOTO(__nvoc_status, LEVEL_ERROR, pGpuhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT, __nvoc_objCreate_KernelHead_cleanup);

    // Initialize vtable, RTTI, etc., then call constructor.
    __nvoc_init__KernelHead(__nvoc_pThis, pRmhalspecowner, pGpuhalspecowner);
    __nvoc_status = __nvoc_ctor_KernelHead(__nvoc_pThis, pRmhalspecowner, pGpuhalspecowner);
    if (__nvoc_status != NV_OK) goto __nvoc_objCreate_KernelHead_cleanup;

    // Assignment has no effect if NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT is set.
    *__nvoc_ppThis = __nvoc_pThis;

    // Success
    return NV_OK;

    // Do not call destructors here since the constructor already called them.
__nvoc_objCreate_KernelHead_cleanup:

    // Unlink the child from the parent if it was linked above.
    if (__nvoc_pParentObj != NULL)
        objRemoveChild(__nvoc_pParentObj, &__nvoc_pThis->__nvoc_base_Object);

    // Zero out memory that was allocated by caller.
    if (__nvoc_createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(__nvoc_pThis, 0, sizeof(KernelHead));

    // Free memory allocated by `__nvoc_handleObjCreateMemAlloc`.
    else
    {
        portMemFree(__nvoc_pThis);
        *__nvoc_ppThis = NULL;
    }

    // Failure
    return __nvoc_status;
}

NV_STATUS __nvoc_objCreateDynamic_KernelHead(KernelHead **__nvoc_ppThis, Dynamic *__nvoc_pParent, NvU32 __nvoc_createFlags, va_list __nvoc_args) {
    NV_STATUS __nvoc_status;

    __nvoc_status = __nvoc_objCreate_KernelHead(__nvoc_ppThis, __nvoc_pParent, __nvoc_createFlags);

    return __nvoc_status;
}

