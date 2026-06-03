/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_GB10B_VGPU_H
#define NVGPU_PBDMA_GB10B_VGPU_H

struct gk20a;

u32 vgpu_gb10b_pbdma_get_mmu_fault_id(struct gk20a *g, u32 pbdma_id);

#endif
