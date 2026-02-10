/*
 * SPDX-FileCopyrightText: Copyright (c) 2018-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "mem_mgr_internal.h"
#include "mem_mgr/console_mem.h"
#include "gpu/mem_mgr/mem_desc.h"
#include <gpu/disp/kern_disp.h>
#include <gpu/disp/console_mem/disp_console_mem.h>
#include "os/os.h"
#include "gpu/mem_mgr/mem_mgr.h"
#include "rmapi/client.h"
#include "virtualization/hypervisor/hypervisor.h"

#include "class/cl003e.h" // NV01_MEMORY_SYSTEM
#include "class/cl0040.h" // NV01_MEMORY_LOCAL_USER

static PMEMORY_DESCRIPTOR
_getDisplayConsoleMemDesc
(
    OBJGPU *pGpu
)
{
    MEMORY_DESCRIPTOR *pMemDesc = NULL;

    KernelDisplay     *pKernelDisplay = GPU_GET_KERNEL_DISPLAY(pGpu);
    DisplayConsoleMemory *pConsoleMem = KERNEL_DISPLAY_GET_CONSOLE_MEM(pKernelDisplay);

    pMemDesc = consolememGetMemDesc(pGpu, pConsoleMem);

    return pMemDesc;
}

NV_STATUS
conmemConstruct_IMPL
(
    ConsoleMemory                *pConsoleMemory,
    CALL_CONTEXT                 *pCallContext,
    RS_RES_ALLOC_PARAMS_INTERNAL *pParams
)
{
    NV_STATUS          status         = NV_OK;
    Memory            *pMemory        = staticCast(pConsoleMemory, Memory);
    OBJGPU            *pGpu           = pMemory->pGpu;
    MEMORY_DESCRIPTOR *pMemDesc       = _getDisplayConsoleMemDesc(pGpu);

    NV_ASSERT_OR_RETURN(RMCFG_FEATURE_KERNEL_RM, NV_ERR_NOT_SUPPORTED);

    // Copy-construction has already been done by the base Memory class
    if (RS_IS_COPY_CTOR(pParams))
        return NV_OK;

    if (pMemDesc == NULL)
    {
        return NV_ERR_INVALID_ARGUMENT;
    }

    NV_ASSERT(pMemDesc->Allocated == 0);
    memdescAddRef(pMemDesc);
    pMemDesc->DupCount++;

    status = memConstructCommon(pMemory, NV01_MEMORY_SYSTEM, 0, pMemDesc,
                                0, NULL, 0, 0, 0, 0, NVOS32_MEM_TAG_NONE,
                                (HWRESOURCE_INFO *)NULL);
    if (status != NV_OK)
    {
        memdescDestroy(pMemDesc);
    }
    return status;
}

NvBool
conmemCanCopy_IMPL
(
    ConsoleMemory *pConsoleMemory
)
{
    return NV_TRUE;
}
