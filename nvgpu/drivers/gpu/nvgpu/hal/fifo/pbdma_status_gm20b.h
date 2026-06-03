/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_STATUS_GM20B
#define NVGPU_PBDMA_STATUS_GM20B

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_pbdma_status_info;

void gm20b_read_pbdma_status_info(struct gk20a *g, u32 pbdma_id,
		struct nvgpu_pbdma_status_info *status);

#endif /* NVGPU_PBDMA_STATUS_GM20B */
