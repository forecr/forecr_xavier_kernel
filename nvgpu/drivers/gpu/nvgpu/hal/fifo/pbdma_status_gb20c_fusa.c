// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include "pbdma_status_gb20c.h"
#include <nvgpu/hw/gb20c/hw_pbdma_gb20c.h>

u32 gb20c_pbdma_status_sched_reg_addr(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base;

	pbdma_base = g->pbdma_id_to_base[pbdma_id];
	return nvgpu_safe_add_u32(pbdma_base, pbdma_status_sched_r());
}

u32 gb20c_pbdma_channel_reg_addr(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base;

	pbdma_base = g->pbdma_id_to_base[pbdma_id];
	return nvgpu_safe_add_u32(pbdma_base, pbdma_channel_r());
}

u32 gb20c_pbdma_next_channel_reg_addr(struct gk20a *g, u32 pbdma_id)
{
	u32 pbdma_base;

	pbdma_base = g->pbdma_id_to_base[pbdma_id];
	return nvgpu_safe_add_u32(pbdma_base, pbdma_next_channel_r());
}
