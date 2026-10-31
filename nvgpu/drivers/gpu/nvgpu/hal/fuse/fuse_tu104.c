// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "fuse_tu104.h"

#include <nvgpu/hw/tu104/hw_fuse_tu104.h>

int tu104_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi)
{
	u32 lo = nvgpu_readl(g, fuse_opt_pdi_0_r());
	u32 hi = nvgpu_readl(g, fuse_opt_pdi_1_r());

	*pdi = ((u64)lo) | (((u64)hi) << 32);

	return 0;
}
