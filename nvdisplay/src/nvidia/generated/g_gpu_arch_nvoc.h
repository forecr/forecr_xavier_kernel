
#ifndef _G_GPU_ARCH_NVOC_H_
#define _G_GPU_ARCH_NVOC_H_

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
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "g_gpu_arch_nvoc.h"

#ifndef _GPU_ARCH_H_
#define _GPU_ARCH_H_

#include "gpu/gpu_halspec.h"
#include "nvoc/object.h"


// Private field names are wrapped in PRIVATE_FIELD, which does nothing for
// the matching C source file, but causes diagnostics to be issued if another
// source file references the field.
#ifdef NVOC_GPU_ARCH_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif


// Metadata with per-class RTTI with ancestor(s)
struct NVOC_METADATA__GpuArch;
struct NVOC_METADATA__Object;
struct NVOC_METADATA__GpuHalspecOwner;


struct GpuArch {

    // Metadata starts with RTTI structure.
    union {
         const struct NVOC_METADATA__GpuArch *__nvoc_metadata_ptr;
         const struct NVOC_RTTI *__nvoc_rtti;
    };

    // Parent (i.e. superclass or base class) objects
    struct Object __nvoc_base_Object;
    struct GpuHalspecOwner __nvoc_base_GpuHalspecOwner;

    // Ancestor object pointers for `staticCast` feature
    struct Object *__nvoc_pbase_Object;    // obj super
    struct GpuHalspecOwner *__nvoc_pbase_GpuHalspecOwner;    // gpuhalspecowner super
    struct GpuArch *__nvoc_pbase_GpuArch;    // gpuarch

    // Data members
    NvU32 chipArch;
    NvU32 chipImpl;
    NvU32 hidrev;
    TEGRA_CHIP_TYPE tegraType;
};


// Metadata with per-class RTTI with ancestor(s)
struct NVOC_METADATA__GpuArch {
    const struct NVOC_RTTI rtti;
    const struct NVOC_METADATA__Object metadata__Object;
    const struct NVOC_METADATA__GpuHalspecOwner metadata__GpuHalspecOwner;
};

#ifndef __NVOC_CLASS_GpuArch_TYPEDEF__
#define __NVOC_CLASS_GpuArch_TYPEDEF__
typedef struct GpuArch GpuArch;
#endif /* __NVOC_CLASS_GpuArch_TYPEDEF__ */

#ifndef __nvoc_class_id_GpuArch
#define __nvoc_class_id_GpuArch 0x4b33af
#endif /* __nvoc_class_id_GpuArch */

// Casting support
extern const struct NVOC_CLASS_DEF __nvoc_class_def_GpuArch;

#define __staticCast_GpuArch(pThis) \
    ((pThis)->__nvoc_pbase_GpuArch)

#ifdef __nvoc_gpu_arch_h_disabled
#define __dynamicCast_GpuArch(pThis) ((GpuArch*) NULL)
#else //__nvoc_gpu_arch_h_disabled
#define __dynamicCast_GpuArch(pThis) \
    ((GpuArch*) __nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(GpuArch)))
#endif //__nvoc_gpu_arch_h_disabled

NV_STATUS __nvoc_objCreateDynamic_GpuArch(GpuArch**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_GpuArch(GpuArch**, Dynamic*, NvU32,
        NvU32 ChipHal_arch, NvU32 ChipHal_impl, NvU32 ChipHal_hidrev,
        TEGRA_CHIP_TYPE TegraChipHal_tegraType, NvU32 arg_chipArch, NvU32 arg_chipImpl, NvU32 arg_hidrev, TEGRA_CHIP_TYPE arg_tegraType);
#define __objCreate_GpuArch(ppNewObj, pParent, createFlags, ChipHal_arch, ChipHal_impl, ChipHal_hidrev, TegraChipHal_tegraType, arg_chipArch, arg_chipImpl, arg_hidrev, arg_tegraType) \
    __nvoc_objCreate_GpuArch((ppNewObj), staticCast((pParent), Dynamic), (createFlags), ChipHal_arch, ChipHal_impl, ChipHal_hidrev, TegraChipHal_tegraType, arg_chipArch, arg_chipImpl, arg_hidrev, arg_tegraType)


// Wrapper macros

// Dispatch functions
NvU32 gpuarchGetSystemPhysAddrWidth_T234D(struct GpuArch *pGpuArch);


#ifdef __nvoc_gpu_arch_h_disabled
static inline NvU32 gpuarchGetSystemPhysAddrWidth(struct GpuArch *pGpuArch) {
    NV_ASSERT_FAILED_PRECOMP("GpuArch was disabled!");
    return 0;
}
#else //__nvoc_gpu_arch_h_disabled
#define gpuarchGetSystemPhysAddrWidth(pGpuArch) gpuarchGetSystemPhysAddrWidth_T234D(pGpuArch)
#endif //__nvoc_gpu_arch_h_disabled

#define gpuarchGetSystemPhysAddrWidth_HAL(pGpuArch) gpuarchGetSystemPhysAddrWidth(pGpuArch)

static inline NvU32 gpuarchGetDmaAddrWidth_4a4dee(struct GpuArch *pGpuArch) {
    return 0;
}


#ifdef __nvoc_gpu_arch_h_disabled
static inline NvU32 gpuarchGetDmaAddrWidth(struct GpuArch *pGpuArch) {
    NV_ASSERT_FAILED_PRECOMP("GpuArch was disabled!");
    return 0;
}
#else //__nvoc_gpu_arch_h_disabled
#define gpuarchGetDmaAddrWidth(pGpuArch) gpuarchGetDmaAddrWidth_4a4dee(pGpuArch)
#endif //__nvoc_gpu_arch_h_disabled

#define gpuarchGetDmaAddrWidth_HAL(pGpuArch) gpuarchGetDmaAddrWidth(pGpuArch)

static inline NvBool gpuarchIsZeroFb_491d52(struct GpuArch *pGpuArch) {
    return ((NvBool)(0 != 0));
}


#ifdef __nvoc_gpu_arch_h_disabled
static inline NvBool gpuarchIsZeroFb(struct GpuArch *pGpuArch) {
    NV_ASSERT_FAILED_PRECOMP("GpuArch was disabled!");
    return NV_FALSE;
}
#else //__nvoc_gpu_arch_h_disabled
#define gpuarchIsZeroFb(pGpuArch) gpuarchIsZeroFb_491d52(pGpuArch)
#endif //__nvoc_gpu_arch_h_disabled

#define gpuarchIsZeroFb_HAL(pGpuArch) gpuarchIsZeroFb(pGpuArch)

static inline NvBool gpuarchSupportsIgpuRg_491d52(struct GpuArch *pGpuArch) {
    return ((NvBool)(0 != 0));
}


#ifdef __nvoc_gpu_arch_h_disabled
static inline NvBool gpuarchSupportsIgpuRg(struct GpuArch *pGpuArch) {
    NV_ASSERT_FAILED_PRECOMP("GpuArch was disabled!");
    return NV_FALSE;
}
#else //__nvoc_gpu_arch_h_disabled
#define gpuarchSupportsIgpuRg(pGpuArch) gpuarchSupportsIgpuRg_491d52(pGpuArch)
#endif //__nvoc_gpu_arch_h_disabled

#define gpuarchSupportsIgpuRg_HAL(pGpuArch) gpuarchSupportsIgpuRg(pGpuArch)

NV_STATUS gpuarchConstruct_IMPL(struct GpuArch *arg_pGpuArch, NvU32 arg_chipArch, NvU32 arg_chipImpl, NvU32 arg_hidrev, TEGRA_CHIP_TYPE arg_tegraType);

#define __nvoc_gpuarchConstruct(arg_pGpuArch, arg_chipArch, arg_chipImpl, arg_hidrev, arg_tegraType) gpuarchConstruct_IMPL(arg_pGpuArch, arg_chipArch, arg_chipImpl, arg_hidrev, arg_tegraType)
#undef PRIVATE_FIELD


#endif // _GPU_ARCH_H_

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _G_GPU_ARCH_NVOC_H_
