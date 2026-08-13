#define NVOC_DISP_CONSOLE_MEM_H_PRIVATE_ACCESS_ALLOWED
#include "nvoc/runtime.h"
#include "nvoc/rtti.h"
#include "nvtypes.h"
#include "nvport/nvport.h"
#include "nvport/inline/util_valist.h"
#include "utils/nvassert.h"
#include "g_disp_console_mem_nvoc.h"

#ifdef DEBUG
char __nvoc_class_id_uniqueness_check_0x0f96d3 = 1;
#endif

extern const struct NVOC_CLASS_DEF __nvoc_class_def_DisplayConsoleMemory;

extern const struct NVOC_CLASS_DEF __nvoc_class_def_Object;

void __nvoc_init_DisplayConsoleMemory(DisplayConsoleMemory*);
void __nvoc_init_funcTable_DisplayConsoleMemory(DisplayConsoleMemory*);
NV_STATUS __nvoc_ctor_DisplayConsoleMemory(DisplayConsoleMemory*);
void __nvoc_init_dataField_DisplayConsoleMemory(DisplayConsoleMemory*);
void __nvoc_dtor_DisplayConsoleMemory(DisplayConsoleMemory*);
extern const struct NVOC_EXPORT_INFO __nvoc_export_info_DisplayConsoleMemory;

static const struct NVOC_RTTI __nvoc_rtti_DisplayConsoleMemory_DisplayConsoleMemory = {
    /*pClassDef=*/          &__nvoc_class_def_DisplayConsoleMemory,
    /*dtor=*/               (NVOC_DYNAMIC_DTOR) &__nvoc_dtor_DisplayConsoleMemory,
    /*offset=*/             0,
};

static const struct NVOC_RTTI __nvoc_rtti_DisplayConsoleMemory_Object = {
    /*pClassDef=*/          &__nvoc_class_def_Object,
    /*dtor=*/               &__nvoc_destructFromBase,
    /*offset=*/             NV_OFFSETOF(DisplayConsoleMemory, __nvoc_base_Object),
};

static const struct NVOC_CASTINFO __nvoc_castinfo_DisplayConsoleMemory = {
    /*numRelatives=*/       2,
    /*relatives=*/ {
        &__nvoc_rtti_DisplayConsoleMemory_DisplayConsoleMemory,
        &__nvoc_rtti_DisplayConsoleMemory_Object,
    },
};

const struct NVOC_CLASS_DEF __nvoc_class_def_DisplayConsoleMemory = 
{
    /*classInfo=*/ {
        /*size=*/               sizeof(DisplayConsoleMemory),
        /*classId=*/            classId(DisplayConsoleMemory),
        /*providerId=*/         &__nvoc_rtti_provider,
#if NV_PRINTF_STRINGS_ALLOWED
        /*name=*/               "DisplayConsoleMemory",
#endif
    },
    /*objCreatefn=*/        (NVOC_DYNAMIC_OBJ_CREATE) &__nvoc_objCreateDynamic_DisplayConsoleMemory,
    /*pCastInfo=*/          &__nvoc_castinfo_DisplayConsoleMemory,
    /*pExportInfo=*/        &__nvoc_export_info_DisplayConsoleMemory
};

const struct NVOC_EXPORT_INFO __nvoc_export_info_DisplayConsoleMemory = 
{
    /*numEntries=*/     0,
    /*pExportEntries=*/  0
};

void __nvoc_dtor_Object(Object*);
void __nvoc_dtor_DisplayConsoleMemory(DisplayConsoleMemory *pThis) {
    __nvoc_consolememDestruct(pThis);
    __nvoc_dtor_Object(&pThis->__nvoc_base_Object);
    PORT_UNREFERENCED_VARIABLE(pThis);
}

void __nvoc_init_dataField_DisplayConsoleMemory(DisplayConsoleMemory *pThis) {
    PORT_UNREFERENCED_VARIABLE(pThis);
}

NV_STATUS __nvoc_ctor_Object(Object* );
NV_STATUS __nvoc_ctor_DisplayConsoleMemory(DisplayConsoleMemory *pThis) {
    NV_STATUS status = NV_OK;
    status = __nvoc_ctor_Object(&pThis->__nvoc_base_Object);
    if (status != NV_OK) goto __nvoc_ctor_DisplayConsoleMemory_fail_Object;
    __nvoc_init_dataField_DisplayConsoleMemory(pThis);

    status = __nvoc_consolememConstruct(pThis);
    if (status != NV_OK) goto __nvoc_ctor_DisplayConsoleMemory_fail__init;
    goto __nvoc_ctor_DisplayConsoleMemory_exit; // Success

__nvoc_ctor_DisplayConsoleMemory_fail__init:
    __nvoc_dtor_Object(&pThis->__nvoc_base_Object);
__nvoc_ctor_DisplayConsoleMemory_fail_Object:
__nvoc_ctor_DisplayConsoleMemory_exit:

    return status;
}

static void __nvoc_init_funcTable_DisplayConsoleMemory_1(DisplayConsoleMemory *pThis) {
    PORT_UNREFERENCED_VARIABLE(pThis);
}

void __nvoc_init_funcTable_DisplayConsoleMemory(DisplayConsoleMemory *pThis) {
    __nvoc_init_funcTable_DisplayConsoleMemory_1(pThis);
}

void __nvoc_init_Object(Object*);
void __nvoc_init_DisplayConsoleMemory(DisplayConsoleMemory *pThis) {
    pThis->__nvoc_pbase_DisplayConsoleMemory = pThis;
    pThis->__nvoc_pbase_Object = &pThis->__nvoc_base_Object;
    __nvoc_init_Object(&pThis->__nvoc_base_Object);
    __nvoc_init_funcTable_DisplayConsoleMemory(pThis);
}

NV_STATUS __nvoc_objCreate_DisplayConsoleMemory(DisplayConsoleMemory **ppThis, Dynamic *pParent, NvU32 createFlags) {
    NV_STATUS status;
    Object *pParentObj;
    DisplayConsoleMemory *pThis;

    status = __nvoc_handleObjCreateMemAlloc(createFlags, sizeof(DisplayConsoleMemory), (void**)&pThis, (void**)ppThis);
    if (status != NV_OK)
        return status;

    portMemSet(pThis, 0, sizeof(DisplayConsoleMemory));

    __nvoc_initRtti(staticCast(pThis, Dynamic), &__nvoc_class_def_DisplayConsoleMemory);

    pThis->__nvoc_base_Object.createFlags = createFlags;

    if (pParent != NULL && !(createFlags & NVOC_OBJ_CREATE_FLAGS_PARENT_HALSPEC_ONLY))
    {
        pParentObj = dynamicCast(pParent, Object);
        objAddChild(pParentObj, &pThis->__nvoc_base_Object);
    }
    else
    {
        pThis->__nvoc_base_Object.pParent = NULL;
    }

    __nvoc_init_DisplayConsoleMemory(pThis);
    status = __nvoc_ctor_DisplayConsoleMemory(pThis);
    if (status != NV_OK) goto __nvoc_objCreate_DisplayConsoleMemory_cleanup;

    *ppThis = pThis;

    return NV_OK;

__nvoc_objCreate_DisplayConsoleMemory_cleanup:
    // do not call destructors here since the constructor already called them
    if (createFlags & NVOC_OBJ_CREATE_FLAGS_IN_PLACE_CONSTRUCT)
        portMemSet(pThis, 0, sizeof(DisplayConsoleMemory));
    else
        portMemFree(pThis);

    // coverity[leaked_storage:FALSE]
    return status;
}

NV_STATUS __nvoc_objCreateDynamic_DisplayConsoleMemory(DisplayConsoleMemory **ppThis, Dynamic *pParent, NvU32 createFlags, va_list args) {
    NV_STATUS status;

    status = __nvoc_objCreate_DisplayConsoleMemory(ppThis, pParent, createFlags);

    return status;
}

