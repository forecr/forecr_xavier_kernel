/*
 * SPDX-FileCopyrightText: Copyright (c) 1993-2014 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "nvkms-types.h"
#include "nvkms-3dvision.h"

void nv3DVisionAuthenticationEvo(NVDispEvoRec *pDispEvo, const NvU32 apiHead)
{
    return;
}

void nvDpyCheck3DVisionCapsEvo(NVDpyEvoPtr pDpyEvo)
{
    return;
}

NvBool
nvPatch3DVisionModeTimingsEvo(NVT_TIMING *pTiming, NVDpyEvoPtr pDpyEvo,
                              NVEvoInfoStringPtr pInfoString)
{
    return FALSE;
}

void nvDisable3DVisionAegis(const NVDpyEvoRec *pDpyEvo)
{
    return;
}

void nvSendHwModeTimingsToAegisEvo(const NVDispEvoRec *pDispEvo,
                                   const NvU32 apiHead)
{
    return;
}

