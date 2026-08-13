/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_TU104_H
#define NVGPU_PBDMA_TU104_H

#include <nvgpu/types.h>

struct gk20a;

void tu104_pbdma_reset_header(struct gk20a *g, u32 pbdma_id);
u32 tu104_pbdma_read_data(struct gk20a *g, u32 pbdma_id);

#endif /* NVGPU_PBDMA_TU104_H */

