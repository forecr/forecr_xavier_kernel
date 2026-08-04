
#ifndef _G_IO_VASPACE_NVOC_H_
#define _G_IO_VASPACE_NVOC_H_

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
 * SPDX-FileCopyrightText: Copyright (c) 2013-2021 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "g_io_vaspace_nvoc.h"

#ifndef _IOVASPACE_H_
#define _IOVASPACE_H_

/**************** Resource Manager Defines and Structures ******************\
*                                                                           *
* Module: IOVASPACE.H                                                       *
*       Defines and structures used for IOMMU Virtual Address Space Object. *
\***************************************************************************/

#include "mem_mgr/vaspace.h"               // base class object header

#define NV_IOVA_DOMAIN_NONE (~(NvU32)0)

typedef struct IOVAMAPPING IOVAMAPPING;
typedef struct IOVAMAPPING *PIOVAMAPPING;

// Opaque pointer for the OS layer to use
typedef struct OS_IOVA_MAPPING_DATA *POS_IOVA_MAPPING_DATA;

struct IOVAMAPPING
{
    NvU32 iovaspaceId;

    //
    // Refcount of the mapping.
    //
    // Each iovaspaceAcquireMapping() call increments the refcount, and each
    // iovaspaceReleaseMapping() call decrements it. Additionally, submappings
    // increment the refcount of their root mapping on creation and only
    // decrement it when they are destroyed.
    //
    // Mappings are destroyed when their refcount reaches 0.
    //
    // Notably a mapping can be destroyed regardless of its refcount with
    // iovaspaceDestroyMapping(). Destroying a root mapping destroys all of its
    // submappings as well.
    //
    NvU32 refcount;

    PMEMORY_DESCRIPTOR pPhysMemDesc;

    //
    // Maintain a hierarchy of IOVA mappings. The "root" mapping will generally
    // be tied to the root memory descriptor. That mapping can have submappings
    // within the same IOVA space that correspond to submemory descriptors of
    // the root memory descriptor.
    //
    // Also, the root memory descriptor may have multiple IOVA mappings (up to
    // one per IOVA space), so those need to be tracked in association directly
    // with the root memory descriptor.
    //
    // The memory descriptor (root or submemory) always points to a single IOVA
    // mapping. For root memory descriptors, that mapping is the head of a list
    // in which each mapping covers a unique IOVA space. For submemory
    // descriptors, there can only be one IOVA mapping, corresponding to the
    // IOVA space of the pGpu associated with the submemory descriptor.
    //
    union
    {
        struct IOVAMAPPING *pParent;
        struct IOVAMAPPING *pChildren;
    } link;

    //
    // For root mappings, this points to the next root mapping for the same
    // parent physical memory descriptor (e.g., a root mapping for a different
    // IOVA space).
    //
    // For submappings, this instead points to the next submapping of the
    // parent root mapping, since a submemory descriptor may only have a single
    // IOVA mapping (which is a submapping of an IOVA mapping on the root
    // memory descriptor).
    //
    struct IOVAMAPPING *pNext;

    // OS data associated with this mapping. Core RM doesn't touch this.
    POS_IOVA_MAPPING_DATA pOsData;

    //
    // If the memory is contiguous, this array consists of one element.
    // If the memory is discontiguous, this array is actually larger and has
    // one entry for each physical page in pPhysMemDesc. As a result, this
    // structure must be allocated from the heap.
    //
    RmPhysAddr iovaArray[1];
    // WARNING: DO NOT place anything behind the IOVA array!
};

/*!
 * Virtual address space for a system's IOMMU translation.
 */

// Private field names are wrapped in PRIVATE_FIELD, which does nothing for
// the matching C source file, but causes diagnostics to be issued if another
// source file references the field.
#ifdef NVOC_IO_VASPACE_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif


// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__OBJIOVASPACE;
struct NVOC_METADATA__OBJVASPACE;
struct NVOC_VTABLE__OBJIOVASPACE;


struct OBJIOVASPACE {

    // Metadata starts with RTTI structure.
    union {
         const struct NVOC_METADATA__OBJIOVASPACE *__nvoc_metadata_ptr;
         const struct NVOC_RTTI *__nvoc_rtti;
    };

    // Parent (i.e. superclass or base class) objects
    struct OBJVASPACE __nvoc_base_OBJVASPACE;

    // Ancestor object pointers for `staticCast` feature
    struct Object *__nvoc_pbase_Object;    // obj super^2
    struct OBJVASPACE *__nvoc_pbase_OBJVASPACE;    // vaspace super
    struct OBJIOVASPACE *__nvoc_pbase_OBJIOVASPACE;    // iovaspace

    // Data members
    NvU64 mappingCount;
};


// Vtable with 9 per-class function pointers
struct NVOC_VTABLE__OBJIOVASPACE {
    NV_STATUS (*__iovaspaceConstruct___)(struct OBJIOVASPACE * /*this*/, NvU32, NvU32, NvU64, NvU64, NvU64, NvU64, NvU32);  // virtual override (vaspace) base (vaspace)
    NV_STATUS (*__iovaspaceAlloc__)(struct OBJIOVASPACE * /*this*/, NvU64, NvU64, NvU64, NvU64, NvU64, VAS_ALLOC_FLAGS, NvU64 *);  // virtual override (vaspace) base (vaspace)
    NV_STATUS (*__iovaspaceFree__)(struct OBJIOVASPACE * /*this*/, NvU64);  // virtual override (vaspace) base (vaspace)
    NV_STATUS (*__iovaspaceApplyDefaultAlignment__)(struct OBJIOVASPACE * /*this*/, const FB_ALLOC_INFO *, NvU64 *, NvU64 *, NvU64 *);  // virtual override (vaspace) base (vaspace)
    NV_STATUS (*__iovaspaceIncAllocRefCnt__)(struct OBJIOVASPACE * /*this*/, NvU64);  // virtual override (vaspace) base (vaspace)
    NvU64 (*__iovaspaceGetVaStart__)(struct OBJIOVASPACE * /*this*/);  // virtual override (vaspace) base (vaspace)
    NvU64 (*__iovaspaceGetVaLimit__)(struct OBJIOVASPACE * /*this*/);  // virtual override (vaspace) base (vaspace)
    NV_STATUS (*__iovaspaceGetVasInfo__)(struct OBJIOVASPACE * /*this*/, NV0080_CTRL_DMA_ADV_SCHED_GET_VA_CAPS_PARAMS *);  // virtual override (vaspace) base (vaspace)
    NvU32 (*__iovaspaceGetFlags__)(struct OBJIOVASPACE * /*this*/);  // inline virtual inherited (vaspace) base (vaspace) body
};

// Metadata with per-class RTTI and vtable with ancestor(s)
struct NVOC_METADATA__OBJIOVASPACE {
    const struct NVOC_RTTI rtti;
    const struct NVOC_METADATA__OBJVASPACE metadata__OBJVASPACE;
    const struct NVOC_VTABLE__OBJIOVASPACE vtable;
};

#ifndef __NVOC_CLASS_OBJIOVASPACE_TYPEDEF__
#define __NVOC_CLASS_OBJIOVASPACE_TYPEDEF__
typedef struct OBJIOVASPACE OBJIOVASPACE;
#endif /* __NVOC_CLASS_OBJIOVASPACE_TYPEDEF__ */

#ifndef __nvoc_class_id_OBJIOVASPACE
#define __nvoc_class_id_OBJIOVASPACE 0x28ed9c
#endif /* __nvoc_class_id_OBJIOVASPACE */

// Casting support
extern const struct NVOC_CLASS_DEF __nvoc_class_def_OBJIOVASPACE;

#define __staticCast_OBJIOVASPACE(pThis) \
    ((pThis)->__nvoc_pbase_OBJIOVASPACE)

#ifdef __nvoc_io_vaspace_h_disabled
#define __dynamicCast_OBJIOVASPACE(pThis) ((OBJIOVASPACE*) NULL)
#else //__nvoc_io_vaspace_h_disabled
#define __dynamicCast_OBJIOVASPACE(pThis) \
    ((OBJIOVASPACE*) __nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(OBJIOVASPACE)))
#endif //__nvoc_io_vaspace_h_disabled

NV_STATUS __nvoc_objCreateDynamic_OBJIOVASPACE(OBJIOVASPACE**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_OBJIOVASPACE(OBJIOVASPACE**, Dynamic*, NvU32);
#define __objCreate_OBJIOVASPACE(ppNewObj, pParent, createFlags) \
    __nvoc_objCreate_OBJIOVASPACE((ppNewObj), staticCast((pParent), Dynamic), (createFlags))


// Wrapper macros
#define iovaspaceConstruct__FNPTR(pVAS) pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceConstruct___
#define iovaspaceConstruct_(pVAS, classId, vaspaceId, vaStart, vaLimit, vaStartInternal, vaLimitInternal, flags) iovaspaceConstruct__DISPATCH(pVAS, classId, vaspaceId, vaStart, vaLimit, vaStartInternal, vaLimitInternal, flags)
#define iovaspaceAlloc_FNPTR(pVAS) pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceAlloc__
#define iovaspaceAlloc(pVAS, size, align, rangeLo, rangeHi, pageSizeLockMask, flags, pAddr) iovaspaceAlloc_DISPATCH(pVAS, size, align, rangeLo, rangeHi, pageSizeLockMask, flags, pAddr)
#define iovaspaceFree_FNPTR(pVAS) pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceFree__
#define iovaspaceFree(pVAS, vAddr) iovaspaceFree_DISPATCH(pVAS, vAddr)
#define iovaspaceApplyDefaultAlignment_FNPTR(pVAS) pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceApplyDefaultAlignment__
#define iovaspaceApplyDefaultAlignment(pVAS, pAllocInfo, pAlign, pSize, pPageSizeLockMask) iovaspaceApplyDefaultAlignment_DISPATCH(pVAS, pAllocInfo, pAlign, pSize, pPageSizeLockMask)
#define iovaspaceIncAllocRefCnt_FNPTR(pVAS) pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceIncAllocRefCnt__
#define iovaspaceIncAllocRefCnt(pVAS, vAddr) iovaspaceIncAllocRefCnt_DISPATCH(pVAS, vAddr)
#define iovaspaceGetVaStart_FNPTR(pVAS) pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceGetVaStart__
#define iovaspaceGetVaStart(pVAS) iovaspaceGetVaStart_DISPATCH(pVAS)
#define iovaspaceGetVaLimit_FNPTR(pVAS) pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceGetVaLimit__
#define iovaspaceGetVaLimit(pVAS) iovaspaceGetVaLimit_DISPATCH(pVAS)
#define iovaspaceGetVasInfo_FNPTR(pVAS) pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceGetVasInfo__
#define iovaspaceGetVasInfo(pVAS, pParams) iovaspaceGetVasInfo_DISPATCH(pVAS, pParams)
#define iovaspaceGetFlags_FNPTR(pVAS) pVAS->__nvoc_base_OBJVASPACE.__nvoc_metadata_ptr->vtable.__vaspaceGetFlags__
#define iovaspaceGetFlags(pVAS) iovaspaceGetFlags_DISPATCH(pVAS)

// Dispatch functions
static inline NV_STATUS iovaspaceConstruct__DISPATCH(struct OBJIOVASPACE *pVAS, NvU32 classId, NvU32 vaspaceId, NvU64 vaStart, NvU64 vaLimit, NvU64 vaStartInternal, NvU64 vaLimitInternal, NvU32 flags) {
    return pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceConstruct___(pVAS, classId, vaspaceId, vaStart, vaLimit, vaStartInternal, vaLimitInternal, flags);
}

static inline NV_STATUS iovaspaceAlloc_DISPATCH(struct OBJIOVASPACE *pVAS, NvU64 size, NvU64 align, NvU64 rangeLo, NvU64 rangeHi, NvU64 pageSizeLockMask, VAS_ALLOC_FLAGS flags, NvU64 *pAddr) {
    return pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceAlloc__(pVAS, size, align, rangeLo, rangeHi, pageSizeLockMask, flags, pAddr);
}

static inline NV_STATUS iovaspaceFree_DISPATCH(struct OBJIOVASPACE *pVAS, NvU64 vAddr) {
    return pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceFree__(pVAS, vAddr);
}

static inline NV_STATUS iovaspaceApplyDefaultAlignment_DISPATCH(struct OBJIOVASPACE *pVAS, const FB_ALLOC_INFO *pAllocInfo, NvU64 *pAlign, NvU64 *pSize, NvU64 *pPageSizeLockMask) {
    return pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceApplyDefaultAlignment__(pVAS, pAllocInfo, pAlign, pSize, pPageSizeLockMask);
}

static inline NV_STATUS iovaspaceIncAllocRefCnt_DISPATCH(struct OBJIOVASPACE *pVAS, NvU64 vAddr) {
    return pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceIncAllocRefCnt__(pVAS, vAddr);
}

static inline NvU64 iovaspaceGetVaStart_DISPATCH(struct OBJIOVASPACE *pVAS) {
    return pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceGetVaStart__(pVAS);
}

static inline NvU64 iovaspaceGetVaLimit_DISPATCH(struct OBJIOVASPACE *pVAS) {
    return pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceGetVaLimit__(pVAS);
}

static inline NV_STATUS iovaspaceGetVasInfo_DISPATCH(struct OBJIOVASPACE *pVAS, NV0080_CTRL_DMA_ADV_SCHED_GET_VA_CAPS_PARAMS *pParams) {
    return pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceGetVasInfo__(pVAS, pParams);
}

static inline NvU32 iovaspaceGetFlags_DISPATCH(struct OBJIOVASPACE *pVAS) {
    return pVAS->__nvoc_metadata_ptr->vtable.__iovaspaceGetFlags__(pVAS);
}

NV_STATUS iovaspaceConstruct__IMPL(struct OBJIOVASPACE *pVAS, NvU32 classId, NvU32 vaspaceId, NvU64 vaStart, NvU64 vaLimit, NvU64 vaStartInternal, NvU64 vaLimitInternal, NvU32 flags);

NV_STATUS iovaspaceAlloc_IMPL(struct OBJIOVASPACE *pVAS, NvU64 size, NvU64 align, NvU64 rangeLo, NvU64 rangeHi, NvU64 pageSizeLockMask, VAS_ALLOC_FLAGS flags, NvU64 *pAddr);

NV_STATUS iovaspaceFree_IMPL(struct OBJIOVASPACE *pVAS, NvU64 vAddr);

NV_STATUS iovaspaceApplyDefaultAlignment_IMPL(struct OBJIOVASPACE *pVAS, const FB_ALLOC_INFO *pAllocInfo, NvU64 *pAlign, NvU64 *pSize, NvU64 *pPageSizeLockMask);

NV_STATUS iovaspaceIncAllocRefCnt_IMPL(struct OBJIOVASPACE *pVAS, NvU64 vAddr);

NvU64 iovaspaceGetVaStart_IMPL(struct OBJIOVASPACE *pVAS);

NvU64 iovaspaceGetVaLimit_IMPL(struct OBJIOVASPACE *pVAS);

NV_STATUS iovaspaceGetVasInfo_IMPL(struct OBJIOVASPACE *pVAS, NV0080_CTRL_DMA_ADV_SCHED_GET_VA_CAPS_PARAMS *pParams);

void iovaspaceDestruct_IMPL(struct OBJIOVASPACE *pIOVAS);

#define __nvoc_iovaspaceDestruct(pIOVAS) iovaspaceDestruct_IMPL(pIOVAS)
NV_STATUS iovaspaceAcquireMapping_IMPL(struct OBJIOVASPACE *pIOVAS, PMEMORY_DESCRIPTOR pIovaMapping);

#ifdef __nvoc_io_vaspace_h_disabled
static inline NV_STATUS iovaspaceAcquireMapping(struct OBJIOVASPACE *pIOVAS, PMEMORY_DESCRIPTOR pIovaMapping) {
    NV_ASSERT_FAILED_PRECOMP("OBJIOVASPACE was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_io_vaspace_h_disabled
#define iovaspaceAcquireMapping(pIOVAS, pIovaMapping) iovaspaceAcquireMapping_IMPL(pIOVAS, pIovaMapping)
#endif //__nvoc_io_vaspace_h_disabled

void iovaspaceReleaseMapping_IMPL(struct OBJIOVASPACE *pIOVAS, PIOVAMAPPING pIovaMapping);

#ifdef __nvoc_io_vaspace_h_disabled
static inline void iovaspaceReleaseMapping(struct OBJIOVASPACE *pIOVAS, PIOVAMAPPING pIovaMapping) {
    NV_ASSERT_FAILED_PRECOMP("OBJIOVASPACE was disabled!");
}
#else //__nvoc_io_vaspace_h_disabled
#define iovaspaceReleaseMapping(pIOVAS, pIovaMapping) iovaspaceReleaseMapping_IMPL(pIOVAS, pIovaMapping)
#endif //__nvoc_io_vaspace_h_disabled

void iovaspaceDestroyMapping_IMPL(struct OBJIOVASPACE *pIOVAS, PIOVAMAPPING pIovaMapping);

#ifdef __nvoc_io_vaspace_h_disabled
static inline void iovaspaceDestroyMapping(struct OBJIOVASPACE *pIOVAS, PIOVAMAPPING pIovaMapping) {
    NV_ASSERT_FAILED_PRECOMP("OBJIOVASPACE was disabled!");
}
#else //__nvoc_io_vaspace_h_disabled
#define iovaspaceDestroyMapping(pIOVAS, pIovaMapping) iovaspaceDestroyMapping_IMPL(pIOVAS, pIovaMapping)
#endif //__nvoc_io_vaspace_h_disabled

#undef PRIVATE_FIELD


struct OBJIOVASPACE* iovaspaceFromId(NvU32 iovaspaceId);
struct OBJIOVASPACE* iovaspaceFromMapping(PIOVAMAPPING pIovaMapping);

//
// Helper that looks up the IOVAS from the mapping and then calls
// iovaspaceDestroyMapping().
//
void iovaMappingDestroy(PIOVAMAPPING pIovaMapping);

#endif // _IOVASPACE_H_

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _G_IO_VASPACE_NVOC_H_
