/*
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NVGPU_GSP_VM_SCHEDULER
#define NVGPU_GSP_VM_SCHEDULER

struct nv_flcn_cmd_gsp;
struct gk20a;

int nvgpu_gsp_vm_sched_sw_init(struct gk20a *g);
int nvgpu_gsp_vm_sched_runlist_update(struct gk20a *g,
		u32 runlistId,
		u32 runlistGfid,
		u32 runlistIovaLo,
		u32 runlistIovaHi,
		u32 runlistEntries,
		u32 runlistAperture);
int nvgpu_gsp_vm_sched_tsg_preempt(struct gk20a *g,
		u32 runlistId,
		u32 runlistGfid,
		u32 tsgId);

#endif /* NVGPU_GSP_VM_SCHEDULER */
