// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/fuse.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "fuse_gb20c.h"
#include <nvgpu/hw/gb20c/hw_fuse_gb20c.h>

u32 gb20c_fuse_status_opt_cpc_gpc(struct gk20a *g, u32 gpc)
{
	return nvgpu_readl(g, fuse_status_opt_cpc_gpc_r(gpc));
}
