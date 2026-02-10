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

/**************************** Instmem Routines *****************************\
*                                                                          *
*         Display console memory object function Definitions.             *
*                                                                          *
\***************************************************************************/

#define RM_STRICT_CONFIG_EMIT_DISP_ENGINE_DEFINITIONS     0

#include "gpu/disp/kern_disp.h"
#include "gpu/disp/console_mem/disp_console_mem.h"
#include "gpu/mem_mgr/mem_mgr.h"
#include "os/nv_memory_type.h"


/*! @brief Constructor */
NV_STATUS
consolememConstruct_IMPL
(
    DisplayConsoleMemory *pConsoleMem
)
{
    pConsoleMem->pConsoleMem = NULL;
    pConsoleMem->pConsoleMemDesc = NULL;

    return NV_OK;
}


/*! @brief destructor */
void
consolememDestruct_IMPL
(
    DisplayConsoleMemory *pConsoleMem
)
{
    // Free up the console mem descriptors
    memdescDestroy(pConsoleMem->pConsoleMemDesc);
    pConsoleMem->pConsoleMemDesc = NULL;
}

/*! @brief Set console memory params */
void
consolememSetMemory_IMPL
(
    OBJGPU                *pGpu,
    DisplayConsoleMemory  *pConsoleMem,
    NvU64                  memoryRegionBaseAddress,
    NvU64                  memoryRegionSize
)
{
    pConsoleMem->consoleMemBaseAddress = memoryRegionBaseAddress;
    pConsoleMem->consoleMemSize        = memoryRegionSize;
}

/*! @brief Initialize console memory descriptor */
static NV_STATUS
consolememInitMemDesc
(
    OBJGPU                *pGpu,
    DisplayConsoleMemory  *pConsoleMem
)
{
    NV_STATUS         status     = NV_OK;
    MEMORY_DESCRIPTOR *pMemDesc  = NULL;
    RmPhysAddr        *pPteArray = NULL;

    if (pGpu->getProperty(pGpu, PDB_PROP_GPU_TEGRA_SOC_NVDISPLAY))
    {
        //
        // FB reserved memory logic not be getting called for Tegra system memory scanout.
        // So as ConsoleMem Desc is not getting initialized, currently hardcoding
        // dispConsoleMemAttr to NV_MEMORY_UNCACHED this needs to be set based on system configuration/registry parameter.
        //
        pConsoleMem->consoleMemAttr      = NV_MEMORY_UNCACHED;
        pConsoleMem->consoleMemAddrSpace = ADDR_SYSMEM;
    }
    else
    {
        NV_PRINTF(LEVEL_ERROR, "DisplayConsoleMemory is not supported for non Tegra SOC Display yet.\n");
        NV_ASSERT(0);
        status = NV_ERR_NOT_SUPPORTED;
        goto exit;
    }

    switch (pConsoleMem->consoleMemAddrSpace)
    {
        default:
        case ADDR_FBMEM:
            {
                NV_PRINTF(LEVEL_ERROR, "DisplayConsoleMemory is not supported for ADDR_FBMEM address space yet.\n");
                NV_ASSERT(0);
                status = NV_ERR_NOT_SUPPORTED;
            }
            break;

        case ADDR_SYSMEM:
            {
                if (pConsoleMem->consoleMemSize > 0)
                {
                    NV_CHECK_OK_OR_GOTO(status, LEVEL_ERROR,
                        memdescCreate(&pConsoleMem->pConsoleMemDesc, pGpu,
                                      pConsoleMem->consoleMemSize,
                                      RM_PAGE_SIZE,
                                      NV_MEMORY_CONTIGUOUS, pConsoleMem->consoleMemAddrSpace,
                                      pConsoleMem->consoleMemAttr,
                                      MEMDESC_FLAGS_SKIP_RESOURCE_COMPUTE),
                        exit);

                    pMemDesc = pConsoleMem->pConsoleMemDesc;
                    memdescSetAddress(pMemDesc, NvP64_NULL);
                    memdescSetMemData(pMemDesc, NULL, NULL);
                    memdescSetFlag(pMemDesc, MEMDESC_FLAGS_SKIP_IOMMU_MAPPING, NV_TRUE);
                    pPteArray = memdescGetPteArray(pMemDesc, AT_GPU);
                    pPteArray[0] = pConsoleMem->consoleMemBaseAddress;
                }
            }
            break;
    }

exit:
    // Clean-up is handled by the caller
    return status;
}

/*! @brief Free all memory allocations done for display console memory */
static void
consolememDestroy
(
    OBJGPU                *pGpu,
    DisplayConsoleMemory  *pConsoleMem
)
{
    // Free up the console mem descriptors
    memdescDestroy(pConsoleMem->pConsoleMemDesc);
    pConsoleMem->pConsoleMemDesc = NULL;
}

NV_STATUS
consolememStateInitLocked_IMPL
(
    OBJGPU               *pGpu,
    DisplayConsoleMemory *pConsoleMem
)
{
    NV_STATUS status = NV_OK;
    // Memory descriptor has to be created in StateInit call and not in Construct call
    // because console memory region parameters are read from linux kernel after
    // construct call.
    NV_CHECK_OK_OR_GOTO(status, LEVEL_ERROR,
            consolememInitMemDesc(pGpu, pConsoleMem), exit);

exit:
    if (status != NV_OK)
    {
        consolememDestroy(pGpu, pConsoleMem);
    }

    return status;
}

void
consolememStateDestroy_IMPL
(
    OBJGPU                *pGpu,
    DisplayConsoleMemory *pConsoleMem
)
{
    consolememDestroy(pGpu, pConsoleMem);
}

NV_STATUS
consolememStateLoad_IMPL
(
    OBJGPU                *pGpu,
    DisplayConsoleMemory  *pConsoleMem,
    NvU32                  flags
)
{
    return NV_OK;
}

NV_STATUS
consolememStateUnload_IMPL
(
    OBJGPU                *pGpu,
    DisplayConsoleMemory  *pConsoleMem,
    NvU32                  flags
)
{
    return NV_OK;
}

PMEMORY_DESCRIPTOR
consolememGetMemDesc_IMPL
(
    OBJGPU        *pGpu,
    DisplayConsoleMemory *pConsoleMem
)
{
    return pConsoleMem->pConsoleMemDesc;
}
