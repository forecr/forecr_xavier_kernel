/*
 * SPDX-FileCopyrightText: Copyright (c) 2021-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#define RM_STRICT_CONFIG_EMIT_DISP_ENGINE_DEFINITIONS     0

#include "class/cl5070.h"
#include "gpu/disp/disp_objs.h"
#include "gpu/disp/kern_disp.h"
#include "gpu/gpu.h"
#include "gpu/subdevice/subdevice.h"
#include "gpu_mgr/gpu_mgr.h"
#include "mem_mgr/mem.h"
#include "rmapi/client_resource.h"
#include "rmapi/rmapi.h"
#include "rmapi/rs_utils.h"

NV_STATUS
dispobjCtrlCmdGetRgConnectedLockpinStateless_IMPL
(
    DispObject *pDispObject,
    NV5070_CTRL_GET_RG_CONNECTED_LOCKPIN_STATELESS_PARAMS *pParams
)
{
    return NV_ERR_NOT_SUPPORTED;
}

NV_STATUS
nvdispapiCtrlCmdChannelCancelFlip_IMPL
(
    NvDispApi *pNvDispApi,
    NVC370_CTRL_CHANNEL_CANCEL_FLIP_PARAMS *pParams
)
{
    OBJGPU *pGpu = DISPAPI_GET_GPU(pNvDispApi);
    KernelDisplay *pKernelDisplay = GPU_GET_KERNEL_DISPLAY(pGpu);
    NvHandle hClient = RES_GET_CLIENT_HANDLE(pNvDispApi);
    DISPCHNCLASS internalChnClass = dispChnClass_Supported;
    NvU32 dispChannelNum = 0;
    NV_STATUS status = NV_OK;

    NV_ASSERT_OR_RETURN(pParams != NULL, NV_ERR_INVALID_ARGUMENT);

    status = kdispGetIntChnClsForHwCls(pKernelDisplay, pParams->channelClass, &internalChnClass);
    if (status != NV_OK)
    {
        return status;
    }

    if (kdispGetChannelNum_HAL(pKernelDisplay, internalChnClass, pParams->channelInstance, &dispChannelNum) != NV_OK)
    {
        return NV_ERR_INVALID_CHANNEL;
    }

    if (pKernelDisplay->pClientChannelTable[dispChannelNum].bInUse != NV_TRUE)
    {
        NV_PRINTF(LEVEL_WARNING, "disp Channel not allocated by RM yet!\n");
        return NV_ERR_INVALID_CHANNEL;
    }
    else
    {
        // Does HW also think the same
        if (!kdispIsChannelAllocatedHw_HAL(pGpu, pKernelDisplay, internalChnClass, pParams->channelInstance))
        {
            NV_PRINTF(LEVEL_WARNING, "disp Channel not allocated by HW yet!\n");
            return NV_ERR_INVALID_CHANNEL;
        }
    }

    if (internalChnClass == dispChnClass_Core)
    {
        // Ensure that only core channel owner can touch it.
        if (pKernelDisplay->pClientChannelTable[dispChannelNum].pClient->hClient != hClient)
        {
            NV_ASSERT(0);
            return NV_ERR_INVALID_OWNER;
        }
    }

    kdispSetChannelTrashAndAbortAccel_HAL(pGpu, pKernelDisplay, internalChnClass, pParams->channelInstance, NV_TRUE);

    if (!kdispIsChannelIdle_HAL(pGpu, pKernelDisplay, internalChnClass, pParams->channelInstance))
    {
        NV_PRINTF(LEVEL_WARNING, "disp channel not in idle state! %u %u\n", internalChnClass, pParams->channelInstance);
        NV_ASSERT(0);
    }

    kdispSetChannelTrashAndAbortAccel_HAL(pGpu, pKernelDisplay, internalChnClass, pParams->channelInstance, NV_FALSE);
   
    return status;
}
