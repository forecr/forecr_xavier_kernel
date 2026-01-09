// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/gk20a.h>

#include "fuse_gv11b.h"

#include <nvgpu/hw/gv11b/hw_fuse_gv11b.h>

u32 gv11b_fuse_status_opt_pes_gpc(struct gk20a *g, u32 gpc)
{
	return nvgpu_readl(g, fuse_status_opt_pes_gpc_r(gpc));
}
