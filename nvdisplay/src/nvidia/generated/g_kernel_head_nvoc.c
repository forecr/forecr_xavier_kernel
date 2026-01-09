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
char __nvoc_class_id_uniqueness_check__0x0145e6 = 1;
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

const struct NVOC_CLASS_DEF __nvoc_class_def_KernelHead = 
{
    /*classInfo=*/ {
        /*size=*/               sizeof(KernelHead),
        /*classId=*/            classId(KernelHead),
        /*providerId=*/         &__nvoc_rtti_provider,
#if NV_PRINTF_STRINGS_ALLOWED
        /*name=*/               "KernelHead",
#endif
    },
    /*objCreatefn=*/        (NVOC_DYNAMIC_OBJ_CREATE) &__nvoc_objCreateDynamic_KernelHead,
    /*pCastInfo=*/          &__nvoc_castinfo__KernelHead,
    /*pExportInfo=*/        &__nvoc_export_info__KernelHead
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
    /*numEntries=*/     0,
    /*pExportEntries=*/  0
};

void __nvoc_dtor_Object(Object*);
void __nvoc_dtor_KernelHead(KernelHead *pThis) {
    __nvoc_kheadDestruct(pThis);
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
}

NV_STATUS __nvoc_ctor_Object(Object* );
NV_STATUS __nvoc_ctor_KernelHead(KernelHead *pThis, RmHalspecOwner *pRmhalspecowner, GpuHalspecOwner *pGpuhalspecowner) {
    NV_STATUS status = NV_OK;
    status = __nvoc_ctor_Object(&pThis->__nvoc_base_Object);
    if (status != NV_OK) goto __nvoc_ctor_KernelHead_fail_Object;
    __nvoc_init_dataField_KernelHead(pThis, pRmhalspecowner, pGpuhalspecowner);

    status = __nvoc_kheadConstruct(pThis);
    if (status != NV_OK) goto __nvoc_ctor_KernelHead_fail__init;
    goto __nvoc_ctor_KernelHead_exit; // Success

__nvoc_ctor_KernelHead_fail__init:
    __nvoc_dtor_Object(&pThis->__nvoc_base_Object);
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
        pThis->__kheadGetCrashLockCounterV__ = &kheadGetCrashLockCounterV_4a4dee;
    }

    // kheadVsyncNotificationOverRgVblankIntr -- halified (2 hals) body
    if (( ((dispIpHal_HalVarIdx >> 5) == 0UL) && ((1UL << (dispIpHal_HalVarIdx & 0x1f)) & 0x00001000UL) )) /* DispIpHal: DISPv0402 */ 
    {
        pThis->__kheadVsyncNotificationOverRgVblankIntr__ = &kheadVsyncNotificationOverRgVblankIntr_b3696a;
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

NV_STATUS __nvoc_objCreate_KernelHead(KernelHead **ppThis, Dynamic *pParent, NvU32 createFlags)
{
    NV_STATUS status;
    Object *pParentObj = NULL;
    KernelHead *pThis;
    RmHalspecOwner *pRmhalspecowner;
    GpuHalspecOwner *pGpuhalspecowner;

    // Assign `pThis`, allocating memory unless suppressed by flag.
    status = __nvoc_handleObjCreateMemAlloc(createFlags, sizeof(KernelHead), (void**)&pThis, (void**)ppThis);
    if (status != NV_OK)
        return status;

    // Zero is the initial value for everything.
    portMemSet(pThis, 0, sizeof(KernelHead));

    pThis->__nvoc_base_Object.createFlags = createFlags;

    // pParent must be a valid object that derives from a halspec owner class.
    NV_ASSERT_OR_RETURN(pParent != NULL, NV_ERR_INVALID_ARGUMENT);

    // Link the child into the parent unless flagged not to do so.
    if (!(createFlags & NVOC_OBJ_CREATE_FLAGS_PARENT_HALSPEC_ONLY))
    {
        pParentObj = dynamicCast(pParent, Object);
        objAddChild(pParentObj, &pThis->__nvoc_base_Object);
    }
    else
    {
        pThis->__nvoc_base_Object.pParent = NULL;
    }

    if ((pRmhalspecowner = dynamicCast(pParent, RmHalspecOwner)) == NULL)
        pRmhalspecowner = objFindAncestorOfType(RmHalspecOwner, pParent);
    NV_ASSERT_OR_RETURN(pRmhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT);
    if ((pGpuhalspecowner = dynamicCast(pParent, GpuHalspecOwner)) == NULL)
        pGpuhalspecowner = objFindAncestorOfType(GpuHalspecOwner, pParent);
    NV_ASSERT_OR_RETURN(pGpuhalspecowner != NULL, NV_ERR_INVALID_ARGUMENT);

    __nvoc_init__KernelHead(pThis, pRmhalspecowner, pGpuhalspecowner);
    status = __nvoc_ctor_KernelHead(pThis, pRmhalspecowner, pGpuhalspecowner);
    if (status != NV_OK) goto __nvoc_objCreate_KernelHead_cleanup;

    // Assignment has no effect if NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT is set.
    *ppThis = pThis;

    return NV_OK;

__nvoc_objCreate_KernelHead_cleanup:

    // Unlink the child from the parent if it was linked above.
    if (pParentObj != NULL)
        objRemoveChild(pParentObj, &pThis->__nvoc_base_Object);

    // Do not call destructors here since the constructor already called them.
    if (createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(pThis, 0, sizeof(KernelHead));
    else
    {
        portMemFree(pThis);
        *ppThis = NULL;
    }

    // coverity[leaked_storage:FALSE]
    return status;
}

NV_STATUS __nvoc_objCreateDynamic_KernelHead(KernelHead **ppThis, Dynamic *pParent, NvU32 createFlags, va_list args) {
    NV_STATUS status;

    status = __nvoc_objCreate_KernelHead(ppThis, pParent, createFlags);

    return status;
}

