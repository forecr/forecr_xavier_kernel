/*
 * SPDX-FileCopyrightText: Copyright (c) 1993-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "core/core.h"
#include "os/os.h"
#include "mem_mgr/virt_mem_mgr.h"
#include "mem_mgr/vaspace.h"
#include "core/system.h"
#include "gpu/mem_mgr/mem_desc.h"
#include "gpu_mgr/gpu_group.h"
#include "class/cl00f2.h"  // IO_VASPACE_A
#include "rmapi/rs_utils.h"
#include "gpu/device/device.h"
#include "gpu/subdevice/subdevice.h"
#include "gpu/mem_mgr/mem_mgr.h"

#include "gpu_mgr/gpu_mgr.h"

/*!
 * @brief Save client share allocation information for this device
 *
 * Save client share allocation information for this device. The
 * client share is actually allocated as a result of CliGetVASpace()
 * before the VAShare is actually used.
 *
 * @param[in]   pDevice
 * @param[in]   hClientShare        RM client specified share handle
 * @param[in]   deviceAllocFlags    Allocation flags from RM client
 *
 * @returns NV_STATUS
 */
NV_STATUS
deviceSetClientShare_IMPL
(
    Device    *pDevice,
    NvHandle   hClientShare,
    NvU64      vaSize,
    NvU64      vaStartInternal,
    NvU64      vaLimitInternal,
    NvU32      deviceAllocFlags
)
{
    pDevice->pVASpace = NULL;
    pDevice->hClientShare = hClientShare;
    pDevice->deviceAllocFlags = deviceAllocFlags;
    pDevice->deviceInternalAllocFlags = 0;
    pDevice->vaSize = vaSize;

    if (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_RESTRICT_RESERVED_VALIMITS)
    {
        pDevice->vaStartInternal = vaStartInternal;
        pDevice->vaLimitInternal = vaLimitInternal;
    }

    if ((deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_VASPACE_SIZE) && (vaSize == 0))
    {
        return NV_ERR_INVALID_ARGUMENT;
    }

    return NV_OK;
}

/*!
 * @brief Initialize the device VASPACE
 */
static NV_STATUS
deviceInitClientShare
(
    Device     *pDevice,
    NvHandle    hClientShare,
    NvU64       vaSize,
    NvU32       deviceAllocFlags,
    NvU32       deviceAllocInternalFlags
)
{
    Device      *pShareDevice;
    RsClient    *pClientShare;
    OBJVASPACE  *pVAS        = NULL;
    OBJSYS      *pSys        = SYS_GET_INSTANCE();
    OBJVMM      *pVmm        = SYS_GET_VMM(pSys);
    NV_STATUS    status;
    OBJGPU      *pGpu        = GPU_RES_GET_GPU(pDevice);
    NvU32        gpuMask      = gpumgrGetGpuMask(pGpu);
    NvU32        vaspaceClass = 0;

    pDevice->pVASpace = NULL;

    // Set broadcast state for thread
    GPU_RES_SET_THREAD_BC_STATE(pDevice);

    //
    // Share "default" behavior is defined by "share w/null", which
    // attaches to the global address space.
    //
    if (hClientShare == NV01_NULL_OBJECT)
    {
        OBJGPUGRP *pGpuGrp = gpumgrGetGpuGrpFromGpu(pGpu);
        status = gpugrpGetGlobalVASpace(pGpuGrp, &pVAS);
        NV_ASSERT_OR_RETURN(status == NV_OK, status);

        vaspaceIncRefCnt(pVAS);
        status = NV_OK;
    }

    //
    // "Force a new share" behavior is defined by "share w/myself"
    //
    else if (hClientShare == RES_GET_CLIENT_HANDLE(pDevice))
    {
        NvU32 flags = 0;
        NvU64 vaLimit;

        flags |= (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_VASPACE_SHARED_MANAGEMENT) ?
                     VASPACE_FLAGS_SHARED_MANAGEMENT : 0;

        if (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_MINIMIZE_PTETABLE_SIZE)
        {
            flags |= VASPACE_FLAGS_MINIMIZE_PTETABLE_SIZE;
        }
        if (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_RETRY_PTE_ALLOC_IN_SYS)
        {
            flags |= VASPACE_FLAGS_RETRY_PTE_ALLOC_IN_SYS;
        }
        if (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_VASPACE_SIZE)
        {
            vaLimit = pDevice->vaSize - 1;
        }
        else
        {
            vaLimit = 0;
        }

        if ( (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_VASPACE_BIG_PAGE_SIZE_64k) &&
              (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_VASPACE_BIG_PAGE_SIZE_128k) )
        {
            return NV_ERR_INVALID_ARGUMENT;
        }

        if (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_VASPACE_BIG_PAGE_SIZE_64k)
        {
            flags |= DRF_DEF(_VASPACE, _FLAGS, _BIG_PAGE_SIZE, _DEFAULT);
        }
        else if (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_VASPACE_BIG_PAGE_SIZE_128k)
        {
            flags |= DRF_DEF(_VASPACE, _FLAGS, _BIG_PAGE_SIZE, _DEFAULT);
        }
        else
        {
            // will cause it to use the default size
            flags |= DRF_DEF(_VASPACE, _FLAGS, _BIG_PAGE_SIZE, _DEFAULT);
        }

        if (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_RESTRICT_RESERVED_VALIMITS)
        {
            flags |= VASPACE_FLAGS_RESTRICTED_RM_INTERNAL_VALIMITS;
            NV_ASSERT(pDevice->vaStartInternal);
            NV_ASSERT(pDevice->vaLimitInternal);
        }
        else
        {
            NV_ASSERT(!pDevice->vaStartInternal);
            NV_ASSERT(!pDevice->vaLimitInternal);
        }

        //
        // NV_DEVICE_ALLOCATION_FLAGS_VASPACE_IS_MIRRORED will be removed once CUDA phases out
        // and uses the ctrl call  NV0080_CTRL_DMA_ENABLE_PRIVILEGED_RANGE
        // to set privileged address space
        //
        if ((deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_VASPACE_IS_MIRRORED)
           )
        {
            flags |= VASPACE_FLAGS_SET_MIRRORED;
        }
        if (NULL == GPU_GET_KERNEL_GMMU(pGpu) && (pGpu->getProperty(pGpu, PDB_PROP_GPU_TEGRA_SOC_NVDISPLAY) || IsDFPGA(pGpu)))
            vaspaceClass = IO_VASPACE_A;
        else if (vaspaceClass == 0)
        {
            NV_ASSERT(0);
            return NV_ERR_OBJECT_NOT_FOUND;
        }

        flags |= VASPACE_FLAGS_ENABLE_VMM;

        //
        // Page tables are allocated in guest subheap only inside non SRIOV guests
        // and on host RM.
        //
        if (!gpuIsSplitVasManagementServerClientRmEnabled(pGpu) ||
            !IS_VIRTUAL(pGpu))
        {
            flags |= VASPACE_FLAGS_ALLOW_PAGES_IN_PHYS_MEM_SUBALLOCATOR;
        }

        //
        // For RM unlinked SLI: the fixed offset requirement is enforced at the OBJGVASPACE
        // level during allocations and mappings, so the Device flag must be converted
        // into the internal VASPACE flag.
        //
        if (deviceAllocFlags & NV_DEVICE_ALLOCATION_FLAGS_VASPACE_REQUIRE_FIXED_OFFSET)
        {
            flags |= VASPACE_FLAGS_REQUIRE_FIXED_OFFSET;
        }

        status = vmmCreateVaspace(pVmm, vaspaceClass, 0, gpuMask, 0,
                                        vaLimit, pDevice->vaStartInternal,
                                        pDevice->vaLimitInternal, NULL, flags, &pVAS);
        if (NV_OK != status)
        {
            NV_ASSERT(0);
            return status;
        }
    }

    //
    // Try to attach to another clients VA Share.  Validate client and pull the
    // share information off the first device.
    //
    else
    {
        status = serverGetClientUnderLock(&g_resServ, hClientShare, &pClientShare);
        if (status != NV_OK)
            return status;

        //
        // If the share client doesn't have a device allocated for this GPU,
        // there's no address space to share.
        //
        status = deviceGetByInstance(pClientShare, pDevice->deviceInst, &pShareDevice);
        if (status != NV_OK)
            return status;

        // Init target share if needed
        if (pShareDevice->pVASpace == NULL)
        {
            status = deviceInitClientShare(pShareDevice,
                                           pShareDevice->hClientShare,
                                           pShareDevice->vaSize,
                                           pShareDevice->deviceAllocFlags,
                                           pShareDevice->deviceInternalAllocFlags);
            if (status != NV_OK)
                return status;
        }

        pVAS = pShareDevice->pVASpace;
        vaspaceIncRefCnt(pVAS);
    }

    pDevice->pVASpace = pVAS;
    return status;
}


/*!
 * @brief Detach this pDevice from the share group
 */
void
deviceRemoveFromClientShare_IMPL
(
    Device *pDevice
)
{
    OBJSYS *pSys = SYS_GET_INSTANCE();
    OBJVMM *pVmm = SYS_GET_VMM(pSys);

    if (pDevice->pVASpace != NULL)
    {
        vmmDestroyVaspace(pVmm, pDevice->pVASpace);
        pDevice->pVASpace = NULL;
    }
}

NV_STATUS
deviceGetDefaultVASpace_IMPL
(
    Device      *pDevice,
    OBJVASPACE **ppVAS
)
{
    NV_STATUS status = NV_OK;

    //
    // There are some cases in SLI transitions where we allocate
    // a device before the hal is initialized.
    //
    if (pDevice->pVASpace == NULL)
    {
        status = deviceInitClientShare(pDevice,
                                       pDevice->hClientShare,
                                       pDevice->vaSize,
                                       pDevice->deviceAllocFlags,
                                       pDevice->deviceInternalAllocFlags);
    }

    *ppVAS = pDevice->pVASpace;

    return status;
}

