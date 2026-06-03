// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/config.h>

#include "gr_config_gm20b.h"

#include <nvgpu/hw/gm20b/hw_gr_gm20b.h>

u32 gm20b_gr_config_get_gpc_tpc_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index)
{
	u32 val;
	u32 tpc_cnt = nvgpu_gr_config_get_max_tpc_per_gpc_count(config);

	/* Toggle the bits of NV_FUSE_STATUS_OPT_TPC_GPC */
	val = g->ops.fuse.fuse_status_opt_tpc_gpc(g, gpc_index);

	return (~val) & nvgpu_safe_sub_u32(BIT32(tpc_cnt), 1U);
}

u32 gm20b_gr_config_get_tpc_count_in_gpc(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tmp, tmp1, tmp2;

	(void)config;

	tmp1 = nvgpu_safe_mult_u32(gpc_stride, gpc_index);
	tmp2 = nvgpu_safe_add_u32(gr_gpc0_fs_gpc_r(), tmp1);
	tmp = nvgpu_readl(g, tmp2);

	return gr_gpc0_fs_gpc_num_available_tpcs_v(tmp);
}

u32 gm20b_gr_config_get_pes_tpc_mask(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index, u32 pes_index)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tmp, tmp1, tmp2;

	(void)config;

	tmp1 = nvgpu_safe_mult_u32(gpc_index, gpc_stride);
	tmp2 = nvgpu_safe_add_u32(gr_gpc0_gpm_pd_pes_tpc_id_mask_r(pes_index),
					tmp1);
	tmp = nvgpu_readl(g, tmp2);

	return gr_gpc0_gpm_pd_pes_tpc_id_mask_mask_v(tmp);
}

u32 gm20b_gr_config_get_pd_dist_skip_table_size(void)
{
	return gr_pd_dist_skip_table__size_1_v();
}

u32 gm20b_gr_config_get_gpc_mask(struct gk20a *g)
{
	u32 val;
	u32 tpc_cnt = g->ops.top.get_max_gpc_count(g);

	/*
	 * For register NV_FUSE_STATUS_OPT_GPC a set bit with index i indicates
	 * corresponding GPC is floorswept
	 * But for s/w mask a set bit means GPC is enabled and it is disabled
	 * otherwise
	 * Hence toggle the bits of register value to get s/w mask
	 */
	val = g->ops.fuse.fuse_status_opt_gpc(g);

	return (~val) & nvgpu_safe_sub_u32(BIT32(tpc_cnt), 1U);
}

#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
u32 gm20b_gr_config_get_zcull_count_in_gpc(struct gk20a *g,
	struct nvgpu_gr_config *config, u32 gpc_index)
{
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tmp, tmp1, tmp2;

	(void)config;

	tmp1 = nvgpu_safe_mult_u32(gpc_stride, gpc_index);
	tmp2 = nvgpu_safe_add_u32(gr_gpc0_fs_gpc_r(), tmp1);
	tmp = nvgpu_readl(g, tmp2);

	return gr_gpc0_fs_gpc_num_available_zculls_v(tmp);
}
#endif
