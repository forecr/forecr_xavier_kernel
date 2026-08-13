/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_VM_SCHEDULER
#define NVGPU_GSP_VM_SCHEDULER

struct nv_flcn_msg_gsp;
struct nv_flcn_cmd_gsp;
struct gk20a;

int nvgpu_gsp_vm_sched_runlist_update(struct gk20a *g,
		u32 runlistId,
		u32 runlistGfid,
		u32 runlistIovaLo,
		u32 runlistIovaHi,
		u32 runlistEntries,
		u32 runlistAperture);
int nvgpu_gsp_vm_sched_runlist_resubmit(struct gk20a *g, u32 runlistId, u32 runlistGfid);
int nvgpu_gsp_vm_sched_tsg_preempt(struct gk20a *g,
		u32 runlistId,
		u32 runlistGfid,
		u32 tsgId,
		u32 rlPbdmaBitMask,
		u32 rlEngBitMask);
int nvgpu_gsp_vm_sched_sw_init(struct gk20a *g);
void nvgpu_gsp_vm_sched_sw_deinit(struct gk20a *g);
int gsp_vm_sched_event_handler(struct gk20a *g, struct nv_flcn_msg_gsp *msg);

#endif /* NVGPU_GSP_VM_SCHEDULER */
