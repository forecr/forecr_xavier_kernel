/*
 * Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NVGPU_GIN_H
#define NVGPU_GIN_H

/**
 * Value to be passed to gin.get_intr_ctrl_msg to enable routing the interrupt
 * to the CPU.
 */
#define NVGPU_GIN_CPU_ENABLE		true

/**
 * Value to be passed to gin.get_intr_ctrl_msg to disable routing the interrupt
 * to the CPU.
 */
#define NVGPU_GIN_CPU_DISABLE		false

/**
 * Value to be passed to gin.get_intr_ctrl_msg to enable routing the interrupt
 * to the GSP.
 */
#define NVGPU_GIN_GSP_ENABLE		true

/**
 * Value to be passed to gin.get_intr_ctrl_msg to disable routing the interrupt
 * to the GSP.
 */
#define NVGPU_GIN_GSP_DISABLE		false

#endif /* NVGPU_GIN_H */
