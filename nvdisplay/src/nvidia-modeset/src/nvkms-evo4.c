/*
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

/*
 * This file contains implementations of the EVO HAL methods for display class
 * 9.x (also known as "nvdisplay").
 */

#include "nvkms-dma.h"
#include "nvkms-types.h"
#include "nvkms-rmapi.h"
#include "nvkms-surface.h"
#include "nvkms-softfloat.h"
#include "nvkms-evo.h"
#include "nvkms-evo1.h"
#include "nvkms-evo3.h"
#include "nvkms-prealloc.h"
#include "nv-float.h"
#include "nvkms-dpy.h"
#include "nvkms-vrr.h"
#include "nvkms-sync.h"
#include <class/clc37dcrcnotif.h> // NVC37D_NOTIFIER_CRC
#include <ctrl/ctrlc372/ctrlc372chnc.h>
#include <ctrl/ctrl0041.h> // NV0041_CTRL_GET_SURFACE_PHYS_ATTR_PARAMS
#include <nvmisc.h>
#include <class/clc973.h> // NVC973_DISP_CAPABILITIES
#include "class/clc97d.h" // NVC97D_CORE_CHANNEL_DMA
#include <class/clc97dswspare.h> // NVC97D_HEAD_SET_SW_SPARE_*
#include "class/clc97e.h" // NVC97E_WINDOW_CHANNEL_DMA

/*
 * XXX temporary WAR: See Bug 4146656
 * Currently RM expects ctxdma handle in hObjectBuffer field
 * of NV50VAIO_CHANNELDMA_ALLOCATION_PARAMETERS, which is used to
 * allocate PB channel.
 */
#define NV_EVO4_PB_ALLOC_WAR

#if defined(NV_EVO4_PB_ALLOC_WAR)
#include "nvkms-ctxdma.h"
#endif

static NvU8 GetSurfaceAddressTarget(const NVDevEvoRec *pDevEvo,
                                    const NVSurfaceDescriptor *pSurfaceDesc)
{
    switch (pSurfaceDesc->memAperture) {
        case NV0041_CTRL_CMD_GET_SURFACE_PHYS_ATTR_APERTURE_VIDMEM:
            nvAssert(!pDevEvo->isSOCDisplay);
            return NVC97E_SET_SURFACE_ADDRESS_LO_ISO_TARGET_PHYSICAL_NVM;
        case NV0041_CTRL_CMD_GET_SURFACE_PHYS_ATTR_APERTURE_SYSMEM:
            if (pDevEvo->isSOCDisplay) {
                return NVC97E_SET_SURFACE_ADDRESS_LO_ISO_TARGET_IOVA;
            } else {
                return NVC97E_SET_SURFACE_ADDRESS_LO_ISO_TARGET_PHYSICAL_PCI_COHERENT;
            }
        default:
            nvAssert(!"Unknow memory aperture");
            break;
    }
    return 0x0;
}

static void GetSurfaceAddress(const NVDevEvoRec *pDevEvo,
                              const NVSurfaceDescriptor *pSurfaceDesc,
                              const NvU32 offset,
                              NvU32 *pAddressHi,
                              NvU32 *pAddressLo,
                              NvBool *pEnable,
                              NvU8 *pTarget)
{
    if (pSurfaceDesc == NULL) {
        *pAddressHi = *pAddressLo = 0;
        *pEnable = FALSE;
        *pTarget = 0x0;
        return;
    }

    NvU64 address = pSurfaceDesc->memOffset + offset;
    *pTarget = GetSurfaceAddressTarget(pDevEvo, pSurfaceDesc);
    *pAddressHi = (address >> 32) & 0xFFFFFFFF;
    *pAddressLo = ((address) & 0xFFFFFFFF) >> 4;
    *pEnable = TRUE;
}

static void InitScalerCoefficientsPostcomp9(NVDevEvoPtr pDevEvo,
                                            NVEvoChannelPtr pChannel,
                                            NvU32 coeff, NvU32 index)
{
    NvU32 h;

    for (h = 0; h < pDevEvo->numHeads; h++) {
        nvDmaSetStartEvoMethod(pChannel,
            NVC97D_HEAD_SET_OUTPUT_SCALER_COEFF_VALUE(h), 1);
        nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_OUTPUT_SCALER_COEFF_VALUE, _DATA, coeff) |
            DRF_NUM(C97D, _HEAD_SET_OUTPUT_SCALER_COEFF_VALUE, _INDEX, index));
    }
}

static void InitTaps5ScalerCoefficientsC9(NVDevEvoPtr pDevEvo,
                                          NVEvoChannelPtr pChannel,
                                          NvBool isPrecomp)
{
    NvU8 ratio;

    if (isPrecomp) {
        const NVEvoWindowCaps *pWinCaps =
            &pDevEvo->gpus[0].capabilities.window[pChannel->instance];
        const NVEvoScalerCaps *pScalerCaps = &pWinCaps->scalerCaps;

        if (!pScalerCaps->present) {
            return;
        }
    }

    for (ratio = 0; ratio < NUM_SCALER_RATIOS; ratio++) {
        NvU8 phase;
        for (phase = 0; phase < NUM_TAPS5_COEFF_PHASES; phase++) {
            NvU8 coeffIdx;
            for (coeffIdx = 0; coeffIdx < NUM_TAPS5_COEFF_VALUES; coeffIdx++) {
                NvU32 coeff = scalerTaps5Coeff[ratio][phase][coeffIdx];
                NvU32 index = ratio << 6 | phase << 2 | coeffIdx;

                if (isPrecomp) {
                    nvInitScalerCoefficientsPrecomp5(pChannel, coeff, index);
                } else {
                    InitScalerCoefficientsPostcomp9(pDevEvo,
                                                    pChannel, coeff, index);
                }
            }
        }
    }
}

static void InitDesktopColorC9(NVDevEvoPtr pDevEvo, NVEvoChannelPtr pChannel)
{
    NvU32 head;

    for (head = 0; head < pDevEvo->numHeads; head++) {
        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DESKTOP_COLOR_ALPHA_RED(head), 1);
        nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_DESKTOP_COLOR_ALPHA_RED, _ALPHA, 255) |
            DRF_NUM(C97D, _HEAD_SET_DESKTOP_COLOR_ALPHA_RED, _RED, 0));

        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DESKTOP_COLOR_GREEN_BLUE(head), 1);
        nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_DESKTOP_COLOR_GREEN_BLUE, _GREEN, 0) |
            DRF_NUM(C97D, _HEAD_SET_DESKTOP_COLOR_GREEN_BLUE, _BLUE, 0));
    }
}

static void EvoInitChannelC9(NVDevEvoPtr pDevEvo, NVEvoChannelPtr pChannel)
{
    const NvBool isCore =
            FLD_TEST_DRF64(_EVO, _CHANNEL_MASK, _CORE, _ENABLE,
                           pChannel->channelMask);
    const NvBool isWindow =
        ((pChannel->channelMask & NV_EVO_CHANNEL_MASK_WINDOW_ALL) != 0);

    nvEvoInitChannel3(pDevEvo, pChannel);

    if (isCore) {
        InitTaps5ScalerCoefficientsC9(pDevEvo, pChannel, FALSE);
        InitDesktopColorC9(pDevEvo, pChannel);
    } else if (isWindow) {
        InitTaps5ScalerCoefficientsC9(pDevEvo, pChannel, TRUE);
    }
}

static void EvoSetRasterParams9(NVDevEvoPtr pDevEvo, int head,
                                const NVHwModeTimingsEvo *pTimings,
                                const NVEvoColorRec *pOverscanColor,
                                NVEvoUpdateState *updateState)
{
    NVEvoChannelPtr pChannel = pDevEvo->core;
    /* XXXnvdisplay: Convert these for YCbCr, as necessary */
    NvU32 overscanColor =
        DRF_NUM(C97D, _HEAD_SET_OVERSCAN_COLOR, _RED_CR, pOverscanColor->red) |
        DRF_NUM(C97D, _HEAD_SET_OVERSCAN_COLOR, _GREEN_Y, pOverscanColor->green) |
        DRF_NUM(C97D, _HEAD_SET_OVERSCAN_COLOR, _BLUE_CB, pOverscanColor->blue);
    NvU32 hdmiStereoCtrl;
    NvU16 minFrameIdleLeadingRasterLines, minFrameIdleTrailingRasterLines;
    NvBool ret;
    const NvU64 pixelClockHz = KHzToHz(pTimings->pixelClock);
    const NvU32 pixelClockLo = (pixelClockHz & DRF_MASK(NVC97D_HEAD_SET_PIXEL_CLOCK_FREQUENCY_HERTZ));
    const NvU32 pixelClockHi = (pixelClockHz >> DRF_SIZE(NVC97D_HEAD_SET_PIXEL_CLOCK_FREQUENCY_HERTZ)) &
        DRF_MASK(NVC97D_HEAD_SET_PIXEL_CLOCK_FREQUENCY_HI_HERTZ);

    /* These methods should only apply to a single pDpy */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    // XXX[AGP]: These methods are sequential and could use an incrementing
    // method, but it's not clear if there's a bug in EVO that causes corruption
    // sometimes.  Play it safe and send methods with count=1.

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_OVERSCAN_COLOR(head), 1);
    nvDmaSetEvoMethodData(pChannel, overscanColor);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_RASTER_SIZE(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_RASTER_SIZE, _WIDTH, pTimings->rasterSize.x) |
        DRF_NUM(C97D, _HEAD_SET_RASTER_SIZE, _HEIGHT, pTimings->rasterSize.y));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_RASTER_SYNC_END(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_RASTER_SYNC_END, _X, pTimings->rasterSyncEnd.x) |
        DRF_NUM(C97D, _HEAD_SET_RASTER_SYNC_END, _Y, pTimings->rasterSyncEnd.y));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_RASTER_BLANK_END(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_RASTER_BLANK_END, _X, pTimings->rasterBlankEnd.x) |
        DRF_NUM(C97D, _HEAD_SET_RASTER_BLANK_END, _Y, pTimings->rasterBlankEnd.y));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_RASTER_BLANK_START(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_RASTER_BLANK_START, _X, pTimings->rasterBlankStart.x) |
        DRF_NUM(C97D, _HEAD_SET_RASTER_BLANK_START, _Y, pTimings->rasterBlankStart.y));

    ret = nvComputeMinFrameIdle(pTimings,
                              &minFrameIdleLeadingRasterLines,
                              &minFrameIdleTrailingRasterLines);
    if (!ret) {
        /* This should have been ensured by IMP in AssignPerHeadImpParams. */
        nvAssert(ret);
        /* In case a mode validation override was used to skip IMP, program the
         * default values.  This may still cause a hardware exception. */
        minFrameIdleLeadingRasterLines = 2;
        minFrameIdleTrailingRasterLines = 1;
    }

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_MIN_FRAME_IDLE(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_MIN_FRAME_IDLE, _LEADING_RASTER_LINES,
                minFrameIdleLeadingRasterLines) |
        DRF_NUM(C97D, _HEAD_SET_MIN_FRAME_IDLE, _TRAILING_RASTER_LINES,
                minFrameIdleTrailingRasterLines));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_PIXEL_CLOCK_FREQUENCY(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_PIXEL_CLOCK_FREQUENCY, _HERTZ, pixelClockLo) |
        DRF_DEF(C97D, _HEAD_SET_PIXEL_CLOCK_FREQUENCY, _ADJ1000DIV1001,_FALSE));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_PIXEL_CLOCK_FREQUENCY_HI(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_PIXEL_CLOCK_FREQUENCY_HI, _HERTZ, pixelClockHi));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_PIXEL_CLOCK_CONFIGURATION(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_DEF(C97D, _HEAD_SET_PIXEL_CLOCK_CONFIGURATION, _NOT_DRIVER, _FALSE) |
        DRF_DEF(C97D, _HEAD_SET_PIXEL_CLOCK_CONFIGURATION, _HOPPING, _DISABLE) |
        DRF_DEF(C97D, _HEAD_SET_PIXEL_CLOCK_CONFIGURATION, _HOPPING_MODE, _VBLANK));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_PIXEL_CLOCK_FREQUENCY_MAX(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_PIXEL_CLOCK_FREQUENCY_MAX, _HERTZ, pixelClockLo) |
        DRF_DEF(C97D, _HEAD_SET_PIXEL_CLOCK_FREQUENCY_MAX, _ADJ1000DIV1001,_FALSE));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_PIXEL_CLOCK_FREQUENCY_HI_MAX(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_PIXEL_CLOCK_FREQUENCY_HI_MAX, _HERTZ, pixelClockHi));

    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_FRAME_PACKED_VACTIVE_COLOR(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_FRAME_PACKED_VACTIVE_COLOR, _RED_CR, 0) |
#if defined(DEBUG)
        DRF_NUM(C97D, _HEAD_SET_FRAME_PACKED_VACTIVE_COLOR, _GREEN_Y,  512) |
#else
        DRF_NUM(C97D, _HEAD_SET_FRAME_PACKED_VACTIVE_COLOR, _GREEN_Y,  0) |
#endif
        DRF_NUM(C97D, _HEAD_SET_FRAME_PACKED_VACTIVE_COLOR, _BLUE_CB, 0));

    hdmiStereoCtrl = DRF_NUM(C97D, _HEAD_SET_HDMI_CTRL, _HDMI_VIC, 0);
    if (pTimings->hdmi3D) {
        hdmiStereoCtrl =
            FLD_SET_DRF(C97D, _HEAD_SET_HDMI_CTRL, _VIDEO_FORMAT, _STEREO3D, hdmiStereoCtrl);
    } else {
        hdmiStereoCtrl =
            FLD_SET_DRF(C97D, _HEAD_SET_HDMI_CTRL, _VIDEO_FORMAT, _NORMAL, hdmiStereoCtrl);
    }
    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_HDMI_CTRL(head), 1);
    nvDmaSetEvoMethodData(pChannel, hdmiStereoCtrl);
}

static void EvoSetRasterParamsC9(NVDevEvoPtr pDevEvo, int head,
                                 const NVHwModeTimingsEvo *pTimings,
                                 const NvU8 tilePosition,
                                 const NVDscInfoEvoRec *pDscInfo,
                                 const NVEvoColorRec *pOverscanColor,
                                 NVEvoUpdateState *updateState)
{
    nvAssert(tilePosition == 0);
    EvoSetRasterParams9(pDevEvo, head, pTimings, pOverscanColor, updateState);
}

static void EvoSetOCsc1C9(NVDispEvoPtr pDispEvo, const NvU32 head)
{
    NVDevEvoPtr pDevEvo = pDispEvo->pDevEvo;
    NVEvoChannelPtr pChannel = pDevEvo->core;
    const NVDispHeadStateEvoRec *pHeadState = &pDispEvo->headState[head];
    const struct NvKmsCscMatrix *matrix = nvEvoGetOCsc1MatrixC5(pHeadState);
    struct EvoClampRangeC5 clamp = nvEvoGetOCsc1ClampRange(pHeadState);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_CLAMP_RANGE_GREEN(head), 1);
    nvDmaSetEvoMethodData(pChannel, clamp.green);
    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_CLAMP_RANGE_RED_BLUE(head), 1);
    nvDmaSetEvoMethodData(pChannel, clamp.red_blue);

    if (matrix) {
        int x, y;
        NvU32 method = NVC97D_HEAD_SET_OCSC1COEFFICIENT_C00(head);

        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_OCSC1CONTROL(head), 1);
        nvDmaSetEvoMethodData(pChannel,
            DRF_DEF(C97D, _HEAD_SET_OCSC1CONTROL, _ENABLE, _ENABLE));

        for (y = 0; y < 3; y++) {
            for (x = 0; x < 4; x++) {
                nvDmaSetStartEvoMethod(pChannel, method, 1);
                nvDmaSetEvoMethodData(pChannel, matrix->m[y][x]);

                method += 4;
            }
        }
    } else {
        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_OCSC1CONTROL(head), 1);
        nvDmaSetEvoMethodData(pChannel,
            DRF_DEF(C97D, _HEAD_SET_OCSC1CONTROL, _ENABLE, _DISABLE));
    }
}

static void EvoSetOCsc0C9(const NVDispEvoRec *pDispEvo, const NvU32 head,
                          NvBool *pOutputRoundingFix)
{
    NVDevEvoPtr pDevEvo = pDispEvo->pDevEvo;
    NVEvoChannelPtr pChannel = pDevEvo->core;
    struct NvKms3x4MatrixF32 ocsc0Matrix;

    nvEvo3PickOCsc0(pDispEvo, head, &ocsc0Matrix, pOutputRoundingFix);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_OCSC0COEFFICIENT_C00(head), 12);
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C00, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[0][0])));
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C01, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[0][1])));
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C02, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[0][2])));
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C03, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[0][3])));

    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C10, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[1][0])));
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C11, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[1][1])));
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C12, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[1][2])));
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C13, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[1][3])));

    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C20, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[2][0])));
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C21, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[2][1])));
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C22, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[2][2])));
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_OCSC0COEFFICIENT_C23, _VALUE, nvCscCoefConvertS514(ocsc0Matrix.m[2][3])));
}

static void EvoSetProcAmpC9(NVDispEvoPtr pDispEvo, const NvU32 head,
                            NVEvoUpdateState *updateState)
{
    NVDevEvoPtr pDevEvo = pDispEvo->pDevEvo;
    NVEvoChannelPtr pChannel = pDevEvo->core;
    const NVDispHeadStateEvoRec *pHeadState = &pDispEvo->headState[head];
    NvU32 dynRange, chromaLpf, chromaDownV;
    NvU32 colorimetry;
    NvBool outputRoundingFix;

    NVT_COLORIMETRY nvtColorimetry = pHeadState->procAmp.colorimetry;
    NVT_COLOR_RANGE nvtColorRange = pHeadState->procAmp.colorRange;

    /* These methods should only apply to a single pDpyEvo */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    switch (nvtColorimetry) {
        default:
            nvAssert(!"Unrecognized colorimetry");
            // fall through
        case NVT_COLORIMETRY_BT2020RGB:
            // fall through
        case NVT_COLORIMETRY_RGB:
            colorimetry = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _COLOR_SPACE, _RGB);
            break;
        case NVT_COLORIMETRY_YUV_601:
            colorimetry = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _COLOR_SPACE, _YUV_601);
            break;
        case NVT_COLORIMETRY_YUV_709:
            colorimetry = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _COLOR_SPACE, _YUV_709);
            break;
        case NVT_COLORIMETRY_BT2020YCC:
            colorimetry = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _COLOR_SPACE, _YUV_2020);
            break;
    }

    if (nvtColorRange == NVT_COLOR_RANGE_FULL) {
        dynRange = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _DYNAMIC_RANGE, _VESA);
    } else {
        nvAssert(nvtColorRange == NVT_COLOR_RANGE_LIMITED);
        dynRange = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _DYNAMIC_RANGE, _CEA);
    }

    if (pHeadState->procAmp.colorFormat == NVT_COLOR_FORMAT_YCbCr420) {
        chromaLpf = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _CHROMA_LPF, _ENABLE);
        chromaDownV = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _CHROMA_DOWN_V, _ENABLE);
    } else {
        chromaLpf = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _CHROMA_LPF, _DISABLE);
        chromaDownV = DRF_DEF(C97D, _HEAD_SET_PROCAMP, _CHROMA_DOWN_V, _DISABLE);
    }

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_PROCAMP(head), 1);
    nvDmaSetEvoMethodData(pChannel,
                          colorimetry | dynRange | chromaLpf | chromaDownV);

    EvoSetOCsc0C9(pDispEvo, head, &outputRoundingFix);
    EvoSetOCsc1C9(pDispEvo, head);
}

static void EvoSetHeadControlC9(NVDevEvoPtr pDevEvo, int sd, int head,
                                NVEvoUpdateState *updateState)
{
    NVEvoChannelPtr pChannel = pDevEvo->core;
    NVEvoSubDevPtr pEvoSubDev = &pDevEvo->gpus[sd];
    /*
     * NOTE: This function should only push state to the hardware based on data
     * in the pHC.  If not, then we may miss updates due to the memcmp of the
     * HeadControl structure in UpdateEvoLockState().
     */
    NVEvoHeadControlPtr pHC = &pEvoSubDev->headControl[head];
    NvU32 data = 0, pin;
    NvU32 serverLockMode, clientLockMode;

    /* These methods should only apply to a single subdevice */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    switch (pHC->serverLock) {
    case NV_EVO_NO_LOCK:
        serverLockMode = NVC97D_HEAD_SET_CONTROL_MASTER_LOCK_MODE_NO_LOCK;
        break;
    /*
     * NOTE: Rasterlock is being dropped in NVD5.0. MASTER/SLAVE_LOCK_MODE_RASTER_LOCK
     * fields have been retained just for compatibility purposes, and will anyways
     * result in framelock being configured.
     */
    case NV_EVO_RASTER_LOCK:
    case NV_EVO_FRAME_LOCK:
        serverLockMode = NVC97D_HEAD_SET_CONTROL_MASTER_LOCK_MODE_FRAME_LOCK;
        break;
    default:
        nvAssert(!"Invalid server lock mode");
        return;
    }

    switch (pHC->clientLock) {
    case NV_EVO_NO_LOCK:
        clientLockMode = NVC97D_HEAD_SET_CONTROL_SLAVE_LOCK_MODE_NO_LOCK;
        break;
    case NV_EVO_RASTER_LOCK:
    case NV_EVO_FRAME_LOCK:
        clientLockMode = NVC97D_HEAD_SET_CONTROL_SLAVE_LOCK_MODE_FRAME_LOCK;
        break;
    default:
        nvAssert(!"Invalid client lock mode");
        return;
    }

    // Convert head control state to EVO method values.
    nvAssert(!pHC->interlaced);
    data |= DRF_DEF(C97D, _HEAD_SET_CONTROL, _STRUCTURE, _PROGRESSIVE);

    nvAssert(pHC->serverLockPin != NV_EVO_LOCK_PIN_ERROR);
    nvAssert(pHC->clientLockPin != NV_EVO_LOCK_PIN_ERROR);

    if (serverLockMode == NVC97D_HEAD_SET_CONTROL_MASTER_LOCK_MODE_NO_LOCK) {
        data |= DRF_DEF(C97D, _HEAD_SET_CONTROL, _MASTER_LOCK_PIN, _LOCK_PIN_NONE);
    } else if (NV_EVO_LOCK_PIN_IS_INTERNAL(pHC->serverLockPin)) {
        pin = pHC->serverLockPin - NV_EVO_LOCK_PIN_INTERNAL_0;
        /*
         * nvdClass_01.mfs says:
         * "master lock pin, if internal, must be set to the corresponding
         * internal pin for that head" (error check #12)
         */
        nvAssert(pin == head);
        data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _MASTER_LOCK_PIN,
                        NVC97D_HEAD_SET_CONTROL_MASTER_LOCK_PIN_INTERNAL_SCAN_LOCK(pin));
    } else {
        pin = pHC->serverLockPin - NV_EVO_LOCK_PIN_0;
        data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _MASTER_LOCK_PIN,
                        NVC97D_HEAD_SET_CONTROL_MASTER_LOCK_PIN_LOCK_PIN(pin));
    }
    data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _MASTER_LOCK_MODE, serverLockMode);

    if (clientLockMode == NVC97D_HEAD_SET_CONTROL_SLAVE_LOCK_MODE_NO_LOCK) {
        data |= DRF_DEF(C97D, _HEAD_SET_CONTROL, _SLAVE_LOCK_PIN, _LOCK_PIN_NONE);
    } else if (NV_EVO_LOCK_PIN_IS_INTERNAL(pHC->clientLockPin)) {
        pin = pHC->clientLockPin - NV_EVO_LOCK_PIN_INTERNAL_0;
        data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _SLAVE_LOCK_PIN,
                        NVC97D_HEAD_SET_CONTROL_SLAVE_LOCK_PIN_INTERNAL_SCAN_LOCK(pin));
    } else {
        pin = pHC->clientLockPin - NV_EVO_LOCK_PIN_0;
        data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _SLAVE_LOCK_PIN,
                        NVC97D_HEAD_SET_CONTROL_SLAVE_LOCK_PIN_LOCK_PIN(pin));
    }
    data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _SLAVE_LOCK_MODE, clientLockMode);
    data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _SLAVE_LOCKOUT_WINDOW,
                    pHC->clientLockoutWindow);

    /*
     * We always enable stereo lock when it's available and either framelock
     * or rasterlock is in use.
     */
    if (pHC->stereoLocked) {
        if (pHC->serverLock != NV_EVO_NO_LOCK) {
            data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _MASTER_STEREO_LOCK_MODE,
                            NVC97D_HEAD_SET_CONTROL_MASTER_STEREO_LOCK_MODE_ENABLE);
        }
        if (pHC->clientLock != NV_EVO_NO_LOCK) {
            data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _SLAVE_STEREO_LOCK_MODE,
                            NVC97D_HEAD_SET_CONTROL_SLAVE_STEREO_LOCK_MODE_ENABLE);
        }
    }

    nvAssert(pHC->stereoPin != NV_EVO_LOCK_PIN_ERROR);
    if (NV_EVO_LOCK_PIN_IS_INTERNAL(pHC->stereoPin)) {
        data |= DRF_DEF(C97D, _HEAD_SET_CONTROL, _STEREO_PIN, _LOCK_PIN_NONE);
    } else {
        pin = pHC->stereoPin - NV_EVO_LOCK_PIN_0;
        data |= DRF_NUM(C97D, _HEAD_SET_CONTROL, _STEREO_PIN,
                        NVC97D_HEAD_SET_CONTROL_STEREO_PIN_LOCK_PIN(pin));
    }

    if (pHC->hdmi3D) {
        data |= DRF_DEF(C97D, _HEAD_SET_CONTROL, _STEREO3D_STRUCTURE, _FRAME_PACKED);
    } else {
        data |= DRF_DEF(C97D, _HEAD_SET_CONTROL, _STEREO3D_STRUCTURE, _NORMAL);
    }

    if (pHC->hwYuv420) {
        data |= DRF_DEF(C97D, _HEAD_SET_CONTROL, _YUV420PACKER, _ENABLE);
    } else {
        data |= DRF_DEF(C97D, _HEAD_SET_CONTROL, _YUV420PACKER, _DISABLE);
    }

    // Send the HeadSetControl method.
    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_CONTROL(head), 1);
    nvDmaSetEvoMethodData(pChannel, data);

    nvEvoSetControlC3(pDevEvo, sd);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_LOCK_CHAIN(head), 1);
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_LOCK_CHAIN, _POSITION,
                                     pHC->lockChainPosition));
}

static void EvoHeadSetControlORC9(NVDevEvoPtr pDevEvo,
                                  const int head,
                                  const NVHwModeTimingsEvo *pTimings,
                                  const enum nvKmsPixelDepth pixelDepth,
                                  const NvBool colorSpaceOverride,
                                  NVEvoUpdateState *updateState)
{
    NVEvoChannelPtr pChannel = pDevEvo->core;
    const NvU32 hwPixelDepth = nvEvoGetPixelDepthC3(pixelDepth);
    const NvU16 colorSpaceFlag = nvEvo1GetColorSpaceFlag(pDevEvo,
                                                         colorSpaceOverride);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_CONTROL_OUTPUT_RESOURCE(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_DEF(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _CRC_MODE, _COMPLETE_RASTER) |
        (pTimings->hSyncPol ?
            DRF_DEF(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _HSYNC_POLARITY, _NEGATIVE_TRUE) :
            DRF_DEF(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _HSYNC_POLARITY, _POSITIVE_TRUE)) |
        (pTimings->vSyncPol ?
            DRF_DEF(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _VSYNC_POLARITY, _NEGATIVE_TRUE) :
            DRF_DEF(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _VSYNC_POLARITY, _POSITIVE_TRUE)) |
         DRF_NUM(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _PIXEL_DEPTH, hwPixelDepth) |
        (colorSpaceOverride ?
            (DRF_DEF(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _COLOR_SPACE_OVERRIDE, _ENABLE) |
             DRF_NUM(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _COLOR_SPACE_FLAG, colorSpaceFlag)) :
            DRF_DEF(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _COLOR_SPACE_OVERRIDE, _DISABLE)) |
        DRF_DEF(C97D, _HEAD_SET_CONTROL_OUTPUT_RESOURCE, _EXT_PACKET_WIN, _NONE));
}

static void EvoHeadSetDisplayIdC9(NVDevEvoPtr pDevEvo,
                                  const NvU32 head, const NvU32 displayId,
                                  NVEvoUpdateState *updateState)
{
    NVEvoChannelPtr pChannel = pDevEvo->core;

    /* These methods should only apply to a single pDpy */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DISPLAY_ID(head, 0), 1);
    nvDmaSetEvoMethodData(pChannel, displayId);
}

static void SetOLUTSurfaceAddress(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 offset,
    NvU32 head)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    GetSurfaceAddress(pDevEvo, pSurfaceDesc, offset, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_SURFACE_ADDRESS_HI_OLUT(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_HI_OLUT, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_OLUT,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_OLUT,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_OLUT,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_SURFACE_ADDRESS_LO_OLUT(head), 1);
    nvDmaSetEvoMethodData(pChannel, value);
}

static void EvoSetOutputLutC9(NVDevEvoPtr pDevEvo, NvU32 sd, NvU32 head,
                              const NVFlipLutHwState *pOutputLut,
                              NvU32 fpNormScale,
                              NVEvoUpdateState *updateState,
                              NvBool bypassComposition)
{
    const NVDispEvoRec *pDispEvo = pDevEvo->pDispEvo[sd];
    NVEvoChannelPtr pChannel = pDevEvo->core;
    NvBool enableOutputLut = (pOutputLut->pLutSurfaceEvo != NULL);
    NVSurfaceEvoPtr pLutSurfEvo = pOutputLut->pLutSurfaceEvo;
    NVSurfaceDescriptor *pSurfaceDesc =
        enableOutputLut ? &pLutSurfEvo->planes[0].surfaceDesc : NULL;
    NvU64 offset = enableOutputLut ? pOutputLut->offset : offsetof(NVEvoLutDataRec, output);
    NvBool isLutModeVss = enableOutputLut ? (pOutputLut->vssSegments != 0) : FALSE;
    NvU32 lutSize = enableOutputLut ? pOutputLut->lutEntries : NV_NUM_EVO_LUT_ENTRIES;
    const NVDispHeadStateEvoRec *pHeadState = &pDispEvo->headState[head];
    NvBool disableOcsc0 = FALSE;
    NvBool outputRoundingFix = nvkms_output_rounding_fix();

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    // XXX HDR TODO: Enable custom output LUTs with HDR
    // XXX HDR TODO: Support other transfer functions
    if (!pOutputLut->fromOverride &&
        (pHeadState->tf == NVKMS_OUTPUT_TF_PQ)) {
        enableOutputLut = FALSE;
    }

    nvSetupOutputLUT5(pDevEvo,
                      pHeadState,
                      enableOutputLut,
                      bypassComposition,
                      &pSurfaceDesc,
                      &lutSize,
                      &offset,
                      &disableOcsc0,
                      &fpNormScale,
                      &isLutModeVss);

    if (disableOcsc0) {
        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_OCSC0CONTROL(head), 1);
        nvDmaSetEvoMethodData(pChannel, DRF_DEF(C97D, _HEAD_SET_OCSC0CONTROL, _ENABLE, _DISABLE));

        outputRoundingFix = FALSE;
    } else {
        /* Update status of output rounding fix. */
        EvoSetOCsc0C9(pDispEvo, head, &outputRoundingFix);
    }

    /* Program the output LUT */
    nvAssert((offset & 0xff) == 0);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_OLUT_CONTROL(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        ((isLutModeVss || !outputRoundingFix) ?
            DRF_DEF(C97D, _HEAD_SET_OLUT_CONTROL, _INTERPOLATE, _ENABLE) :
            DRF_DEF(C97D, _HEAD_SET_OLUT_CONTROL, _INTERPOLATE, _DISABLE)) |
        DRF_DEF(C97D, _HEAD_SET_OLUT_CONTROL, _MIRROR, _DISABLE) |
        (isLutModeVss ? DRF_DEF(C97D, _HEAD_SET_OLUT_CONTROL, _MODE, _SEGMENTED) :
                        DRF_DEF(C97D, _HEAD_SET_OLUT_CONTROL, _MODE, _DIRECT10)) |
        DRF_NUM(C97D, _HEAD_SET_OLUT_CONTROL, _SIZE, NV_LUT_VSS_HEADER_SIZE + lutSize));

    SetOLUTSurfaceAddress(pDevEvo, pChannel, pSurfaceDesc, offset, head);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_OLUT_FP_NORM_SCALE(head), 1);
    nvDmaSetEvoMethodData(pChannel, fpNormScale);

    if (!disableOcsc0) {
        /* only enable OCSC0 after enabling the OLUT */
        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_OCSC0CONTROL(head), 1);
        nvDmaSetEvoMethodData(pChannel, DRF_DEF(C97D, _HEAD_SET_OCSC0CONTROL, _ENABLE, _ENABLE));
    }
}

static void EvoSetViewportPointInC9(NVDevEvoPtr pDevEvo, const int head,
                                    NvU16 x, NvU16 y,
                                    NVEvoUpdateState *updateState)
{
    NVEvoChannelPtr pChannel = pDevEvo->core;

    /* These methods should only apply to a single pDpy */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    /* Set the input viewport point */
    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_VIEWPORT_POINT_IN(head), 1);
    nvDmaSetEvoMethodData(pChannel, DRF_NUM(C97D, _HEAD_SET_VIEWPORT_POINT_IN, _X, x) |
                             DRF_NUM(C97D, _HEAD_SET_VIEWPORT_POINT_IN, _Y, y));
    /* XXXnvdisplay set ViewportValidPointIn to configure overfetch */
}

static void EvoSetOutputScalerC9(const NVDispEvoRec *pDispEvo, const NvU32 head,
                                 const NvU32 imageSharpeningValue,
                                 NVEvoUpdateState *updateState)
{
    NVDevEvoPtr pDevEvo = pDispEvo->pDevEvo;
    NVEvoChannelPtr pChannel = pDevEvo->core;
    const NVDispHeadStateEvoRec *pHeadState = &pDispEvo->headState[head];
    const NVHwModeViewPortEvo *pViewPort = &pHeadState->timings.viewPort;

    /* These methods should only apply to a single pDpyEvo */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    NvU32 vTaps = pViewPort->vTaps > NV_EVO_SCALER_2TAPS ?
                    NVC97D_HEAD_SET_CONTROL_OUTPUT_SCALER_VERTICAL_TAPS_TAPS_5 :
                    NVC97D_HEAD_SET_CONTROL_OUTPUT_SCALER_VERTICAL_TAPS_TAPS_2;
    NvU32 hTaps = pViewPort->hTaps > NV_EVO_SCALER_2TAPS ?
                    NVC97D_HEAD_SET_CONTROL_OUTPUT_SCALER_HORIZONTAL_TAPS_TAPS_5 :
                    NVC97D_HEAD_SET_CONTROL_OUTPUT_SCALER_HORIZONTAL_TAPS_TAPS_2;

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_CONTROL_OUTPUT_SCALER(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_CONTROL_OUTPUT_SCALER, _VERTICAL_TAPS, vTaps) |
        DRF_NUM(C97D, _HEAD_SET_CONTROL_OUTPUT_SCALER, _HORIZONTAL_TAPS, hTaps));
}

static NvBool EvoSetViewportInOut9(NVDevEvoPtr pDevEvo, const int head,
                                   const NVHwModeViewPortEvo *pViewPortMin,
                                   const NVHwModeViewPortEvo *pViewPort,
                                   const NVHwModeViewPortEvo *pViewPortMax,
                                   NVEvoUpdateState *updateState,
                                   NvU32 setWindowUsageBounds)
{
    const NVEvoCapabilitiesPtr pEvoCaps = &pDevEvo->gpus[0].capabilities;
    NVEvoChannelPtr pChannel = pDevEvo->core;
    struct NvKmsScalingUsageBounds scalingUsageBounds = { };
    NvU32 win;

    /* These methods should only apply to a single pDpy */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    /* The input viewport shouldn't vary. */
    nvAssert(pViewPortMin->in.width == pViewPort->in.width);
    nvAssert(pViewPortMax->in.width == pViewPort->in.width);
    nvAssert(pViewPortMin->in.height == pViewPort->in.height);
    nvAssert(pViewPortMax->in.height == pViewPort->in.height);
    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_VIEWPORT_SIZE_IN(head), 1);
    nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_VIEWPORT_SIZE_IN, _WIDTH, pViewPort->in.width) |
            DRF_NUM(C97D, _HEAD_SET_VIEWPORT_SIZE_IN, _HEIGHT, pViewPort->in.height));
    /* XXXnvdisplay set ViewportValidSizeIn to configure overfetch */

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_VIEWPORT_POINT_OUT_ADJUST(head), 1);
    nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_VIEWPORT_POINT_OUT, _ADJUST_X, pViewPort->out.xAdjust) |
            DRF_NUM(C97D, _HEAD_SET_VIEWPORT_POINT_OUT, _ADJUST_Y, pViewPort->out.yAdjust));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_VIEWPORT_SIZE_OUT(head), 1);
    nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_VIEWPORT_SIZE_OUT, _WIDTH, pViewPort->out.width) |
            DRF_NUM(C97D, _HEAD_SET_VIEWPORT_SIZE_OUT, _HEIGHT, pViewPort->out.height));

    /* XXXnvdisplay deal with pViewPortMin, pViewPortMax */

    if (!nvComputeScalingUsageBounds(&pEvoCaps->head[head].scalerCaps,
                                   pViewPort->in.width, pViewPort->in.height,
                                   pViewPort->out.width, pViewPort->out.height,
                                   pViewPort->hTaps, pViewPort->vTaps,
                                   &scalingUsageBounds)) {
        /* Should have been rejected by validation */
        nvAssert(!"Attempt to program invalid viewport");
    }

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_MAX_OUTPUT_SCALE_FACTOR(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_MAX_OUTPUT_SCALE_FACTOR, _HORIZONTAL,
                scalingUsageBounds.maxHDownscaleFactor) |
        DRF_NUM(C97D, _HEAD_SET_MAX_OUTPUT_SCALE_FACTOR, _VERTICAL,
                scalingUsageBounds.maxVDownscaleFactor));

    /*
     * Program MAX_PIXELS_FETCHED_PER_LINE window usage bounds
     * for each window that is attached to the head.
     *
     * Precomp will clip the post-scaled window to the input viewport, reverse-scale
     * this cropped size back to the input surface domain, and isohub will fetch
     * this cropped size. This function assumes that there's no window scaling yet,
     * so the MAX_PIXELS_FETCHED_PER_LINE will be bounded by the input viewport
     * width. SetScalingUsageBoundsOneWindow5() will take care of updating
     * MAX_PIXELS_FETCHED_PER_LINE, if window scaling is enabled later.
     *
     * Program MAX_PIXELS_FETCHED_PER_LINE for each window that is attached to
     * head. For Turing+, SetScalingUsageBoundsOneWindow5() will take care of
     * programming window usage bounds only for the layers/windows in use.
     */
    setWindowUsageBounds |=
        DRF_NUM(C97D, _WINDOW_SET_WINDOW_USAGE_BOUNDS, _MAX_PIXELS_FETCHED_PER_LINE,
                nvGetMaxPixelsFetchedPerLine(pViewPort->in.width,
                NV_EVO_SCALE_FACTOR_1X));

    for (win = 0; win < pDevEvo->numWindows; win++) {
        if (head != pDevEvo->headForWindow[win]) {
            continue;
        }

        nvDmaSetStartEvoMethod(pChannel, NVC97D_WINDOW_SET_WINDOW_USAGE_BOUNDS(win), 1);
        nvDmaSetEvoMethodData(pChannel, setWindowUsageBounds);
    }

    return scalingUsageBounds.vUpscalingAllowed;
}

static void EvoSetViewportInOutC9(NVDevEvoPtr pDevEvo, const int head,
                                  const NVHwModeViewPortEvo *pViewPortMin,
                                  const NVHwModeViewPortEvo *pViewPort,
                                  const NVHwModeViewPortEvo *pViewPortMax,
                                  NVEvoUpdateState *updateState)
{
    NVEvoChannelPtr pChannel = pDevEvo->core;
    NvU32 setWindowUsageBounds =
        (NV_EVO3_DEFAULT_WINDOW_USAGE_BOUNDS_C5 |
         DRF_DEF(C97D, _WINDOW_SET_WINDOW_USAGE_BOUNDS, _INPUT_SCALER_TAPS, _TAPS_2) |
         DRF_DEF(C97D, _WINDOW_SET_WINDOW_USAGE_BOUNDS, _UPSCALING_ALLOWED, _FALSE));
    NvU32 verticalUpscalingAllowed =
        EvoSetViewportInOut9(pDevEvo, head, pViewPortMin, pViewPort,
                             pViewPortMax, updateState, setWindowUsageBounds);

    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_HEAD_USAGE_BOUNDS(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_DEF(C97D, _HEAD_SET_HEAD_USAGE_BOUNDS, _CURSOR, _USAGE_W256_H256) |
        DRF_DEF(C97D, _HEAD_SET_HEAD_USAGE_BOUNDS, _OLUT_ALLOWED, _TRUE) |
        /* Despite the generic name of this field, it's specific to vertical taps. */
        (pViewPort->vTaps > NV_EVO_SCALER_2TAPS ?
            DRF_DEF(C97D, _HEAD_SET_HEAD_USAGE_BOUNDS, _OUTPUT_SCALER_TAPS, _TAPS_5) :
            DRF_DEF(C97D, _HEAD_SET_HEAD_USAGE_BOUNDS, _OUTPUT_SCALER_TAPS, _TAPS_2)) |
        (verticalUpscalingAllowed ?
            DRF_DEF(C97D, _HEAD_SET_HEAD_USAGE_BOUNDS, _UPSCALING_ALLOWED, _TRUE) :
            DRF_DEF(C97D, _HEAD_SET_HEAD_USAGE_BOUNDS, _UPSCALING_ALLOWED, _FALSE)));
}

static void SetCursorSurfaceAddress(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 offset,
    NvU32 head)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    GetSurfaceAddress(pDevEvo, pSurfaceDesc, offset, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_SURFACE_ADDRESS_HI_CURSOR(head, 0), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_HI_CURSOR, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_CURSOR,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_CURSOR,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_CURSOR,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_SURFACE_ADDRESS_LO_CURSOR(head, 0), 1);
    nvDmaSetEvoMethodData(pChannel, value);
}

static void EvoSetCursorImageC9(NVDevEvoPtr pDevEvo, const int head,
                                const NVSurfaceEvoRec *pSurfaceEvo,
                                NVEvoUpdateState *updateState,
                                const struct NvKmsCompositionParams *pCursorCompParams)
{
    NVEvoChannelPtr pChannel = pDevEvo->core;
    const NVSurfaceDescriptor *pSurfaceDesc =
        pSurfaceEvo ? &pSurfaceEvo->planes[0].surfaceDesc : NULL;
    const NvU64 offset = pSurfaceEvo ? pSurfaceEvo->planes[0].offset : 0;
    NvU32 headSetControlCursorValue = 0;
    NvBool ret;

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);
    nvAssert(pCursorCompParams->colorKeySelect ==
                NVKMS_COMPOSITION_COLOR_KEY_SELECT_DISABLE);
    nvAssert(NVBIT(pCursorCompParams->blendingMode[1]) &
                NV_EVO3_SUPPORTED_CURSOR_COMP_BLEND_MODES);
    /* These methods should only apply to a single pDpy */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    ret = nvEvoGetHeadSetControlCursorValueC3(pDevEvo, pSurfaceEvo,
                                            &headSetControlCursorValue);
    /*
     * The caller should have already validated the surface, so there
     * shouldn't be a failure.
     */
    if (!ret) {
        nvAssert(!"Could not construct HEAD_SET_CONTROL_CURSOR value");
    }

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_PRESENT_CONTROL_CURSOR(head), 1);
    nvDmaSetEvoMethodData(pChannel,
            DRF_DEF(C97D, _HEAD_SET_PRESENT_CONTROL_CURSOR, _MODE, _MONO));

    SetCursorSurfaceAddress(pDevEvo, pChannel, pSurfaceDesc, offset, head);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_CONTROL_CURSOR(head), 1);
    nvDmaSetEvoMethodData(pChannel, headSetControlCursorValue);

    nvDmaSetStartEvoMethod(pChannel,
            NVC97D_HEAD_SET_CONTROL_CURSOR_COMPOSITION(head), 1);
    switch (pCursorCompParams->blendingMode[1]) {
    case NVKMS_COMPOSITION_BLENDING_MODE_OPAQUE:
        nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _K1, 255) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _CURSOR_COLOR_FACTOR_SELECT, _K1) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _VIEWPORT_COLOR_FACTOR_SELECT, _ZERO) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _MODE, _BLEND));
        break;
    case NVKMS_COMPOSITION_BLENDING_MODE_NON_PREMULT_ALPHA:
        nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _K1, 255) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _CURSOR_COLOR_FACTOR_SELECT, _K1_TIMES_SRC) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _VIEWPORT_COLOR_FACTOR_SELECT, _NEG_K1_TIMES_SRC) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _MODE, _BLEND));
        break;
    case NVKMS_COMPOSITION_BLENDING_MODE_PREMULT_ALPHA:
        nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _K1, 255) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _CURSOR_COLOR_FACTOR_SELECT, _K1) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _VIEWPORT_COLOR_FACTOR_SELECT, _NEG_K1_TIMES_SRC) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _MODE, _BLEND));
        break;
    case NVKMS_COMPOSITION_BLENDING_MODE_NON_PREMULT_SURFACE_ALPHA:
        nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _K1,
                    pCursorCompParams->surfaceAlpha) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _CURSOR_COLOR_FACTOR_SELECT, _K1_TIMES_SRC) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _VIEWPORT_COLOR_FACTOR_SELECT, _NEG_K1_TIMES_SRC) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _MODE, _BLEND));
        break;
    case NVKMS_COMPOSITION_BLENDING_MODE_PREMULT_SURFACE_ALPHA:
        nvDmaSetEvoMethodData(pChannel,
            DRF_NUM(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _K1,
                    pCursorCompParams->surfaceAlpha) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _CURSOR_COLOR_FACTOR_SELECT, _K1) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION,
                    _VIEWPORT_COLOR_FACTOR_SELECT, _NEG_K1_TIMES_SRC) |
            DRF_DEF(C97D, _HEAD_SET_CONTROL_CURSOR_COMPOSITION, _MODE, _BLEND));
        break;
    default:
        nvEvoLogDevDebug(pDevEvo, EVO_LOG_ERROR,
            "%s: composition mode %d not supported for cursor",
            __func__, pCursorCompParams->blendingMode[1]);
        break;
    }
}

static void EvoSetDitherC9(NVDispEvoPtr pDispEvo, const int head,
                           const NvBool enabled, const NvU32 type,
                           const NvU32 algo,
                           NVEvoUpdateState *updateState)
{
    NVDevEvoPtr pDevEvo = pDispEvo->pDevEvo;
    NVEvoChannelPtr pChannel = pDevEvo->core;
    NvU32 ditherControl;

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    if (enabled) {
        ditherControl = DRF_DEF(C97D, _HEAD_SET_DITHER_CONTROL, _ENABLE, _ENABLE);

        switch (type) {
        case NV0073_CTRL_SPECIFIC_OR_DITHER_TYPE_6_BITS:
            ditherControl |=
                DRF_DEF(C97D, _HEAD_SET_DITHER_CONTROL, _BITS, _TO_6_BITS);
            break;
        case NV0073_CTRL_SPECIFIC_OR_DITHER_TYPE_8_BITS:
            ditherControl |=
                DRF_DEF(C97D, _HEAD_SET_DITHER_CONTROL, _BITS, _TO_8_BITS);
            break;
        /* XXXnvdisplay: Support DITHER_TO_{10,12}_BITS (see also bug 1729668). */
        default:
            nvAssert(!"Unknown ditherType");
            // Fall through
        case NV0073_CTRL_SPECIFIC_OR_DITHER_TYPE_OFF:
            ditherControl = NVC97D_HEAD_SET_DITHER_CONTROL_ENABLE_DISABLE;
            break;
        }

    } else {
        ditherControl = DRF_DEF(C97D, _HEAD_SET_DITHER_CONTROL, _ENABLE, _DISABLE);
    }

    switch (algo) {
    case NV0073_CTRL_SPECIFIC_OR_DITHER_ALGO_STATIC_ERR_ACC:
        ditherControl |=
            DRF_DEF(C97D, _HEAD_SET_DITHER_CONTROL, _MODE, _STATIC_ERR_ACC);
        break;
    case NV0073_CTRL_SPECIFIC_OR_DITHER_ALGO_DYNAMIC_2X2:
        ditherControl |=
            DRF_DEF(C97D, _HEAD_SET_DITHER_CONTROL, _MODE, _DYNAMIC_2X2);
        break;
    case NV0073_CTRL_SPECIFIC_OR_DITHER_ALGO_STATIC_2X2:
        ditherControl |=
            DRF_DEF(C97D, _HEAD_SET_DITHER_CONTROL, _MODE, _STATIC_2X2);
        break;
    case NV0073_CTRL_SPECIFIC_OR_DITHER_ALGO_TEMPORAL:
        ditherControl |=
            DRF_DEF(C97D, _HEAD_SET_DITHER_CONTROL, _MODE, _TEMPORAL);
        break;
    default:
        nvAssert(!"Unknown DitherAlgo");
        // Fall through
    case NV0073_CTRL_SPECIFIC_OR_DITHER_ALGO_UNKNOWN:
    case NV0073_CTRL_SPECIFIC_OR_DITHER_ALGO_DYNAMIC_ERR_ACC:
        ditherControl |=
            DRF_DEF(C97D, _HEAD_SET_DITHER_CONTROL, _MODE, _DYNAMIC_ERR_ACC);
        break;
    }

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DITHER_CONTROL(head), 1);
    nvDmaSetEvoMethodData(pChannel, ditherControl);
}

static void EvoSetDisplayRateC9(NVDispEvoPtr pDispEvo, const int head,
                                NvBool enable,
                                NVEvoUpdateState *updateState,
                                NvU32 timeoutMicroseconds)
{
    NVDevEvoPtr pDevEvo = pDispEvo->pDevEvo;
    NVEvoChannelPtr pChannel = pDevEvo->core;

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    if (enable) {
        timeoutMicroseconds =
            NV_MIN(timeoutMicroseconds,
                   DRF_MASK(NVC97D_HEAD_SET_DISPLAY_RATE_MIN_REFRESH_INTERVAL));

        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DISPLAY_RATE(head), 1);
        nvDmaSetEvoMethodData(pChannel,
            DRF_DEF(C97D, _HEAD_SET_DISPLAY_RATE, _RUN_MODE, _ONE_SHOT) |
            DRF_NUM(C97D, _HEAD_SET_DISPLAY_RATE, _MIN_REFRESH_INTERVAL,
                    timeoutMicroseconds) |
            (timeoutMicroseconds == 0 ?
                DRF_DEF(C97D, _HEAD_SET_DISPLAY_RATE, _MIN_REFRESH, _DISABLE) :
                DRF_DEF(C97D, _HEAD_SET_DISPLAY_RATE, _MIN_REFRESH, _ENABLE)));
    } else {
        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DISPLAY_RATE(head), 1);
        nvDmaSetEvoMethodData(pChannel,
            DRF_DEF(C97D, _HEAD_SET_DISPLAY_RATE, _RUN_MODE, _CONTINUOUS));
    }
}

static void EvoSetStallLockC9(NVDispEvoPtr pDispEvo, const int head,
                              NvBool enable, NVEvoUpdateState *updateState)
{
    NVDevEvoPtr pDevEvo = pDispEvo->pDevEvo;
    NVEvoChannelPtr pChannel = pDevEvo->core;
    NVEvoSubDevPtr pEvoSubDev = &pDevEvo->gpus[pDispEvo->displayOwner];
    NVEvoHeadControlPtr pHC = &pEvoSubDev->headControl[head];
    NvU32 data = 0x0;

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    /* CRASH_LOCK is not supported in NV5.0, only LINE_LOCK will be supported */
    nvAssert(!pHC->crashLockUnstallMode);
    data |= DRF_DEF(C97D, _HEAD_SET_STALL_LOCK, _UNSTALL_MODE, _LINE_LOCK);

    if (enable) {
        data |= DRF_DEF(C97D, _HEAD_SET_STALL_LOCK, _ENABLE, _TRUE) |
                DRF_DEF(C97D, _HEAD_SET_STALL_LOCK, _MODE, _ONE_SHOT);

        if (!pHC->useStallLockPin) {
            data |= DRF_DEF(C97D, _HEAD_SET_STALL_LOCK, _LOCK_PIN, _LOCK_PIN_NONE);
        } else  if (NV_EVO_LOCK_PIN_IS_INTERNAL(pHC->stallLockPin)) {
            NvU32 pin = pHC->stallLockPin - NV_EVO_LOCK_PIN_INTERNAL_0;
            data |= DRF_NUM(C97D, _HEAD_SET_STALL_LOCK, _LOCK_PIN,
                            NVC97D_HEAD_SET_STALL_LOCK_LOCK_PIN_INTERNAL_SCAN_LOCK(pin));
        } else {
            NvU32 pin = pHC->stallLockPin - NV_EVO_LOCK_PIN_0;
            data |= DRF_NUM(C97D, _HEAD_SET_STALL_LOCK, _LOCK_PIN,
                            NVC97D_HEAD_SET_STALL_LOCK_LOCK_PIN_LOCK_PIN(pin));
        }
    } else {
        data |= DRF_DEF(C97D, _HEAD_SET_STALL_LOCK, _ENABLE, _FALSE);
    }

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_STALL_LOCK(head), 1);
    nvDmaSetEvoMethodData(pChannel, data);
}

static void SetCrcSurfaceAddress(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 head)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    GetSurfaceAddress(pDevEvo, pSurfaceDesc, 0, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_SURFACE_ADDRESS_HI_CRC(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_HI_CRC, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_CRC,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_CRC,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_CRC,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_SURFACE_ADDRESS_LO_CRC(head), 1);
    nvDmaSetEvoMethodData(pChannel, value);
}

static void EvoStartHeadCRC32CaptureC9(NVDevEvoPtr pDevEvo,
                                       NVEvoDmaPtr pDma,
                                       NVConnectorEvoPtr pConnectorEvo,
                                       const enum nvKmsTimingsProtocol protocol,
                                       const NvU32 orIndex,
                                       NvU32 head,
                                       NvU32 sd,
                                       NVEvoUpdateState *updateState)
{
    const NvU32 winChannel = head << 1;
    NVEvoChannelPtr pChannel = pDevEvo->core;
    NvU32 orOutput = 0;

    /* These method should only apply to a single pDpy */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    /* The window channel should fit in
     * NVC97D_HEAD_SET_CRC_CONTROL_CONTROLLING_CHANNEL */
    nvAssert(winChannel < DRF_MASK(NVC97D_HEAD_SET_CRC_CONTROL_CONTROLLING_CHANNEL));

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    switch (pConnectorEvo->or.type) {
    case NV0073_CTRL_SPECIFIC_OR_TYPE_SOR:
        if (protocol == NVKMS_PROTOCOL_SOR_DP_A ||
            protocol == NVKMS_PROTOCOL_SOR_DP_B) {
            orOutput = NVC97D_HEAD_SET_CRC_CONTROL_PRIMARY_CRC_SF;
        } else {
            orOutput =
                NVC97D_HEAD_SET_CRC_CONTROL_PRIMARY_CRC_SOR(orIndex);
        }
        break;
    case NV0073_CTRL_SPECIFIC_OR_TYPE_PIOR:
	/* No PIOR support in C9 HAL. Fall through. */
    case NV0073_CTRL_SPECIFIC_OR_TYPE_DAC:
        /* No DAC support on nvdisplay. Fall through. */
    default:
        nvAssert(!"Invalid pConnectorEvo->or.type");
        break;
    }

    SetCrcSurfaceAddress(pDevEvo, pChannel, &pDma->surfaceDesc, head);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_CRC_CONTROL(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_CRC_CONTROL, _PRIMARY_CRC, orOutput) |
        DRF_DEF(C97D, _HEAD_SET_CRC_CONTROL, _SECONDARY_CRC, _NONE) |
        DRF_NUM(C97D, _HEAD_SET_CRC_CONTROL, _CONTROLLING_CHANNEL, winChannel) |
        DRF_DEF(C97D, _HEAD_SET_CRC_CONTROL, _EXPECT_BUFFER_COLLAPSE, _FALSE) |
        DRF_DEF(C97D, _HEAD_SET_CRC_CONTROL, _CRC_DURING_SNOOZE, _DISABLE));

    /* Reset the CRC notifier */
    nvEvoResetCRC32Notifier(pDma->subDeviceAddress[sd],
                            NVC37D_NOTIFIER_CRC_STATUS_0,
                            DRF_BASE(NVC37D_NOTIFIER_CRC_STATUS_0_DONE),
                            NVC37D_NOTIFIER_CRC_STATUS_0_DONE_FALSE);
}

static void EvoStopHeadCRC32CaptureC9(NVDevEvoPtr pDevEvo,
                                      NvU32 head,
                                      NVEvoUpdateState *updateState)
{
    NVEvoChannelPtr pChannel = pDevEvo->core;

    /* These method should only apply to a single pDpy */
    nvAssert(pDevEvo->subDevMaskStackDepth > 0);

    nvUpdateUpdateState(pDevEvo, updateState, pChannel);

    SetCrcSurfaceAddress(pDevEvo, pChannel, NULL, head);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_CRC_CONTROL(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_DEF(C97D, _HEAD_SET_CRC_CONTROL, _PRIMARY_CRC, _NONE) |
        DRF_DEF(C97D, _HEAD_SET_CRC_CONTROL, _SECONDARY_CRC, _NONE) |
        DRF_NUM(C97D, _HEAD_SET_CRC_CONTROL, _CONTROLLING_CHANNEL, 0) |
        DRF_DEF(C97D, _HEAD_SET_CRC_CONTROL, _EXPECT_BUFFER_COLLAPSE, _FALSE) |
        DRF_DEF(C97D, _HEAD_SET_CRC_CONTROL, _CRC_DURING_SNOOZE, _DISABLE));
}

/*
 * This method configures and programs the RG Core Semaphores. Default behavior
 * is to continuously trigger on the specified rasterline when enabled.
 */
static void
EvoConfigureVblankSyncObjectC9(const NVDevEvoPtr pDevEvo,
                               const NvU16 rasterLine,
                               const NvU32 head,
                               const NvU32 semaphoreIndex,
                               const NVSurfaceDescriptor *pSurfaceDesc,
                               NVEvoUpdateState* pUpdateState)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;
    NVEvoChannelPtr pChannel = pDevEvo->core;

    /*
     * Populate the NVEvoUpdateState for the caller. The Update State contains
     * a mask of which display channels need to be updated.
     */
    nvUpdateUpdateState(pDevEvo, pUpdateState, pChannel);

    GetSurfaceAddress(pDevEvo, pSurfaceDesc, 0, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel,
        NVC97D_HEAD_SET_SURFACE_ADDRESS_HI_RG_REL_SEMAPHORE(head, semaphoreIndex), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_HI_RG_REL_SEMAPHORE, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_RG_REL_SEMAPHORE,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_RG_REL_SEMAPHORE,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97D, _HEAD_SET_SURFACE_ADDRESS_LO_RG_REL_SEMAPHORE,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel,
            NVC97D_HEAD_SET_SURFACE_ADDRESS_LO_RG_REL_SEMAPHORE(head, semaphoreIndex), 1);
    nvDmaSetEvoMethodData(pChannel, value);

    if (!enable) {
        /* Disabling semaphore so no configuration necessary. */
        return;
    }

    /*
     * Configure the semaphore with the following:
     * Set PAYLOAD_SIZE to 32bits (default).
     * Set REL_MODE to WRITE (default).
     * Set RUN_MODE to CONTINUOUS.
     * Set RASTER_LINE to start of Vblank: Vsync + Vbp + Vactive.
     *
     * Note that all these options together fit in 32bits, and that all 32 bits
     * must be written each time any given option changes.
     *
     * The actual payload value doesn't currently matter since this RG
     * semaphore will be mapped to a syncpt for now. Each HW-issued payload
     * write is converted to a single syncpt increment irrespective of what the
     * actual semaphore payload value is.
     */
    nvDmaSetStartEvoMethod(pChannel,
                           NVC97D_HEAD_SET_RG_REL_SEMAPHORE_CONTROL(head, semaphoreIndex),
                           1);
    nvDmaSetEvoMethodData(pChannel,
            DRF_DEF(C97D, _HEAD_SET_RG_REL_SEMAPHORE_CONTROL, _PAYLOAD_SIZE,
                    _PAYLOAD_32BIT) |
            DRF_DEF(C97D, _HEAD_SET_RG_REL_SEMAPHORE_CONTROL, _REL_MODE,
                    _WRITE) |
            DRF_DEF(C97D, _HEAD_SET_RG_REL_SEMAPHORE_CONTROL, _RUN_MODE,
                    _CONTINUOUS) |
            DRF_NUM(C97D, _HEAD_SET_RG_REL_SEMAPHORE_CONTROL, _RASTER_LINE,
                    rasterLine));
}

static void EvoSetHdmiDscParamsC9(const NVDispEvoRec *pDispEvo,
                                   const NvU32 head,
                                   const NVDscInfoEvoRec *pDscInfo,
                                   const enum nvKmsPixelDepth pixelDepth)
{
    NVEvoChannelPtr pChannel = pDispEvo->pDevEvo->core;
    NvU32 bpc, flatnessDetThresh;
    NvU32 i;

    nvAssert(pDispEvo->pDevEvo->hal->caps.supportsHDMIFRL &&
             pDscInfo->type == NV_DSC_INFO_EVO_TYPE_HDMI);

    bpc = nvPixelDepthToBitsPerComponent(pixelDepth);
    if (bpc < 8) {
        nvAssert(bpc >= 8);
        bpc = 8;
    }
    flatnessDetThresh = (2 << (bpc - 8));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DSC_CONTROL(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_DEF(C97D, _HEAD_SET_DSC_CONTROL, _ENABLE, _TRUE) |
        DRF_NUM(C97D, _HEAD_SET_DSC_CONTROL, _FLATNESS_DET_THRESH, flatnessDetThresh) |
        DRF_DEF(C97D, _HEAD_SET_DSC_CONTROL, _FULL_ICH_ERR_PRECISION, _ENABLE) |
        DRF_DEF(C97D, _HEAD_SET_DSC_CONTROL, _AUTO_RESET, _ENABLE) |
        DRF_DEF(C97D, _HEAD_SET_DSC_CONTROL, _FORCE_ICH_RESET, _FALSE));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DSC_PPS_CONTROL(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_DEF(C97D, _HEAD_SET_DSC_PPS_CONTROL, _ENABLE, _TRUE) |
        DRF_DEF(C97D, _HEAD_SET_DSC_PPS_CONTROL, _LOCATION, _VBLANK) |
        DRF_DEF(C97D, _HEAD_SET_DSC_PPS_CONTROL, _FREQUENCY, _EVERY_FRAME) |
        /* MFS says "For FRL DSC CVTEM, it should be 0x21 (136bytes)." */
        DRF_NUM(C97D, _HEAD_SET_DSC_PPS_CONTROL, _SIZE, 0x21));

    /* The loop below assumes the methods are tightly packed. */
    ct_assert(ARRAY_LEN(pDscInfo->hdmi.pps) == 32);
    ct_assert((NVC97D_HEAD_SET_DSC_PPS_DATA1(0) - NVC97D_HEAD_SET_DSC_PPS_DATA0(0)) == 4);
    ct_assert((NVC97D_HEAD_SET_DSC_PPS_DATA31(0) - NVC97D_HEAD_SET_DSC_PPS_DATA0(0)) == (31 * 4));
    for (i = 0; i < ARRAY_LEN(pDscInfo->hdmi.pps); i++) {
        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DSC_PPS_DATA0(head) + (i * 4), 1);
        nvDmaSetEvoMethodData(pChannel, pDscInfo->hdmi.pps[i]);
    }

    /* Byte 0 must be 0x7f, the rest are don't care (will be filled in by HW) */
    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DSC_PPS_HEAD(head), 1);
    nvDmaSetEvoMethodData(pChannel,
                          DRF_NUM(C97D, _HEAD_SET_DSC_PPS_HEAD, _BYTE0, 0x7f));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_HDMI_DSC_HCACTIVE(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_HDMI_DSC_HCACTIVE, _BYTES, pDscInfo->hdmi.dscHActiveBytes) |
        DRF_NUM(C97D, _HEAD_SET_HDMI_DSC_HCACTIVE, _TRI_BYTES, pDscInfo->hdmi.dscHActiveTriBytes));
    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_HDMI_DSC_HCBLANK(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _HEAD_SET_HDMI_DSC_HCBLANK, _WIDTH, pDscInfo->hdmi.dscHBlankTriBytes));
}

static void EvoSetDpDscParamsC9(const NVDispEvoRec *pDispEvo,
                              const NvU32 head,
                              const NVDscInfoEvoRec *pDscInfo)
{
    NVEvoChannelPtr pChannel = pDispEvo->pDevEvo->core;
    NvU32 flatnessDetThresh;
    NvU32 i;

    nvAssert(pDscInfo->type == NV_DSC_INFO_EVO_TYPE_DP);

    // XXX: I'm pretty sure that this is wrong.
    // BitsPerPixelx16 is something like (24 * 16) = 384, and 2 << (384 - 8) is
    // an insanely large number.
    flatnessDetThresh = (2 << (pDscInfo->dp.bitsPerPixelX16 - 8)); /* ??? */

    nvAssert((pDscInfo->dp.dscMode == NV_DSC_EVO_MODE_DUAL) ||
                (pDscInfo->dp.dscMode == NV_DSC_EVO_MODE_SINGLE));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DSC_CONTROL(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_DEF(C97D, _HEAD_SET_DSC_CONTROL, _ENABLE, _TRUE) |
        DRF_NUM(C97D, _HEAD_SET_DSC_CONTROL, _FLATNESS_DET_THRESH, flatnessDetThresh) |
        DRF_DEF(C97D, _HEAD_SET_DSC_CONTROL, _FULL_ICH_ERR_PRECISION, _ENABLE) |
        DRF_DEF(C97D, _HEAD_SET_DSC_CONTROL, _AUTO_RESET, _DISABLE) |
        DRF_DEF(C97D, _HEAD_SET_DSC_CONTROL, _FORCE_ICH_RESET, _TRUE));

    nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DSC_PPS_CONTROL(head), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_DEF(C97D, _HEAD_SET_DSC_PPS_CONTROL, _ENABLE, _TRUE) |
        DRF_DEF(C97D, _HEAD_SET_DSC_PPS_CONTROL, _LOCATION, _VSYNC) |
        DRF_DEF(C97D, _HEAD_SET_DSC_PPS_CONTROL, _FREQUENCY, _EVERY_FRAME) |
        DRF_NUM(C97D, _HEAD_SET_DSC_PPS_CONTROL, _SIZE, 0x1F /* 32 PPS Dwords - 1 = 31 */));


#define NV_EVO5_NUM_HEAD_SET_DSC_PPS_DATA_DWORDS \
    (((NVC97D_HEAD_SET_DSC_PPS_DATA31(0) - NVC97D_HEAD_SET_DSC_PPS_DATA0(0)) / 4) + 1)

    ct_assert(NV_EVO5_NUM_HEAD_SET_DSC_PPS_DATA_DWORDS <= ARRAY_LEN(pDscInfo->dp.pps));

    for (i = 0; i < NV_EVO5_NUM_HEAD_SET_DSC_PPS_DATA_DWORDS; i++) {
        nvDmaSetStartEvoMethod(pChannel,(NVC97D_HEAD_SET_DSC_PPS_DATA0(head) + (i * 4)), 1);
        nvDmaSetEvoMethodData(pChannel, pDscInfo->dp.pps[i]);
    }

    /*
     * In case of DP, PPS is sent using the SDP over the Main-Link
     * during the vertical blanking interval. The PPS SDP header is defined
     * in DP 1.4 specification under section 2.2.5.9.1.
     */

    nvDmaSetStartEvoMethod(pChannel,
                           NVC97D_HEAD_SET_DSC_PPS_HEAD(head), 1);
    nvDmaSetEvoMethodData(pChannel,
                          DRF_NUM(C97D, _HEAD_SET_DSC_PPS_HEAD, _BYTE0, 0x00) | /* SDP ID = 0x0 */
                          DRF_NUM(C97D, _HEAD_SET_DSC_PPS_HEAD, _BYTE1, 0x10) | /* SDP Type = 0x10 */
                          DRF_NUM(C97D, _HEAD_SET_DSC_PPS_HEAD, _BYTE2, 0x7f) | /* Number of payload data bytes - 1 = 0x7F */
                          DRF_NUM(C97D, _HEAD_SET_DSC_PPS_HEAD, _BYTE3, 0x00)); /* Reserved */
}

static void EvoSetDscParamsC9(const NVDispEvoRec *pDispEvo,
                              const NvU32 head,
                              const NVDscInfoEvoRec *pDscInfo,
                              const enum nvKmsPixelDepth pixelDepth)
{
    if (pDscInfo->type == NV_DSC_INFO_EVO_TYPE_HDMI) {
        EvoSetHdmiDscParamsC9(pDispEvo, head, pDscInfo, pixelDepth);
    } else if (pDscInfo->type == NV_DSC_INFO_EVO_TYPE_DP) {
        EvoSetDpDscParamsC9(pDispEvo, head, pDscInfo);
    } else {
        NVEvoChannelPtr pChannel = pDispEvo->pDevEvo->core;

        nvAssert(pDscInfo->type == NV_DSC_INFO_EVO_TYPE_DISABLED);

        /* Disable DSC function */
        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DSC_CONTROL(head), 1);
        nvDmaSetEvoMethodData(pChannel,
            DRF_DEF(C97D, _HEAD_SET_DSC_CONTROL, _ENABLE, _FALSE));

        /* Disable PPS SDP (Secondary-Data Packet), DP won't send out PPS SDP */
        nvDmaSetStartEvoMethod(pChannel, NVC97D_HEAD_SET_DSC_PPS_CONTROL(head), 1);
        nvDmaSetEvoMethodData(pChannel,
            DRF_DEF(C97D, _HEAD_SET_DSC_PPS_CONTROL, _ENABLE, _FALSE));
    }
}

static NvU32 EvoAllocSurfaceDescriptorC9(
    NVDevEvoPtr pDevEvo, NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 memoryHandle, NvU32 localCtxDmaFlags,
    NvU64 limit,
    NvBool mapToDisplayRm)
{
    NV0041_CTRL_GET_SURFACE_PHYS_ATTR_PARAMS params = { };
    NvU32 ret;

   /*
    * NV0041_CTRL_CMD_GET_SURFACE_PHYS_ATTR is supposed to work
    * irrespective of whether the allocation is from sysmem or
    * vidmem and SMMU is enabled or bypassed.
    */
    ret = nvRmApiControl(nvEvoGlobal.clientHandle,
                         memoryHandle,
                         NV0041_CTRL_CMD_GET_SURFACE_PHYS_ATTR,
                         &params, sizeof(params));

    if (ret != NVOS_STATUS_SUCCESS) {
        return ret;
    }

    pSurfaceDesc->memAperture = params.memAperture;

#if defined(NV_EVO4_PB_ALLOC_WAR)
    ret = nvCtxDmaAlloc(pDevEvo, &pSurfaceDesc->ctxDmaHandle,
                        memoryHandle,
                        localCtxDmaFlags, limit);
#endif

    if (ret != NVOS_STATUS_SUCCESS) {
        nvEvoLog(EVO_LOG_ERROR, "nvCtxDmaAlloc failed\n");
        return ret;
    }

    if (mapToDisplayRm) {
        NV0041_CTRL_MAP_MEMORY_FOR_GPU_ACCESS_PARAMS mapParams = { };

        mapParams.hSubdevice = pDevEvo->pSubDevices[0]->handle;
        ret = nvRmApiControl(nvEvoGlobal.clientHandle,
                             memoryHandle,
                             NV0041_CTRL_CMD_MAP_MEMORY_FOR_GPU_ACCESS,
                             &mapParams, sizeof(mapParams));

        if (ret != NVOS_STATUS_SUCCESS) {
            nvEvoLog(EVO_LOG_ERROR, "NV0041_CTRL_CMD_MAP_MEMORY_FOR_GPU_ACCESS failed\n");
#if defined(NV_EVO4_PB_ALLOC_WAR)
            nvCtxDmaFree(pDevEvo, pDevEvo->deviceHandle, &pSurfaceDesc->ctxDmaHandle);
#endif
            return ret;
        }

        pSurfaceDesc->memOffset = mapParams.address;
        pSurfaceDesc->memoryHandle = memoryHandle;
        pSurfaceDesc->isMemoryMappedForDisplayAccess = TRUE;
    } else {
        pSurfaceDesc->memOffset = params.memOffset;
        pSurfaceDesc->isMemoryMappedForDisplayAccess = FALSE;
    }

    pSurfaceDesc->bValid = TRUE;

    return ret;
}

static void EvoFreeSurfaceDescriptorC9(
    NVDevEvoPtr pDevEvo,
    NvU32 deviceHandle,
    NVSurfaceDescriptor *pSurfaceDesc)
{
    NvU32 ret;

    if (!pSurfaceDesc->bValid) {
        return;
    }

    if (pSurfaceDesc->isMemoryMappedForDisplayAccess) {
        NV0041_CTRL_UNMAP_MEMORY_FOR_GPU_ACCESS_PARAMS params = { };

        params.hSubdevice = pDevEvo->pSubDevices[0]->handle;
        ret = nvRmApiControl(nvEvoGlobal.clientHandle,
                             pSurfaceDesc->memoryHandle,
                             NV0041_CTRL_CMD_UNMAP_MEMORY_FOR_GPU_ACCESS,
                             &params, sizeof(params));

        if (ret != NVOS_STATUS_SUCCESS) {
            nvEvoLog(EVO_LOG_ERROR, "NV0041_CTRL_CMD_UNMAP_MEMORY_FOR_GPU_ACCESS failed\n");
        }
        pSurfaceDesc->isMemoryMappedForDisplayAccess = FALSE;
    }

#if defined(NV_EVO4_PB_ALLOC_WAR)
   nvCtxDmaFree(pDevEvo, deviceHandle, &pSurfaceDesc->ctxDmaHandle);
#endif
    pSurfaceDesc->bValid = FALSE;
}

static NvU32 EvoBindSurfaceDescriptorC9(
    NVDevEvoPtr pDevEvo,
    NVEvoChannelPtr pChannel,
    NVSurfaceDescriptor *pSurfaceDesc)
{
    return NVOS_STATUS_SUCCESS;
}

static void EvoSetTmoLutSurfaceAddressC9(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 offset)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    GetSurfaceAddress(pDevEvo, pSurfaceDesc, offset, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_HI_TMO_LUT, 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97E, _SET_SURFACE_ADDRESS_HI_TMO_LUT, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_TMO_LUT,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_TMO_LUT,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_TMO_LUT,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_LO_TMO_LUT, 1);
    nvDmaSetEvoMethodData(pChannel, value);
}

static void EvoSetILUTSurfaceAddressC9(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 offset)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    GetSurfaceAddress(pDevEvo, pSurfaceDesc, offset, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_HI_ILUT, 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97E, _SET_SURFACE_ADDRESS_HI_ILUT, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_ILUT,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_ILUT,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_ILUT,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_LO_ILUT, 1);
    nvDmaSetEvoMethodData(pChannel, value);
}

static void EvoSetISOSurfaceAddressC9(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 offset,
    NvU32 idx,
    NvBool isBlocklinear)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    GetSurfaceAddress(pDevEvo, pSurfaceDesc, offset, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_HI_ISO(idx), 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97E, _SET_SURFACE_ADDRESS_HI_ISO, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_ISO,
                            _ADDRESS_LO, addrLo, value);
    if (isBlocklinear) {
        value = FLD_SET_DRF(C97E, _SET_SURFACE_ADDRESS_LO_ISO,
                            _KIND, _BLOCKLINEAR, value);
    }
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_ISO,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_ISO,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_LO_ISO(idx), 1);
    nvDmaSetEvoMethodData(pChannel, value);
}

static void EvoSetCoreNotifierSurfaceAddressAndControlC9(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 notifierOffset,
    NvU32 ctrlVal)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    // The unit of the notifierOffset is size of the notifier. Convert it to bytes.
    notifierOffset *=
        nvKmsSizeOfNotifier(NVKMS_NISO_FORMAT_FOUR_WORD_NVDISPLAY,  FALSE /* overlay */);
    GetSurfaceAddress(pDevEvo, pSurfaceDesc, notifierOffset, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_SET_SURFACE_ADDRESS_HI_NOTIFIER, 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97D, _SET_SURFACE_ADDRESS_HI_NOTIFIER, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97D, _SET_SURFACE_ADDRESS_LO_NOTIFIER,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97D, _SET_SURFACE_ADDRESS_LO_NOTIFIER,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97D, _SET_SURFACE_ADDRESS_LO_NOTIFIER,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel, NVC97D_SET_SURFACE_ADDRESS_LO_NOTIFIER, 1);
    nvDmaSetEvoMethodData(pChannel, value);

    nvDmaSetStartEvoMethod(pChannel, NVC97D_SET_NOTIFIER_CONTROL, 1);
    nvDmaSetEvoMethodData(pChannel, ctrlVal);
}

static void EvoSetWinNotifierSurfaceAddressAndControlC9(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 notifierOffset,
    NvU32 ctrlVal)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    notifierOffset *=
        nvKmsSizeOfNotifier(NVKMS_NISO_FORMAT_FOUR_WORD_NVDISPLAY,  FALSE /* overlay */);
    GetSurfaceAddress(pDevEvo, pSurfaceDesc, notifierOffset, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_HI_NOTIFIER, 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97E, _SET_SURFACE_ADDRESS_HI_NOTIFIER, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_NOTIFIER,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_NOTIFIER,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_NOTIFIER,
                        _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_LO_NOTIFIER, 1);
    nvDmaSetEvoMethodData(pChannel, value);

    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_NOTIFIER_CONTROL, 1);
    nvDmaSetEvoMethodData(pChannel, ctrlVal);
}

static void EvoSetSemaphoreSurfaceAddressAndControlC9(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 semaphoreOffset,
    NvU32 ctrlVal)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    // The unit of the semaphoreOffset is size of the semaphore. Convert it to bytes.
    semaphoreOffset *=
        nvKmsSizeOfSemaphore(NVKMS_NISO_FORMAT_FOUR_WORD_NVDISPLAY);
    GetSurfaceAddress(pDevEvo, pSurfaceDesc, semaphoreOffset, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_HI_SEMAPHORE, 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97E, _SET_SURFACE_ADDRESS_HI_SEMAPHORE, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_SEMAPHORE,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_SEMAPHORE,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_SEMAPHORE,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_LO_SEMAPHORE, 1);
    nvDmaSetEvoMethodData(pChannel, value);

    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SEMAPHORE_CONTROL, 1);
    nvDmaSetEvoMethodData(pChannel, ctrlVal);
}

static void EvoSetAcqSemaphoreSurfaceAddressAndControlC9(
    const NVDevEvoRec *pDevEvo,
    NVEvoChannelPtr pChannel,
    const NVSurfaceDescriptor *pSurfaceDesc,
    NvU32 semaphoreOffset,
    NvU32 ctrlVal)
{
    NvU32 addrHi, addrLo;
    NvBool enable;
    NvU8 target;
    NvU32 value = 0;

    // The unit of the semaphoreOffset is size of the semaphore. Convert it to bytes.
    semaphoreOffset *=
        nvKmsSizeOfSemaphore(NVKMS_NISO_FORMAT_FOUR_WORD_NVDISPLAY);
    GetSurfaceAddress(pDevEvo, pSurfaceDesc, semaphoreOffset, &addrHi, &addrLo,
                      &enable, &target);

    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_HI_ACQ_SEMAPHORE, 1);
    nvDmaSetEvoMethodData(pChannel,
        DRF_NUM(C97E, _SET_SURFACE_ADDRESS_HI_ACQ_SEMAPHORE, _ADDRESS_HI, addrHi));

    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_ACQ_SEMAPHORE,
                            _ADDRESS_LO, addrLo, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_ACQ_SEMAPHORE,
                            _TARGET, target, value);
    value = FLD_SET_DRF_NUM(C97E, _SET_SURFACE_ADDRESS_LO_ACQ_SEMAPHORE,
                            _ENABLE, enable, value);
    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_SURFACE_ADDRESS_LO_ACQ_SEMAPHORE, 1);
    nvDmaSetEvoMethodData(pChannel, value);

    nvDmaSetStartEvoMethod(pChannel, NVC97E_SET_ACQ_SEMAPHORE_CONTROL, 1);
    nvDmaSetEvoMethodData(pChannel, ctrlVal);
}

NVEvoHAL nvEvoC9 = {
    EvoSetRasterParamsC9,                         /* SetRasterParams */
    EvoSetProcAmpC9,                              /* SetProcAmp */
    EvoSetHeadControlC9,                          /* SetHeadControl */
    NULL,                                         /* SetHeadRefClk */
    EvoHeadSetControlORC9,                        /* HeadSetControlOR */
    nvEvoORSetControlC3,                          /* ORSetControl */
    EvoHeadSetDisplayIdC9,                        /* HeadSetDisplayId */
    nvEvoSetUsageBoundsC5,                        /* SetUsageBounds */
    nvEvoUpdateC3,                                /* Update */
    nvEvoIsModePossibleC3,                        /* IsModePossible */
    nvEvoPrePostIMPC3,                            /* PrePostIMP */
    nvEvoSetNotifierC3,                           /* SetNotifier */
    nvEvoGetCapabilitiesC6,                       /* GetCapabilities */
    nvEvoFlipC6,                                  /* Flip */
    nvEvoFlipTransitionWARC6,                     /* FlipTransitionWAR */
    nvEvoFillLUTSurfaceC5,                        /* FillLUTSurface */
    EvoSetOutputLutC9,                            /* SetOutputLut */
    EvoSetOutputScalerC9,                         /* SetOutputScaler */
    EvoSetViewportPointInC9,                      /* SetViewportPointIn */
    EvoSetViewportInOutC9,                        /* SetViewportInOut */
    EvoSetCursorImageC9,                          /* SetCursorImage */
    nvEvoValidateCursorSurfaceC3,                 /* ValidateCursorSurface */
    nvEvoValidateWindowFormatC6,                  /* ValidateWindowFormat */
    nvEvoInitCompNotifierC3,                      /* InitCompNotifier */
    nvEvoIsCompNotifierCompleteC3,                /* IsCompNotifierComplete */
    nvEvoWaitForCompNotifierC3,                   /* WaitForCompNotifier */
    EvoSetDitherC9,                               /* SetDither */
    EvoSetStallLockC9,                            /* SetStallLock */
    EvoSetDisplayRateC9,                          /* SetDisplayRate */
    EvoInitChannelC9,                             /* InitChannel */
    nvEvoInitDefaultLutC5,                        /* InitDefaultLut */
    nvEvoInitWindowMappingC5,                     /* InitWindowMapping */
    nvEvoIsChannelIdleC3,                         /* IsChannelIdle */
    nvEvoIsChannelMethodPendingC3,                /* IsChannelMethodPending */
    nvEvoForceIdleSatelliteChannelC3,             /* ForceIdleSatelliteChannel */
    nvEvoForceIdleSatelliteChannelIgnoreLockC3,   /* ForceIdleSatelliteChannelIgnoreLock */
    nvEvoAccelerateChannelC3,                     /* AccelerateChannel */
    nvEvoResetChannelAcceleratorsC3,              /* ResetChannelAccelerators */
    nvEvoAllocRmCtrlObjectC3,                     /* AllocRmCtrlObject */
    nvEvoFreeRmCtrlObjectC3,                      /* FreeRmCtrlObject */
    nvEvoSetImmPointOutC3,                        /* SetImmPointOut */
    EvoStartHeadCRC32CaptureC9,                   /* StartCRC32Capture */
    EvoStopHeadCRC32CaptureC9,                    /* StopCRC32Capture */
    nvEvoQueryHeadCRC32_C3,                       /* QueryCRC32 */
    nvEvoGetScanLineC3,                           /* GetScanLine */
    EvoConfigureVblankSyncObjectC9,               /* ConfigureVblankSyncObject */
    EvoSetDscParamsC9,                            /* SetDscParams */
    NULL,                                         /* EnableMidFrameAndDWCFWatermark */
    nvEvoGetActiveViewportOffsetC3,               /* GetActiveViewportOffset */
    NULL,                                         /* ClearSurfaceUsage */
    nvEvoComputeWindowScalingTapsC5,              /* ComputeWindowScalingTaps */
    nvEvoGetWindowScalingCapsC3,                  /* GetWindowScalingCaps */
    NULL,                                         /* SetMergeMode */
    nvEvoSendHdmiInfoFrameC8,                     /* SendHdmiInfoFrame */
    nvEvoDisableHdmiInfoFrameC8,                  /* DisableHdmiInfoFrame */
    nvEvoSendDpInfoFrameSdpC8,                    /* SendDpInfoFrameSdp */
    EvoAllocSurfaceDescriptorC9,                  /* AllocSurfaceDescriptor */
    EvoFreeSurfaceDescriptorC9,                   /* FreeSurfaceDescriptor */
    EvoBindSurfaceDescriptorC9,                   /* BindSurfaceDescriptor */
    EvoSetTmoLutSurfaceAddressC9,                 /* SetTmoLutSurfaceAddress */
    EvoSetILUTSurfaceAddressC9,                   /* SetILUTSurfaceAddress */
    EvoSetISOSurfaceAddressC9,                    /* SetISOSurfaceAddress */
    EvoSetCoreNotifierSurfaceAddressAndControlC9, /* SetCoreNotifierSurfaceAddressAndControl */
    EvoSetWinNotifierSurfaceAddressAndControlC9,  /* SetWinNotifierSurfaceAddressAndControl */
    EvoSetSemaphoreSurfaceAddressAndControlC9,    /* SetSemaphoreSurfaceAddressAndControl */
    EvoSetAcqSemaphoreSurfaceAddressAndControlC9, /* SetAcqSemaphoreSurfaceAddressAndControl */
    {                                             /* caps */
        TRUE,                                     /* supportsNonInterlockedUsageBoundsUpdate */
        TRUE,                                     /* supportsDisplayRate */
        FALSE,                                    /* supportsFlipLockRGStatus */
        TRUE,                                     /* needDefaultLutSurface */
        TRUE,                                     /* hasUnorm10OLUT */
        FALSE,                                    /* supportsImageSharpening */
        TRUE,                                     /* supportsHDMIVRR */
        FALSE,                                    /* supportsCoreChannelSurface */
        FALSE,                                     /* supportsHDMIFRL */
        FALSE,                                    /* supportsSetStorageMemoryLayout */
        TRUE,                                     /* supportsIndependentAcqRelSemaphore */
        FALSE,                                    /* supportsCoreLut */
        TRUE,                                     /* supportsSynchronizedOverlayPositionUpdate */
        TRUE,                                     /* supportsVblankSyncObjects */
        FALSE,                                    /* requiresScalingTapsInBothDimensions */
        FALSE,                                    /* supportsMergeMode */
        FALSE,                                     /* supportsHDMI10BPC */
        TRUE,                                     /* supportsDPAudio192KHz */
        TRUE,                                     /* supportsInputColorSpace */
        TRUE,                                     /* supportsInputColorRange */
        TRUE,                                     /* supportsYCbCr422OverHDMIFRL */
        NV_EVO3_SUPPORTED_DITHERING_MODES,        /* supportedDitheringModes */
        sizeof(NVC372_CTRL_IS_MODE_POSSIBLE_PARAMS), /* impStructSize */
        NV_EVO_SCALER_2TAPS,                      /* minScalerTaps */
        NV_EVO3_X_EMULATED_SURFACE_MEMORY_FORMATS_C6, /* xEmulatedSurfaceMemoryFormats */
    },
};

