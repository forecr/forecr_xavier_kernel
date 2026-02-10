/*
 * SPDX-FileCopyrightText: Copyright (c) 2019-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

/***************************** HW State Routines ***************************\
*                                                                           *
*         Implementation specific Descriptor List management functions      *
*                                                                           *
\***************************************************************************/

#include "core/core.h"
#include "gpu/gpu.h"
#include "gpu/eng_desc.h"
#include "gpu/gpu_child_class_defs.h"
#include "g_allclasses.h"
#include <nv_arch.h>

#include "swref/published/t23x/t234/dev_fuse.h"

/*!
 * @brief fill in the GPU ID information
 */
void
gpuGetIdInfo_T234D
(
    OBJGPU   *pGpu
)
{
    pGpu->chipInfo.platformId       = GPU_ARCHITECTURE_T23X;
    pGpu->chipInfo.implementationId = GPU_IMPLEMENTATION_T234D;
    pGpu->chipInfo.revisionId = 0;
}

// See gpuChildOrderList_GM200 for documentation
static const GPUCHILDORDER
gpuChildOrderList_T234D[] =
{
    {classId(OBJDCECLIENTRM),       GCO_ALL},
    {classId(MemorySystem),         GCO_ALL},
    {classId(KernelMemorySystem),   GCO_ALL},
    {classId(MemoryManager),        GCO_ALL},
    {classId(OBJDCB),               GCO_ALL},
    {classId(OBJDISP),              GCO_ALL},
    {classId(KernelDisplay),        GCO_ALL},
    {classId(OBJDPAUX),             GCO_ALL},
    {classId(I2c),                  GCO_ALL},
    {classId(OBJGPIO),              GCO_ALL},
    {classId(OBJHDACODEC),          GCO_ALL},
};

// See gpuChildrenPresent_GM200 for documentation on GPUCHILDPRESENT
static const GPUCHILDPRESENT gpuChildrenPresent_T234D[] =
{
    GPU_CHILD_PRESENT(OBJTMR, 1),
    GPU_CHILD_PRESENT(OBJDCECLIENTRM, 1),
    GPU_CHILD_PRESENT(KernelDisplay, 1),
    GPU_CHILD_PRESENT(MemoryManager, 1),
};


const GPUCHILDORDER *
gpuGetChildrenOrder_T234D(OBJGPU *pGpu, NvU32 *pNumEntries)
{
    *pNumEntries = NV_ARRAY_ELEMENTS(gpuChildOrderList_T234D);
    return gpuChildOrderList_T234D;
}

const GPUCHILDPRESENT *
gpuGetChildrenPresent_T234D(OBJGPU *pGpu, NvU32 *pNumEntries)
{
    *pNumEntries = NV_ARRAY_ELEMENTS(gpuChildrenPresent_T234D);
     return gpuChildrenPresent_T234D;
}

