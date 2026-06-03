/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_STATUS_GB20C_H
#define NVGPU_PBDMA_STATUS_GB20C_H

#include <nvgpu/types.h>

struct gk20a;

u32 gb20c_pbdma_status_sched_reg_addr(struct gk20a *g, u32 pbdma_id);
u32 gb20c_pbdma_channel_reg_addr(struct gk20a *g, u32 pbdma_id);
u32 gb20c_pbdma_next_channel_reg_addr(struct gk20a *g, u32 pbdma_id);

#endif /* NVGPU_PBDMA_STATUS_GB20C_H */
