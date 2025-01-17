/*
 * SPDX-FileCopyrightText: Copyright (c) 2017-2021 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "gpu/gpu.h"
#include "gpu/mem_mgr/mem_mgr.h"

#include "turing/tu102/dev_mmu.h"
#include "turing/tu102/kind_macros.h"

/*!
 * @brief Returns NV_TRUE if memory kind matches the given op.
 *
 * @param[in] op    Kind-type to check for
 * @param[in] kind  Value to check
 *
 * @return NV_TRUE if "kind" matches kind-type specified by op.
 *         NV_FALSE otherwise.
 */
NvBool
memmgrIsKind_TU102
(
    MemoryManager *pMemoryManager,
    FB_IS_KIND_OP  op,
    NvU32          kind
)
{
    switch (op)
    {
        case FB_IS_KIND_Z:
            return KIND_Z(kind);
        case FB_IS_KIND_ZBC:
            return PTEKIND_COMPRESSIBLE(kind);
        case FB_IS_KIND_COMPRESSIBLE:
            return PTEKIND_COMPRESSIBLE(kind);
        case FB_IS_KIND_ZBC_ALLOWS_1:
        case FB_IS_KIND_ZBC_ALLOWS_2:
        case FB_IS_KIND_COMPRESSIBLE_1:
        case FB_IS_KIND_COMPRESSIBLE_2:
        case FB_IS_KIND_COMPRESSIBLE_4:
            return NV_FALSE;
        case FB_IS_KIND_SUPPORTED:
            return (PTEKIND_SUPPORTED(kind) && !(KIND_INVALID(kind)));
        case FB_IS_KIND_DISALLOW_PLC:
            return PTEKIND_DISALLOWS_PLC(kind);
        default:
            NV_PRINTF(LEVEL_ERROR, "Bad op (%08x) passed in\n", op);
            DBG_BREAKPOINT();
            return NV_FALSE;
    }
}

/**
 *  From Turing, we will not have Pitch Kind, so this function will  determine
 *  type of surface from pMemoryInfo of the allocation.
 *  return NV_TRUE for BL surfaces and NV_FALSE otherwise.
 */
NvBool
memmgrIsSurfaceBlockLinear_TU102
(
    MemoryManager     *pMemoryManager,
    Memory            *pMemory,
    NvU32              kind,
    NvU32              dmaFlags
)
{
    if (FLD_TEST_DRF(OS03, _FLAGS, _PTE_KIND, _BL, dmaFlags))
    {
        return NV_TRUE;
    }
    else if (FLD_TEST_DRF(OS03, _FLAGS, _PTE_KIND, _PITCH, dmaFlags))
    {
        return NV_FALSE;
    }

    return FLD_TEST_DRF(OS32, _ATTR, _FORMAT, _BLOCK_LINEAR, pMemory->Attr);
}
