/*
 * SPDX-FileCopyrightText: Copyright (c) 2004-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

/*!
 * @file
 * @brief This module contains the gpu control interfaces for the
 *        subdevice (NV20_SUBDEVICE_0) class. Subdevice-level control calls
 *        are directed unicast to the associated GPU.
 *        File contains ctrls related to general GPU
 */

#include "core/core.h"
#include "core/locks.h"
#include "gpu/subdevice/subdevice.h"
#include "gpu/gpu.h"
#include "gpu_mgr/gpu_db.h"
#include "nvrm_registry.h"
#include "nvVer.h"
#include "gpu/disp/kern_disp.h"
#include "gpu/timer/objtmr.h"
#include "vgpu/rpc.h"
#include "gpu/mem_mgr/mem_mgr.h"
#include "virtualization/hypervisor/hypervisor.h"
#include "rmapi/resource_fwd_decls.h"
#include "rmapi/client.h"

#include "class/cl00de.h"
#include "class/cl900e.h"
#include "ctrl/ctrl2080/ctrl2080thermal.h"


#include "g_finn_rm_api.h"

#define RPC_STRUCTURES
#define RPC_GENERIC_UNION
#include "g_rpc-structures.h"
#undef RPC_STRUCTURES
#undef RPC_GENERIC_UNION

#define RPC_MESSAGE_STRUCTURES
#include "g_rpc-message-header.h"
#undef RPC_MESSAGE_STRUCTURES

// bit to set when telling physical to fill in an info entry
#define INDEX_FORWARD_TO_PHYSICAL 0x80000000
ct_assert(INDEX_FORWARD_TO_PHYSICAL == DRF_NUM(2080, _CTRL_GPU_INFO_INDEX, _RESERVED, 1));


static NV_STATUS
getGpuInfos(Subdevice *pSubdevice, NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pParams, NvBool bCanAccessHw)
{
    return NV_OK;
}

#undef INDEX_FORWARD_TO_PHYSICAL

NV_STATUS
subdeviceCtrlCmdGpuGetInfoV2_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams
)
{
    return getGpuInfos(pSubdevice, pGpuInfoParams, NV_TRUE);
}

NV_STATUS
subdeviceCtrlCmdGpuGetVfCaps_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_VF_CAPS_PARAMS *pParams
)
{
    return NV_ERR_NOT_SUPPORTED;
}

//
// subdeviceCtrlCmdGpuGetCachedInfo: As subdeviceCtrlCmdGpuGetInfoV2, except
// does not perform any HW access (NO_GPUS_ACCESS and NO_GPUS_LOCK flags)
//
NV_STATUS
subdeviceCtrlCmdGpuGetCachedInfo_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_INFO_V2_PARAMS *pGpuInfoParams
)
{
    return getGpuInfos(pSubdevice, pGpuInfoParams, NV_FALSE);
}

/*!
 * @brief This command can be used for Optimus enabled system.
 *
 * @return :
 *    NV_OK
 */
NV_STATUS
subdeviceCtrlCmdGpuSetOptimusInfo_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_OPTIMUS_INFO_PARAMS *pGpuOptimusInfoParams
)
{
    NvU32   status  =   NV_OK;
    OBJGPU *pGpu    =   GPU_RES_GET_GPU(pSubdevice);

    if (pGpuOptimusInfoParams->isOptimusEnabled)
    {
        //
        // Setting pMemoryManager->bPersistentStandbyBuffer for Optimus system.
        // It is used for sys_mem allocation which is pinned across
        // S3 transitions.Sys_mem allocations are done at first S3 cycle
        // and release during driver unload, which reduces system
        // VM fragmentation, which was a problem in optimus system.
        // For more details refer bug 754122.
        //
        GPU_GET_MEMORY_MANAGER(pGpu)->bPersistentStandbyBuffer = NV_TRUE;
    }
    return status;
}

//
// subdeviceCtrlCmdGpuGetSdm
//
// Lock Requirements:
//      Assert that API lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetSdm_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_SDM_PARAMS *pSdmParams
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    pSdmParams->subdeviceMask = gpuGetSubdeviceMask(pGpu);

    return NV_OK;
}

//
// subdeviceCtrlCmdGpuGetSimulationInfo
//
// Lock Requirements:
//      Assert that API lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetSimulationInfo_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_SIMULATION_INFO_PARAMS *pGpuSimulationInfoParams
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    if (IS_SILICON(pGpu))
    {
        pGpuSimulationInfoParams->type = NV2080_CTRL_GPU_GET_SIMULATION_INFO_TYPE_NONE;
    }
    else
    {
        pGpuSimulationInfoParams->type = NV2080_CTRL_GPU_GET_SIMULATION_INFO_TYPE_UNKNOWN;
    }

    return NV_OK;
}

//
// subdeviceCtrlCmdGpuGetEngines
//
// Lock Requirements:
//      Assert that API lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetEngines_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_ENGINES_PARAMS *pParams
)
{
    NV2080_CTRL_GPU_GET_ENGINES_V2_PARAMS getEngineParamsV2;
    NvU32    *pKernelEngineList = NvP64_VALUE(pParams->engineList);
    NV_STATUS status = NV_OK;

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    portMemSet(&getEngineParamsV2, 0, sizeof(getEngineParamsV2));

    status = subdeviceCtrlCmdGpuGetEnginesV2(pSubdevice, &getEngineParamsV2);
    NV_CHECK_OR_RETURN(LEVEL_INFO, NV_OK == status, status);

    // NULL clients just want an engine count
    if (NULL != pKernelEngineList)
    {
        NV_CHECK_OR_RETURN(LEVEL_INFO, pParams->engineCount >= getEngineParamsV2.engineCount,
                         NV_ERR_BUFFER_TOO_SMALL);
        portMemCopy(pKernelEngineList,
                    getEngineParamsV2.engineCount * sizeof(*getEngineParamsV2.engineList), getEngineParamsV2.engineList,
                    getEngineParamsV2.engineCount * sizeof(*getEngineParamsV2.engineList));
    }

    pParams->engineCount = getEngineParamsV2.engineCount;

    return status;
}

//
// subdeviceCtrlCmdGpuGetEnginesV2
//
// Lock Requirements:
//      Assert that API lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetEnginesV2_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_ENGINES_V2_PARAMS *pEngineParams
)
{
    OBJGPU   *pGpu = GPU_RES_GET_GPU(pSubdevice);
    NV_STATUS status = NV_OK;

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    // Update the engine Database
    NV_ASSERT_OK_OR_RETURN(gpuUpdateEngineTable(pGpu));

    // Validate engine count
    if (pGpu->engineDB.size > NV2080_GPU_MAX_ENGINES_LIST_SIZE)
    {
        NV_PRINTF(LEVEL_ERROR, "The engine database's size (0x%x) exceeds NV2080_GPU_MAX_ENGINES_LIST_SIZE (0x%x)!\n",
                  pGpu->engineDB.size, NV2080_GPU_MAX_ENGINES_LIST_SIZE);
        DBG_BREAKPOINT();
        return NV_ERR_INVALID_STATE;
    }

    {
        NvU32 i;
        pEngineParams->engineCount = pGpu->engineDB.size;
        for (i = 0; i < pEngineParams->engineCount; i++)
        {
            pEngineParams->engineList[i] = gpuGetNv2080EngineType(pGpu->engineDB.pType[i]);
        }
    }

    return status;
}

//
// subdeviceCtrlCmdGpuGetEngineClasslist
//
// Lock Requirements:
//      Assert that API lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetEngineClasslist_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_ENGINE_CLASSLIST_PARAMS *pClassParams
)
{
    OBJGPU       *pGpu = GPU_RES_GET_GPU(pSubdevice);
    ENGDESCRIPTOR engDesc;
    NV_STATUS     status = NV_OK;
    RM_ENGINE_TYPE rmEngineType = gpuGetRmEngineType(pClassParams->engineType);

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    {
    }

    status = gpuXlateClientEngineIdToEngDesc(pGpu, rmEngineType, &engDesc);

    NV_ASSERT(status == NV_OK);

    if (status != NV_OK)
    {
        NV_PRINTF(LEVEL_ERROR,
                  "NV2080_CTRL_CMD_GPU_GET_ENGINE_CLASSLIST Invalid engine ID 0x%x\n",
                  pClassParams->engineType);
        DBG_BREAKPOINT();
        return status;
    }

    status = gpuGetClassList(pGpu, &pClassParams->numClasses, NvP64_VALUE(pClassParams->classList), engDesc);

    if (status != NV_OK)
    {
        NV_PRINTF(LEVEL_ERROR,
                  "NV2080_CTRL_CMD_GPU_GET_ENGINE_CLASSLIST Class List query failed\n");
    }

    return status;
}

//
// subdeviceCtrlCmdGpuGetEnginePartnerList
//
// Lock Requirements:
//      Assert that API lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetEnginePartnerList_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_ENGINE_PARTNERLIST_PARAMS *pPartnerListParams
)
{
    OBJGPU          *pGpu = GPU_RES_GET_GPU(pSubdevice);
    ENGDESCRIPTOR    engDesc;
    NvU32            nv2080EngineType;
    RM_ENGINE_TYPE   rmEngineType;
    NvU32            i;
    CLASSDESCRIPTOR *pClass;
    NV_STATUS        status = NV_OK;

    pPartnerListParams->numPartners = 0;

    rmEngineType = gpuGetRmEngineType(pPartnerListParams->engineType);

    status = gpuXlateClientEngineIdToEngDesc(pGpu, rmEngineType, &engDesc);
    if (NV_OK != status)
    {
        NV_PRINTF(LEVEL_ERROR, "Invalid engine ID 0x%x (0x%x)\n",
                  pPartnerListParams->engineType, rmEngineType);
        return status;
    }

    // find class in class db
    status = gpuGetClassByClassId(pGpu, pPartnerListParams->partnershipClassId, &pClass);
    if (NV_OK != status)
    {
        NV_PRINTF(LEVEL_ERROR, "Invalid class ID 0x%x\n",
                  pPartnerListParams->partnershipClassId);
        return status;
    }

    // Make sure that the engine related to this class is FIFO...
    if (pClass->engDesc != ENG_KERNEL_FIFO)
    {
        NV_PRINTF(LEVEL_ERROR,
                  "Class 0x%x is not considered a partnership class.\n",
                  pPartnerListParams->partnershipClassId);
        return NV_ERR_NOT_SUPPORTED;
    }

    nv2080EngineType = pPartnerListParams->engineType;

    // Translate the instance-local engine type to the global engine type in MIG mode

    // Restore the client's passed engineType
    pPartnerListParams->engineType = nv2080EngineType;

    //
    // For channels that the hal didn't handle, we should just return
    // all of the supported engines except for the target engine.
    //

    // Update the engine Database
    NV_ASSERT_OK_OR_RETURN(gpuUpdateEngineTable(pGpu));

    // Make sure it all will fit
    if (pGpu->engineDB.size > NV2080_CTRL_GPU_MAX_ENGINE_PARTNERS)
    {
        NV_PRINTF(LEVEL_ERROR,
                  "partnerList space is too small, time to increase. This is fatal\n");
        DBG_BREAKPOINT();
        return status;
    }

    // Copy over all of the engines except the target
    for (i = 0; i < pGpu->engineDB.size; i++)
    {
        nv2080EngineType = gpuGetNv2080EngineType(pGpu->engineDB.pType[i]);

        // Skip the engine handed in
        if (nv2080EngineType != pPartnerListParams->engineType )
        {
            pPartnerListParams->partnerList[pPartnerListParams->numPartners++] = nv2080EngineType;
        }
    }

    return status;
}

//
// subdeviceCtrlCmdGpuQueryMode_IMPL
//
// Lock Requirements:
//      Assert that API and GPUs lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuQueryMode_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_QUERY_MODE_PARAMS *pQueryMode
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner() && rmGpuLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    switch (gpuGetMode(pGpu))
    {
        case NV_GPU_MODE_GRAPHICS_MODE:
        {
            pQueryMode->mode = NV2080_CTRL_GPU_QUERY_MODE_GRAPHICS_MODE;
            break;
        }
        case NV_GPU_MODE_COMPUTE_MODE:
        {
            pQueryMode->mode = NV2080_CTRL_GPU_QUERY_MODE_COMPUTE_MODE;
            break;
        }
        default:
        {
            pQueryMode->mode = NV2080_CTRL_GPU_QUERY_MODE_UNKNOWN_MODE;
            break;
        }
    }

    return NV_OK;
}

//
// subdeviceCtrlCmdGpuHandleGpuSR
//
// Lock Requirements:
//      Assert that API lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuHandleGpuSR_IMPL
(
    Subdevice *pSubdevice
)
{
    return NV_OK;
}

//
// subdeviceCtrlCmdGpuGetId
//
// Lock Requirements:
//      Assert that API lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetId_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_ID_PARAMS *pIdParams
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    pIdParams->gpuId = pGpu->gpuId;

    return NV_OK;
}

//
// nv2080CtrlCmdGpuGetPids
//
// Lock Requirements:
//      Assert that API and GPUs lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetPids_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_PIDS_PARAMS *pGetPidsParams
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);
    NvU32 internalClassId;
    NV_STATUS status;
    MIG_INSTANCE_REF *pRef = NULL;

    NV_ASSERT_OR_RETURN(RMCFG_FEATURE_KERNEL_RM, NV_ERR_NOT_SUPPORTED);

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner() && rmGpuLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    switch (pGetPidsParams->idType)
    {
        case (NV2080_CTRL_GPU_GET_PIDS_ID_TYPE_CLASS):
        {
            if (pGetPidsParams->id == NV20_SUBDEVICE_0)
            {
                internalClassId = classId(Subdevice);
            }
            else if (pGetPidsParams->id == MPS_COMPUTE)
            {
                internalClassId = classId(MpsApi);
            }
            else
            {
                internalClassId = classId(ChannelDescendant);
            }
            break;
        }

        default:
            return NV_ERR_INVALID_ARGUMENT;
    }

    //
    // Search over all clients to see if any contain objects of type = id.
    // If they do, then add their PID to the PIDArray param and also
    // return the amount of valid entries in the Array through pidTblCount.
    //
    status = gpuGetProcWithObject(pGpu, pGetPidsParams->id, internalClassId,
                                  pGetPidsParams->pidTbl, &pGetPidsParams->pidTblCount,
                                  pRef);
    return status;
}

//
// subdeviceCtrlCmdGpuGetPidInfo
//
// Lock Requirements:
//      Assert that API and GPUs lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetPidInfo_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_PID_INFO_PARAMS *pGetPidInfoParams
)
{
    NV2080_CTRL_GPU_PID_INFO_DATA *pPidInfoData;
    NV2080_CTRL_SMC_SUBSCRIPTION_INFO *pSmcInfo;
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);
    NV2080_CTRL_GPU_PID_INFO *pPidInfo;
    NvU32 internalClassId;
    NvU32 i;
    MIG_INSTANCE_REF *pRef = NULL;
    NvBool bGlobalInfo = NV_TRUE;

    NV_ASSERT_OR_RETURN(RMCFG_FEATURE_KERNEL_RM, NV_ERR_NOT_SUPPORTED);

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner() && rmGpuLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    if ((pGetPidInfoParams->pidInfoListCount <= 0) ||
        (pGetPidInfoParams->pidInfoListCount >
         NV2080_CTRL_GPU_GET_PID_INFO_MAX_COUNT))
    {
        return NV_ERR_INVALID_ARGUMENT;
    }

    for (i = 0; i < pGetPidInfoParams->pidInfoListCount; ++i)
    {
        pPidInfo = &pGetPidInfoParams->pidInfoList[i];

        pSmcInfo = &pPidInfo->smcSubscription;
        pSmcInfo->computeInstanceId = PARTITIONID_INVALID;
        pSmcInfo->gpuInstanceId = PARTITIONID_INVALID;

        switch (pPidInfo->index)
        {
            case (NV2080_CTRL_GPU_PID_INFO_INDEX_VIDEO_MEMORY_USAGE):
            {
                internalClassId = classId(Memory);

                pPidInfoData = &pPidInfo->data;
                portMemSet(pPidInfoData, 0, sizeof(NV2080_CTRL_GPU_PID_INFO_DATA));
                pPidInfo->result = gpuFindClientInfoWithPidIterator(pGpu, pPidInfo->pid, 0,
                                                                    internalClassId,
                                                                    pPidInfoData,
                                                                    pSmcInfo,
                                                                    pRef,
                                                                    bGlobalInfo);
                break;
            }
            default:
            {
                pPidInfo->result = NV_ERR_INVALID_ARGUMENT;
                break;
            }
        }
    }

    return NV_OK;
}

NV_STATUS
subdeviceCtrlCmdGpuGetMaxSupportedPageSize_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_MAX_SUPPORTED_PAGE_SIZE_PARAMS *pParams
)
{
    OBJGPU   *pGpu = GPU_RES_GET_GPU(pSubdevice);
    NV_STATUS status  = NV_OK;

    NV_ASSERT_OR_RETURN((rmapiLockIsOwner() && rmDeviceGpuLockIsOwner(pGpu->gpuInstance)) || rmapiInRtd3PmPath(),
        NV_ERR_INVALID_LOCK_STATE);

    // Default to minimal page size (4k)
    pParams->maxSupportedPageSize = RM_PAGE_SIZE;

    if (gpuIsSriovEnabled(pGpu) || IS_VIRTUAL_WITH_SRIOV(pGpu)
        )
    {
        NvU64 vmmuSegmentSize = gpuGetVmmuSegmentSize(pGpu);
        if (vmmuSegmentSize > 0 &&
            vmmuSegmentSize < NV2080_CTRL_GPU_VMMU_SEGMENT_SIZE_1024MB)
        {
            pParams->maxSupportedPageSize = RM_PAGE_SIZE_HUGE;
        }
    }

    return status;
}

/*!
 * @brief Check if address range is within the provided limits
 *
 * @param[in]  addrStart       Staring address of address range
 * @param[in]  addrLength      Size of address range
 * @param[in]  limitStart      Staring address of limit
 * @param[in]  limitLength     Size of limit
 *
 * @return
 *     NV_TRUE, if address range is within the provided limits
 *     NV_FALSE, if address range is outside the provided limits
 *
 */
static NvBool isAddressWithinLimits
(
    NvU64 addrStart,
    NvU64 addrLength,
    NvU64 limitStart,
    NvU64 limitLength
)
{
    NvU64 addrEnd  = 0;
    NvU64 limitEnd = 0;

    //
    // Calculate End address of address range and limit,
    // Return NV_FALSE in case of 64-bit addition overflow
    //
    if (!portSafeAddU64(addrStart, addrLength - 1, &addrEnd) ||
        !portSafeAddU64(limitStart, limitLength - 1, &limitEnd))
    {
        return NV_FALSE;
    }

    return ((addrStart >= limitStart) && (addrEnd <= limitEnd));
}

/*!
 * @brief Validate the address range for Memory Map request by comparing the
 *        user supplied address range with GPU BAR0/BAR1 range.
 *
 * Lock Requirements:
 *      Assert that API and GPUs lock held on entry
 *
 * @param[in] pSubdevice
 * @param[in] pParams    pointer to control parameters
 *
 * Possible status values returned are:
 *     NV_OK
 *     NV_ERR_PROTECTION_FAULT
 *
 */
NV_STATUS subdeviceCtrlCmdValidateMemMapRequest_IMPL
(
    Subdevice                                       *pSubdevice,
    NV2080_CTRL_GPU_VALIDATE_MEM_MAP_REQUEST_PARAMS *pParams
)
{
    OBJGPU       *pGpu     = GPU_RES_GET_GPU(pSubdevice);
    NvU64         start    = pParams->addressStart;
    NvU64         length   = pParams->addressLength;
    NV_STATUS     rmStatus;
    NvU32         bar0MapSize;
    NvU64         bar0MapOffset;

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner() && rmDeviceGpuLockIsOwner(GPU_RES_GET_GPU(pSubdevice)->gpuInstance),
        NV_ERR_INVALID_LOCK_STATE);

    pParams->protection = NV_PROTECT_READ_WRITE;

    if (isAddressWithinLimits(start, length, pGpu->busInfo.gpuPhysAddr,
                        pGpu->deviceMappings[0].gpuNvLength))
    {
        start -= pGpu->busInfo.gpuPhysAddr;

        OBJTMR *pTmr = GPU_GET_TIMER(pGpu);
        rmStatus = tmrGetTimerBar0MapInfo_HAL(pGpu, pTmr,
                                              &bar0MapOffset, &bar0MapSize);
        if ((rmStatus == NV_OK) &&
            isAddressWithinLimits(start, length, bar0MapOffset, bar0MapSize))
        {
            pParams->protection = NV_PROTECT_READABLE;
            return NV_OK;
        }

        //
        // If the kernel side does not know about the object being mapped,
        // fall-through to GSP and see if it knows anything.
        //
        if (IS_FW_CLIENT(pGpu))
        {
            RM_API *pRmApi = GPU_GET_PHYSICAL_RMAPI(pGpu);

            return pRmApi->Control(pRmApi,
                                   pGpu->hInternalClient,
                                   pGpu->hInternalSubdevice,
                                   NV2080_CTRL_CMD_GPU_VALIDATE_MEM_MAP_REQUEST,
                                   pParams, sizeof(*pParams));
        }

        return NV_ERR_PROTECTION_FAULT;
    }

    return NV_ERR_PROTECTION_FAULT;
}

/*!
 * @brief Computes the GFID (GPU Function ID) for a given SR-IOV
 *        Virtual Function (VF) of the physical GPU based on the
 *        BDF parameters provided by the caller.
 *
 * Lock Requirements:
 *      Assert that API and GPUs lock held on entry
 *
 * @param[in] pSubdevice
 * @param[in] pParams    pointer to control parameters
 *
 * Possible status values returned are:
 *     NV_OK                on successful computation of a valid GFID
 *     NV_ERR_NOT_SUPPORTED if ctrl call is made when
 *                          SRIOV is not enabled OR
 *                          caller is not FM from Host RM
 *     NV_ERR_INVALID_STATE if computed GFID is greater than
 *                          max GFID that is expected/allowed
 */
NV_STATUS
subdeviceCtrlCmdGpuGetGfid_IMPL
(
    Subdevice                           *pSubdevice,
    NV2080_CTRL_GPU_GET_GFID_PARAMS     *pParams
)
{
    OBJSYS *pSys = SYS_GET_INSTANCE();
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);
    NvU32  pciFunction, gfid;

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner() && rmGpuLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    if (!gpuIsSriovEnabled(pGpu))
        return NV_ERR_NOT_SUPPORTED;

    // Host RM && FM
    if ((!IS_VIRTUAL(pGpu)) &&
        (pSys->getProperty(pSys, PDB_PROP_SYS_FABRIC_IS_EXTERNALLY_MANAGED)))
    {
        //
        // In unix based systems, OS uses lspci format which is "ssss:bb:dd.f",
        // so device is 5 bits and function 3 bits.
        // for SR-IOV when ARI is enabled, device and function gets combined and
        // we need to consider 8 bits function.
        //
        pciFunction = (pParams->device << 3) | pParams->func;
        gfid = (pciFunction - pGpu->sriovState.firstVFOffset) + 1;

        if (gfid > pGpu->sriovState.maxGfid)
        {
            NV_PRINTF(LEVEL_ERROR, "Computed GFID %d greater than max supported GFID\n", gfid);
            return NV_ERR_INVALID_STATE;
        }

        pParams->gfid = gfid;
        // Also set the mask for max gfid supported currently in the driver
        pParams->gfidMask = (pGpu->sriovState.maxGfid - 1);
    }
    else
    {
        return NV_ERR_NOT_SUPPORTED;
    }

    return NV_OK;
}

/*!
 * @brief Sets or unsets the SW state to inform the GPU driver that the GPU instance
 *        associated with input GFID has been activated or de-activated respectively.
 *
 * Lock Requirements:
 *      Assert that API and GPUs lock held on entry
 *
 * @param[in] pSubdevice
 * @param[in] pParams    pointer to control parameters
 *
 * Possible status values returned are:
 *     NV_OK                    on success
 *     NV_ERR_INVALID_STATE     if SRIOV state for P2P in driver is not setup
 *     NV_ERR_INVALID_ARGUMENT  if input GFID is greater than the max GFID allowed
 *     NV_ERR_NOT_SUPPORTED     if ctrl call is made when
 *                              SRIOV is not enabled OR
 *                              caller is not FM from Host RM
 *     NV_ERR_IN_USE            If MAX_NUM_P2P_GFIDS have already been enabled for P2P
 */
NV_STATUS
gpuUpdateGfidP2pCapability
(
    OBJGPU                                                  *pGpu,
    NV2080_CTRL_CMD_GPU_UPDATE_GFID_P2P_CAPABILITY_PARAMS   *pParams
)
{
    OBJSYS *pSys = SYS_GET_INSTANCE();
    PSRIOV_P2P_INFO pP2PInfo = pGpu->sriovState.pP2PInfo;
    NvBool  bSetP2PAccess = NV_FALSE;
    NvU32   idx;

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner() && rmGpuLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    if (!gpuIsSriovEnabled(pGpu))
        return NV_ERR_NOT_SUPPORTED;

    NV_ASSERT_OR_RETURN(pP2PInfo != NULL, NV_ERR_INVALID_STATE);

    // Ctrl call should only be called by the FM from Host RM
    if ((!IS_VIRTUAL(pGpu)) &&
        (pSys->getProperty(pSys, PDB_PROP_SYS_FABRIC_IS_EXTERNALLY_MANAGED)))
    {
        if (pParams->gfid > pGpu->sriovState.maxGfid)
        {
            NV_PRINTF(LEVEL_ERROR, "Input GFID %d greater than max allowed GFID\n", pParams->gfid);
            return NV_ERR_INVALID_ARGUMENT;
        }

        for (idx = 0; idx < pGpu->sriovState.maxP2pGfid; idx++)
        {
            //
            // Check if Host RM is already using a GFID for P2P,
            // Since only "MAX_NUM_P2P_GFIDS" GFID(s) is(are) allowed to do P2P at any time,
            // we should fail here if a GFID greater than supported number is being enabled
            //
            if (pParams->bEnable)
            {
                if (pP2PInfo[idx].gfid == INVALID_P2P_GFID)
                {
                    pP2PInfo[idx].gfid = pParams->gfid;
                    pGpu->sriovState.p2pFabricPartitionId = pParams->fabricPartitionId;
                    bSetP2PAccess = NV_TRUE;
                    break;
                }
            }
            else
            {
                if (pP2PInfo[idx].gfid == pParams->gfid)
                {
                    pP2PInfo[idx].gfid = INVALID_P2P_GFID;
                    pGpu->sriovState.p2pFabricPartitionId = INVALID_FABRIC_PARTITION_ID;
                    bSetP2PAccess = NV_TRUE;
                    break;
                }
            }
        }

        if (bSetP2PAccess == NV_TRUE)
        {
            pP2PInfo[idx].bAllowP2pAccess = pParams->bEnable;
        }
        else
        {
            // Some other GFID(s) has already been enabled to do P2P
            // Fail the call
            return NV_ERR_IN_USE;
        }
    }
    else
    {
        return NV_ERR_NOT_SUPPORTED;
    }

    return NV_OK;
}

NV_STATUS
subdeviceCtrlCmdUpdateGfidP2pCapability_IMPL
(
    Subdevice                                               *pSubdevice,
    NV2080_CTRL_CMD_GPU_UPDATE_GFID_P2P_CAPABILITY_PARAMS   *pParams
)
{
    return gpuUpdateGfidP2pCapability(GPU_RES_GET_GPU(pSubdevice), pParams);
}

/*!
 * @brief: This command returns the load time (latency) of each engine,
 *         implementing NV2080_CTRL_CMD_GPU_GET_ENGINE_LOAD_TIMES control call.
 *
 * @param[in]   pSubdevice
 * @param[in]   pParams
 *
 * @return
 *  NV_OK       Success
 */
NV_STATUS
subdeviceCtrlCmdGpuGetEngineLoadTimes_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_ENGINE_LOAD_TIMES_PARAMS *pParams
)
{
    OBJGPU        *pGpu               = GPU_RES_GET_GPU(pSubdevice);
    ENGDESCRIPTOR *pEngDescriptorList = gpuGetInitEngineDescriptors(pGpu);
    NvU32          numEngDescriptors  = gpuGetNumEngDescriptors(pGpu);
    NvU32          curEngDescIdx;

    NV_ASSERT_OR_RETURN(numEngDescriptors < NV2080_CTRL_GPU_MAX_ENGINE_OBJECTS, NV_ERR_BUFFER_TOO_SMALL);

    pParams->engineCount = numEngDescriptors;
    for (curEngDescIdx = 0; curEngDescIdx < numEngDescriptors; curEngDescIdx++)
    {
        ENGDESCRIPTOR curEngDescriptor = pEngDescriptorList[curEngDescIdx];
        OBJENGSTATE  *pEngstate        = gpuGetEngstate(pGpu, curEngDescriptor);

        if (pEngstate == NULL)
        {
            pParams->engineIsInit[curEngDescIdx] = NV_FALSE;
            continue;
        }

        pParams->engineList[curEngDescIdx]          = pEngstate->engDesc;
        pParams->engineStateLoadTime[curEngDescIdx] = pEngstate->stats[ENGSTATE_STATE_LOAD].transitionTimeUs * 1000;
        pParams->engineIsInit[curEngDescIdx]        = NV_TRUE;
    }

    return NV_OK;
}

/*!
 * @brief   This command is used to determine which GSP features are
 *          supported on this GPU.
 *
 * @param[in]     pSubdevice
 * @param[in,out] pGspFeaturesParams
 *
 * @return  Returns NV_STATUS
 *          NV_OK                     Success
 */
NV_STATUS
subdeviceCtrlCmdGspGetFeatures_KERNEL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GSP_GET_FEATURES_PARAMS *pGspFeaturesParams
)
{
    pGspFeaturesParams->bValid = NV_FALSE;
    return NV_OK;
}

//
// Lock Requirements:
//      Assert that API lock and GPUs lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetNameString_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_NAME_STRING_PARAMS *pNameStringParams
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner() && rmGpuLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    return gpuGetNameString(pGpu,
                            pNameStringParams->gpuNameStringFlags,
                            (void *)&pNameStringParams->gpuNameString);
}

//
// subdeviceCtrlCmdGpuGetShortNameString
//
// Lock Requirements:
//      Assert that API lock and GPUs lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetShortNameString_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_SHORT_NAME_STRING_PARAMS *pShortNameStringParams
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    NV_ASSERT_OR_RETURN(rmapiLockIsOwner() && rmGpuLockIsOwner(), NV_ERR_INVALID_LOCK_STATE);

    return gpuGetShortNameString(pGpu, (void *)&pShortNameStringParams->gpuShortNameString);
}

//
// subdeviceCtrlCmdGpuGetGidInfo
//
// Lock Requirements:
//      Assert that API lock held on entry
//
NV_STATUS
subdeviceCtrlCmdGpuGetGidInfo_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_GID_INFO_PARAMS *pGidInfoParams
)
{
    NV_STATUS rmStatus = NV_OK;
    OBJGPU   *pGpu = GPU_RES_GET_GPU(pSubdevice);
    NvU8     *pGidString;
    NvU32     flags = pGidInfoParams->flags;
    NvU32     gidStrlen;

    rmStatus = gpuGetGidInfo(pGpu, &pGidString, &gidStrlen, flags);
    if (rmStatus == NV_OK)
    {
        if (sizeof(pGidInfoParams->data) >= gidStrlen)
        {
            portMemCopy(pGidInfoParams->data, gidStrlen, pGidString, gidStrlen);
            pGidInfoParams->length = gidStrlen;
        }
        else
        {
            rmStatus = NV_ERR_INSUFFICIENT_RESOURCES;
        }

        portMemFree(pGidString);
    }

    return rmStatus;
}

NV_STATUS
subdeviceCtrlCmdGpuGetChipDetails_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_CHIP_DETAILS_PARAMS *pParams
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    return gpuGetChipDetails(pGpu, pParams);
}

NV_STATUS
subdeviceCtrlCmdBiosGetSKUInfo_KERNEL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_BIOS_GET_SKU_INFO_PARAMS *pBiosGetSKUInfoParams
)
{
    return NV_ERR_NOT_SUPPORTED;
}

/*!
 * @brief   This Command is used to query the recovery action for a GPU to get
 *          back to an operational state.
 *
 * @param[in,out]   pParams
 *                  action: The GPU recovery action.
 *
 * @return  Returns NV_STATUS
 *          NV_ERR_GPU_IS_LOST        The GPU has fallen off the bus
 *          NV_OK                     Success
 *
 */
NV_STATUS
subdeviceCtrlCmdGpuGetRecoveryAction_IMPL(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_GET_RECOVERY_ACTION_PARAMS *pParams
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    if (!API_GPU_ATTACHED_SANITY_CHECK(pGpu))
    {
        return NV_ERR_GPU_IS_LOST;
    }

    gpuGetRecoveryAction(pGpu, pParams);

    return NV_OK;
}

NV_STATUS
subdeviceCtrlCmdGpuGetVgpuHeapStats_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_CMD_GSP_GET_VGPU_HEAP_STATS_PARAMS *pParams
)
{
    return NV_ERR_NOT_SUPPORTED;
}

/*
 * GSP RPC integrity test
 */

NV_STATUS
subdeviceCtrlCmdGpuRpcGspTest_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_RPC_GSP_TEST_PARAMS *pParams
)
{
    NV_STATUS status =  NV_OK;
    NvU32 *data = (NvU32 *) pParams->data;
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    if (IS_FW_CLIENT(pGpu))
    {
        CALL_CONTEXT *pCallContext  = resservGetTlsCallContext();
        RmCtrlParams *pRmCtrlParams = pCallContext->pControlParams;
        pParams->startTimestamp = osGetTimestamp();
        if ((pParams->test == NV2080_CTRL_GPU_RPC_GSP_TEST_SERIALIZED_INTEGRITY) ||
            (pParams->test == NV2080_CTRL_GPU_RPC_GSP_TEST_SERIALIZED_NOP))
        {
            NV_RM_RPC_CONTROL(pGpu,
                        pRmCtrlParams->hClient,
                        pRmCtrlParams->hObject,
                        pRmCtrlParams->cmd,
                        pRmCtrlParams->pParams,
                        pRmCtrlParams->paramsSize,
                        status);
        }
        else if (pParams->test == NV2080_CTRL_GPU_RPC_GSP_TEST_UNSERIALIZED)
        {
            RM_API *pRmApi = GPU_GET_PHYSICAL_RMAPI(pGpu);
            NV2080_CTRL_GPU_GET_IP_VERSION_PARAMS params = {0};
            params.targetEngine = NV2080_CTRL_GPU_GET_IP_VERSION_PPWR_PMU;
            status = pRmApi->Control(pRmApi,
                                     pGpu->hInternalClient,
                                     pGpu->hInternalSubdevice,
                                     NV2080_CTRL_CMD_GPU_GET_IP_VERSION,
                             &params, sizeof(params));
        }
        else
        {
            status = NV_ERR_INVALID_ARGUMENT;
        }
        pParams->stopTimestamp = osGetTimestamp();
        return status;
    }

    if (pParams->test == NV2080_CTRL_GPU_RPC_GSP_TEST_SERIALIZED_INTEGRITY)
    {
        for (NvU32 i = 0; i < pParams->dataSize; i++)
        {
            if (data[i] != i * 2) {
                status = NV_ERR_INVALID_DATA;
                NV_PRINTF(LEVEL_ERROR, "RPC TEST: mismatch in input data, expected %u, received %u\n", i * 2, data[i]);
            }
            data[i] = i * 3;
        }
        return status;
    }

    return NV_OK;
}

/*
 * Used to query information for RPC integrity tests
 */

NV_STATUS
subdeviceCtrlCmdGpuRpcGspQuerySizes_IMPL
(
    Subdevice *pSubdevice,
    NV2080_CTRL_GPU_RPC_GSP_QUERY_SIZES_PARAMS *pParams
)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);
    OBJRPC *pRpc = GPU_GET_RPC(pGpu);

    pParams->maxRpcSize = pRpc->maxRpcSize;
    pParams->finnRmapiSize = sizeof(FINN_RM_API);
    pParams->rpcGspControlSize = sizeof(rpc_gsp_rm_control_v);
    pParams->rpcMessageHeaderSize = sizeof(rpc_message_header_v);
    pParams->timestampFreq = osGetTimestampFreq();

    return NV_OK;
}

static void
subdeviceCtrlCmdThermalSystemExecuteV2_updateCache(Subdevice *pSubdevice,
                                NV2080_CTRL_THERMAL_SYSTEM_EXECUTE_V2_PARAMS *pSystemExecuteParams)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);

    for (NvU32 i = 0; i < pSystemExecuteParams->instructionListSize; i++)
    {
        if ((!pSystemExecuteParams->instructionList[i].executed) ||
            ( pSystemExecuteParams->instructionList[i].result != NV_OK))
            continue;

        switch (pSystemExecuteParams->instructionList[i].opcode)
        {
            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSORS_AVAILABLE_OPCODE:
            {
                pGpu->thermalSystemExecuteV2Cache.numSensors =
                    pSystemExecuteParams->instructionList[i].operands.getInfoSensorsAvailable.availableSensors;

                pGpu->thermalSystemExecuteV2Cache.bNumSensorsCached = NV_TRUE;

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_PROVIDER_TYPE_OPCODE:
            {
                for (NvU32 j = 0; j < NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors); j++)
                {
                    if ((pGpu->thermalSystemExecuteV2Cache.sensors[j].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_INDEX]) &&
                        (pGpu->thermalSystemExecuteV2Cache.sensors[j].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_INDEX] ==
                         pSystemExecuteParams->instructionList[i].operands.getInfoProviderType.providerIndex))
                    {
                        pGpu->thermalSystemExecuteV2Cache.sensors[j].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_TYPE] =
                            pSystemExecuteParams->instructionList[i].operands.getInfoProviderType.type;
                        pGpu->thermalSystemExecuteV2Cache.sensors[j].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_TYPE] = NV_TRUE;
                        break;
                    }
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_TARGET_TYPE_OPCODE:
            {
                for (NvU32 j = 0; j < NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors); j++)
                {
                    if ((pGpu->thermalSystemExecuteV2Cache.sensors[j].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_INDEX]) &&
                        (pGpu->thermalSystemExecuteV2Cache.sensors[j].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_INDEX] ==
                         pSystemExecuteParams->instructionList[i].operands.getInfoTargetType.targetIndex))
                    {
                        pGpu->thermalSystemExecuteV2Cache.sensors[j].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_TYPE] =
                            pSystemExecuteParams->instructionList[i].operands.getInfoTargetType.type;
                        pGpu->thermalSystemExecuteV2Cache.sensors[j].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_TYPE] = NV_TRUE;
                        break;
                    }
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_READING_RANGE_OPCODE:
            {
                NvU32 sensorIdx = pSystemExecuteParams->instructionList[i].operands.getInfoSensorReadingRange.sensorIndex;

                if (sensorIdx < NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors))
                {
                    pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MIN_TEMP] =
                        pSystemExecuteParams->instructionList[i].operands.getInfoSensorReadingRange.minimum;
                    pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MIN_TEMP] = NV_TRUE;

                    pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MAX_TEMP] =
                        pSystemExecuteParams->instructionList[i].operands.getInfoSensorReadingRange.maximum;
                    pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MAX_TEMP] = NV_TRUE;
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_PROVIDER_OPCODE:
            {
                NvU32 sensorIdx = pSystemExecuteParams->instructionList[i].operands.getInfoSensorProvider.sensorIndex;

                if (sensorIdx < NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors))
                {
                    pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_INDEX] =
                        pSystemExecuteParams->instructionList[i].operands.getInfoSensorProvider.providerIndex;
                    pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_INDEX] = NV_TRUE;
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_TARGET_OPCODE:
            {
                NvU32 sensorIdx = pSystemExecuteParams->instructionList[i].operands.getInfoSensorTarget.sensorIndex;

                if (sensorIdx < NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors))
                {
                    pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_INDEX] =
                        pSystemExecuteParams->instructionList[i].operands.getInfoSensorTarget.targetIndex;
                    pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_INDEX] = NV_TRUE;
                }

                break;
            }
        }
    }
}

NV_STATUS
subdeviceCtrlCmdThermalSystemExecuteV2_IMPL(Subdevice *pSubdevice,
                                NV2080_CTRL_THERMAL_SYSTEM_EXECUTE_V2_PARAMS *pSystemExecuteParams)
{
    OBJGPU *pGpu = GPU_RES_GET_GPU(pSubdevice);
    RM_API *pRmApi = GPU_GET_PHYSICAL_RMAPI(pGpu);
    NvU32 instructionListSize = pSystemExecuteParams->instructionListSize;

    NV_CHECK_OR_RETURN(LEVEL_ERROR,
                       instructionListSize <= NV_ARRAY_ELEMENTS(pSystemExecuteParams->instructionList),
                       NV_ERR_INVALID_ARGUMENT);

    NV_STATUS status = NV_OK;
    NvBool bForwardRmctrl;

    for (NvU32 i = 0; i < instructionListSize; i++)
    {
        pSystemExecuteParams->instructionList[i].executed = NV_FALSE;
    }

    // Skip unsupported version
    if (pSystemExecuteParams->clientAPIVersion != THERMAL_SYSTEM_API_VER  ||
        pSystemExecuteParams->clientAPIRevision != THERMAL_SYSTEM_API_REV ||
        pSystemExecuteParams->clientInstructionSizeOf != sizeof(NV2080_CTRL_THERMAL_SYSTEM_INSTRUCTION))
    {
        return pRmApi->Control(pRmApi, pGpu->hInternalClient, pGpu->hInternalSubdevice,
                               NV2080_CTRL_CMD_THERMAL_SYSTEM_EXECUTE_V2_PHYSICAL,
                               pSystemExecuteParams, sizeof(*pSystemExecuteParams));
    }

    // If we cannot service the control by servicing data from cache entirely, forward to physical RM
    bForwardRmctrl = NV_FALSE;

    // Service values from cache
    for (NvU32 i = 0; i < instructionListSize; i++)
    {
        // Verify that the size of the union NV2080_CTRL_THERMAL_SYSTEM_INSTRUCTION_OPERANDS is dictated by
        // NV2080_CTRL_THERMAL_SYSTEM_INSTRUCTION_OPERANDS::space
        ct_assert(sizeof(pSystemExecuteParams->instructionList[0].operands.space) ==
                  sizeof(NV2080_CTRL_THERMAL_SYSTEM_INSTRUCTION_OPERANDS));

        switch (pSystemExecuteParams->instructionList[i].opcode)
        {
            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSORS_AVAILABLE_OPCODE:
            {
                if (pGpu->thermalSystemExecuteV2Cache.bNumSensorsCached)
                {
                    pSystemExecuteParams->instructionList[i].operands.getInfoSensorsAvailable.availableSensors =
                        pGpu->thermalSystemExecuteV2Cache.numSensors;

                    pSystemExecuteParams->instructionList[i].executed = NV_TRUE;
                    pSystemExecuteParams->instructionList[i].result = NV_OK;
                    pSystemExecuteParams->successfulInstructions++;
                }
                else
                {
                    bForwardRmctrl = NV_TRUE;
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_PROVIDER_TYPE_OPCODE:
            {
                NvBool bTypeFound = NV_FALSE;

                for (NvU32 j = 0; j < NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors); j++)
                {
                    if ((pGpu->thermalSystemExecuteV2Cache.sensors[j].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_INDEX]) &&
                        (pGpu->thermalSystemExecuteV2Cache.sensors[j].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_INDEX] ==
                         pSystemExecuteParams->instructionList[i].operands.getInfoProviderType.providerIndex) &&
                        (pGpu->thermalSystemExecuteV2Cache.sensors[j].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_TYPE]))
                    {
                        pSystemExecuteParams->instructionList[i].operands.getInfoProviderType.type =
                            pGpu->thermalSystemExecuteV2Cache.sensors[j].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_TYPE];

                        pSystemExecuteParams->instructionList[i].executed = NV_TRUE;
                        pSystemExecuteParams->instructionList[i].result = NV_OK;
                        pSystemExecuteParams->successfulInstructions++;

                        bTypeFound = NV_TRUE;
                        break;
                    }
                }

                if (!bTypeFound)
                {
                    bForwardRmctrl = NV_TRUE;
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_TARGET_TYPE_OPCODE:
            {
                NvBool bTypeFound = NV_FALSE;

                for (NvU32 j = 0; j < NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors); j++)
                {
                    if ((pGpu->thermalSystemExecuteV2Cache.sensors[j].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_INDEX]) &&
                        (pGpu->thermalSystemExecuteV2Cache.sensors[j].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_INDEX] ==
                         pSystemExecuteParams->instructionList[i].operands.getInfoProviderType.providerIndex) &&
                        (pGpu->thermalSystemExecuteV2Cache.sensors[j].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_TYPE]))
                    {
                        pSystemExecuteParams->instructionList[i].operands.getInfoProviderType.type =
                            pGpu->thermalSystemExecuteV2Cache.sensors[j].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_TYPE];

                        pSystemExecuteParams->instructionList[i].executed = NV_TRUE;
                        pSystemExecuteParams->instructionList[i].result = NV_OK;
                        pSystemExecuteParams->successfulInstructions++;

                        bTypeFound = NV_TRUE;
                        break;
                    }
                }

                if (!bTypeFound)
                {
                    bForwardRmctrl = NV_TRUE;
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_READING_RANGE_OPCODE:
            {
                NvU32 sensorIdx = pSystemExecuteParams->instructionList[i].operands.getInfoSensorReadingRange.sensorIndex;

                if (sensorIdx >= NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors))
                {
                    bForwardRmctrl = NV_TRUE;
                    break;
                }

                if (pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MIN_TEMP] &&
                    pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MAX_TEMP])
                {
                    pSystemExecuteParams->instructionList[i].operands.getInfoSensorReadingRange.minimum =
                        pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MIN_TEMP];
                    pSystemExecuteParams->instructionList[i].operands.getInfoSensorReadingRange.maximum =
                        pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_DEFAULT_MAX_TEMP];

                    pSystemExecuteParams->instructionList[i].executed = NV_TRUE;
                    pSystemExecuteParams->instructionList[i].result = NV_OK;
                    pSystemExecuteParams->successfulInstructions++;
                }
                else
                {
                    bForwardRmctrl = NV_TRUE;
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_PROVIDER_OPCODE:
            {
                NvU32 sensorIdx = pSystemExecuteParams->instructionList[i].operands.getInfoSensorProvider.sensorIndex;

                if (sensorIdx >= NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors))
                {
                    bForwardRmctrl = NV_TRUE;
                    break;
                }

                if (pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_INDEX])
                {
                    pSystemExecuteParams->instructionList[i].operands.getInfoSensorProvider.providerIndex =
                        pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_PROVIDER_INDEX];

                    pSystemExecuteParams->instructionList[i].executed = NV_TRUE;
                    pSystemExecuteParams->instructionList[i].result = NV_OK;
                    pSystemExecuteParams->successfulInstructions++;
                }
                else
                {
                    bForwardRmctrl = NV_TRUE;
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_INFO_SENSOR_TARGET_OPCODE:
            {
                NvU32 sensorIdx = pSystemExecuteParams->instructionList[i].operands.getInfoSensorTarget.sensorIndex;

                if (sensorIdx >= NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors))
                {
                    bForwardRmctrl = NV_TRUE;
                    break;
                }

                if (pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_INDEX])
                {
                    pSystemExecuteParams->instructionList[i].operands.getInfoSensorTarget.targetIndex =
                        pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_INDEX];

                    pSystemExecuteParams->instructionList[i].executed = NV_TRUE;
                    pSystemExecuteParams->instructionList[i].result = NV_OK;
                    pSystemExecuteParams->successfulInstructions++;
                }
                else
                {
                    bForwardRmctrl = NV_TRUE;
                }

                break;
            }

            case NV2080_CTRL_THERMAL_SYSTEM_GET_STATUS_SENSOR_READING_OPCODE:
            {
                NvU32 sensorIdx = pSystemExecuteParams->instructionList[i].operands.getStatusSensorReading.sensorIndex;

                if ((sensorIdx >= NV_ARRAY_ELEMENTS(pGpu->thermalSystemExecuteV2Cache.sensors)) ||
                    !(pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].bIsCached[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_TYPE]) ||
                    (pGpu->userSharedData.pMapBuffer == NULL))
                {
                    bForwardRmctrl = NV_TRUE;
                    break;
                }

                RUSD_TEMPERATURE rusdTemperature;
                RUSD_TEMPERATURE_TYPE temperatureType;

                temperatureType = RUSD_TEMPERATURE_TYPE_MAX;
                switch (pGpu->thermalSystemExecuteV2Cache.sensors[sensorIdx].cache[THERMAL_SYSTEM_EXECUTE_V2_CACHE_ENTRY_TARGET_TYPE])
                {
                    case NV2080_CTRL_THERMAL_SYSTEM_TARGET_GPU:
                    {
                        temperatureType = RUSD_TEMPERATURE_TYPE_GPU;
                        break;
                    }
                    case NV2080_CTRL_THERMAL_SYSTEM_TARGET_MEMORY:
                    {
                        temperatureType = RUSD_TEMPERATURE_TYPE_MEMORY;
                        break;
                    }
                    case NV2080_CTRL_THERMAL_SYSTEM_TARGET_POWER_SUPPLY:
                    {
                        temperatureType = RUSD_TEMPERATURE_TYPE_POWER_SUPPLY;
                        break;
                    }
                    case NV2080_CTRL_THERMAL_SYSTEM_TARGET_BOARD:
                    {
                        temperatureType = RUSD_TEMPERATURE_TYPE_BOARD;
                        break;
                    }
                }

                if (temperatureType != RUSD_TEMPERATURE_TYPE_MAX)
                {
                    RUSD_READ_DATA((NV00DE_SHARED_DATA*)(pGpu->userSharedData.pMapBuffer), temperatures[temperatureType], &rusdTemperature);
                }

                if ((temperatureType == RUSD_TEMPERATURE_TYPE_MAX) ||
                    (rusdTemperature.lastModifiedTimestamp == RUSD_TIMESTAMP_INVALID))
                {
                    bForwardRmctrl = NV_TRUE;
                    break;
                }


                pSystemExecuteParams->instructionList[i].operands.getStatusSensorReading.value =
                    NV_TYPES_NV_TEMP_TO_CELSIUS_ROUNDED(rusdTemperature.temperature);

                pSystemExecuteParams->instructionList[i].executed = NV_TRUE;
                pSystemExecuteParams->instructionList[i].result = NV_OK;
                pSystemExecuteParams->successfulInstructions++;

                break;
            }

            // Unknown opcode
            default:
            {
                bForwardRmctrl = NV_TRUE;
            }
        }
    }

    if (!bForwardRmctrl)
    {
        status = NV_OK;
    }
    else
    {
        status = pRmApi->Control(pRmApi, pGpu->hInternalClient, pGpu->hInternalSubdevice,
                                 NV2080_CTRL_CMD_THERMAL_SYSTEM_EXECUTE_V2_PHYSICAL,
                                 pSystemExecuteParams, sizeof(*pSystemExecuteParams));

        subdeviceCtrlCmdThermalSystemExecuteV2_updateCache(pSubdevice, pSystemExecuteParams);
    }

    return status;

}

