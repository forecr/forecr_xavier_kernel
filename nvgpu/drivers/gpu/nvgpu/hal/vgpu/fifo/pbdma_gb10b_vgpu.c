// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/vgpu.h>

#include "pbdma_gb10b_vgpu.h"

u32 vgpu_gb10b_pbdma_get_mmu_fault_id(struct gk20a *g, u32 pbdma_id)
{
	(void)g;
	(void)pbdma_id;

	/*
	 * For now, the VF does not need the PBDMA fault ID for anything besides
	 * debug logs. If needed, we could obtain this information from the server
	 * in the future.
	 */
	return INVAL_ID;
}
