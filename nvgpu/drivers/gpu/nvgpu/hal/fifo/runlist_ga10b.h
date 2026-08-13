/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_GA10B_H
#define NVGPU_RUNLIST_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_pbdma_info;

u32 ga10b_runlist_get_runlist_id(struct gk20a *g, u32 runlist_pri_base);
u32 ga10b_runlist_get_engine_id_from_rleng_id(struct gk20a *g,
			u32 rleng_id, u32 runlist_pri_base);
u32 ga10b_runlist_get_chram_bar0_offset(struct gk20a *g, u32 runlist_pri_base);
void ga10b_runlist_get_pbdma_info(struct gk20a *g, u32 runlist_pri_base,
		struct nvgpu_pbdma_info *pbdma_info);
u32 ga10b_runlist_get_engine_intr_id(struct gk20a *g, u32 runlist_pri_base,
		u32 rleng_id);
u32 ga10b_runlist_get_esched_fb_thread_id(struct gk20a *g, u32 runlist_pri_base);

#endif /* NVGPU_RUNLIST_GA10B_H */
