#ifndef _G_DISP_CONSOLE_MEM_NVOC_H_
#define _G_DISP_CONSOLE_MEM_NVOC_H_
#include "nvoc/runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "g_disp_console_mem_nvoc.h"

#ifndef DISPLAY_CONSOLE_MEMORY_H
#define DISPLAY_CONSOLE_MEMORY_H

/* ------------------------ Includes --------------------------------------- */
#include "nvtypes.h"
#include "nvoc/utility.h"
#include "gpu/disp/kern_disp.h"
#include "gpu/mem_mgr/mem_desc.h"

/* ------------------------ Forward Declaration ---------------------------- */

/* ------------------------ Macros & Defines ------------------------------- */
#define KERNEL_DISPLAY_GET_CONSOLE_MEM(p)    ((p)->pConsoleMem)

/* ------------------------ Types definitions ------------------------------ */

#ifdef NVOC_DISP_CONSOLE_MEM_H_PRIVATE_ACCESS_ALLOWED
#define PRIVATE_FIELD(x) x
#else
#define PRIVATE_FIELD(x) NVOC_PRIVATE_FIELD(x)
#endif
struct DisplayConsoleMemory {
    const struct NVOC_RTTI *__nvoc_rtti;
    struct Object __nvoc_base_Object;
    struct Object *__nvoc_pbase_Object;
    struct DisplayConsoleMemory *__nvoc_pbase_DisplayConsoleMemory;
    NV_ADDRESS_SPACE consoleMemAddrSpace;
    NvU32 consoleMemAttr;
    NvU64 consoleMemBaseAddress;
    NvU64 consoleMemSize;
    MEMORY_DESCRIPTOR *pConsoleMemDesc;
    void *pConsoleMem;
};

#ifndef __NVOC_CLASS_DisplayConsoleMemory_TYPEDEF__
#define __NVOC_CLASS_DisplayConsoleMemory_TYPEDEF__
typedef struct DisplayConsoleMemory DisplayConsoleMemory;
#endif /* __NVOC_CLASS_DisplayConsoleMemory_TYPEDEF__ */

#ifndef __nvoc_class_id_DisplayConsoleMemory
#define __nvoc_class_id_DisplayConsoleMemory 0x0f96d3
#endif /* __nvoc_class_id_DisplayConsoleMemory */

extern const struct NVOC_CLASS_DEF __nvoc_class_def_DisplayConsoleMemory;

#define __staticCast_DisplayConsoleMemory(pThis) \
    ((pThis)->__nvoc_pbase_DisplayConsoleMemory)

#ifdef __nvoc_disp_console_mem_h_disabled
#define __dynamicCast_DisplayConsoleMemory(pThis) ((DisplayConsoleMemory*)NULL)
#else //__nvoc_disp_console_mem_h_disabled
#define __dynamicCast_DisplayConsoleMemory(pThis) \
    ((DisplayConsoleMemory*)__nvoc_dynamicCast(staticCast((pThis), Dynamic), classInfo(DisplayConsoleMemory)))
#endif //__nvoc_disp_console_mem_h_disabled


NV_STATUS __nvoc_objCreateDynamic_DisplayConsoleMemory(DisplayConsoleMemory**, Dynamic*, NvU32, va_list);

NV_STATUS __nvoc_objCreate_DisplayConsoleMemory(DisplayConsoleMemory**, Dynamic*, NvU32);
#define __objCreate_DisplayConsoleMemory(ppNewObj, pParent, createFlags) \
    __nvoc_objCreate_DisplayConsoleMemory((ppNewObj), staticCast((pParent), Dynamic), (createFlags))

NV_STATUS consolememConstruct_IMPL(struct DisplayConsoleMemory *arg_pConsoleMem);

#define __nvoc_consolememConstruct(arg_pConsoleMem) consolememConstruct_IMPL(arg_pConsoleMem)
void consolememDestruct_IMPL(struct DisplayConsoleMemory *pConsoleMem);

#define __nvoc_consolememDestruct(pConsoleMem) consolememDestruct_IMPL(pConsoleMem)
NV_STATUS consolememStateInitLocked_IMPL(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem);

#ifdef __nvoc_disp_console_mem_h_disabled
static inline NV_STATUS consolememStateInitLocked(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem) {
    NV_ASSERT_FAILED_PRECOMP("DisplayConsoleMemory was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_disp_console_mem_h_disabled
#define consolememStateInitLocked(pGpu, pConsoleMem) consolememStateInitLocked_IMPL(pGpu, pConsoleMem)
#endif //__nvoc_disp_console_mem_h_disabled

void consolememStateDestroy_IMPL(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem);

#ifdef __nvoc_disp_console_mem_h_disabled
static inline void consolememStateDestroy(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem) {
    NV_ASSERT_FAILED_PRECOMP("DisplayConsoleMemory was disabled!");
}
#else //__nvoc_disp_console_mem_h_disabled
#define consolememStateDestroy(pGpu, pConsoleMem) consolememStateDestroy_IMPL(pGpu, pConsoleMem)
#endif //__nvoc_disp_console_mem_h_disabled

NV_STATUS consolememStateLoad_IMPL(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem, NvU32 flags);

#ifdef __nvoc_disp_console_mem_h_disabled
static inline NV_STATUS consolememStateLoad(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("DisplayConsoleMemory was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_disp_console_mem_h_disabled
#define consolememStateLoad(pGpu, pConsoleMem, flags) consolememStateLoad_IMPL(pGpu, pConsoleMem, flags)
#endif //__nvoc_disp_console_mem_h_disabled

NV_STATUS consolememStateUnload_IMPL(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem, NvU32 flags);

#ifdef __nvoc_disp_console_mem_h_disabled
static inline NV_STATUS consolememStateUnload(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem, NvU32 flags) {
    NV_ASSERT_FAILED_PRECOMP("DisplayConsoleMemory was disabled!");
    return NV_ERR_NOT_SUPPORTED;
}
#else //__nvoc_disp_console_mem_h_disabled
#define consolememStateUnload(pGpu, pConsoleMem, flags) consolememStateUnload_IMPL(pGpu, pConsoleMem, flags)
#endif //__nvoc_disp_console_mem_h_disabled

PMEMORY_DESCRIPTOR consolememGetMemDesc_IMPL(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem);

#ifdef __nvoc_disp_console_mem_h_disabled
static inline PMEMORY_DESCRIPTOR consolememGetMemDesc(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem) {
    NV_ASSERT_FAILED_PRECOMP("DisplayConsoleMemory was disabled!");
    return NULL;
}
#else //__nvoc_disp_console_mem_h_disabled
#define consolememGetMemDesc(pGpu, pConsoleMem) consolememGetMemDesc_IMPL(pGpu, pConsoleMem)
#endif //__nvoc_disp_console_mem_h_disabled

void consolememSetMemory_IMPL(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem, NvU64 memoryRegionBaseAddress, NvU64 memoryRegionSize);

#ifdef __nvoc_disp_console_mem_h_disabled
static inline void consolememSetMemory(struct OBJGPU *pGpu, struct DisplayConsoleMemory *pConsoleMem, NvU64 memoryRegionBaseAddress, NvU64 memoryRegionSize) {
    NV_ASSERT_FAILED_PRECOMP("DisplayConsoleMemory was disabled!");
}
#else //__nvoc_disp_console_mem_h_disabled
#define consolememSetMemory(pGpu, pConsoleMem, memoryRegionBaseAddress, memoryRegionSize) consolememSetMemory_IMPL(pGpu, pConsoleMem, memoryRegionBaseAddress, memoryRegionSize)
#endif //__nvoc_disp_console_mem_h_disabled

#undef PRIVATE_FIELD


#endif // DISPLAY_CONSOLE_MEMORY_H

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _G_DISP_CONSOLE_MEM_NVOC_H_
