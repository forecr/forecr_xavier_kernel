/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_VF_H
#define NVGPU_RUNLIST_VF_H

void vgpu_vf_runlist_hw_submit(struct gk20a *g, u32 runlist_id,
		u64 runlist_iova, enum nvgpu_aperture aperture, u32 count, u32 gfid);
int vgpu_vf_runlist_check_pending(struct gk20a *g,
		struct nvgpu_runlist *runlist);

#endif
