/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_STATUS_GA10B_H
#define NVGPU_PBDMA_STATUS_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_pbdma_status_info;

u32 ga10b_pbdma_status_sched_reg_addr(struct gk20a *g, u32 pbdma_id);
u32 ga10b_pbdma_channel_reg_addr(struct gk20a *g, u32 pbdma_id);
u32 ga10b_pbdma_next_channel_reg_addr(struct gk20a *g, u32 pbdma_id);
void ga10b_read_pbdma_status_info(struct gk20a *g, u32 pbdma_id,
		struct nvgpu_pbdma_status_info *status);

#endif /* NVGPU_PBDMA_STATUS_GA10B_H */
