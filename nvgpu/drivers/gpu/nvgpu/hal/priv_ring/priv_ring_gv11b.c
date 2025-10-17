// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "priv_ring_gv11b.h"

#include <nvgpu/hw/gv11b/hw_pri_ringstation_sys_gv11b.h>
#include <nvgpu/hw/gv11b/hw_pri_ringstation_gpc_gv11b.h>
#include <nvgpu/hw/gv11b/hw_pri_ringstation_fbp_gv11b.h>

void gv11b_priv_ring_read_pri_fence(struct gk20a *g)
{
	/* Read back to ensure all writes to all chiplets are complete. */
	nvgpu_readl(g, pri_ringstation_sys_pri_fence_r());
	nvgpu_readl(g, pri_ringstation_gpc_pri_fence_r());
	nvgpu_readl(g, pri_ringstation_fbp_pri_fence_r());
}

