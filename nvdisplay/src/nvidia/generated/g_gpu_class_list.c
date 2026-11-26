/*
 * SPDX-FileCopyrightText: Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include <core/core.h>
#include <gpu/gpu.h>
#include <gpu/eng_desc.h>
#include <g_allclasses.h>
#include <ctrl/ctrl0080/ctrl0080gpu.h> // NV0080_CTRL_GPU_CLASSLIST_MAX_SIZE


const NvU32 *
gpuGetGenericClassList_IMPL(OBJGPU *pGpu, NvU32 *pNumClassDescriptors)
{
    static const NvU32 genericClassDescriptorList[] = {
        LOCK_STRESS_OBJECT,
        LOCK_TEST_RELAXED_DUP_OBJECT,
        NV01_CONTEXT_DMA,
        NV01_DEVICE_0,
        NV01_EVENT,
        NV01_EVENT_KERNEL_CALLBACK,
        NV01_EVENT_KERNEL_CALLBACK_EX,
        NV01_EVENT_OS_EVENT,
        NV01_MEMORY_SYNCPOINT,
        NV01_MEMORY_SYSTEM,
        NV01_MEMORY_SYSTEM_OS_DESCRIPTOR,
        NV01_ROOT,
        NV01_ROOT_CLIENT,
        NV01_ROOT_NON_PRIV,
        NV20_SUBDEVICE_0,
    };
    *pNumClassDescriptors = 15;
    return genericClassDescriptorList;
}

const NvU32 *
gpuGetNoEngClassList_T234D(OBJGPU *pGpu, NvU32 *pNumClassDescriptors)
{
    *pNumClassDescriptors = 0;
    return NULL;
}

const CLASSDESCRIPTOR *
gpuGetEngClassDescriptorList_T234D(OBJGPU *pGpu, NvU32 *pNumClassDescriptors)
{
    static const CLASSDESCRIPTOR halT234DClassDescriptorList[] = {
        { GF100_HDACODEC, ENG_HDACODEC },
        { IO_VASPACE_A, ENG_INVALID },
        { NV04_DISPLAY_COMMON, ENG_KERNEL_DISPLAY },
        { NVC372_DISPLAY_SW, ENG_KERNEL_DISPLAY },
        { NVC670_DISPLAY, ENG_KERNEL_DISPLAY },
        { NVC671_DISP_SF_USER, ENG_KERNEL_DISPLAY },
        { NVC673_DISP_CAPABILITIES, ENG_KERNEL_DISPLAY },
        { NVC67A_CURSOR_IMM_CHANNEL_PIO, ENG_KERNEL_DISPLAY },
        { NVC67B_WINDOW_IMM_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
        { NVC67D_CORE_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
        { NVC67E_WINDOW_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
        { NVC77F_ANY_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
    };
    *pNumClassDescriptors = NV_ARRAY_ELEMENTS(halT234DClassDescriptorList);
    return halT234DClassDescriptorList;
}

const NvU32 *
gpuGetNoEngClassList_T264D(OBJGPU *pGpu, NvU32 *pNumClassDescriptors)
{
    *pNumClassDescriptors = 0;
    return NULL;
}

const CLASSDESCRIPTOR *
gpuGetEngClassDescriptorList_T264D(OBJGPU *pGpu, NvU32 *pNumClassDescriptors)
{
    static const CLASSDESCRIPTOR halT264DClassDescriptorList[] = {
        { GF100_HDACODEC, ENG_HDACODEC },
        { IO_VASPACE_A, ENG_INVALID },
        { NV04_DISPLAY_COMMON, ENG_KERNEL_DISPLAY },
        { NVC372_DISPLAY_SW, ENG_KERNEL_DISPLAY },
        { NVC970_DISPLAY, ENG_KERNEL_DISPLAY },
        { NVC971_DISP_SF_USER, ENG_KERNEL_DISPLAY },
        { NVC973_DISP_CAPABILITIES, ENG_KERNEL_DISPLAY },
        { NVC97A_CURSOR_IMM_CHANNEL_PIO, ENG_KERNEL_DISPLAY },
        { NVC97B_WINDOW_IMM_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
        { NVC97D_CORE_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
        { NVC97E_WINDOW_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
    };
    *pNumClassDescriptors = NV_ARRAY_ELEMENTS(halT264DClassDescriptorList);
    return halT264DClassDescriptorList;
}

const NvU32 *
gpuGetNoEngClassList_T256D(OBJGPU *pGpu, NvU32 *pNumClassDescriptors)
{
    *pNumClassDescriptors = 0;
    return NULL;
}

const CLASSDESCRIPTOR *
gpuGetEngClassDescriptorList_T256D(OBJGPU *pGpu, NvU32 *pNumClassDescriptors)
{
    static const CLASSDESCRIPTOR halT256DClassDescriptorList[] = {
        { GF100_HDACODEC, ENG_HDACODEC },
        { IO_VASPACE_A, ENG_INVALID },
        { NV04_DISPLAY_COMMON, ENG_KERNEL_DISPLAY },
        { NVC372_DISPLAY_SW, ENG_KERNEL_DISPLAY },
        { NVCC70_DISPLAY, ENG_KERNEL_DISPLAY },
        { NVCC71_DISP_SF_USER, ENG_KERNEL_DISPLAY },
        { NVCC73_DISP_CAPABILITIES, ENG_KERNEL_DISPLAY },
        { NVCC7A_CURSOR_IMM_CHANNEL_PIO, ENG_KERNEL_DISPLAY },
        { NVCC7B_WINDOW_IMM_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
        { NVCC7D_CORE_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
        { NVCC7E_WINDOW_CHANNEL_DMA, ENG_KERNEL_DISPLAY },
    };
    *pNumClassDescriptors = NV_ARRAY_ELEMENTS(halT256DClassDescriptorList);
    return halT256DClassDescriptorList;
}

ct_assert(NV0080_CTRL_GPU_CLASSLIST_MAX_SIZE >= (15 /* generic */ + 12 /* T234D */));
