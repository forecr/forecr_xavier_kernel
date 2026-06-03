// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/fuse.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "fuse_gm20b.h"

#include <nvgpu/hw/gm20b/hw_fuse_gm20b.h>

u32 gm20b_fuse_status_opt_fbio(struct gk20a *g)
{
	return nvgpu_readl(g, fuse_status_opt_fbio_r());
}

u32 gm20b_fuse_status_opt_fbp(struct gk20a *g)
{
	return nvgpu_readl(g, fuse_status_opt_fbp_r());
}

u32 gm20b_fuse_status_opt_l2_fbp(struct gk20a *g, u32 fbp)
{
	return nvgpu_readl(g, fuse_status_opt_rop_l2_fbp_r(fbp));
}

u32 gm20b_fuse_status_opt_tpc_gpc(struct gk20a *g, u32 gpc)
{
	u32 max_gpc_count = g->ops.top.get_max_gpc_count(g);
	if (gpc >= max_gpc_count) {
		BUG();
	}

	return nvgpu_readl(g, fuse_status_opt_tpc_gpc_r(gpc));
}

void gm20b_fuse_ctrl_opt_tpc_gpc(struct gk20a *g, u32 gpc, u32 val)
{
	nvgpu_writel(g, fuse_ctrl_opt_tpc_gpc_r(gpc), val);
}

u32 gm20b_fuse_opt_sec_debug_en(struct gk20a *g)
{
	return nvgpu_readl(g, fuse_opt_sec_debug_en_r());
}

u32 gm20b_fuse_opt_priv_sec_en(struct gk20a *g)
{
	return nvgpu_readl(g, fuse_opt_priv_sec_en_r());
}
