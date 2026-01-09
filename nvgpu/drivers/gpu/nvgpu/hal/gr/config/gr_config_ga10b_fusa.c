// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/config.h>

#include "gr_config_ga10b.h"

#include <nvgpu/hw/ga10b/hw_gr_ga10b.h>

u32 ga10b_gr_config_get_gpc_rop_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index)
{
	u32 val;
	u32 rop_cnt = nvgpu_gr_config_get_max_rop_per_gpc_count(config);

	/*
	 * Toggle the bits of NV_FUSE_STATUS_OPT_ROP_GPC to get the non-FSed
	 * ROP mask.
	 */
	val = g->ops.fuse.fuse_status_opt_rop_gpc(g, gpc_index);

	return (~val) & nvgpu_safe_sub_u32(BIT32(rop_cnt), 1U);
}
