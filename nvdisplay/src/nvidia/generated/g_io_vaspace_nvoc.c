#define NVOC_IO_VASPACE_H_PRIVATE_ACCESS_ALLOWED

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
#include "g_io_vaspace_nvoc.h"


#ifdef DEBUG
char __nvoc_class_id_uniqueness_check__0x28ed9c = 1;
#endif

extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJIOVASPACE;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_Object;
extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJVASPACE;

// Forward declarations for OBJIOVASPACE
void __nvoc_init__OBJVASPACE(OBJVASPACE*);
void __nvoc_init__OBJIOVASPACE(OBJIOVASPACE*);
void __nvoc_init_funcTable_OBJIOVASPACE(OBJIOVASPACE*);
NV_STATUS __nvoc_ctor_OBJIOVASPACE(OBJIOVASPACE*);
void __nvoc_init_dataField_OBJIOVASPACE(OBJIOVASPACE*);
void __nvoc_dtor_OBJIOVASPACE(OBJIOVASPACE*);

// Structures used within RTTI (run-time type information)
extern const struct NVOC_CASTINFO __nvoc_castinfo__OBJIOVASPACE;
extern const struct NVOC_EXPORT_INFO __nvoc_export_info__OBJIOVASPACE;

// Down-thunk(s) to bridge OBJIOVASPACE methods from ancestors (if any)
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceConstruct_(struct OBJVASPACE *pVAS, NvU32 classId, NvU32 vaspaceId, NvU64 vaStart, NvU64 vaLimit, NvU64 vaStartInternal, NvU64 vaLimitInternal, NvU32 flags);    // this
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceAlloc(struct OBJVASPACE *pVAS, NvU64 size, NvU64 align, NvU64 rangeLo, NvU64 rangeHi, NvU64 pageSizeLockMask, VAS_ALLOC_FLAGS flags, NvU64 *pAddr);    // this
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceFree(struct OBJVASPACE *pVAS, NvU64 vAddr);    // this
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceApplyDefaultAlignment(struct OBJVASPACE *pVAS, const FB_ALLOC_INFO *pAllocInfo, NvU64 *pAlign, NvU64 *pSize, NvU64 *pPageSizeLockMask);    // this
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceIncAllocRefCnt(struct OBJVASPACE *pVAS, NvU64 vAddr);    // this
NvU64 __nvoc_down_thunk_OBJIOVASPACE_vaspaceGetVaStart(struct OBJVASPACE *pVAS);    // this
NvU64 __nvoc_down_thunk_OBJIOVASPACE_vaspaceGetVaLimit(struct OBJVASPACE *pVAS);    // this
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceGetVasInfo(struct OBJVASPACE *pVAS, NV0080_CTRL_DMA_ADV_SCHED_GET_VA_CAPS_PARAMS *pParams);    // this

// Up-thunk(s) to bridge OBJIOVASPACE methods to ancestors (if any)
NvU32 __nvoc_up_thunk_OBJVASPACE_iovaspaceGetFlags(struct OBJIOVASPACE *pVAS);    // this

const struct NVOC_CLASS_DEF __nvoc_class_def_OBJIOVASPACE = 
{
    /*classInfo=*/ {
        /*size=*/               sizeof(OBJIOVASPACE),
        /*classId=*/            classId(OBJIOVASPACE),
        /*providerId=*/         &__nvoc_rtti_provider,
#if NV_PRINTF_STRINGS_ALLOWED
        /*name=*/               "OBJIOVASPACE",
#endif
    },
    /*objCreatefn=*/        (NVOC_DYNAMIC_OBJ_CREATE) &__nvoc_objCreateDynamic_OBJIOVASPACE,
    /*pCastInfo=*/          &__nvoc_castinfo__OBJIOVASPACE,
    /*pExportInfo=*/        &__nvoc_export_info__OBJIOVASPACE
};


// Metadata with per-class RTTI and vtable with ancestor(s)
static const struct NVOC_METADATA__OBJIOVASPACE __nvoc_metadata__OBJIOVASPACE = {
    .rtti.pClassDef = &__nvoc_class_def_OBJIOVASPACE,    // (iovaspace) this
    .rtti.dtor      = (NVOC_DYNAMIC_DTOR) &__nvoc_dtor_OBJIOVASPACE,
    .rtti.offset    = 0,
    .metadata__OBJVASPACE.rtti.pClassDef = &__nvoc_class_def_OBJVASPACE,    // (vaspace) super
    .metadata__OBJVASPACE.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__OBJVASPACE.rtti.offset    = NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE),
    .metadata__OBJVASPACE.metadata__Object.rtti.pClassDef = &__nvoc_class_def_Object,    // (obj) super^2
    .metadata__OBJVASPACE.metadata__Object.rtti.dtor      = &__nvoc_destructFromBase,
    .metadata__OBJVASPACE.metadata__Object.rtti.offset    = NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE.__nvoc_base_Object),

    .vtable.__iovaspaceConstruct___ = &iovaspaceConstruct__IMPL,    // virtual override (vaspace) base (vaspace)
    .metadata__OBJVASPACE.vtable.__vaspaceConstruct___ = &__nvoc_down_thunk_OBJIOVASPACE_vaspaceConstruct_,    // pure virtual
    .vtable.__iovaspaceAlloc__ = &iovaspaceAlloc_IMPL,    // virtual override (vaspace) base (vaspace)
    .metadata__OBJVASPACE.vtable.__vaspaceAlloc__ = &__nvoc_down_thunk_OBJIOVASPACE_vaspaceAlloc,    // pure virtual
    .vtable.__iovaspaceFree__ = &iovaspaceFree_IMPL,    // virtual override (vaspace) base (vaspace)
    .metadata__OBJVASPACE.vtable.__vaspaceFree__ = &__nvoc_down_thunk_OBJIOVASPACE_vaspaceFree,    // pure virtual
    .vtable.__iovaspaceApplyDefaultAlignment__ = &iovaspaceApplyDefaultAlignment_IMPL,    // virtual override (vaspace) base (vaspace)
    .metadata__OBJVASPACE.vtable.__vaspaceApplyDefaultAlignment__ = &__nvoc_down_thunk_OBJIOVASPACE_vaspaceApplyDefaultAlignment,    // pure virtual
    .vtable.__iovaspaceIncAllocRefCnt__ = &iovaspaceIncAllocRefCnt_IMPL,    // virtual override (vaspace) base (vaspace)
    .metadata__OBJVASPACE.vtable.__vaspaceIncAllocRefCnt__ = &__nvoc_down_thunk_OBJIOVASPACE_vaspaceIncAllocRefCnt,    // inline virtual body
    .vtable.__iovaspaceGetVaStart__ = &iovaspaceGetVaStart_IMPL,    // virtual override (vaspace) base (vaspace)
    .metadata__OBJVASPACE.vtable.__vaspaceGetVaStart__ = &__nvoc_down_thunk_OBJIOVASPACE_vaspaceGetVaStart,    // virtual
    .vtable.__iovaspaceGetVaLimit__ = &iovaspaceGetVaLimit_IMPL,    // virtual override (vaspace) base (vaspace)
    .metadata__OBJVASPACE.vtable.__vaspaceGetVaLimit__ = &__nvoc_down_thunk_OBJIOVASPACE_vaspaceGetVaLimit,    // virtual
    .vtable.__iovaspaceGetVasInfo__ = &iovaspaceGetVasInfo_IMPL,    // virtual override (vaspace) base (vaspace)
    .metadata__OBJVASPACE.vtable.__vaspaceGetVasInfo__ = &__nvoc_down_thunk_OBJIOVASPACE_vaspaceGetVasInfo,    // pure virtual
    .vtable.__iovaspaceGetFlags__ = &__nvoc_up_thunk_OBJVASPACE_iovaspaceGetFlags,    // inline virtual inherited (vaspace) base (vaspace) body
    .metadata__OBJVASPACE.vtable.__vaspaceGetFlags__ = &vaspaceGetFlags_edd98b,    // inline virtual body
};


// Dynamic down-casting information
const struct NVOC_CASTINFO __nvoc_castinfo__OBJIOVASPACE = {
    .numRelatives = 3,
    .relatives = {
        &__nvoc_metadata__OBJIOVASPACE.rtti,    // [0]: (iovaspace) this
        &__nvoc_metadata__OBJIOVASPACE.metadata__OBJVASPACE.rtti,    // [1]: (vaspace) super
        &__nvoc_metadata__OBJIOVASPACE.metadata__OBJVASPACE.metadata__Object.rtti,    // [2]: (obj) super^2
    }
};

// 8 down-thunk(s) defined to bridge methods in OBJIOVASPACE from superclasses

// iovaspaceConstruct_: virtual override (vaspace) base (vaspace)
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceConstruct_(struct OBJVASPACE *pVAS, NvU32 classId, NvU32 vaspaceId, NvU64 vaStart, NvU64 vaLimit, NvU64 vaStartInternal, NvU64 vaLimitInternal, NvU32 flags) {
    return iovaspaceConstruct_((struct OBJIOVASPACE *)(((unsigned char *) pVAS) - NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE)), classId, vaspaceId, vaStart, vaLimit, vaStartInternal, vaLimitInternal, flags);
}

// iovaspaceAlloc: virtual override (vaspace) base (vaspace)
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceAlloc(struct OBJVASPACE *pVAS, NvU64 size, NvU64 align, NvU64 rangeLo, NvU64 rangeHi, NvU64 pageSizeLockMask, VAS_ALLOC_FLAGS flags, NvU64 *pAddr) {
    return iovaspaceAlloc((struct OBJIOVASPACE *)(((unsigned char *) pVAS) - NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE)), size, align, rangeLo, rangeHi, pageSizeLockMask, flags, pAddr);
}

// iovaspaceFree: virtual override (vaspace) base (vaspace)
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceFree(struct OBJVASPACE *pVAS, NvU64 vAddr) {
    return iovaspaceFree((struct OBJIOVASPACE *)(((unsigned char *) pVAS) - NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE)), vAddr);
}

// iovaspaceApplyDefaultAlignment: virtual override (vaspace) base (vaspace)
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceApplyDefaultAlignment(struct OBJVASPACE *pVAS, const FB_ALLOC_INFO *pAllocInfo, NvU64 *pAlign, NvU64 *pSize, NvU64 *pPageSizeLockMask) {
    return iovaspaceApplyDefaultAlignment((struct OBJIOVASPACE *)(((unsigned char *) pVAS) - NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE)), pAllocInfo, pAlign, pSize, pPageSizeLockMask);
}

// iovaspaceIncAllocRefCnt: virtual override (vaspace) base (vaspace)
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceIncAllocRefCnt(struct OBJVASPACE *pVAS, NvU64 vAddr) {
    return iovaspaceIncAllocRefCnt((struct OBJIOVASPACE *)(((unsigned char *) pVAS) - NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE)), vAddr);
}

// iovaspaceGetVaStart: virtual override (vaspace) base (vaspace)
NvU64 __nvoc_down_thunk_OBJIOVASPACE_vaspaceGetVaStart(struct OBJVASPACE *pVAS) {
    return iovaspaceGetVaStart((struct OBJIOVASPACE *)(((unsigned char *) pVAS) - NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE)));
}

// iovaspaceGetVaLimit: virtual override (vaspace) base (vaspace)
NvU64 __nvoc_down_thunk_OBJIOVASPACE_vaspaceGetVaLimit(struct OBJVASPACE *pVAS) {
    return iovaspaceGetVaLimit((struct OBJIOVASPACE *)(((unsigned char *) pVAS) - NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE)));
}

// iovaspaceGetVasInfo: virtual override (vaspace) base (vaspace)
NV_STATUS __nvoc_down_thunk_OBJIOVASPACE_vaspaceGetVasInfo(struct OBJVASPACE *pVAS, NV0080_CTRL_DMA_ADV_SCHED_GET_VA_CAPS_PARAMS *pParams) {
    return iovaspaceGetVasInfo((struct OBJIOVASPACE *)(((unsigned char *) pVAS) - NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE)), pParams);
}


// 1 up-thunk(s) defined to bridge methods in OBJIOVASPACE to superclasses

// iovaspaceGetFlags: inline virtual inherited (vaspace) base (vaspace) body
NvU32 __nvoc_up_thunk_OBJVASPACE_iovaspaceGetFlags(struct OBJIOVASPACE *pVAS) {
    return vaspaceGetFlags((struct OBJVASPACE *)(((unsigned char *) pVAS) + NV_OFFSETOF(OBJIOVASPACE, __nvoc_base_OBJVASPACE)));
}


const struct NVOC_EXPORT_INFO __nvoc_export_info__OBJIOVASPACE = 
{
    /*numEntries=*/     0,
    /*pExportEntries=*/  0
};

void __nvoc_dtor_OBJVASPACE(OBJVASPACE*);
void __nvoc_dtor_OBJIOVASPACE(OBJIOVASPACE *pThis) {
    __nvoc_iovaspaceDestruct(pThis);
    __nvoc_dtor_OBJVASPACE(&pThis->__nvoc_base_OBJVASPACE);
    PORT_UNREFERENCED_VARIABLE(pThis);
}

void __nvoc_init_dataField_OBJIOVASPACE(OBJIOVASPACE *pThis) {
    PORT_UNREFERENCED_VARIABLE(pThis);
}

NV_STATUS __nvoc_ctor_OBJVASPACE(OBJVASPACE* );
NV_STATUS __nvoc_ctor_OBJIOVASPACE(OBJIOVASPACE *pThis) {
    NV_STATUS status = NV_OK;
    status = __nvoc_ctor_OBJVASPACE(&pThis->__nvoc_base_OBJVASPACE);
    if (status != NV_OK) goto __nvoc_ctor_OBJIOVASPACE_fail_OBJVASPACE;
    __nvoc_init_dataField_OBJIOVASPACE(pThis);
    goto __nvoc_ctor_OBJIOVASPACE_exit; // Success

__nvoc_ctor_OBJIOVASPACE_fail_OBJVASPACE:
__nvoc_ctor_OBJIOVASPACE_exit:

    return status;
}

// Vtable initialization
static void __nvoc_init_funcTable_OBJIOVASPACE_1(OBJIOVASPACE *pThis) {
    PORT_UNREFERENCED_VARIABLE(pThis);
} // End __nvoc_init_funcTable_OBJIOVASPACE_1


// Initialize vtable(s) for 9 virtual method(s).
void __nvoc_init_funcTable_OBJIOVASPACE(OBJIOVASPACE *pThis) {
    __nvoc_init_funcTable_OBJIOVASPACE_1(pThis);
}

// Initialize newly constructed object.
void __nvoc_init__OBJIOVASPACE(OBJIOVASPACE *pThis) {

    // Initialize pointers to inherited data.
    pThis->__nvoc_pbase_Object = &pThis->__nvoc_base_OBJVASPACE.__nvoc_base_Object;    // (obj) super^2
    pThis->__nvoc_pbase_OBJVASPACE = &pThis->__nvoc_base_OBJVASPACE;    // (vaspace) super
    pThis->__nvoc_pbase_OBJIOVASPACE = pThis;    // (iovaspace) this

    // Recurse to superclass initialization function(s).
    __nvoc_init__OBJVASPACE(&pThis->__nvoc_base_OBJVASPACE);

    // Pointer(s) to metadata structures(s)
    pThis->__nvoc_base_OBJVASPACE.__nvoc_base_Object.__nvoc_metadata_ptr = &__nvoc_metadata__OBJIOVASPACE.metadata__OBJVASPACE.metadata__Object;    // (obj) super^2
    pThis->__nvoc_base_OBJVASPACE.__nvoc_metadata_ptr = &__nvoc_metadata__OBJIOVASPACE.metadata__OBJVASPACE;    // (vaspace) super
    pThis->__nvoc_metadata_ptr = &__nvoc_metadata__OBJIOVASPACE;    // (iovaspace) this

    // Initialize per-object vtables.
    __nvoc_init_funcTable_OBJIOVASPACE(pThis);
}

NV_STATUS __nvoc_objCreate_OBJIOVASPACE(OBJIOVASPACE **ppThis, Dynamic *pParent, NvU32 createFlags)
{
    NV_STATUS status;
    Object *pParentObj = NULL;
    OBJIOVASPACE *pThis;

    // Assign `pThis`, allocating memory unless suppressed by flag.
    status = __nvoc_handleObjCreateMemAlloc(createFlags, sizeof(OBJIOVASPACE), (void**)&pThis, (void**)ppThis);
    if (status != NV_OK)
        return status;

    // Zero is the initial value for everything.
    portMemSet(pThis, 0, sizeof(OBJIOVASPACE));

    pThis->__nvoc_base_OBJVASPACE.__nvoc_base_Object.createFlags = createFlags;

    // Link the child into the parent if there is one unless flagged not to do so.
    if (pParent != NULL && !(createFlags & NVOC_OBJ_CREATE_FLAGS_PARENT_HALSPEC_ONLY))
    {
        pParentObj = dynamicCast(pParent, Object);
        objAddChild(pParentObj, &pThis->__nvoc_base_OBJVASPACE.__nvoc_base_Object);
    }
    else
    {
        pThis->__nvoc_base_OBJVASPACE.__nvoc_base_Object.pParent = NULL;
    }

    __nvoc_init__OBJIOVASPACE(pThis);
    status = __nvoc_ctor_OBJIOVASPACE(pThis);
    if (status != NV_OK) goto __nvoc_objCreate_OBJIOVASPACE_cleanup;

    // Assignment has no effect if NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT is set.
    *ppThis = pThis;

    return NV_OK;

__nvoc_objCreate_OBJIOVASPACE_cleanup:

    // Unlink the child from the parent if it was linked above.
    if (pParentObj != NULL)
        objRemoveChild(pParentObj, &pThis->__nvoc_base_OBJVASPACE.__nvoc_base_Object);

    // Do not call destructors here since the constructor already called them.
    if (createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(pThis, 0, sizeof(OBJIOVASPACE));
    else
    {
        portMemFree(pThis);
        *ppThis = NULL;
    }

    // coverity[leaked_storage:FALSE]
    return status;
}

NV_STATUS __nvoc_objCreateDynamic_OBJIOVASPACE(OBJIOVASPACE **ppThis, Dynamic *pParent, NvU32 createFlags, va_list args) {
    NV_STATUS status;

    status = __nvoc_objCreate_OBJIOVASPACE(ppThis, pParent, createFlags);

    return status;
}

