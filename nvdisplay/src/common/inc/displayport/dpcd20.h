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
#ifndef _DISPLAYPORT20_H_
#define _DISPLAYPORT20_H_
#include "nvcfg_sdk.h"

// DSC Pass Through related DPCD. New bits in DPCD 0x0060h defined in DPCD2.0.
#define NV_DPCD20_DSC_SUPPORT_PASS_THROUGH_SUPPORT                                       1:1 /* R-XUF */
#define NV_DPCD20_DSC_SUPPORT_PASS_THROUGH_SUPPORT_NO                           (0x00000000) /* R-XUV */
#define NV_DPCD20_DSC_SUPPORT_PASS_THROUGH_SUPPORT_YES                          (0x00000001) /* R-XUV */
// DSC Pass Through related DPCD. New bits in DPCD 0x0160h defined in DPCD2.0.
#define NV_DPCD20_DSC_ENABLE_PASS_THROUGH                                                1:1 /* R-XUF */
#define NV_DPCD20_DSC_ENABLE_PASS_THROUGH_NO                                    (0x00000000) /* R-XUV */
#define NV_DPCD20_DSC_ENABLE_PASS_THROUGH_YES                                   (0x00000001) /* R-XUV */

// PANEL REPLAY RELATED DPCD
#define NV_DPCD20_PANEL_REPLAY_CAPABILITY                                         (0x000000B0)
#define NV_DPCD20_PANEL_REPLAY_CAPABILITY_SUPPORTED                                       0:0
#define NV_DPCD20_PANEL_REPLAY_CAPABILITY_SUPPORTED_NO                            (0x00000000)
#define NV_DPCD20_PANEL_REPLAY_CAPABILITY_SUPPORTED_YES                           (0x00000001)
#define NV_DPCD20_PANEL_REPLAY_CAPABILITY_SEL_UPDATE                                      1:1
#define NV_DPCD20_PANEL_REPLAY_CAPABILITY_SEL_UPDATE_NO                           (0x00000000)
#define NV_DPCD20_PANEL_REPLAY_CAPABILITY_SEL_UPDATE_YES                          (0x00000001)

#define NV_DPCD20_PANEL_REPLAY_CONFIGURATION                                      (0x000001B0)
#define NV_DPCD20_PANEL_REPLAY_CONFIGURATION_ENABLE_PR_MODE                               0:0
#define NV_DPCD20_PANEL_REPLAY_CONFIGURATION_ENABLE_PR_MODE_NO                    (0x00000000)
#define NV_DPCD20_PANEL_REPLAY_CONFIGURATION_ENABLE_PR_MODE_YES                   (0x00000001)

#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS                              (0x00002022)
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_PR_STATUS                            2:0
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_PR_STATUS_STATE_0            (0x00000000)
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_PR_STATUS_STATE_1            (0x00000001)
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_PR_STATUS_STATE_2            (0x00000002)
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_PR_STATUS_STATE_ERROR        (0x00000007)
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_SINK_FRAME_LOCKED                    4:3
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_SINK_FRAME_LOCKED_LOCKED     (0x00000000)
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_SINK_FRAME_LOCKED_COASTING   (0x00000001)
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_SINK_FRAME_LOCKED_GOVERNING  (0x00000002)
#define NV_DPCD20_PANEL_REPLAY_AND_FRAME_LOCK_STATUS_SINK_FRAME_LOCKED_RELOCKING  (0x00000003)

#endif // #ifndef _DISPLAYPORT20_H_
